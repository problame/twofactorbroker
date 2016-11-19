#include "common.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

int dummy_transform_password(char *pw, size_t pw_len, FILE *out) {
    assert(pw);
    assert(out);

    for (int c = 0; c < transformer_argc; c++) {
        fprintf(stderr, "%s\n", transformer_args[c]);
    }

    for (int c = strlen(pw)-1; c >= 0; c--) {
        fprintf(out, "%c", pw[c]);
    }

    return 0;
}

