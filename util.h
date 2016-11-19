#include <stdlib.h>

char *concat_buffers(char *a, size_t a_len, char *b, size_t b_len, size_t *buffer_len);
int new_hexencode(const char *buffer, size_t buffer_len, char **out, size_t *out_len);
int new_hexdecode(const char *hex, size_t hex_len, char **out, size_t *out_len);
