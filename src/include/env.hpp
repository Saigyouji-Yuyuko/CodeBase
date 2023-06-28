#pragma once

#include "Task.hpp"
#include "error.hpp"
#include "logger.hpp"
#include <sys/types.h>

namespace CodeBase {

using PageId_t = uint64_t;

struct PageHeader {
    uint64_t checksum;
    uint64_t version;
    PageId_t page_id;
    uint64_t last_modified_journal_id;
    uint64_t page_type;
    uint64_t page_header_size;
};

struct Page : public PageHeader {
    uint8_t *data;
    uint64_t pin_count;
};

class Allocator;
class Context;
struct NewPageArgument {};


class StorageInterface {
public:
    StorageInterface() = default;
    virtual Task<ErrorCode> CreateEmptyPage(Context *, NewPageArgument, Page **page) = 0;
    virtual Task<ErrorCode> LoadPage(Context *, PageId_t, Page **page) = 0;
    virtual Task<ErrorCode> PinPage(Context *, PageId_t, Page *page = nullptr) = 0;
    virtual Task<ErrorCode> UnpinPage(Context *, PageId_t, Page *page = nullptr) = 0;
    virtual Task<ErrorCode> FlushPage(Context *, PageId_t, Page *page = nullptr) = 0;
    virtual Task<ErrorCode> DeletePage(Context *, PageId_t, Page *page = nullptr) = 0;
    virtual ~StorageInterface() = default;

    Allocator *GetAllocator();
    ErrorCode  CreateEmptyPageSync(Context *, NewPageArgument, Page **page);
    ErrorCode  LoadPageSync(Context *, PageId_t, Page **page);
    ErrorCode  PinPageSync(Context *, PageId_t);
    ErrorCode  UnpinPageSync(Context *, PageId_t);
    ErrorCode  FlushPageSync(Context *, PageId_t, Page *page);
    ErrorCode  DeletePageSync(Context *, PageId_t);

protected:
    void SetAllocator(Allocator *allocator);

private:
    Allocator *allocator = nullptr;
};

class IndexManager {};

class LockManager {};
class TransactionManager {};

class JournalManager {};

class File;
class Transaction;

struct CreateFileArgument {
    const char *name;
};

struct OpenFileArgument {
    const char *path;
    mode_t      mode;
    int         flags;
};


class nocopy {
public:
    nocopy() = default;
    nocopy(const nocopy &) = delete;
    nocopy &operator=(const nocopy &) = delete;
    ~nocopy() = default;
};

class nomove : public nocopy {
public:
    nomove() = default;
    nomove(nomove &&) = delete;
    nomove &operator=(nomove &&) = delete;
    ~nomove() = default;
};
class StreamFile;
class ReadOnlyFile;
class RandomFile;

class File : public nocopy {
public:
    virtual ~File() = default;
    virtual Task<ErrorCode> Close(Context *context) = 0;
    virtual Task<ErrorCode> pread(Context *context, char *len, size_t size, off64_t offset) = 0;
    virtual Task<ErrorCode> preadv(Context *context, iovec *vec, size_t len, off64_t offset) = 0;
    virtual Task<ErrorCode> read(Context *context, char *len, size_t size) = 0;
    virtual Task<ErrorCode> readv(Context *context, iovec *vec, size_t len) = 0;

    virtual Task<ErrorCode> write(Context *context, const char *len, size_t size) = 0;
    virtual Task<ErrorCode> writev(Context *context, iovec *vec, size_t len) = 0;
    virtual Task<ErrorCode> pwrite(Context *context, const char *len, size_t size, off64_t offset) = 0;
    virtual Task<ErrorCode> pwritev(Context *context, iovec *vec, size_t len, off64_t offset) = 0;
};


class FileSystem {
public:
    FileSystem() = default;
    virtual ~FileSystem() = default;

    FileSystem(const FileSystem &) = delete;
    FileSystem &operator=(const FileSystem &) = delete;
    FileSystem(FileSystem &&) = delete;
    FileSystem &operator=(FileSystem &&) = delete;

    virtual Task<ErrorCode> CreateFile(Context *, CreateFileArgument, File **) = 0;
    virtual Task<ErrorCode> OpenFile(Context *, OpenFileArgument, File **) = 0;
    virtual Task<ErrorCode> DeleteFile(Context *, const char *path) = 0;
    virtual Task<ErrorCode> CreateDirectory(Context *, const char *path) = 0;
    virtual Task<ErrorCode> DeleteDirectory(Context *, const char *ath) = 0;

    ErrorCode CreateFileSync(Context *, std::string path, File **);
    ErrorCode OpenFileSync(Context *, std::string path, File **);
    ErrorCode DeleteFileSync(Context *, std::string path);
    ErrorCode CreateDirectorySync(Context *, std::string path);
    ErrorCode DeleteDirectorySync(Context *, std::string path);
};

class LogManager;

class QueryEngine {};
class TaskScheduler;
class MetaDateEngine {};
struct SchedulerTask;

class Context {
public:
    void   Submit(SchedulerTask *task);
    Task<> Yield();
    Task<> Sleep(std::chrono::nanoseconds);

private:
    TaskScheduler *scheduler = nullptr;
};

}// namespace CodeBase