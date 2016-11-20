#include "common.h"

#include <getopt.h>
#include <assert.h>
#include <stdbool.h>

int subcommand_provision(int argc, char *args[]) {

    int longopt_index = 0;
    static struct option long_options[] = {
        {"platform", required_argument, 0, 0},
        {0, 0, 0, 0}
    };

    while (true) {
        int c = getopt_long(argc, args, "", long_options, &longopt_index);
        if (c == -1) {
            break;
        }
        assert(c == 0);

        switch(longopt_index) {
            case 0:
                assert(strequal(long_options[longopt_index].name, "platform"));
                setup_transformer(optarg);
        }
    }

    if (!transformer.provision) {
        fprintf(stderr, "provisioning not implemented for this platform\n");
        return -1;
    }

    assert(transformer.provision);
    return (*transformer.provision)(argc - (optind -1), args + (optind - 1));

}
