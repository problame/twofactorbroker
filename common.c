#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <termios.h>

#include <gcrypt.h>

// Transformer function prototypes
int dummy_init(int argc, char **args);
int dummy_transform_password(char *pw, size_t pw_len, FILE *out);
int yubikey_init(int argc, char **args);
int yubikey_provision(int argc, char**args);
int yubikey_transform_password(char *pw, size_t pw_len, FILE *out);

struct transformer_t transformer = {};

const size_t MAX_MSGLEN = 1024;
const size_t MAX_SALTLEN = 64;
const size_t MAX_PWLEN = 1024;

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

char *secure_getpass(const char *prompt)
{

    fprintf(stdout, "%s", prompt);

    struct termios old_term, new_term;

    /* Turn echoing off and fail if we can’t. */
    if (tcgetattr (STDIN_FILENO, &old_term) != 0) {
        goto ereturn;
    }
    new_term = old_term;
    new_term.c_lflag &= ~ECHO;
    if (tcsetattr (STDIN_FILENO, TCSAFLUSH, &new_term) != 0) {
        goto ereturn;
    }

    /* Read the password. */
    char *pw = gcry_malloc_secure(MAX_PWLEN + 2);
    if (pw == NULL) {
        goto eterm;
    }
    pw = fgets(pw, MAX_PWLEN+2, stdin);
    if (pw != NULL) { // Nibble off newline, which is appended by fgets
        size_t pw_len = strlen(pw);
        pw[pw_len-1] = '\0';
    }
    fprintf(stdout, "\n");

    /* Restore terminal. */
    if (tcsetattr (STDIN_FILENO, TCSAFLUSH, &old_term) != 0) {
        goto ebuf;
    }


    return pw;

ebuf:
    gcry_free(pw);
eterm:
    tcsetattr (STDIN_FILENO, TCSAFLUSH, &old_term);
ereturn:
    return NULL;

}
