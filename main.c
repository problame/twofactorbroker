#include "common.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

int subcommand_broker(int argc, char *args[]);
int subcommand_client(int argc, char *args[]);


int main(int argc, char *argv[]) {

    if (argc < 2) {
        exit(1);
    }

    char *subcommand = argv[1];

    if (strequal("broker", subcommand)) {
        return subcommand_broker(argc-1, argv+1);
    }

    if (strequal("client", subcommand)) {
        return subcommand_client(argc-1, argv+1);
    }


    exit(1);
}


