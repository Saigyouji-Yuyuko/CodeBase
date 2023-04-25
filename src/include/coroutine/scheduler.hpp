#pragma once
#include <cstdint>
#include <functional>
#include <thread>
namespace CodeBase {

struct schedulerTask {
    enum class schedulerTaskType {
        Invalid,
        IOWait,
        TimeWait,
        ReScheduler,
    };
};

class SingleThreadScheduler {
public:
    SingleThreadScheduler();
    ~SingleThreadScheduler();

    void run();

private:
    std::thread th;
};


struct ExcuteTaskBase {
    enum IOEventType { Read = 1, Write = 2 };
    void     excute();
    int      fd;
    uint64_t type;
};

struct TimeOutEvent {
    uint64_t                            timeStamp;
    std::function<void(TimeOutEvent *)> callback;
    std::strong_ordering                operator<=>(const TimeOutEvent &other) const;
};


}// namespace CodeBase