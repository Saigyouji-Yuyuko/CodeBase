#include "gtest/gtest.h"

#include "Storage.hpp"
#include <memory>
#include <string_view>
using namespace CodeBase;
using namespace testing;

TEST(StorageMemTest, Test1) {
    auto  storage = std::make_unique<MemTeatStorage>();
    Page *page = nullptr;
    auto  error = storage->AllocPage(&page).sync();
    EXPECT_EQ(error, Error::Success);
    EXPECT_NE(page, nullptr);
    EXPECT_GT(page->pageID, 0);
    EXPECT_EQ(page->PinCount, 1);
    auto tmp = page->pageID;
    char t[PageSize];
    ::memset(t, 0, PageSize);
    EXPECT_EQ(::memcmp(page->data, t, PageSize), 0);
    ::memset(page->data, 1, PageSize);
    error = storage->MarkDirty(page->pageID).sync();
    EXPECT_EQ(error, Error::Success);
    error = storage->UnPinPage(page->pageID).sync();
    EXPECT_EQ(error, Error::Success);
    error = storage->ReadPageWithPin(tmp, &page).sync();
    EXPECT_EQ(error, Error::Success);
    ::memset(t, 1, PageSize);
    EXPECT_EQ(page->PinCount, 1);
    EXPECT_EQ(::memcmp(page->data, t, PageSize), 0);
    error = storage->UnPinPage(page->pageID).sync();
    EXPECT_EQ(error, Error::Success);
    error = storage->FreePage(page->pageID).sync();
    EXPECT_EQ(error, Error::Success);
}

class MemTeatStorageTest : public ::testing::Test {
protected:
    void SetUp() override {
        memTeatStorage = MemTeatStorage();
    }

    MemTeatStorage memTeatStorage;
};

TEST_F(MemTeatStorageTest, AllocAndFreePages) {
    Page     *page = nullptr;
    ErrorCode err = memTeatStorage.AllocPage(&page).sync();
    ASSERT_EQ(err, Error::Success);
    std::cout << page->PinCount << std::endl;
    PageID page_id = page->GetPageID();
    memTeatStorage.MarkDirty(page_id).sync();
    memTeatStorage.UnPinPage(page_id).sync();
    std::cout << page->PinCount << std::endl;
    err = memTeatStorage.FreePage(page_id).sync();
    ASSERT_EQ(err, Error::Success);
}

TEST_F(MemTeatStorageTest, ReadPageWithPin) {
    Page     *page = nullptr;
    ErrorCode err = memTeatStorage.AllocPage(&page).sync();
    ASSERT_EQ(err, Error::Success);

    PageID page_id = page->GetPageID();
    memTeatStorage.MarkDirty(page_id).sync();
    memTeatStorage.UnPinPage(page_id).sync();

    Page *read_page = nullptr;
    err = memTeatStorage.ReadPageWithPin(page_id, &read_page).sync();
    ASSERT_EQ(err, Error::Success);

    ASSERT_EQ(read_page->GetPageID(), page_id);
    memTeatStorage.UnPinPage(page_id);
}

TEST_F(MemTeatStorageTest, FlushAllDirtyPages) {
    Page     *page = nullptr;
    ErrorCode err = memTeatStorage.AllocPage(&page).sync();
    ASSERT_EQ(err, Error::Success);

    PageID page_id = page->GetPageID();
    memTeatStorage.MarkDirty(page_id);
    memTeatStorage.UnPinPage(page_id);

    err = memTeatStorage.FlushAllDirtyPages().sync();
    ASSERT_EQ(err, Error::Success);
}

TEST_F(MemTeatStorageTest, AllocAndFreePagesAsync) {
    auto test = [&]() -> Task<void> {
        Page     *page = nullptr;
        ErrorCode err = co_await memTeatStorage.AllocPage(&page);
        assert(err == Error::Success);

        PageID page_id = page->GetPageID();
        err = co_await memTeatStorage.MarkDirty(page_id);
        assert(err == Error::Success);

        err = co_await memTeatStorage.UnPinPage(page_id);
        assert(err == Error::Success);

        err = co_await memTeatStorage.FreePage(page_id);
        assert(err == Error::Success);
    };
    test().sync();
}

TEST_F(MemTeatStorageTest, ReadPageWithPinAsync) {
    auto test = [&]() -> Task<void> {
        Page     *page = nullptr;
        ErrorCode err = co_await memTeatStorage.AllocPage(&page);
        assert(err == Error::Success);

        PageID page_id = page->GetPageID();
        err = co_await memTeatStorage.MarkDirty(page_id);
        assert(err == Error::Success);

        err = co_await memTeatStorage.UnPinPage(page_id);
        assert(err == Error::Success);

        Page *read_page = nullptr;
        err = co_await memTeatStorage.ReadPageWithPin(page_id, &read_page);
        assert(err == Error::Success);

        assert(read_page->GetPageID() == page_id);
        err = co_await memTeatStorage.UnPinPage(page_id);
        assert(err == Error::Success);
    };
    test().sync();
}

TEST_F(MemTeatStorageTest, FlushAllDirtyPagesAsync) {
    auto test = [&]() -> Task<void> {
        Page     *page = nullptr;
        ErrorCode err = co_await memTeatStorage.AllocPage(&page);
        assert(err == Error::Success);

        PageID page_id = page->GetPageID();
        err = co_await memTeatStorage.MarkDirty(page_id);
        assert(err == Error::Success);

        err = co_await memTeatStorage.UnPinPage(page_id);
        assert(err == Error::Success);

        err = co_await memTeatStorage.FlushAllDirtyPages();
        assert(err == Error::Success);
    };
    test().sync();
}

TEST_F(MemTeatStorageTest, PageCRUDAsync) {
    auto test = [&]() -> Task<void> {
        Page     *page = nullptr;
        ErrorCode err = co_await memTeatStorage.AllocPage(&page);
        assert(err == Error::Success);

        PageID page_id = page->GetPageID();

        // Write data to the page
        std::string data = "Hello, world!";
        err = co_await page->WriteAsync(data.c_str(), 0, data.size());
        assert(err == Error::Success);

        // Mark the page as dirty and unpin it
        err = co_await memTeatStorage.MarkDirty(page_id);
        assert(err == Error::Success);
        err = co_await memTeatStorage.UnPinPage(page_id);
        assert(err == Error::Success);

        // Read the page with pinning
        Page *read_page = nullptr;
        err = co_await memTeatStorage.ReadPageWithPin(page_id, &read_page);
        assert(err == Error::Success);
        assert(read_page->GetPageID() == page_id);

        // Read data from the page and compare it to the written data
        std::string tttt(data.size(), '\0');
        err = co_await read_page->ReadAsync(tttt.data(), 0, data.size());
        assert(err == Error::Success);
        assert(tttt == data);

        // Unpin the page and free it
        err = co_await memTeatStorage.UnPinPage(page_id);
        assert(err == Error::Success);
        err = co_await memTeatStorage.FreePage(page_id);
        assert(err == Error::Success);
    };
    test().sync();
}