#include "common.h"
#include "passphrase_logic.h"

#include <stdio.h>
#include <getopt.h>
#include <assert.h>
#include <unistd.h>

#include <ykpers-1/ykdef.h>
#include <ykpers-1/ykcore.h>

#include <gcrypt.h>

int yubikey_transform_password(char *pw, size_t pw_len, FILE *out) {

    int ret_code = 0;
    int yk_slot = -1;
    int passphrase_hmac_algo = 0;
    int challenge_hmac_algo = 0;
    char *salt_file_path = NULL;

    int longopt_index;
    static struct option long_options[] = {
        {"slot", required_argument, 0, 0},
        {"phmac", required_argument, 0, 0},
        {"chmac", required_argument, 0, 0},
        {"salt", required_argument, 0, 0},
{0, 0, 0, 0}
    };

    optind = 1; // Reset optind <=> reset getopt
    while (true) {
        int c = getopt_long(transformer_argc, transformer_args, "", long_options, &longopt_index);
        if (c == -1) {
            break;
        }
        assert(c == 0);

        switch(longopt_index) {
            case 0:
                assert(strequal(long_options[longopt_index].name, "slot"));
                if (strequal(optarg, "1")) {
                    yk_slot = SLOT_CHAL_HMAC1;
                } else if (strequal(optarg, "2")) {
                    yk_slot = SLOT_CHAL_HMAC2;
                }
                break;
            case 1:
                assert(strequal(long_options[longopt_index].name, "phmac"));
                passphrase_hmac_algo = gcry_mac_map_name(optarg);
                break;
            case 2:
                assert(strequal(long_options[longopt_index].name, "chmac"));
                challenge_hmac_algo = gcry_mac_map_name(optarg);
                break;
            case 3:
                assert(strequal(long_options[longopt_index].name, "salt"));
                salt_file_path = optarg;
                break;
        }
    }

    assert (yk_slot != -1);
    assert(passphrase_hmac_algo != 0);  // 0 is error value of libgcrypt
    assert(challenge_hmac_algo != 0);   // 0 is error value of libgcrypt
    assert(salt_file_path);

    if (gcry_mac_get_algo_maclen(challenge_hmac_algo) > SHA1_MAX_BLOCK_SIZE) {
        fprintf(stderr, "challenge_hmac_algo size is greater than maximum Yubikey challenge size\n");
        goto ereturn;
    }

    if (access(salt_file_path, R_OK | F_OK) != 0) {
        perror("cannot access path provided by --salt argument");
        goto ereturn;
    }

    YK_KEY *yk;

    if (yk_init() < 0) {
        goto ereturn;
    }

    if ((yk = yk_open_first_key()) == NULL) {
        goto einit;
    }


    void *salt = NULL, *challenge = NULL, *response = NULL;
    char *passphrase = NULL;
    size_t salt_len = 0, challenge_len = 0;
    int ret;

    ret = load_salt(salt_file_path, pw, pw_len, &salt, &salt_len);
    switch (ret) {
        case 0: break;
        case 1:
            fprintf(stderr, "warning: the salt file is longer than maximum size %d bytes\n", MAX_SALTLEN);
            break;
        default:
            goto ebufs;
    }

    ret = new_challenge(pw, pw_len, salt, salt_len,
                        challenge_hmac_algo, &challenge, &challenge_len);
    if (ret != 0) {
        goto ebufs;
    }
    gcry_free(salt);

    response = gcry_malloc_secure(SHA1_DIGEST_SIZE);
    if (response == NULL) {
        goto ebufs;
    }
    if ((yk_challenge_response(yk,
                    yk_slot,
                    true, // may block
                    challenge_len, // challenge_len
                    challenge, // challengep
                    SHA1_DIGEST_SIZE, // response bufferlen
                    response // responsep
                    ) < 0)) {
           goto ebufs;
    }
    gcry_free(challenge);

    ret = new_passphrase(pw, pw_len,
                         response, SHA1_DIGEST_SIZE,
                         passphrase_hmac_algo, &passphrase);
    if (ret != 0) {
        goto ebufs;
    }
    gcry_free(response);

    // TODO sanity checks: max allowed passphrase length?, ...

    fprintf(out, "%s", passphrase); // assuming passphrase is already hex-encoded

    gcry_free(passphrase);

    yk_release();
    return 0;

ebufs:
    gcry_free(challenge);
    gcry_free(response);
    gcry_free(passphrase);
einit:
    yk_release();
ereturn:
    return -1;
}


