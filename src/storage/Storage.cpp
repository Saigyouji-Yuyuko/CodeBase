#include "Storage.hpp"
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

namespace CodeBase {

ErrorCode MemTeatStorage::Format() {
    return Error::Success;
}

ErrorCode MemTeatStorage::Load() {
    return Error::Success;
}
MemTeatStorage::MemTeatStorage(int maxcount) : maxcount(maxcount) {}

ErrorCode MemTeatStorage::AllocPage(Page **outPage) {
    if (!freeList.empty()) {
        PageID page_id = freeList.back();
        freeList.pop_back();
        *outPage = &pageMap[page_id];
        return Error::Success;
    }
    if (maxcount != 0 && pageMap.size() >= maxcount)
        return Error::Fail;
    pageMap[++page_id] = PageWrap();
    pageMap[page_id].pageID = page_id;
    pageMap[page_id].PinCount = 1;
    *outPage = &pageMap[page_id];
    return Error::Success;
}

ErrorCode MemTeatStorage::FreePage(PageID page_id) {
    if (pageMap.find(page_id) == pageMap.end())
        return Error::InvalidArgs;
    if (pageMap[page_id].PinCount != 0)
        return Error::Fail;
    ::memset(pageMap[page_id].data, 0, PageSize);
    freeList.push_back(page_id);
    return Error::Success;
}

ErrorCode MemTeatStorage::ReadPageWithPin(PageID page_id, Page **outPage) {
    if (pageMap.find(page_id) == pageMap.end())
        return Error::InvalidArgs;
    pageMap[page_id].PinCount++;
    *outPage = &pageMap[page_id];
    return Error::Success;
}

ErrorCode MemTeatStorage::MarkDirty(PageID page_id) {
    if (pageMap.find(page_id) == pageMap.end())
        return Error::InvalidArgs;
    pageMap[page_id].dirty = true;
    dirtyList.insert(page_id);
    return Error::Success;
}

ErrorCode MemTeatStorage::UnPinPage(PageID page_id) {
    if (pageMap.find(page_id) == pageMap.end())
        return Error::InvalidArgs;
    pageMap[page_id].PinCount--;
    return Error::Success;
}

ErrorCode MemTeatStorage::FlushAllDirtyPages() {
    dirtyList.clear();
    return Error::Success;
}

}// namespace CodeBase