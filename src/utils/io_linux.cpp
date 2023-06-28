#include "env.hpp"
#include "error.hpp"
#include <atomic>
#include <bits/types/struct_iovec.h>
#include <cassert>
#include <chrono>
#include <coroutine>
#include <cstddef>
#include <cstdio>
#include <fcntl.h>
#include <functional>
#include <liburing.h>
#include <queue>
#include <stdexcept>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <unistd.h>

namespace CodeBase {
struct SchedulerTask {
public:
    SchedulerTask() = delete;

    void await_resume() const noexcept {}
    bool await_ready() const noexcept {
        return false;
    }
    void await_suspend(std::coroutine_handle<> awaiter) noexcept {
        this->awaiter = awaiter;
        this->context->Submit(this);
    }

    void resume() const {
        if (iscoroutine) {
            awaiter.resume();
        } else {
            func();
        }
    }

    enum class SchedulerTaskType { Undefined, IO, Timeout, Yield };
    SchedulerTask(Context *context, SchedulerTaskType taskType) : context(context), TaskType(taskType) {}

    SchedulerTaskType GetTaskType() const noexcept {
        return TaskType;
    }

protected:
    ~SchedulerTask() {}
    void clear() {
        if (awaiter != nullptr) {
            assert(awaiter.done());
            awaiter.destroy();
        }
    }

    bool              iscoroutine = true;
    SchedulerTaskType TaskType = SchedulerTaskType::Undefined;
    union {
        std::coroutine_handle<> awaiter = nullptr;
        std::function<void()>   func;
    };
    Context *context = nullptr;
};

enum class IOOpcode {
    None = 0,
    Read,
    Write,
    Connect,
    Accept,
    Readv,
    Writev,
    Fsync,
    Fallocate,
    Openat,
    Close,
    Regiserbuffer,
    Unlinkat,
    Mkdirat,
};

class IOTask : public SchedulerTask {
public:
    friend class IOUringIOScheduler;
    friend class TaskScheduler;

    //openfile unlinkfile mkdiratfile
    IOTask(Context *context, IOOpcode op, int fd, const char *path, int flags, mode_t mode) : IOTask(context, op) {
        this->path = path;
        this->flags = flags;
        this->open_mode = mode;
    }

    //closefile
    IOTask(Context *context, int fd) : IOTask(context, IOOpcode::Close) {
        this->fd = fd;
    }

    //rwfile
    IOTask(Context *context, IOOpcode op, int fd, char *data, size_t size, off64_t offset) : IOTask(context, op) {
        assert(op == IOOpcode::Read || op == IOOpcode::Write);
        this->buf = data;
        this->len = size;
        this->offset = offset;
        this->fd = fd;
    }

    //rwvfile
    IOTask(Context *context, IOOpcode op, int fd, iovec *vec, size_t len, off64_t offset) : IOTask(context, op) {
        assert(op == IOOpcode::Read || op == IOOpcode::Write);
        this->vec = vec;
        this->len = len;
        this->offset = offset;
        this->fd = fd;
    }


    IOTask(Context *context, IOOpcode opcode) : SchedulerTask(context, SchedulerTaskType::IO), opcode(opcode) {}

    ~IOTask() {
        clear();
    }

    ErrorCode GetResult() const noexcept {
        return error;
    }

    void resume() {
        awaiter.resume();
    }

    void SetResult(ErrorCode error) {
        this->error = error;
    }

    IOTask *getNext() const noexcept {
        return IOCompleteTask;
    }

    int getFd() const noexcept {
        return fd;
    }

private:
    IOTask   *IOCompleteTask = nullptr;
    ErrorCode error = Error::Success;
    IOOpcode  opcode = IOOpcode::None;
    int       fd = 0;
    union {
        // for rw IO
        struct {
            size_t len;
            union {
                size_t offset = 0;
                int    count;
            };

            int bgid;
            union {
                char  *buf;
                iovec *vec;
                int    mode;//fallocate
                int    bid;
            };
        };
        // for connect and accept
        struct {
            sockaddr  addr;
            socklen_t addrlen;
        };

        struct {
            int         flags;
            const char *path;
            mode_t      open_mode;//for openat
        };
    };
};

class YiledTask : public SchedulerTask {
public:
    friend class IOUringIOScheduler;
    friend class TaskScheduler;

    YiledTask(Context *context) : SchedulerTask(context, SchedulerTaskType::Yield) {}

    ~YiledTask() {
        clear();
    }
};

class TimeOutTask : public SchedulerTask {
public:
    friend class IOUringIOScheduler;
    friend class TaskScheduler;
    TimeOutTask(Context *context, std::chrono::nanoseconds timeout)
        : SchedulerTask(context, SchedulerTaskType::Timeout), timeout(timeout) {}

    ~TimeOutTask() {
        clear();
    }

    std::strong_ordering operator<=>(const TimeOutTask &rhs) const noexcept {
        return timeout <=> rhs.timeout;
    }

private:
    std::chrono::nanoseconds timeout;
};

using namespace std::chrono_literals;

ErrorCode transFromLinuxFail(int errorno) {

    return Error::Success;
}

class IOUringIOScheduler {
public:
    IOUringIOScheduler(uint32_t queue_depth = 128) {
        auto res = io_uring_queue_init(queue_depth, &ring, 0);
        if (res < 0) {
            throw std::runtime_error("io_uring_queue_init failed");
        }
    }

    ErrorCode Submit(IOTask *task) {
        auto sqe = io_uring_get_sqe(&ring);
        if (task->bgid != 0) {
            sqe->buf_group = task->bgid;
            sqe->flags = IOSQE_BUFFER_SELECT;
        }
        switch (task->opcode) {
            case IOOpcode::Read:
                io_uring_prep_read(sqe, task->fd, task->buf, task->len, task->offset);
                break;
            case IOOpcode::Write:
                io_uring_prep_write(sqe, task->fd, task->buf, task->len, task->offset);
                break;
            case IOOpcode::Connect:
                io_uring_prep_connect(sqe, task->fd, &task->addr, task->addrlen);
                break;
            case IOOpcode::Accept:
                io_uring_prep_multishot_accept(sqe, task->fd, &task->addr, &task->addrlen,
                                               SOCK_NONBLOCK | SOCK_CLOEXEC);
                break;
            case IOOpcode::Readv:
                io_uring_prep_readv(sqe, task->fd, task->vec, task->len, task->offset);
                break;
            case IOOpcode::Writev:
                io_uring_prep_writev(sqe, task->fd, task->vec, task->len, task->offset);
                break;
            case IOOpcode::Fsync:
                io_uring_prep_fsync(sqe, task->fd, task->flags);
                break;
            case IOOpcode::Fallocate:
                io_uring_prep_fallocate(sqe, task->fd, task->mode, task->offset, task->len);
                break;
            case IOOpcode::Close:
                io_uring_prep_close(sqe, task->fd);
                break;
            case IOOpcode::Openat:
                io_uring_prep_openat(sqe, task->fd, task->path, task->flags, task->open_mode);
                break;
            case IOOpcode::Regiserbuffer:
                io_uring_prep_provide_buffers(sqe, task->buf, task->len, task->count, task->bgid, 0);
                break;
            case IOOpcode::Unlinkat:
                io_uring_prep_unlinkat(sqe, task->fd, task->path, task->flags);
                break;
            case IOOpcode::Mkdirat:
                io_uring_prep_mkdirat(sqe, task->fd, task->path, task->open_mode);
                break;
            case IOOpcode::None:
                [[fallthrough]];
            default:
                return Error::InvalidArgument;
        }
        io_uring_sqe_set_data(sqe, task);
        return Error::Success;
    }

    IOTask *GetCompletedTask(std::chrono::nanoseconds timeout = 0ns) {
        io_uring_cqe     *cqe;
        __kernel_timespec ts = {.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(timeout).count(),
                                .tv_nsec = timeout.count() % 1000000000};

        auto    count = io_uring_submit_and_wait_timeout(&ring, &cqe, 0, &ts, nullptr);
        IOTask *first = nullptr;
        IOTask *last = nullptr;
        for (; cqe != cqe + count; ++cqe) {
            auto task = reinterpret_cast<IOTask *>(io_uring_cqe_get_data(cqe));
            task->error = transFromLinuxFail(cqe->res);
            if (cqe->flags & IORING_CQE_F_MORE) {
                task->bid = cqe->flags >> IORING_CQE_BUFFER_SHIFT;
            }
            if (cqe->flags & IORING_CQE_F_MORE) {
                continue;
            }
            task->IOCompleteTask = nullptr;
            if (first == nullptr)
                first = task;
            else
                last->IOCompleteTask = task;
            last = task;
        }

        io_uring_cqe_seen(&ring, cqe);
        return first;
    }

    ~IOUringIOScheduler() {
        io_uring_queue_exit(&ring);
    }

private:
    size_t   queue_size = 0;
    io_uring ring;
};

class TaskScheduler {
public:
    TaskScheduler() = default;
    TaskScheduler(const TaskScheduler &) = delete;
    TaskScheduler &operator=(const TaskScheduler &) = delete;
    TaskScheduler(TaskScheduler &&) = delete;
    TaskScheduler &operator=(TaskScheduler &&) = delete;
    ~TaskScheduler();

    ErrorCode Submit(SchedulerTask *task) {
        switch (task->GetTaskType()) {
            case SchedulerTask::SchedulerTaskType::IO: {
                auto iotask = reinterpret_cast<IOTask *>(task);
                return io_scheduler.Submit(iotask);
            }
            case SchedulerTask::SchedulerTaskType::Timeout: {
                auto timeoutTask = reinterpret_cast<TimeOutTask *>(task);
                timeoutqueue.push(timeoutTask);
                break;
            }
            case SchedulerTask::SchedulerTaskType::Yield: {
                auto yiledTask = reinterpret_cast<YiledTask *>(task);
                taskqueue.push(yiledTask);
                break;
            }
            case SchedulerTask::SchedulerTaskType::Undefined:
                return Error::NotSupport;
        }
        return Error::Success;
    }

    void run() {
        while (running) {

            while (!timeoutqueue.empty() &&
                   timeoutqueue.top()->timeout < std::chrono::system_clock::now().time_since_epoch()) {
                timeoutqueue.top()->resume();
                timeoutqueue.pop();
            }

            auto timeout = timeoutqueue.empty()
                                   ? 0ns
                                   : timeoutqueue.top()->timeout - std::chrono::system_clock::now().time_since_epoch();
            auto task = io_scheduler.GetCompletedTask(timeout);
            for (; task != nullptr; task = task->getNext()) {
                task->resume();
            }

            while (!timeoutqueue.empty() &&
                   timeoutqueue.top()->timeout < std::chrono::system_clock::now().time_since_epoch()) {
                timeoutqueue.top()->resume();
                timeoutqueue.pop();
            }

            auto runningqueue = std::move(taskqueue);
            while (!runningqueue.empty()) {
                runningqueue.front()->resume();
                runningqueue.pop();
            }
        }
    }


private:
    std::priority_queue<TimeOutTask *> timeoutqueue;
    std::queue<YiledTask *>            taskqueue;
    bool                               running = true;
    IOUringIOScheduler                 io_scheduler;
};


Task<> Sleep(Context *context, std::chrono::nanoseconds timeout) {
    TimeOutTask task(context, timeout);
    co_await task;
}

Task<> Yield(Context *context) {
    YiledTask task(context);
    co_await task;
}

class LinuxFile : public File {
public:
    static Task<ErrorCode> OpenFile(Context *context, OpenFileArgument argument, LinuxFile **fileptr) {
        IOTask task(context, IOOpcode::Openat, 0, argument.path, argument.flags, argument.mode);
        co_await task;
        if (task.GetResult() != Error::Success) {
            co_return task.GetResult();
        }
        *fileptr = new LinuxFile(task.getFd());
        co_return Error::Success;
    }

    Task<ErrorCode> read(Context *context, char *data, size_t size) override {
        return pread(context, data, size, -1);
    }

    Task<ErrorCode> readv(Context *context, iovec *vec, size_t len) override {
        return preadv(context, vec, len, -1);
    }

    Task<ErrorCode> write(Context *context, const char *data, size_t size) override {
        return pwrite(context, data, size, -1);
    }

    Task<ErrorCode> writev(Context *context, iovec *vec, size_t len) override {
        return pwritev(context, vec, len, -1);
    }


    Task<ErrorCode> pread(Context *context, char *data, size_t size, off64_t offset) override {
        return IONormalRW(context, IOOpcode::Read, data, size, offset);
    }

    Task<ErrorCode> preadv(Context *context, iovec *vec, size_t len, off64_t offset) override {
        return IONormalRWv(context, IOOpcode::Readv, vec, len, offset);
    }

    Task<ErrorCode> pwrite(Context *context, const char *data, size_t size, off64_t offset) override {
        return IONormalRW(context, IOOpcode::Write, const_cast<char *>(data), size, offset);
    }

    Task<ErrorCode> pwritev(Context *context, iovec *vec, size_t len, off64_t offset) override {
        return IONormalRWv(context, IOOpcode::Writev, vec, len, offset);
    }

    Task<ErrorCode> Close(Context *context) override {
        IOTask task(context, fd);
        co_await task;
        fd = 0;
        co_return task.GetResult();
    }

    ~LinuxFile() {
        assert(fd == 0);
    }

private:
    explicit LinuxFile(int fd) : fd(fd) {}
    Task<ErrorCode> IONormalRW(Context *context, IOOpcode opcode, char *data, size_t size, off64_t offset) {
        IOTask task(context, opcode, fd, data, size, offset);
        co_await task;
        co_return task.GetResult();
    }

    Task<ErrorCode> IONormalRWv(Context *context, IOOpcode opcode, iovec *vec, size_t len, size_t offset) {
        IOTask task(context, opcode, fd, vec, len, offset);
        co_await task;
        co_return task.GetResult();
    }

    int fd = 0;
};


class LinuxFileSystem : public FileSystem {
    Task<ErrorCode> CreateFile(Context *context, CreateFileArgument argument, File **fileptr) override {
        LinuxFile *tmp = nullptr;
        auto err = co_await LinuxFile::OpenFile(context, OpenFileArgument{.path = argument.name, O_CREAT, 777}, &tmp);
        if (err != Error::Success) {
            co_return err;
        }
        tmp->Close(context);
        co_return Error::Success;
    }

    Task<ErrorCode> OpenFile(Context *context, OpenFileArgument args, File **ptr) override {
        LinuxFile *tmp = nullptr;

        auto err = co_await LinuxFile::OpenFile(context, args, &tmp);
        if (err != Error::Success) {
            co_return err;
        }

        *ptr = tmp;
        co_return Error::Success;
    }

    Task<ErrorCode> DeleteFile(Context *context, const char *path) override {
        IOTask task(context, IOOpcode::Unlinkat, 0, path, 0, 0);
        co_await task;
        co_return task.GetResult();
    }

    Task<ErrorCode> CreateDirectory(Context *context, const char *path) override {
        IOTask task(context, IOOpcode::Mkdirat, 0, path, 0, 0);
        co_await task;
        co_return task.GetResult();
    }

    Task<ErrorCode> DeleteDirectory(Context *context, const char *path) override {
        IOTask task(context, IOOpcode::Unlinkat, 0, path, AT_REMOVEDIR, 0);
        co_await task;
        co_return task.GetResult();
    }
};

class Latch : nomove {
public:
    Latch() = default;
    ~Latch() = default;

    Task<>     lock();
    Task<bool> try_lock();
    Task<>     unlock();

private:
    std::queue<>
};


}// namespace CodeBase