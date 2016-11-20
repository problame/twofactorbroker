#include "common.h"

#include <stdio.h>
#include <unistd.h>
#include "passphrase_logic.h"

#include <stdlib.h>
#include <assert.h>
#include <signal.h>

int subcommand_broker(int argc, char *args[]);
int subcommand_client(int argc, char *args[]);
int subcommand_provision(int argc, char *args[]);
int subcommand_salt(int argc, char *args[]);

static void sigint_handler(int signum) {
    fprintf(stderr, "exiting...\n");
    // TODO yk_release?
    exit(1);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        exit(1);
    }

    struct sigaction sa = {};
    sa.sa_handler = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("cannot register for SIGINT");
        exit(1);
    }

    if (initialize_crypto_lib() != 0) {
        perror("could not initialize libgcrypt");
        exit(1);
    }

    char *subcommand = argv[1];

    if (strequal("broker", subcommand)) {
        return subcommand_broker(argc-1, argv+1);
    }

    if (strequal("client", subcommand)) {
        return subcommand_client(argc-1, argv+1);
    }

    if (strequal("provision", subcommand)) {
        return subcommand_provision(argc-1, argv+1);
    }

    if (strequal("salt", subcommand)) {
        return subcommand_salt(argc-1, argv+1);
    }

    exit(1);
}


