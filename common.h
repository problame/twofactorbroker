#include <string.h>

#pragma once

#define strequal(a, b) strcmp(a,b) == 0
extern size_t MAX_MSGLEN;
void exit_error(char *error);

// use: __attribute__((cleanup(free_ptr)))
void free_ptr(void **ptr);

// Variables used by transformer implementation. arguments passed after --
extern int   transformer_argc;
extern char **transformer_args; // to use getopt in the transformer, the first array entry is not an option (like in main())


