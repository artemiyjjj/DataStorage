#include "include/storage.h"

static int create_open_storage(const char* filename, int*fd) {
    int res_open;
    int res_trunc;
    int res_crt_bl;

    res_open = open_storage(filename, true, fd);
    if (res_open != 0) {
        return 1;
    }
    // res_trunc = trunc_storage(*fd, 0);
    // if (res_trunc != 0) {
    //     close(*fd);
    //     return 2;

    // trunc region on file (if needed), load region


    // Create header block for storage meta info
    res_crt_bl = create_block(*fd, 0, BLOCK_HEAD);
    if (res_crt_bl != 0) {
        close(*fd);
        return 3;
    }
    return 0;
}

int init_storage(const char* filename, int* fd) {
    if (open_storage(filename, false, fd) == 0) {
        fprintf(stdout, "Storage \"%s\" opened.\n", filename);
    } else if (create_open_storage(filename, fd) == 0) {
        // fprintf(stderr, "Failed to open storage with filename \"%s\"\n", filename);


    } else {
        fprintf(stderr, "Failed to open or create storage");
        return 1;
    }

    // create struct to manager loaded blocks (hashset?)
    // load header block only

    return 0;
}

int close_storage(int* fd) {
    return close(*fd);
}
