#define TEST_STORAGE_INIT

#include "../../storage/storage.h"
#include "../utils/test.h"

char* storage_name = "strg_sample";
int storage_fd;
void* block_addr = NULL;
void* new_block_addr = NULL;

DEFINE_TEST(stotage_new_init) {
    assert(init_storage(storage_name, &storage_fd) == 0);
    assert(storage_fd != -1);
}

DEFINE_TEST(storage_new_close) {
    assert(close_storage(&storage_fd) == 0);
}

DEFINE_TEST(storage_existing_init) {
    assert(1 == 0);
}

DEFINE_TEST(storage_existing_close) {
    assert(1 == 0);
}

DEFINE_TEST_GROUP(storage_new) {
    TEST_IN_GROUP(stotage_new_init),
    TEST_IN_GROUP(storage_new_close),
};

DEFINE_TEST_GROUP(storage_existing) {
    TEST_IN_GROUP(stotage_existing_init),
    TEST_IN_GROUP(storage_exsisting_close),
};

int main() {
    RUN_TEST_GROUP(storage_new);
}
