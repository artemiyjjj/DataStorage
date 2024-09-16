#define TEST_STORAGE_INIT

#include <storage/storage.h>
#include <gtest/gtest.h>

const char* storage_name = "strg_sample";
int storage_fd;
void* block_addr = NULL;
void* new_block_addr = NULL;

TEST(stotage, init_new) {
    ASSERT_EQ(init_storage(storage_name, &storage_fd), 0) << "Failed to init new empty storage";
    ASSERT_NE(storage_fd, -1) << "Returned fd of storage is not valid";
}

TEST(storage, close_new) {
    ASSERT_EQ(close_storage(&storage_fd), 0);
}

TEST(storage, init_existing) {
    ASSERT_EQ(1, 0);
}

TEST(storage, close_existing) {
    ASSERT_EQ(1, 0);
}

