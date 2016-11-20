#include "common.h"

#include "passphrase_logic.h"

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <getopt.h>

#include <gcrypt.h>

static char *salt_path = NULL;

int subcommand_salt(int argc, char *args[]) {
    assert(argc >= 2);
    
    int longopt_index = 0;
    static struct option long_options[] = {
        {"path", required_argument, 0, 0},
        {0, 0, 0, 0} 
    };
    while (true) {
        int c = getopt_long(argc-1, args+1, "", long_options, &longopt_index);
        if (c == -1) {
            break;
        }
        assert(c == 0);

        switch(longopt_index) {
            case 0:
                  assert(strequal(long_options[longopt_index].name, "path"));
                  salt_path = optarg;
                  break;
        }

    }

    assert(salt_path != NULL);
 
	if (strequal(args[1], "show")) {
		char *pw = secure_getpass("Disk-encryption password: ");
		if (pw == NULL) {
			return 1;
		}
		void *salt; size_t salt_len;
		int ret = load_salt(salt_path, pw, strlen(pw), &salt, &salt_len);
		gcry_free(pw);
		switch(ret) {
			case 0:
				fprintf(stdout, "salt is: %.*s", salt_len, salt);
				break;
			case 1:
				fprintf(stderr, "salt is longer greater than maximum size %d\n", MAX_SALTLEN);
				break;
			default:
				fprintf(stderr, "failed to load salt: %d\n", ret);
				break;
		}
		return ret;
	}

    if (strequal(args[1], "update")) {
		fprintf(stderr, "updating the salt requires touching the LUKS slot and is not implemented yet\n");
		return 1;
    }

	assert(false);
	return 1;
   
}
