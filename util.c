#include "util.h"

#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#include <gcrypt.h>

int new_hexencode(const char *buffer, size_t buffer_len, char **out, size_t *out_len)
{

    static const char hexalphabet[] = "0123456789abcdef";

    char *o = gcry_malloc_secure(2 * buffer_len + 1);
    if (o == NULL) {
        return EXIT_FAILURE;
    }
    memset(o, 0, 2 * buffer_len + 1);

    for (size_t c = 0; c < buffer_len; c++) {
        /* TODO: hex encoding byte order independent? */
        o[2 * c] = hexalphabet[(buffer[c] & 0xF0) >> 4];
        o[2 * c + 1] = hexalphabet[(buffer[c] & 0x0F) >> 0];
    }
    assert(o[2 * buffer_len] == '\0');

    *out = o;
    if (out_len) {
        *out_len = strlen(o);
    }

    return EXIT_SUCCESS;
}

int new_hexdecode(const char *hex, size_t hex_len, char **out, size_t *out_len)
{

    assert(hex_len % 2 == 0);

    char *o = gcry_malloc_secure(hex_len / 2);
    if (o == NULL)
        goto out00;

    for (size_t c = 0; c < hex_len; c += 2) {
        unsigned int cur = 0;
        sscanf(hex + c, "%02x", &cur);
        assert((cur & 0xFFFFFF00) == 0);
        o[c / 2] = (char)cur;
    }

    *out = o;
    *out_len = hex_len / 2;
    return EXIT_SUCCESS;

out00:
    return EXIT_FAILURE;
}
