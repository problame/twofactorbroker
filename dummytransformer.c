#include <stdio.h>
#include <string.h>
#include <assert.h>

int dummy_transform_password(char *pw, FILE *out) {
    assert(pw);
    assert(out);

    for (int c = strlen(pw)-1; c >= 0; c--) {
        fprintf(out, "%c", pw[c]);
    }

    return 0;
}

