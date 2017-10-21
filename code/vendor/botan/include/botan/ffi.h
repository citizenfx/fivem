/*
* FFI (C89 API)
* (C) 2015,2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_FFI_H_
#define BOTAN_FFI_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
This header exports some of botan's functionality via a C89
interface. This API is uesd by the Python and OCaml bindings via those
languages respective ctypes libraries.

The API is intended to be as easy as possible to call from other
languages, which often have easy ways to call C, because C. But some C
code is easier to deal with than others, so to make things easy this
API follows a few simple rules:

- All interactions are via pointers to opaque structs. No need to worry about
  structure padding issues and the like.

- All functions return an int error code (except the version calls, which are
  assumed to always have something to say).

- Use simple types: size_t for lengths, const char* NULL terminated strings,
  uint8_t for binary.

- No ownership of memory transfers across the API boundary. The API will
  consume data from const pointers, and will produce output by writing to
  variables provided by the caller.

- If exporting a value (a string or a blob) the function takes a pointer to the
  output array and a read/write pointer to the length. If the length is insufficient, an
  error is returned. So passing nullptr/0 allows querying the final value.

  Note this does not apply to all functions, like `botan_hash_final`
  which is not idempotent and are documented specially. But it's a
  general theory of operation.

The API is not currently documented, nor should it be considered
stable. It is buggy as heck, most likely, and error handling is a
mess. However the goal is to provide a long term API usable for
language bindings, or for use by systems written in C. Suggestions on
how to provide the cleanest API for such users would be most welcome.

* TODO:
* - Better error reporting
* - User callback for exception logging?
* - Doxygen comments for all functions/params
* - X.509 certs and PKIX path validation goo
* - TLS
*/

#include <botan/build.h>
#include <stdint.h>
#include <stddef.h>

/**
* Return the version of the currently supported FFI API. This is
* expressed in the form YYYYMMDD of the release date of this version
* of the API.
*/
BOTAN_PUBLIC_API(2,0) uint32_t botan_ffi_api_version();

/**
* Return 0 (ok) if the version given is one this library supports.
* botan_ffi_supports_api(botan_ffi_api_version()) will always return 0.
*/
BOTAN_PUBLIC_API(2,0) int botan_ffi_supports_api(uint32_t api_version);

/**
* Return a free-form version string, e.g., 2.0.0
*/
BOTAN_PUBLIC_API(2,0) const char* botan_version_string();

/**
* Return the major version of the library
*/
BOTAN_PUBLIC_API(2,0) uint32_t botan_version_major();

/**
* Return the minor version of the library
*/
BOTAN_PUBLIC_API(2,0) uint32_t botan_version_minor();

/**
* Return the patch version of the library
*/
BOTAN_PUBLIC_API(2,0) uint32_t botan_version_patch();

/**
* Return the date this version was released as
* an integer, or 0 if an unreleased version
*/
BOTAN_PUBLIC_API(2,0) uint32_t botan_version_datestamp();

/*
* Error handling
*
* Some way of exporting these values to other languages would be useful


 THIS FUNCTION ASSUMES BOTH ARGUMENTS ARE LITERAL STRINGS
 so it retains only the pointers and does not make a copy.

int botan_make_error(const char* msg, const char* func, int line);
* This value is returned to callers ^^

 normally called like
   return botan_make_error(BOTAN_ERROR_STRING_NOT_IMPLEMENTED, BOTAN_FUNCTION, __LINE__);

// This would seem to require both saving the message permanently
catch(std::exception& e) {
return botan_make_error_from_transient_string(e.what(), BOTAN_FUNCTION, __LINE__);
}

#define botan_make_error_inf(s) return botan_make_error(s, BOTAN_FUNCTION, __LINE__);

Easier to return a const char* from each function directly? However,

catch(std::exception& e) { return e.what(); }

doesn't exactly work well either!

*
* Later call:
* const char* botan_get_error_str(int);
* To recover the msg, func, and line

*/
#define BOTAN_FFI_SUCCESS (0)

#define BOTAN_FFI_INVALID_VERIFIER (1)

#define BOTAN_FFI_ERROR_INVALID_INPUT (-1)
#define BOTAN_FFI_ERROR_BAD_MAC (-2)

#define BOTAN_FFI_ERROR_INSUFFICIENT_BUFFER_SPACE (-10)
#define BOTAN_FFI_ERROR_EXCEPTION_THROWN (-20)
#define BOTAN_FFI_ERROR_BAD_FLAG (-30)
#define BOTAN_FFI_ERROR_NULL_POINTER (-31)
#define BOTAN_FFI_ERROR_BAD_PARAMETER (-32)
#define BOTAN_FFI_ERROR_NOT_IMPLEMENTED (-40)

#define BOTAN_FFI_ERROR_UNKNOWN_ERROR (-100)

//const char* botan_error_description(int err);

/**
* Returns 0 if x[0..len] == y[0..len], or otherwise -1
*/
BOTAN_PUBLIC_API(2,3) int botan_constant_time_compare(const uint8_t* x, const uint8_t* y, size_t len);

/**
* Deprecated equivalent to botan_constant_time_compare
*/
BOTAN_PUBLIC_API(2,0) int botan_same_mem(const uint8_t* x, const uint8_t* y, size_t len);

/**
* Clear out memory using a system specific approach to bypass elision by the
* compiler (currently using RtlSecureZeroMemory or tricks with volatile pointers).
*/
BOTAN_PUBLIC_API(2,2) int botan_scrub_mem(void* mem, size_t bytes);

#define BOTAN_FFI_HEX_LOWER_CASE 1

/**
* Perform hex encoding
* @param x is some binary data
* @param len length of x in bytes
* @param out an array of at least x*2 bytes
* @param flags flags out be upper or lower case?
* @return 0 on success, 1 on failure
*/
BOTAN_PUBLIC_API(2,0) int botan_hex_encode(const uint8_t* x, size_t len, char* out, uint32_t flags);

/**
* Perform hex decoding
* @param hex_str a string of hex chars (whitespace is ignored)
* @param in_len the length of hex_str
* @param out the output buffer should be at least strlen(hex_str)/2 bytes
* @param out_len the size of out
*/
BOTAN_PUBLIC_API(2,3) int botan_hex_decode(const char* hex_str, size_t in_len, uint8_t* out, size_t* out_len);

/**
* Perform base64 encoding
*/
BOTAN_PUBLIC_API(2,3) int botan_base64_encode(const uint8_t* x, size_t len, char* out, size_t* out_len);


/**
* Perform base64 decoding
*/
BOTAN_PUBLIC_API(2,3) int botan_base64_decode(const char* base64_str, size_t in_len,
                                              uint8_t* out, size_t* out_len);

/**
* RNG type
*/
typedef struct botan_rng_struct* botan_rng_t;

/**
* Initialize a random number generator object
* @param rng rng object
* @param rng_type type of the rng, possible values:
*    "system": System_RNG, "user": AutoSeeded_RNG
* Set rng_type to null or empty string to let the library choose
*
* TODO: replace rng_type with simple flags?
*/
BOTAN_PUBLIC_API(2,0) int botan_rng_init(botan_rng_t* rng, const char* rng_type);

/**
* Get random bytes from a random number generator
* @param rng rng object
* @param out output buffer of size out_len
* @param out_len number of requested bytes
* @return 0 on success, negative on failure
*
* TODO: better name
*/
BOTAN_PUBLIC_API(2,0) int botan_rng_get(botan_rng_t rng, uint8_t* out, size_t out_len);

/**
* Reseed a random number generator
* Uses the System_RNG as a seed generator.
*
* @param rng rng object
* @param bits number of bits to to reseed with
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,0) int botan_rng_reseed(botan_rng_t rng, size_t bits);

/**
* Frees all resources of the random number generator object
* @param rng rng object
* @return always returns 0
*/
BOTAN_PUBLIC_API(2,0) int botan_rng_destroy(botan_rng_t rng);

/*
* Hash type
*/
typedef struct botan_hash_struct* botan_hash_t;

/**
* Initialize a hash function object
* @param hash hash object
* @param hash_name name of the hash function, e.g., "SHA-384"
* @param flags should be 0 in current API revision, all other uses are reserved
*       and return BOTAN_FFI_ERROR_BAD_FLAG
*
* TODO: since output_length is effectively required to use this API,
* return it from init as an output parameter
*/
BOTAN_PUBLIC_API(2,0) int botan_hash_init(botan_hash_t* hash, const char* hash_name, uint32_t flags);

/**
* Copy the state of a hash function object
* @param dest destination hash object
* @param source source hash object
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,2) int botan_hash_copy_state(botan_hash_t *dest, const botan_hash_t source);

/**
* Writes the output length of the hash function to *output_length
* @param hash hash object
* @param output_length output buffer to hold the hash function output length
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,0) int botan_hash_output_length(botan_hash_t hash, size_t* output_length);

/**
* Writes the block size of the hash function to *block_size
* @param hash hash object
* @param block_size output buffer to hold the hash function output length
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,2) int botan_hash_block_size(botan_hash_t hash, size_t* block_size);

/**
* Send more input to the hash function
* @param hash hash object
* @param in input buffer
* @param in_len number of bytes to read from the input buffer
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,0) int botan_hash_update(botan_hash_t hash, const uint8_t* in, size_t in_len);

/**
* Finalizes the hash computation and writes the output to
* out[0:botan_hash_output_length()] then reinitializes for computing
* another digest as if botan_hash_clear had been called.
* @param hash hash object
* @param out output buffer
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,0) int botan_hash_final(botan_hash_t hash, uint8_t out[]);

/**
* Reinitializes the state of the hash computation. A hash can
* be computed (with update/final) immediately.
* @param hash hash object
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,0) int botan_hash_clear(botan_hash_t hash);

/**
* Frees all resources of the hash object
* @param hash hash object
* @return always returns 0
*/
BOTAN_PUBLIC_API(2,0) int botan_hash_destroy(botan_hash_t hash);

/**
* TODO has no implementation
*/
BOTAN_PUBLIC_API(2,0) int botan_hash_name(botan_hash_t hash, char* name, size_t name_len);

/*
* Message Authentication type
*/
typedef struct botan_mac_struct* botan_mac_t;

/**
* Initialize a message authentication code object
* @param mac mac object
* @param mac_name name of the hash function, e.g., "HMAC(SHA-384)"
* @param flags should be 0 in current API revision, all other uses are reserved
*       and return a negative value (error code)
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,0) int botan_mac_init(botan_mac_t* mac, const char* mac_name, uint32_t flags);

/**
* Writes the output length of the message authentication code to *output_length
* @param mac mac object
* @param output_length output buffer to hold the MAC output length
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,0) int botan_mac_output_length(botan_mac_t mac, size_t* output_length);

/**
* Sets the key on the MAC
* @param mac mac object
* @param key buffer holding the key
* @param key_len size of the key buffer in bytes
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,0) int botan_mac_set_key(botan_mac_t mac, const uint8_t* key, size_t key_len);

/**
* Send more input to the message authentication code
* @param mac mac object
* @param buf input buffer
* @param len number of bytes to read from the input buffer
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,0) int botan_mac_update(botan_mac_t mac, const uint8_t* buf, size_t len);

/**
* Finalizes the MAC computation and writes the output to
* out[0:botan_mac_output_length()] then reinitializes for computing
* another MAC as if botan_mac_clear had been called.
* @param mac mac object
* @param out output buffer
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,0) int botan_mac_final(botan_mac_t mac, uint8_t out[]);

/**
* Reinitializes the state of the MAC computation. A MAC can
* be computed (with update/final) immediately.
* @param mac mac object
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,0) int botan_mac_clear(botan_mac_t mac);

/**
* Frees all resources of the MAC object
* @param mac mac object
* @return always returns 0
*/
BOTAN_PUBLIC_API(2,0) int botan_mac_destroy(botan_mac_t mac);

/*
* Cipher modes
*/
typedef struct botan_cipher_struct* botan_cipher_t;

#define BOTAN_CIPHER_INIT_FLAG_MASK_DIRECTION 1
#define BOTAN_CIPHER_INIT_FLAG_ENCRYPT 0
#define BOTAN_CIPHER_INIT_FLAG_DECRYPT 1

BOTAN_PUBLIC_API(2,0) int botan_cipher_init(botan_cipher_t* cipher, const char* name, uint32_t flags);

BOTAN_PUBLIC_API(2,0) int botan_cipher_valid_nonce_length(botan_cipher_t cipher, size_t nl);
BOTAN_PUBLIC_API(2,0) int botan_cipher_get_tag_length(botan_cipher_t cipher, size_t* tag_size);
BOTAN_PUBLIC_API(2,0) int botan_cipher_get_default_nonce_length(botan_cipher_t cipher, size_t* nl);
BOTAN_PUBLIC_API(2,0) int botan_cipher_get_update_granularity(botan_cipher_t cipher, size_t* ug);

BOTAN_PUBLIC_API(2,0) int botan_cipher_query_keylen(botan_cipher_t,
                                        size_t* out_minimum_keylength,
                                        size_t* out_maximum_keylength);

BOTAN_PUBLIC_API(2,0) int botan_cipher_set_key(botan_cipher_t cipher,
                                   const uint8_t* key, size_t key_len);

BOTAN_PUBLIC_API(2,0) int botan_cipher_set_associated_data(botan_cipher_t cipher,
                                               const uint8_t* ad, size_t ad_len);

BOTAN_PUBLIC_API(2,0) int botan_cipher_start(botan_cipher_t cipher,
                                 const uint8_t* nonce, size_t nonce_len);

#define BOTAN_CIPHER_UPDATE_FLAG_FINAL (1U << 0)

BOTAN_PUBLIC_API(2,0) int botan_cipher_update(botan_cipher_t cipher,
                                  uint32_t flags,
                                  uint8_t output[],
                                  size_t output_size,
                                  size_t* output_written,
                                  const uint8_t input_bytes[],
                                  size_t input_size,
                                  size_t* input_consumed);

BOTAN_PUBLIC_API(2,0) int botan_cipher_clear(botan_cipher_t hash);
BOTAN_PUBLIC_API(2,0) int botan_cipher_destroy(botan_cipher_t cipher);

/*
* Derive a key from a passphrase for a number of iterations
* @param pbkdf_algo PBKDF algorithm, e.g., "PBKDF2"
* @param out buffer to store the derived key, must be of out_len bytes
* @param out_len the desired length of the key to produce
* @param passphrase the password to derive the key from
* @param salt a randomly chosen salt
* @param salt_len length of salt in bytes
* @param iterations the number of iterations to use (use 10K or more)
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,0) int botan_pbkdf(const char* pbkdf_algo,
                          uint8_t out[], size_t out_len,
                          const char* passphrase,
                          const uint8_t salt[], size_t salt_len,
                          size_t iterations);

/**
* Derive a key from a passphrase, running until msec time has elapsed.
* @param pbkdf_algo PBKDF algorithm, e.g., "PBKDF2"
* @param out buffer to store the derived key, must be of out_len bytes
* @param out_len the desired length of the key to produce
* @param passphrase the password to derive the key from
* @param salt a randomly chosen salt
* @param salt_len length of salt in bytes
* @param milliseconds_to_run if iterations is zero, then instead the PBKDF is
*        run until milliseconds_to_run milliseconds has passed
* @param out_iterations_used set to the number iterations executed
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,0) int botan_pbkdf_timed(const char* pbkdf_algo,
                                uint8_t out[], size_t out_len,
                                const char* passphrase,
                                const uint8_t salt[], size_t salt_len,
                                size_t milliseconds_to_run,
                                size_t* out_iterations_used);

/**
* Derive a key
* @param kdf_algo KDF algorithm, e.g., "SP800-56C"
* @param out buffer holding the derived key, must be of length out_len
* @param out_len the desired output length in bytes
* @param secret the secret input
* @param secret_len size of secret in bytes
* @param salt a diversifier
* @param salt_len size of salt in bytes
* @param label purpose for the derived keying material
* @param label_len size of label in bytes
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,0) int botan_kdf(const char* kdf_algo,
                        uint8_t out[], size_t out_len,
                        const uint8_t secret[], size_t secret_len,
                        const uint8_t salt[], size_t salt_len,
                        const uint8_t label[], size_t label_len);

/**
* Create a password hash using Bcrypt
* @param out buffer holding the password hash, should be of length 64 bytes
* @param out_len the desired output length in bytes
* @param password the password
* @param rng a random number generator
* @param work_factor how much work to do to slow down guessing attacks
* @param flags should be 0 in current API revision, all other uses are reserved
*       and return BOTAN_FFI_ERROR_BAD_FLAG
* @return 0 on success, a negative value on failure

* Output is formatted bcrypt $2a$...
*/
BOTAN_PUBLIC_API(2,0) int botan_bcrypt_generate(uint8_t* out, size_t* out_len,
                                    const char* password,
                                    botan_rng_t rng,
                                    size_t work_factor,
                                    uint32_t flags);

/*
* Raw Block Cipher (PRP) interface
*/
typedef struct botan_block_cipher_struct* botan_block_cipher_t;

/**
* Initialize a block cipher object
*/
BOTAN_PUBLIC_API(2,1) int botan_block_cipher_init(botan_block_cipher_t* bc,
                                      const char* cipher_name);

/**
* Destroy a block cipher object
*/
BOTAN_PUBLIC_API(2,1) int botan_block_cipher_destroy(botan_block_cipher_t bc);

/**
* Reinitializes the block cipher
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,1) int botan_block_cipher_clear(botan_block_cipher_t bc);

/**
* Set the key for a block cipher instance
*/
BOTAN_PUBLIC_API(2,1) int botan_block_cipher_set_key(botan_block_cipher_t bc,
                                         const uint8_t key[], size_t len);

/**
* Return the positive block size of this block cipher, or negative to
* indicate an error
*/
BOTAN_PUBLIC_API(2,1) int botan_block_cipher_block_size(botan_block_cipher_t bc);

BOTAN_PUBLIC_API(2,1) int botan_block_cipher_encrypt_blocks(botan_block_cipher_t bc,
                                                const uint8_t in[],
                                                uint8_t out[],
                                                size_t blocks);

BOTAN_PUBLIC_API(2,1) int botan_block_cipher_decrypt_blocks(botan_block_cipher_t bc,
                                                const uint8_t in[],
                                                uint8_t out[],
                                                size_t blocks);


/*
* Multiple precision integers
*/
typedef struct botan_mp_struct* botan_mp_t;

BOTAN_PUBLIC_API(2,1) int botan_mp_init(botan_mp_t* mp);
BOTAN_PUBLIC_API(2,1) int botan_mp_destroy(botan_mp_t mp);

// writes botan_mp_num_bytes(mp)*2 + 1 bytes to out[]
BOTAN_PUBLIC_API(2,1) int botan_mp_to_hex(const botan_mp_t mp, char* out);
BOTAN_PUBLIC_API(2,1) int botan_mp_to_str(const botan_mp_t mp, uint8_t base, char* out, size_t* out_len);

BOTAN_PUBLIC_API(2,1) int botan_mp_clear(botan_mp_t mp);

BOTAN_PUBLIC_API(2,1) int botan_mp_set_from_int(botan_mp_t mp, int initial_value);
BOTAN_PUBLIC_API(2,1) int botan_mp_set_from_mp(botan_mp_t dest, const botan_mp_t source);
BOTAN_PUBLIC_API(2,1) int botan_mp_set_from_str(botan_mp_t dest, const char* str);
BOTAN_PUBLIC_API(2,1) int botan_mp_set_from_radix_str(botan_mp_t dest, const char* str, size_t radix);

BOTAN_PUBLIC_API(2,1) int botan_mp_num_bits(const botan_mp_t n, size_t* bits);
BOTAN_PUBLIC_API(2,1) int botan_mp_num_bytes(const botan_mp_t n, size_t* bytes);

// Writes botan_mp_num_bytes(mp) to vec
BOTAN_PUBLIC_API(2,1) int botan_mp_to_bin(const botan_mp_t mp, uint8_t vec[]);
BOTAN_PUBLIC_API(2,1) int botan_mp_from_bin(const botan_mp_t mp, const uint8_t vec[], size_t vec_len);

BOTAN_PUBLIC_API(2,1) int botan_mp_to_uint32(const botan_mp_t mp, uint32_t* val);

/**
* This function is not well named. Returns 1 iff mp is greater than
* *or equal to* zero. Use botan_mp_is_negative to detect negative
* numbers, botan_mp_is_zero to check for zero.
*/
BOTAN_PUBLIC_API(2,1) int botan_mp_is_positive(const botan_mp_t mp);

/**
* Return 1 iff mp is less than 0
*/
BOTAN_PUBLIC_API(2,1) int botan_mp_is_negative(const botan_mp_t mp);

BOTAN_PUBLIC_API(2,1) int botan_mp_flip_sign(botan_mp_t mp);
//BOTAN_PUBLIC_API(2,1) int botan_mp_set_negative(botan_mp_t mp);

BOTAN_PUBLIC_API(2,1) int botan_mp_is_zero(const botan_mp_t mp);
BOTAN_PUBLIC_API(2,1) int botan_mp_is_odd(const botan_mp_t mp);
BOTAN_PUBLIC_API(2,1) int botan_mp_is_even(const botan_mp_t mp);

BOTAN_PUBLIC_API(2,1) int botan_mp_add(botan_mp_t result, const botan_mp_t x, const botan_mp_t y);
BOTAN_PUBLIC_API(2,1) int botan_mp_sub(botan_mp_t result, const botan_mp_t x, const botan_mp_t y);
BOTAN_PUBLIC_API(2,1) int botan_mp_mul(botan_mp_t result, const botan_mp_t x, const botan_mp_t y);

BOTAN_PUBLIC_API(2,1) int botan_mp_div(botan_mp_t quotient,
                           botan_mp_t remainder,
                           const botan_mp_t x, const botan_mp_t y);

BOTAN_PUBLIC_API(2,1) int botan_mp_mod_mul(botan_mp_t result, const botan_mp_t x,
                               const botan_mp_t y, const botan_mp_t mod);

/*
* Returns 0 if x != y
* Returns 1 if x == y
* Returns negative number on error
*/
BOTAN_PUBLIC_API(2,1) int botan_mp_equal(const botan_mp_t x, const botan_mp_t y);

/*
* Sets *result to comparison result:
* -1 if x < y, 0 if x == y, 1 if x > y
* Returns negative number on error or zero on success
*/
BOTAN_PUBLIC_API(2,1) int botan_mp_cmp(int* result, const botan_mp_t x, const botan_mp_t y);

/*
* Swap two botan_mp_t
*/
BOTAN_PUBLIC_API(2,1) int botan_mp_swap(botan_mp_t x, botan_mp_t y);

// Return (base^exponent) % modulus
BOTAN_PUBLIC_API(2,1) int botan_mp_powmod(botan_mp_t out, const botan_mp_t base, const botan_mp_t exponent, const botan_mp_t modulus);

BOTAN_PUBLIC_API(2,1) int botan_mp_lshift(botan_mp_t out, const botan_mp_t in, size_t shift);
BOTAN_PUBLIC_API(2,1) int botan_mp_rshift(botan_mp_t out, const botan_mp_t in, size_t shift);

BOTAN_PUBLIC_API(2,1) int botan_mp_mod_inverse(botan_mp_t out, const botan_mp_t in, const botan_mp_t modulus);

BOTAN_PUBLIC_API(2,1) int botan_mp_rand_bits(botan_mp_t rand_out, botan_rng_t rng, size_t bits);

BOTAN_PUBLIC_API(2,1) int botan_mp_rand_range(botan_mp_t rand_out, botan_rng_t rng,
                                  const botan_mp_t lower_bound, const botan_mp_t upper_bound);

BOTAN_PUBLIC_API(2,1) int botan_mp_gcd(botan_mp_t out, const botan_mp_t x, const botan_mp_t y);

/**
* Returns 0 if n is not prime
* Returns 1 if n is prime
* Returns negative number on error
*/
BOTAN_PUBLIC_API(2,1) int botan_mp_is_prime(const botan_mp_t n, botan_rng_t rng, size_t test_prob);

/**
* Returns 0 if specified bit of n is not set
* Returns 1 if specified bit of n is set
* Returns negative number on error
*/
BOTAN_PUBLIC_API(2,1) int botan_mp_get_bit(const botan_mp_t n, size_t bit);

/**
* Set the specified bit
*/
BOTAN_PUBLIC_API(2,1) int botan_mp_set_bit(botan_mp_t n, size_t bit);

/**
* Clear the specified bit
*/
BOTAN_PUBLIC_API(2,1) int botan_mp_clear_bit(botan_mp_t n, size_t bit);

/* Bcrypt password hashing */

/**
* Check a previously created password hash
* @param pass the password to check against
* @param hash the stored hash to check against
* @return 0 if if this password/hash combination is valid,
*       1 if the combination is not valid (but otherwise well formed),
*       negative on error
*/
BOTAN_PUBLIC_API(2,0) int botan_bcrypt_is_valid(const char* pass, const char* hash);

/*
* Public/private key creation, import, ...
*/
typedef struct botan_privkey_struct* botan_privkey_t;

BOTAN_PUBLIC_API(2,0) int botan_privkey_create(botan_privkey_t* key,
                                   const char* algo_name,
                                   const char* algo_params,
                                   botan_rng_t rng);

#define BOTAN_CHECK_KEY_EXPENSIVE_TESTS 1

BOTAN_PUBLIC_API(2,0) int botan_privkey_check_key(botan_privkey_t key, botan_rng_t rng, uint32_t flags);

BOTAN_PUBLIC_API(2,0) int botan_privkey_create_rsa(botan_privkey_t* key, botan_rng_t rng, size_t n_bits);
BOTAN_PUBLIC_API(2,0) int botan_privkey_create_ecdsa(botan_privkey_t* key, botan_rng_t rng, const char* params);
BOTAN_PUBLIC_API(2,0) int botan_privkey_create_ecdh(botan_privkey_t* key, botan_rng_t rng, const char* params);
BOTAN_PUBLIC_API(2,0) int botan_privkey_create_mceliece(botan_privkey_t* key, botan_rng_t rng, size_t n, size_t t);
BOTAN_PUBLIC_API(2,0) int botan_privkey_create_dh(botan_privkey_t* key, botan_rng_t rng, const char* param);

/*
* Input currently assumed to be PKCS #8 structure;
* Set password to NULL to indicate no encryption expected
*/
BOTAN_PUBLIC_API(2,0) int botan_privkey_load(botan_privkey_t* key, botan_rng_t rng,
                                 const uint8_t bits[], size_t len,
                                 const char* password);

BOTAN_PUBLIC_API(2,0) int botan_privkey_destroy(botan_privkey_t key);

#define BOTAN_PRIVKEY_EXPORT_FLAG_DER 0
#define BOTAN_PRIVKEY_EXPORT_FLAG_PEM 1

/*
* On input *out_len is number of bytes in out[]
* On output *out_len is number of bytes written (or required)
* If out is not big enough no output is written, *out_len is set and 1 is returned
* Returns 0 on success and sets
* If some other error occurs a negative integer is returned.
*/
BOTAN_PUBLIC_API(2,0) int botan_privkey_export(botan_privkey_t key,
                                   uint8_t out[], size_t* out_len,
                                   uint32_t flags);

/*
* Set encryption_algo to NULL or "" to have the library choose a default (recommended)
*/
BOTAN_DEPRECATED("Use botan_privkey_export_encrypted_pbkdf_{msec,iter}")
BOTAN_PUBLIC_API(2,0) int botan_privkey_export_encrypted(botan_privkey_t key,
                                             uint8_t out[], size_t* out_len,
                                             botan_rng_t rng,
                                             const char* passphrase,
                                             const char* encryption_algo,
                                             uint32_t flags);

/*
* Export a private key, running PBKDF for specified amount of time
* @param key the private key to export
*/
BOTAN_PUBLIC_API(2,0) int botan_privkey_export_encrypted_pbkdf_msec(botan_privkey_t key,
                                                        uint8_t out[], size_t* out_len,
                                                        botan_rng_t rng,
                                                        const char* passphrase,
                                                        uint32_t pbkdf_msec_runtime,
                                                        size_t* pbkdf_iterations_out,
                                                        const char* cipher_algo,
                                                        const char* pbkdf_algo,
                                                        uint32_t flags);

/*
* Export a private key using the specified number of iterations.
*/
BOTAN_PUBLIC_API(2,0) int botan_privkey_export_encrypted_pbkdf_iter(botan_privkey_t key,
                                                        uint8_t out[], size_t* out_len,
                                                        botan_rng_t rng,
                                                        const char* passphrase,
                                                        size_t pbkdf_iterations,
                                                        const char* cipher_algo,
                                                        const char* pbkdf_algo,
                                                        uint32_t flags);

typedef struct botan_pubkey_struct* botan_pubkey_t;

BOTAN_PUBLIC_API(2,0) int botan_pubkey_load(botan_pubkey_t* key, const uint8_t bits[], size_t len);

BOTAN_PUBLIC_API(2,0) int botan_privkey_export_pubkey(botan_pubkey_t* out, botan_privkey_t in);

BOTAN_PUBLIC_API(2,0) int botan_pubkey_export(botan_pubkey_t key, uint8_t out[], size_t* out_len, uint32_t flags);

BOTAN_PUBLIC_API(2,0) int botan_pubkey_algo_name(botan_pubkey_t key, char out[], size_t* out_len);

/**
* Returns 0 if key is valid, negative if invalid key or some other error
*/
BOTAN_PUBLIC_API(2,0) int botan_pubkey_check_key(botan_pubkey_t key, botan_rng_t rng, uint32_t flags);

BOTAN_PUBLIC_API(2,0) int botan_pubkey_estimated_strength(botan_pubkey_t key, size_t* estimate);

BOTAN_PUBLIC_API(2,0) int botan_pubkey_fingerprint(botan_pubkey_t key, const char* hash,
                                       uint8_t out[], size_t* out_len);

BOTAN_PUBLIC_API(2,0) int botan_pubkey_destroy(botan_pubkey_t key);

/*
* Get arbitrary named fields from public or privat keys
*/
BOTAN_PUBLIC_API(2,0) int botan_pubkey_get_field(botan_mp_t output,
                                     botan_pubkey_t key,
                                     const char* field_name);

BOTAN_PUBLIC_API(2,0) int botan_privkey_get_field(botan_mp_t output,
                                      botan_privkey_t key,
                                      const char* field_name);

/*
* Algorithm specific key operations: RSA
*/
BOTAN_PUBLIC_API(2,0) int botan_privkey_load_rsa(botan_privkey_t* key,
                                     botan_mp_t p,
                                     botan_mp_t q,
                                     botan_mp_t e);

BOTAN_PUBLIC_API(2,0) int botan_privkey_rsa_get_p(botan_mp_t p, botan_privkey_t rsa_key);
BOTAN_PUBLIC_API(2,0) int botan_privkey_rsa_get_q(botan_mp_t q, botan_privkey_t rsa_key);
BOTAN_PUBLIC_API(2,0) int botan_privkey_rsa_get_d(botan_mp_t d, botan_privkey_t rsa_key);
BOTAN_PUBLIC_API(2,0) int botan_privkey_rsa_get_n(botan_mp_t n, botan_privkey_t rsa_key);
BOTAN_PUBLIC_API(2,0) int botan_privkey_rsa_get_e(botan_mp_t e, botan_privkey_t rsa_key);

BOTAN_PUBLIC_API(2,0) int botan_pubkey_load_rsa(botan_pubkey_t* key,
                                    botan_mp_t n,
                                    botan_mp_t e);

BOTAN_PUBLIC_API(2,0) int botan_pubkey_rsa_get_e(botan_mp_t e, botan_pubkey_t rsa_key);
BOTAN_PUBLIC_API(2,0) int botan_pubkey_rsa_get_n(botan_mp_t n, botan_pubkey_t rsa_key);

/*
* Algorithm specific key operations: DSA
*/
BOTAN_PUBLIC_API(2,0) int botan_privkey_load_dsa(botan_privkey_t* key,
                                     botan_mp_t p,
                                     botan_mp_t q,
                                     botan_mp_t g,
                                     botan_mp_t x);

BOTAN_PUBLIC_API(2,0) int botan_pubkey_load_dsa(botan_pubkey_t* key,
                                    botan_mp_t p,
                                    botan_mp_t q,
                                    botan_mp_t g,
                                    botan_mp_t y);

BOTAN_PUBLIC_API(2,0) int botan_privkey_dsa_get_x(botan_mp_t n, botan_privkey_t key);

BOTAN_PUBLIC_API(2,0) int botan_pubkey_dsa_get_p(botan_mp_t p, botan_pubkey_t key);
BOTAN_PUBLIC_API(2,0) int botan_pubkey_dsa_get_q(botan_mp_t q, botan_pubkey_t key);
BOTAN_PUBLIC_API(2,0) int botan_pubkey_dsa_get_g(botan_mp_t d, botan_pubkey_t key);
BOTAN_PUBLIC_API(2,0) int botan_pubkey_dsa_get_y(botan_mp_t y, botan_pubkey_t key);

/*
* Loads Diffie Hellman private key
*
* @param key variable populated with key material
* @param p prime order of a Z_p group
* @param g group generator
* @param x private key
*
* @pre key is NULL on input
* @post function allocates memory and assigns to `key'
*
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,0) int botan_privkey_load_dh(botan_privkey_t* key,
                                         botan_mp_t p,
                                         botan_mp_t g,
                                         botan_mp_t x);
/*
* Loads Diffie Hellman public key
*
* @param key variable populated with key material
* @param p prime order of a Z_p group
* @param g group generator
* @param y public key
*
* @pre key is NULL on input
* @post function allocates memory and assigns to `key'
*
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,0) int botan_pubkey_load_dh(botan_pubkey_t* key,
                                        botan_mp_t p,
                                        botan_mp_t g,
                                        botan_mp_t y);

/*
* Algorithm specific key operations: ElGamal
*/

/*
* Loads ElGamal public key
* @param key variable populated with key material
* @param p prime order of a Z_p group
* @param g group generator
* @param y public key
*
* @pre key is NULL on input
* @post function allocates memory and assigns to `key'
*
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,0) int botan_pubkey_load_elgamal(botan_pubkey_t* key,
                                        botan_mp_t p,
                                        botan_mp_t g,
                                        botan_mp_t y);

/*
* Loads ElGamal private key
*
* @param key variable populated with key material
* @param p prime order of a Z_p group
* @param g group generator
* @param x private key
*
* @pre key is NULL on input
* @post function allocates memory and assigns to `key'
*
* @return 0 on success, a negative value on failure
*/
BOTAN_PUBLIC_API(2,0) int botan_privkey_load_elgamal(botan_privkey_t* key,
                                         botan_mp_t p,
                                         botan_mp_t g,
                                         botan_mp_t x);

/*
* Algorithm specific key operations: Ed25519
*/

BOTAN_PUBLIC_API(2,2) int botan_privkey_load_ed25519(botan_privkey_t* key,
                                         const uint8_t privkey[32]);

BOTAN_PUBLIC_API(2,2) int botan_pubkey_load_ed25519(botan_pubkey_t* key,
                                        const uint8_t pubkey[32]);

BOTAN_PUBLIC_API(2,2) int botan_privkey_ed25519_get_privkey(botan_privkey_t key,
                                                uint8_t output[64]);

BOTAN_PUBLIC_API(2,2) int botan_pubkey_ed25519_get_pubkey(botan_pubkey_t key,
                                              uint8_t pubkey[32]);

/*
* Algorithm specific key operations: ECDSA and ECDH
*/
BOTAN_PUBLIC_API(2,2)
int botan_privkey_load_ecdsa(botan_privkey_t* key,
                             const botan_mp_t scalar,
                             const char* curve_name);

BOTAN_PUBLIC_API(2,2)
int botan_pubkey_load_ecdsa(botan_pubkey_t* key,
                            const botan_mp_t public_x,
                            const botan_mp_t public_y,
                            const char* curve_name);

BOTAN_PUBLIC_API(2,2)
int botan_pubkey_load_ecdh(botan_pubkey_t* key,
                           const botan_mp_t public_x,
                           const botan_mp_t public_y,
                           const char* curve_name);

BOTAN_PUBLIC_API(2,2)
int botan_privkey_load_ecdh(botan_privkey_t* key,
                            const botan_mp_t scalar,
                            const char* curve_name);

BOTAN_PUBLIC_API(2,2)
int botan_pubkey_load_sm2(botan_pubkey_t* key,
                          const botan_mp_t public_x,
                          const botan_mp_t public_y,
                          const char* curve_name);

BOTAN_PUBLIC_API(2,2)
int botan_privkey_load_sm2(botan_privkey_t* key,
                           const botan_mp_t scalar,
                           const char* curve_name);

BOTAN_PUBLIC_API(2,2)
int botan_pubkey_load_sm2_enc(botan_pubkey_t* key,
                              const botan_mp_t public_x,
                              const botan_mp_t public_y,
                              const char* curve_name);

BOTAN_PUBLIC_API(2,2)
int botan_privkey_load_sm2_enc(botan_privkey_t* key,
                               const botan_mp_t scalar,
                               const char* curve_name);

BOTAN_PUBLIC_API(2,3)
int botan_pubkey_sm2_compute_za(uint8_t out[],
                                size_t* out_len,
                                const char* ident,
                                const char* hash_algo,
                                const botan_pubkey_t key);

/*
* Public Key Encryption
*/
typedef struct botan_pk_op_encrypt_struct* botan_pk_op_encrypt_t;

BOTAN_PUBLIC_API(2,0) int botan_pk_op_encrypt_create(botan_pk_op_encrypt_t* op,
                                         botan_pubkey_t key,
                                         const char* padding,
                                         uint32_t flags);

BOTAN_PUBLIC_API(2,0) int botan_pk_op_encrypt_destroy(botan_pk_op_encrypt_t op);

BOTAN_PUBLIC_API(2,0) int botan_pk_op_encrypt(botan_pk_op_encrypt_t op,
                                  botan_rng_t rng,
                                  uint8_t out[], size_t* out_len,
                                  const uint8_t plaintext[], size_t plaintext_len);

/*
* Public Key Decryption
*/
typedef struct botan_pk_op_decrypt_struct* botan_pk_op_decrypt_t;

BOTAN_PUBLIC_API(2,0) int botan_pk_op_decrypt_create(botan_pk_op_decrypt_t* op,
                                         botan_privkey_t key,
                                         const char* padding,
                                         uint32_t flags);
BOTAN_PUBLIC_API(2,0) int botan_pk_op_decrypt_destroy(botan_pk_op_decrypt_t op);

BOTAN_PUBLIC_API(2,0) int botan_pk_op_decrypt(botan_pk_op_decrypt_t op,
                                  uint8_t out[], size_t* out_len,
                                  const uint8_t ciphertext[], size_t ciphertext_len);

/*
* Signature Generation
*/
typedef struct botan_pk_op_sign_struct* botan_pk_op_sign_t;

BOTAN_PUBLIC_API(2,0)
int botan_pk_op_sign_create(botan_pk_op_sign_t* op,
                            botan_privkey_t key,
                            const char* hash_and_padding,
                            uint32_t flags);

BOTAN_PUBLIC_API(2,0) int botan_pk_op_sign_destroy(botan_pk_op_sign_t op);

BOTAN_PUBLIC_API(2,0) int botan_pk_op_sign_update(botan_pk_op_sign_t op, const uint8_t in[], size_t in_len);

BOTAN_PUBLIC_API(2,0)
int botan_pk_op_sign_finish(botan_pk_op_sign_t op, botan_rng_t rng,
                            uint8_t sig[], size_t* sig_len);

/*
* Signature Verification
*/
typedef struct botan_pk_op_verify_struct* botan_pk_op_verify_t;

BOTAN_PUBLIC_API(2,0)
int botan_pk_op_verify_create(botan_pk_op_verify_t* op,
                              botan_pubkey_t key,
                              const char* hash_and_padding,
                              uint32_t flags);

BOTAN_PUBLIC_API(2,0) int botan_pk_op_verify_destroy(botan_pk_op_verify_t op);

BOTAN_PUBLIC_API(2,0) int botan_pk_op_verify_update(botan_pk_op_verify_t op, const uint8_t in[], size_t in_len);
BOTAN_PUBLIC_API(2,0) int botan_pk_op_verify_finish(botan_pk_op_verify_t op, const uint8_t sig[], size_t sig_len);

/*
* Key Agreement
*/
typedef struct botan_pk_op_ka_struct* botan_pk_op_ka_t;

BOTAN_PUBLIC_API(2,0)
int botan_pk_op_key_agreement_create(botan_pk_op_ka_t* op,
                                     botan_privkey_t key,
                                     const char* kdf,
                                     uint32_t flags);

BOTAN_PUBLIC_API(2,0) int botan_pk_op_key_agreement_destroy(botan_pk_op_ka_t op);

BOTAN_PUBLIC_API(2,0) int botan_pk_op_key_agreement_export_public(botan_privkey_t key,
                                                      uint8_t out[], size_t* out_len);

BOTAN_PUBLIC_API(2,0)
int botan_pk_op_key_agreement(botan_pk_op_ka_t op,
                              uint8_t out[], size_t* out_len,
                              const uint8_t other_key[], size_t other_key_len,
                              const uint8_t salt[], size_t salt_len);

BOTAN_PUBLIC_API(2,0) int botan_pkcs_hash_id(const char* hash_name, uint8_t pkcs_id[], size_t* pkcs_id_len);


/*
*
* @param mce_key must be a McEliece key
* ct_len should be pt_len + n/8 + a few?
*/
BOTAN_PUBLIC_API(2,0)
int botan_mceies_encrypt(botan_pubkey_t mce_key,
                         botan_rng_t rng,
                         const char* aead,
                         const uint8_t pt[], size_t pt_len,
                         const uint8_t ad[], size_t ad_len,
                         uint8_t ct[], size_t* ct_len);

BOTAN_PUBLIC_API(2,0)
int botan_mceies_decrypt(botan_privkey_t mce_key,
                         const char* aead,
                         const uint8_t ct[], size_t ct_len,
                         const uint8_t ad[], size_t ad_len,
                         uint8_t pt[], size_t* pt_len);

typedef struct botan_x509_cert_struct* botan_x509_cert_t;
BOTAN_PUBLIC_API(2,0) int botan_x509_cert_load(botan_x509_cert_t* cert_obj, const uint8_t cert[], size_t cert_len);
BOTAN_PUBLIC_API(2,0) int botan_x509_cert_load_file(botan_x509_cert_t* cert_obj, const char* filename);
BOTAN_PUBLIC_API(2,0) int botan_x509_cert_destroy(botan_x509_cert_t cert);

BOTAN_PUBLIC_API(2,0)
int botan_x509_cert_gen_selfsigned(botan_x509_cert_t* cert,
                                   botan_privkey_t key,
                                   botan_rng_t rng,
                                   const char* common_name,
                                   const char* org_name);

// TODO: return botan_time_struct instead
BOTAN_PUBLIC_API(2,0) int botan_x509_cert_get_time_starts(botan_x509_cert_t cert, char out[], size_t* out_len);
BOTAN_PUBLIC_API(2,0) int botan_x509_cert_get_time_expires(botan_x509_cert_t cert, char out[], size_t* out_len);

BOTAN_PUBLIC_API(2,0) int botan_x509_cert_get_fingerprint(botan_x509_cert_t cert, const char* hash, uint8_t out[], size_t* out_len);

BOTAN_PUBLIC_API(2,0) int botan_x509_cert_get_serial_number(botan_x509_cert_t cert, uint8_t out[], size_t* out_len);
BOTAN_PUBLIC_API(2,0) int botan_x509_cert_get_authority_key_id(botan_x509_cert_t cert, uint8_t out[], size_t* out_len);
BOTAN_PUBLIC_API(2,0) int botan_x509_cert_get_subject_key_id(botan_x509_cert_t cert, uint8_t out[], size_t* out_len);

BOTAN_PUBLIC_API(2,0) int botan_x509_cert_get_public_key_bits(botan_x509_cert_t cert,
                                                  uint8_t out[], size_t* out_len);

BOTAN_PUBLIC_API(2,0) int botan_x509_cert_get_public_key(botan_x509_cert_t cert, botan_pubkey_t* key);

BOTAN_PUBLIC_API(2,0)
int botan_x509_cert_get_issuer_dn(botan_x509_cert_t cert,
                                  const char* key, size_t index,
                                  uint8_t out[], size_t* out_len);

BOTAN_PUBLIC_API(2,0)
int botan_x509_cert_get_subject_dn(botan_x509_cert_t cert,
                                   const char* key, size_t index,
                                   uint8_t out[], size_t* out_len);

BOTAN_PUBLIC_API(2,0) int botan_x509_cert_to_string(botan_x509_cert_t cert, char out[], size_t* out_len);

// Must match values of Key_Constraints in key_constraints.h
enum botan_x509_cert_key_constraints {
   NO_CONSTRAINTS     = 0,
   DIGITAL_SIGNATURE  = 32768,
   NON_REPUDIATION    = 16384,
   KEY_ENCIPHERMENT   = 8192,
   DATA_ENCIPHERMENT  = 4096,
   KEY_AGREEMENT      = 2048,
   KEY_CERT_SIGN      = 1024,
   CRL_SIGN           = 512,
   ENCIPHER_ONLY      = 256,
   DECIPHER_ONLY      = 128
};

BOTAN_PUBLIC_API(2,0) int botan_x509_cert_allowed_usage(botan_x509_cert_t cert, unsigned int key_usage);

/**
 * Key wrapping as per RFC 3394
 */
BOTAN_PUBLIC_API(2,2)
int botan_key_wrap3394(const uint8_t key[], size_t key_len,
                       const uint8_t kek[], size_t kek_len,
                       uint8_t wrapped_key[], size_t *wrapped_key_len);

BOTAN_PUBLIC_API(2,2)
int botan_key_unwrap3394(const uint8_t wrapped_key[], size_t wrapped_key_len,
                         const uint8_t kek[], size_t kek_len,
                         uint8_t key[], size_t *key_len);

/*
* TLS (WIP)
*/
#if defined(BOTAN_HAS_TLS) && 0

typedef struct botan_tls_session_struct* botan_tls_session_t;

BOTAN_TEST_API int botan_tls_session_decrypt(botan_tls_session_t* session,
                                        const uint8_t key[], size_t key_len,
                                        const uint8_t blob[], size_t blob_len);

BOTAN_TEST_API int botan_tls_session_get_version(botan_tls_session_t session, uint16_t* tls_version);
BOTAN_TEST_API int botan_tls_session_get_ciphersuite(botan_tls_session_t session, uint16_t* ciphersuite);
BOTAN_TEST_API int botan_tls_session_encrypt(botan_tls_session_t session, botan_rng_t rng, uint8_t key[], size_t* key_len);

BOTAN_TEST_API int botan_tls_session_get_peer_certs(botan_tls_session_t session, botan_x509_cert_t certs[], size_t* cert_len);

// TODO: peer certs, validation, ...

typedef struct botan_tls_channel_struct* botan_tls_channel_t;

typedef void (*botan_tls_channel_output_fn)(void* application_data, const uint8_t* data, size_t data_len);

typedef void (*botan_tls_channel_data_cb)(void* application_data, const uint8_t* data, size_t data_len);

typedef void (*botan_tls_channel_alert_cb)(void* application_data, uint16_t alert_code);

typedef void (*botan_tls_channel_session_established)(void* application_data,
                                                      botan_tls_channel_t channel,
                                                      botan_tls_session_t session);

BOTAN_TEST_API int botan_tls_channel_init_client(botan_tls_channel_t* channel,
                                            botan_tls_channel_output_fn output_fn,
                                            botan_tls_channel_data_cb data_cb,
                                            botan_tls_channel_alert_cb alert_cb,
                                            botan_tls_channel_session_established session_cb,
                                            const char* server_name);

BOTAN_TEST_API int botan_tls_channel_init_server(botan_tls_channel_t* channel,
                                            botan_tls_channel_output_fn output_fn,
                                            botan_tls_channel_data_cb data_cb,
                                            botan_tls_channel_alert_cb alert_cb,
                                            botan_tls_channel_session_established session_cb);

BOTAN_TEST_API int botan_tls_channel_received_data(botan_tls_channel_t chan,
                                              const uint8_t input[], size_t len);

/**
* Returns 0 for client, 1 for server, negative for error
*/
BOTAN_TEST_API int botan_tls_channel_type(botan_tls_channel_t chan);

BOTAN_TEST_API int botan_tls_channel_send(botan_tls_channel_t chan,
                                     const uint8_t input[], size_t len);

BOTAN_TEST_API int botan_tls_channel_close(botan_tls_channel_t chan);

BOTAN_TEST_API int botan_tls_channel_destroy(botan_tls_channel_t chan);

#endif
#ifdef __cplusplus
}
#endif

#endif
