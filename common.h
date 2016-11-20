#include <string.h>
#include <stdio.h>

#pragma once

#define strequal(a, b) strcmp(a,b) == 0
extern size_t MAX_MSGLEN;
extern size_t MAX_SALTLEN;

void exit_error(char *error);

// use: __attribute__((cleanup(free_ptr)))
void free_ptr(void **ptr);

// Variables used by transformer implementation. arguments passed after --
struct transformer_t {
    int(*init)(int argc, char **args); //to use getopt in the transformer, the first array entry is not an option, but -- (like the binary path/name in main())
    int(*handler)(char *pw, size_t pw_len, FILE *out);
};

static struct transformer_t transformer = {};

