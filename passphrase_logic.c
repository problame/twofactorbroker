#include "passphrase_logic.h"
#include "common.h"
#include "util.h"

#include <stdint.h>
#include <assert.h>

#include <gcrypt.h>

/* Private function prototypes */

int is_crypto_library_initialized();

void cleanup_crypto_library(void);

int new_hmac(int algo,
             const void*key, size_t key_len,
             const void *buffer, size_t buffer_len,
             void **digest, size_t *digest_len);

/* Public */

int initialize_crypto_lib()
{
    if (!gcry_check_version(GCRYPT_VERSION)) {
        goto out00;
    }

    gcry_control(GCRYCTL_SUSPEND_SECMEM_WARN);

    /* ... If required, other initialization goes here.  Note that the
     *      process might still be running with increased privileges and that
     *           the secure memory has not been initialized.  */

    /* Allocate a pool of 16k secure memory.  This make the secure memory
     *      available and also drops privileges where needed.  */
    gcry_control(GCRYCTL_INIT_SECMEM, 16384, 0);

    /* It is now okay to let Libgcrypt complain when there was/is
     *      a problem with the secure memory. */
    gcry_control(GCRYCTL_RESUME_SECMEM_WARN);

    /* ... If required, other initialization goes here.  */
    atexit(cleanup_crypto_library);

    /* Tell Libgcrypt that initialization has completed. */
    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);

    return 0;
out00:
    return -1;
}

int new_challenge(const void *pw, size_t pw_len,
                  const void *salt, size_t salt_len,
                  int algo, void **challenge, size_t *challenge_len)
{
    assert(algo != 0);
    return new_hmac(algo, pw, pw_len, salt, salt_len, challenge, challenge_len);
}

int new_passphrase(const void *pw, size_t pw_len,
                   const void *response, size_t response_len,
                   int algo, char **passphrase)
{
    assert(passphrase);

    void *digest = NULL;
    size_t digest_len = 0;

    if (new_hmac(algo, pw, pw_len, response, response_len, &digest, &digest_len) < 0) {
        goto out00;
    }

    if (new_hexencode(digest, digest_len, passphrase, NULL) < 0) {
        goto out00;
    }

    return 0;

out00:
    return -1;
}

/* Private */

int is_crypto_library_initialized()
{
    return gcry_control(GCRYCTL_INITIALIZATION_FINISHED_P) ? 0 : -1;
}

void cleanup_crypto_library(void)
{
    /* Erase secure memory */
    gcry_control(GCRYCTL_TERM_SECMEM);
}

int new_hmac(int algo,
             const void*key, size_t key_len,
             const void *buffer, size_t buffer_len,
             void **digest, size_t *digest_len)
{
    char *hmac;
    size_t hmac_len, hmac_expected_len;
    gcry_mac_hd_t hd;
    gcry_error_t err = 0;
    int errno_val = 0;

    if (is_crypto_library_initialized() < 0)
        return -1;

    err = gcry_mac_test_algo(algo);
    if (err)
        goto out00;

    err = gcry_mac_open(&hd, algo, 0, NULL);
    if (err)
        goto out00;

    err = gcry_mac_setkey(hd, key, key_len);
    if (err)
        goto out10;

    err = gcry_mac_write(hd, buffer, buffer_len);
    if (err)
        goto out10;

    hmac_expected_len = gcry_mac_get_algo_maclen(algo);
    if (!(hmac_expected_len))
        goto out10;

    hmac = (char *)gcry_malloc_secure(hmac_expected_len);
    if (hmac == NULL)
        goto out10;

    hmac_len = hmac_expected_len;
    err = gcry_mac_read(hd, hmac, &hmac_len);
    if (err)
        goto out20;

    assert(hmac_len == hmac_expected_len);

    *digest = hmac;
    *digest_len = hmac_len;

    gcry_mac_close(hd);

    return 0;

out20:
    gcry_free(hmac);
out10:
    gcry_mac_close(hd);
out00:
    errno_val = gcry_err_code_to_errno(gcry_err_code(err));
    assert(errno_val >= 0);
    return errno_val == 0 ? -1 : -errno_val;
}

int store_salt(const char *path, const void *pw, size_t pw_len,
              void *salt, size_t salt_len)
{
    // TODO having this part be atomic would be great..
    FILE *salt_file = fopen(path, "w");
    if (salt_file == NULL) {
        goto ereturn;
    }

    size_t nbytes = fwrite(salt, salt_len, 1, salt_file);
    if (nbytes != salt_len || ferror(salt_file)) {
        // a write error occurred
        goto eclose;
    }

    if (fclose(salt_file) != 0) {
        goto eclose;
    }

    return 0;

eclose:
    fclose(salt_file);
ereturn:
    return -1;

}

int load_salt(const char *path, const void *pw, size_t pw_len,
              void **salt, size_t *salt_len)
{
    assert(salt);
    assert(salt_len);

    int ret = 0;

    FILE *salt_file = fopen(path, "r");
    if (salt_file == NULL) {
        goto ereturn;
    }

    void *salt_buf = gcry_malloc_secure(MAX_SALTLEN + 1); // one extra for the check fread, see below
    if (salt_buf == NULL) {
        goto eclose;
    }
    memset(salt_buf, 0, MAX_SALTLEN);

    size_t nbytes = fread(salt_buf, 1, MAX_SALTLEN, salt_file);
    if (!feof(salt_file) && ferror(salt_file)) {
        // a read error occurred
        goto ebuf;
    }
    if (nbytes == MAX_SALTLEN) {
        fread((uint8_t*)salt_buf + MAX_SALTLEN, 1, 1, salt_file);
        if (!feof(salt_file)) {
            // the salt file is longer than what we can handle
            ret = 1;
        }
    }

    if (fclose(salt_file) != 0) {
        goto ebuf;
    }

    *salt = salt_buf;
    *salt_len = nbytes;
    return ret;

ebuf:
    gcry_free(salt_buf);
eclose:
    fclose(salt_file);
ereturn:
    return -1;
}
