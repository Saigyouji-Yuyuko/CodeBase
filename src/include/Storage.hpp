#pragma once
#include "coroutine/Task.hpp"
#include "utils/environment.hpp"
#include "utils/error.hpp"
#include <cstddef>
#include <fcntl.h>
#include <set>
#include <unordered_map>
#include <vector>

namespace CodeBase {

static inline constexpr size_t PageSize = 4096;
using PageID = size_t;

struct Page {
    char   data[PageSize];
    size_t PinCount = 0;
    PageID pageID = 0;
    PageID GetPageID() const {
        return pageID;
    }
    ErrorCode Read(char *data, size_t offset, size_t len) {
        if (offset + len > PageSize)
            return Error::OutOfRange;
        ::memcpy(data, this->data + offset, len);
        return Error::Success;
    }
    ErrorCode Write(const char *data, size_t offset, size_t len) {
        if (offset + len > PageSize)
            return Error::OutOfRange;
        ::memcpy(this->data + offset, data, len);
        return Error::Success;
    }
    Task<ErrorCode> ReadAsync(char *data, size_t offset, size_t len) {
        if (offset + len > PageSize)
            co_return Error::OutOfRange;
        ::memcpy(data, this->data + offset, len);
        co_return Error::Success;
    }
    Task<ErrorCode> WriteAsync(const char *data, size_t offset, size_t len) {
        if (offset + len > PageSize)
            co_return Error::OutOfRange;
        ::memcpy(this->data + offset, data, len);
        co_return Error::Success;
    }
    size_t size() const {
        return PageSize;
    }
};

class IStorage {
public:
    IStorage() = default;
    virtual ~IStorage() = default;

    virtual Task<ErrorCode> Format() = 0;

    virtual Task<ErrorCode> Load() = 0;

    virtual Task<ErrorCode> AllocPage(Page **outPage) = 0;

    virtual Task<ErrorCode> FreePage(PageID page_id) = 0;

    virtual Task<ErrorCode> ReadPageWithPin(PageID page_id, Page **outPage) = 0;

    virtual Task<ErrorCode> MarkDirty(PageID page_id) = 0;

    virtual Task<ErrorCode> UnPinPage(PageID page_id) = 0;

    virtual Task<ErrorCode> FlushAllDirtyPages() = 0;
};

class MemTeatStorage final : public IStorage {
public:
    struct PageWrap : public Page {
        bool dirty = false;
    };

    MemTeatStorage() = default;
    MemTeatStorage(int maxcount);

    ~MemTeatStorage() override = default;

    Task<ErrorCode> Format() override;
    Task<ErrorCode> Load() override;


    Task<ErrorCode> AllocPage(Page **outPage) override;
    Task<ErrorCode> FreePage(PageID page_id) override;
    Task<ErrorCode> ReadPageWithPin(PageID page_id, Page **outPage) override;
    Task<ErrorCode> MarkDirty(PageID page_id) override;
    Task<ErrorCode> UnPinPage(PageID page_id) override;
    Task<ErrorCode> FlushAllDirtyPages() override;

private:
    size_t                               maxcount = 0;
    std::unordered_map<PageID, PageWrap> pageMap;
    std::vector<PageID>                  freeList;
    std::set<PageID>                     dirtyList;
    size_t                               page_id = 0;
};

}// namespace CodeBase