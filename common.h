#include <string.h>

#pragma once

#define strequal(a, b) strcmp(a,b) == 0
extern size_t MAX_MSGLEN;
void exit_error(char *error);

// use: __attribute__((cleanup(free_ptr)))
void free_ptr(void **ptr);
