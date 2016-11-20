#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Transformer function prototypes
int dummy_init(int argc, char **args);
int dummy_transform_password(char *pw, size_t pw_len, FILE *out);
int yubikey_init(int argc, char **args);
int yubikey_provision(int argc, char**args);
int yubikey_transform_password(char *pw, size_t pw_len, FILE *out);

struct transformer_t transformer = {};

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

int setup_transformer(char *name)
{

    if (strequal(name, "ykpersonalize")) {
        transformer.init = &yubikey_init;
        transformer.handler = &yubikey_transform_password;
        transformer.provision = &yubikey_provision;
    } else if (strequal(name, "dummy")) {
        transformer.init = &dummy_init;
        transformer.handler = &dummy_transform_password;
        transformer.provision = NULL;
    } else {
       return -1;
    }

    return 0;
}

