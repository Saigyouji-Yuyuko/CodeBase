#include "Storage.hpp"
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

namespace CodeBase {

Task<ErrorCode> MemTeatStorage::Format() {
    co_return Error::Success;
}

Task<ErrorCode> MemTeatStorage::Load() {
    co_return Error::Success;
}
MemTeatStorage::MemTeatStorage(int maxcount) : IStorage(), maxcount(maxcount) {}

Task<ErrorCode> MemTeatStorage::AllocPage(Page **outPage) {
    if (!freeList.empty()) {
        PageID page_id = freeList.back();
        freeList.pop_back();
        *outPage = &pageMap[page_id];
        co_return Error::Success;
    }
    if (maxcount != 0 && pageMap.size() >= maxcount)
        co_return Error::Fail;
    pageMap[++page_id] = PageWrap();
    pageMap[page_id].pageID = page_id;
    pageMap[page_id].PinCount = 1;
    *outPage = &pageMap[page_id];
    co_return Error::Success;
}

Task<ErrorCode> MemTeatStorage::FreePage(PageID page_id) {
    if (pageMap.find(page_id) == pageMap.end())
        co_return Error::InvalidArgs;
    if (pageMap[page_id].PinCount != 0)
        co_return Error::Fail;
    ::memset(pageMap[page_id].data, 0, PageSize);
    freeList.push_back(page_id);
    co_return Error::Success;
}

Task<ErrorCode> MemTeatStorage::ReadPageWithPin(PageID page_id, Page **outPage) {
    if (pageMap.find(page_id) == pageMap.end())
        co_return Error::InvalidArgs;
    pageMap[page_id].PinCount++;
    *outPage = &pageMap[page_id];
    co_return Error::Success;
}

Task<ErrorCode> MemTeatStorage::MarkDirty(PageID page_id) {
    if (pageMap.find(page_id) == pageMap.end())
        co_return Error::InvalidArgs;
    pageMap[page_id].dirty = true;
    dirtyList.insert(page_id);
    co_return Error::Success;
}

Task<ErrorCode> MemTeatStorage::UnPinPage(PageID page_id) {
    if (pageMap.find(page_id) == pageMap.end())
        co_return Error::InvalidArgs;
    pageMap[page_id].PinCount--;
    co_return Error::Success;
}

Task<ErrorCode> MemTeatStorage::FlushAllDirtyPages() {
    dirtyList.clear();
    co_return Error::Success;
}

}// namespace CodeBase