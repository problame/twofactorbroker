#include "common.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

int dummy_init(int argc, char **args) {
    assert(args);

    for (int c = 0; c < argc; c++) {
        fprintf(stderr, "%s\n", args[c]);
    }
    return 0;
}

int dummy_transform_password(char *pw, size_t pw_len, FILE *out) {
    assert(pw);
    assert(out);

    for (int c = strlen(pw)-1; c >= 0; c--) {
        fprintf(out, "%c", pw[c]);
    }

    return 0;
}

