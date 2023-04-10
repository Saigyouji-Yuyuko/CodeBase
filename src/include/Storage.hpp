#pragma once
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
};

template<typename T>
concept StorageInterface = requires(T a, Page **outPage) {
                               { a.Format() } -> std::same_as<ErrorCode>;
                               { a.Load() } -> std::same_as<ErrorCode>;
                               { a.AllocPage(outPage) } -> std::same_as<ErrorCode>;
                               { a.FreePage(PageID{}) } -> std::same_as<ErrorCode>;
                               { a.ReadPageWithPin(PageID{}, outPage) } -> std::same_as<ErrorCode>;
                               { a.MarkDirty(PageID{}) } -> std::same_as<ErrorCode>;
                               { a.UnPinPage(PageID{}) } -> std::same_as<ErrorCode>;
                               { a.FlushAllDirtyPages() } -> std::same_as<ErrorCode>;
                           };

class MemTeatStorage {
public:
    struct PageWrap : public Page {
        bool dirty = false;
    };

    MemTeatStorage() = default;
    MemTeatStorage(int maxcount);

    ErrorCode Format();
    ErrorCode Load();

    ~MemTeatStorage() = default;

    ErrorCode AllocPage(Page **outPage);
    ErrorCode FreePage(PageID page_id);
    ErrorCode ReadPageWithPin(PageID page_id, Page **outPage);
    ErrorCode MarkDirty(PageID page_id);
    ErrorCode UnPinPage(PageID page_id);
    ErrorCode FlushAllDirtyPages();


private:
    size_t                               maxcount = 0;
    std::unordered_map<PageID, PageWrap> pageMap;
    std::vector<PageID>                  freeList;
    std::set<PageID>                     dirtyList;
    size_t                               page_id = 0;
};
static_assert(StorageInterface<MemTeatStorage>);

template<typename T>
concept StorageWrap = requires(T a, const T &ca) {
                          requires StorageInterface<typename T::StorageType>;
                          { a.GetStorage() } -> std::same_as<typename T::StorageType &>;
                          { ca.GetStorage() } -> std::same_as<const typename T::StorageType &>;
                      };


}// namespace CodeBase