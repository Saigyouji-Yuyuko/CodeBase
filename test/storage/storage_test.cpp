#include "gtest/gtest.h"

#include "Storage.hpp"
#include <memory>
#include <string_view>
using namespace CodeBase;
using namespace testing;

TEST(StorageMemTest, Test1) {
    auto  storage = std::make_unique<MemTeatStorage>();
    Page *page = nullptr;
    auto  error = storage->AllocPage(&page);
    EXPECT_EQ(error, Error::Success);
    EXPECT_NE(page, nullptr);
    EXPECT_GT(page->pageID, 0);
    EXPECT_EQ(page->PinCount, 1);
    auto tmp = page->pageID;
    char t[PageSize];
    ::memset(t, 0, PageSize);
    EXPECT_EQ(::memcmp(page->data, t, PageSize), 0);
    ::memset(page->data, 1, PageSize);
    error = storage->MarkDirty(page->pageID);
    EXPECT_EQ(error, Error::Success);
    error = storage->UnPinPage(page->pageID);
    EXPECT_EQ(error, Error::Success);
    error = storage->ReadPageWithPin(tmp, &page);
    EXPECT_EQ(error, Error::Success);
    ::memset(t, 1, PageSize);
    EXPECT_EQ(page->PinCount, 1);
    EXPECT_EQ(::memcmp(page->data, t, PageSize), 0);
    error = storage->UnPinPage(page->pageID);
    EXPECT_EQ(error, Error::Success);
    error = storage->FreePage(page->pageID);
    EXPECT_EQ(error, Error::Success);
}