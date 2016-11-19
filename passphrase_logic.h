#include <stdlib.h>

/*
  Passphrase logic should be kept out of transfomers
  as much as possible.
  => Encourage reuse.
  => Designed to work with different HMAC algorithms for
     challenge and response.
  => Use passphrase_logic.h for public interface
  => All allocations returned by passphrase_logic are
     gcry_malloc_secure() and must be freed using gcry_free()
*/

int initialize_crypto_lib();

int load_salt(void **salt, size_t *salt_len);

int new_challenge(const void *pw, size_t pw_len,
                  const void *salt, size_t salt_len,
                  int algo, void **challenge, size_t *challenge_len);

int new_passphrase(const void *pw, size_t pw_len,
                   const void *response, size_t response_len,
                   int algo, char **passphrase);

