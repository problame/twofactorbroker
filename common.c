#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

const size_t MAX_MSGLEN = 1024;
const size_t MAX_SALTLEN = 64;

void exit_error(char *error) {
    fprintf(stderr, "%s\n", error);
    exit(1); 
}

void free_ptr(void **ptr) {
    assert(ptr);
    free(*ptr);
}

