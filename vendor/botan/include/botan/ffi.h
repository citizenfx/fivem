/*
* FFI (C89 API)
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_FFI_H__
#define BOTAN_FFI_H__

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

/*
* Versioning
*/
BOTAN_DLL uint32_t botan_ffi_api_version();

/*
* Return 0 (ok) if the version given is one this library supports.
* botan_ffi_supports_api(botan_ffi_api_version()) will always return 0.
*/
BOTAN_DLL int botan_ffi_supports_api(uint32_t api_version);

BOTAN_DLL const char* botan_version_string();
BOTAN_DLL uint32_t botan_version_major();
BOTAN_DLL uint32_t botan_version_minor();
BOTAN_DLL uint32_t botan_version_patch();
BOTAN_DLL uint32_t botan_version_datestamp();

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
#define BOTAN_FFI_ERROR_INSUFFICIENT_BUFFER_SPACE (-10)
#define BOTAN_FFI_ERROR_EXCEPTION_THROWN (-20)
#define BOTAN_FFI_ERROR_BAD_FLAG (-30)
#define BOTAN_FFI_ERROR_NULL_POINTER (-31)
#define BOTAN_FFI_ERROR_NOT_IMPLEMENTED (-40)

//const char* botan_error_description(int err);

/*
* Returns 0 if x[0..len] == y[0..len], or otherwise -1
*/
BOTAN_DLL int botan_same_mem(const uint8_t* x, const uint8_t* y, size_t len);

#define BOTAN_FFI_HEX_LOWER_CASE 1

BOTAN_DLL int botan_hex_encode(const uint8_t* x, size_t len, char* out, uint32_t flags);
// TODO: botan_hex_decode
// TODO: botan_base64_encode
// TODO: botan_base64_decode

/*
* RNG
*/
typedef struct botan_rng_struct* botan_rng_t;

/**
* TODO: replace rng_type with simple flags? 
*/
BOTAN_DLL int botan_rng_init(botan_rng_t* rng, const char* rng_type);

/**
* TODO: better name
*/
BOTAN_DLL int botan_rng_get(botan_rng_t rng, uint8_t* out, size_t out_len);
BOTAN_DLL int botan_rng_reseed(botan_rng_t rng, size_t bits);
BOTAN_DLL int botan_rng_destroy(botan_rng_t rng);

/*
* Hashing
*/
typedef struct botan_hash_struct* botan_hash_t;

/**
* Initialize a hash object:
*   botan_hash_t hash
*   botan_hash_init(&hash, "SHA-384", 0);
*
* Flags should be 0 in current API revision, all other uses are reserved.
*  and return BOTAN_FFI_ERROR_BAD_FLAG.

* TODO: since output_length is effectively required to use this API,
* return it from init as an output parameter
*/
BOTAN_DLL int botan_hash_init(botan_hash_t* hash, const char* hash_name, uint32_t flags);

/**
* Writes the output length of the hash object to *output_length
*/
BOTAN_DLL int botan_hash_output_length(botan_hash_t hash, size_t* output_length);

/**
* Send more input to the hash function.
*/
BOTAN_DLL int botan_hash_update(botan_hash_t hash, const uint8_t* in, size_t in_len);

/**
* Finalizes the hash computation and writes the output to
* out[0:botan_hash_output_length()] then reinitializes for computing
* another digest as if botan_hash_clear had been called.
*/
BOTAN_DLL int botan_hash_final(botan_hash_t hash, uint8_t out[]);

/**
* Reinitializes the state of the hash computation. A hash can
* be computed (with update/final) immediately.
*/
BOTAN_DLL int botan_hash_clear(botan_hash_t hash);

/**
* Frees all resources of this object
*/
BOTAN_DLL int botan_hash_destroy(botan_hash_t hash);

BOTAN_DLL int botan_hash_name(botan_hash_t hash, char* name, size_t name_len);

/*
* Message Authentication
*/
typedef struct botan_mac_struct* botan_mac_t;

BOTAN_DLL int botan_mac_init(botan_mac_t* mac, const char* mac_name, uint32_t flags);
BOTAN_DLL int botan_mac_output_length(botan_mac_t mac, size_t* output_length);
BOTAN_DLL int botan_mac_set_key(botan_mac_t mac, const uint8_t* key, size_t key_len);
BOTAN_DLL int botan_mac_update(botan_mac_t mac, const uint8_t* buf, size_t len);
BOTAN_DLL int botan_mac_final(botan_mac_t mac, uint8_t out[]);
BOTAN_DLL int botan_mac_clear(botan_mac_t hash);
BOTAN_DLL int botan_mac_destroy(botan_mac_t mac);

/*
* Cipher modes
*/
typedef struct botan_cipher_struct* botan_cipher_t;

#define BOTAN_CIPHER_INIT_FLAG_MASK_DIRECTION 1
#define BOTAN_CIPHER_INIT_FLAG_ENCRYPT 0
#define BOTAN_CIPHER_INIT_FLAG_DECRYPT 1

BOTAN_DLL int botan_cipher_init(botan_cipher_t* cipher, const char* name, uint32_t flags);

BOTAN_DLL int botan_cipher_valid_nonce_length(botan_cipher_t cipher, size_t nl);
BOTAN_DLL int botan_cipher_get_tag_length(botan_cipher_t cipher, size_t* tag_size);
BOTAN_DLL int botan_cipher_get_default_nonce_length(botan_cipher_t cipher, size_t* nl);
BOTAN_DLL int botan_cipher_get_update_granularity(botan_cipher_t cipher, size_t* ug);

BOTAN_DLL int botan_cipher_query_keylen(botan_cipher_t,
                                        size_t* out_minimum_keylength,
                                        size_t* out_maximum_keylength);

BOTAN_DLL int botan_cipher_set_key(botan_cipher_t cipher,
                                   const uint8_t* key, size_t key_len);

BOTAN_DLL int botan_cipher_set_associated_data(botan_cipher_t cipher,
                                               const uint8_t* ad, size_t ad_len);

BOTAN_DLL int botan_cipher_start(botan_cipher_t cipher,
                                 const uint8_t* nonce, size_t nonce_len);

#define BOTAN_CIPHER_UPDATE_FLAG_FINAL (1U << 0)

BOTAN_DLL int botan_cipher_update(botan_cipher_t cipher,
                                  uint32_t flags,
                                  uint8_t output[],
                                  size_t output_size,
                                  size_t* output_written,
                                  const uint8_t input_bytes[],
                                  size_t input_size,
                                  size_t* input_consumed);

BOTAN_DLL int botan_cipher_clear(botan_cipher_t hash);
BOTAN_DLL int botan_cipher_destroy(botan_cipher_t cipher);

/*
* PBKDF
*/
BOTAN_DLL int botan_pbkdf(const char* pbkdf_algo,
                          uint8_t out[], size_t out_len,
                          const char* password,
                          const uint8_t salt[], size_t salt_len,
                          size_t iterations);

BOTAN_DLL int botan_pbkdf_timed(const char* pbkdf_algo,
                                uint8_t out[], size_t out_len,
                                const char* password,
                                const uint8_t salt[], size_t salt_len,
                                size_t milliseconds_to_run,
                                size_t* out_iterations_used);

/*
* KDF
*/
BOTAN_DLL int botan_kdf(const char* kdf_algo,
                        uint8_t out[], size_t out_len,
                        const uint8_t secret[], size_t secret_len,
                        const uint8_t salt[], size_t salt_len,
                        const uint8_t label[], size_t label_len);

/*
* Bcrypt
* *out_len should be 64 bytes
* Output is formatted bcrypt $2a$...
*/
BOTAN_DLL int botan_bcrypt_generate(uint8_t* out, size_t* out_len,
                                    const char* password,
                                    botan_rng_t rng,
                                    size_t work_factor,
                                    uint32_t flags);

/**
* Returns 0 if if this password/hash combination is valid
* Returns 1 if the combination is not valid (but otherwise well formed)
* Returns negative on error
*/
BOTAN_DLL int botan_bcrypt_is_valid(const char* pass, const char* hash);

/*
* Public/private key creation, import, ...
*/
typedef struct botan_privkey_struct* botan_privkey_t;

BOTAN_DLL int botan_privkey_create(botan_privkey_t* key,
                                   const char* algo_name,
                                   const char* algo_params,
                                   botan_rng_t rng);

BOTAN_DLL int botan_privkey_create_rsa(botan_privkey_t* key, botan_rng_t rng, size_t n_bits);
BOTAN_DLL int botan_privkey_create_ecdsa(botan_privkey_t* key, botan_rng_t rng, const char* params);
BOTAN_DLL int botan_privkey_create_ecdh(botan_privkey_t* key, botan_rng_t rng, const char* params);
BOTAN_DLL int botan_privkey_create_mceliece(botan_privkey_t* key, botan_rng_t rng, size_t n, size_t t);


/*
* Input currently assumed to be PKCS #8 structure;
* Set password to NULL to indicate no encryption expected
*/
BOTAN_DLL int botan_privkey_load(botan_privkey_t* key, botan_rng_t rng,
                                 const uint8_t bits[], size_t len,
                                 const char* password);

BOTAN_DLL int botan_privkey_destroy(botan_privkey_t key);

#define BOTAN_PRIVKEY_EXPORT_FLAG_DER 0
#define BOTAN_PRIVKEY_EXPORT_FLAG_PEM 1

/*
* On input *out_len is number of bytes in out[]
* On output *out_len is number of bytes written (or required)
* If out is not big enough no output is written, *out_len is set and 1 is returned
* Returns 0 on success and sets
* If some other error occurs a negative integer is returned.
*/
BOTAN_DLL int botan_privkey_export(botan_privkey_t key,
                                   uint8_t out[], size_t* out_len,
                                   uint32_t flags);

/*
* Set encryption_algo to NULL or "" to have the library choose a default (recommended)
*/
BOTAN_DLL int botan_privkey_export_encrypted(botan_privkey_t key,
                                             uint8_t out[], size_t* out_len,
                                             botan_rng_t rng,
                                             const char* passphrase,
                                             const char* encryption_algo,
                                             uint32_t flags);

typedef struct botan_pubkey_struct* botan_pubkey_t;

BOTAN_DLL int botan_pubkey_load(botan_pubkey_t* key, const uint8_t bits[], size_t len);

BOTAN_DLL int botan_privkey_export_pubkey(botan_pubkey_t* out, botan_privkey_t in);

BOTAN_DLL int botan_pubkey_export(botan_pubkey_t key, uint8_t out[], size_t* out_len, uint32_t flags);

BOTAN_DLL int botan_pubkey_algo_name(botan_pubkey_t key, char out[], size_t* out_len);

BOTAN_DLL int botan_pubkey_estimated_strength(botan_pubkey_t key, size_t* estimate);

BOTAN_DLL int botan_pubkey_fingerprint(botan_pubkey_t key, const char* hash,
                                       uint8_t out[], size_t* out_len);

BOTAN_DLL int botan_pubkey_destroy(botan_pubkey_t key);


/*
* Public Key Encryption
*/
typedef struct botan_pk_op_encrypt_struct* botan_pk_op_encrypt_t;

BOTAN_DLL int botan_pk_op_encrypt_create(botan_pk_op_encrypt_t* op,
                                         botan_pubkey_t key,
                                         const char* padding,
                                         uint32_t flags);

BOTAN_DLL int botan_pk_op_encrypt_destroy(botan_pk_op_encrypt_t op);

BOTAN_DLL int botan_pk_op_encrypt(botan_pk_op_encrypt_t op,
                                  botan_rng_t rng,
                                  uint8_t out[], size_t* out_len,
                                  const uint8_t plaintext[], size_t plaintext_len);

/*
* Public Key Decryption
*/
typedef struct botan_pk_op_decrypt_struct* botan_pk_op_decrypt_t;

BOTAN_DLL int botan_pk_op_decrypt_create(botan_pk_op_decrypt_t* op,
                                         botan_privkey_t key,
                                         const char* padding,
                                         uint32_t flags);
BOTAN_DLL int botan_pk_op_decrypt_destroy(botan_pk_op_decrypt_t op);

BOTAN_DLL int botan_pk_op_decrypt(botan_pk_op_decrypt_t op,
                                  uint8_t out[], size_t* out_len,
                                  uint8_t ciphertext[], size_t ciphertext_len);

/*
* Signature Generation
*/
typedef struct botan_pk_op_sign_struct* botan_pk_op_sign_t;

BOTAN_DLL int botan_pk_op_sign_create(botan_pk_op_sign_t* op,
                                      botan_privkey_t key,
                                      const char* hash_and_padding,
                                      uint32_t flags);
BOTAN_DLL int botan_pk_op_sign_destroy(botan_pk_op_sign_t op);

BOTAN_DLL int botan_pk_op_sign_update(botan_pk_op_sign_t op, const uint8_t in[], size_t in_len);
BOTAN_DLL int botan_pk_op_sign_finish(botan_pk_op_sign_t op, botan_rng_t rng,
                                      uint8_t sig[], size_t* sig_len);

/*
* Signature Verification
*/
typedef struct botan_pk_op_verify_struct* botan_pk_op_verify_t;

BOTAN_DLL int botan_pk_op_verify_create(botan_pk_op_verify_t* op,
                                        botan_pubkey_t key,
                                        const char* hash_and_padding,
                                        uint32_t flags);
BOTAN_DLL int botan_pk_op_verify_destroy(botan_pk_op_verify_t op);

BOTAN_DLL int botan_pk_op_verify_update(botan_pk_op_verify_t op, const uint8_t in[], size_t in_len);
BOTAN_DLL int botan_pk_op_verify_finish(botan_pk_op_verify_t op, const uint8_t sig[], size_t sig_len);

/*
* Key Agreement
*/
typedef struct botan_pk_op_ka_struct* botan_pk_op_ka_t;

BOTAN_DLL int botan_pk_op_key_agreement_create(botan_pk_op_ka_t* op,
                                               botan_privkey_t key,
                                               const char* kdf,
                                               uint32_t flags);
BOTAN_DLL int botan_pk_op_key_agreement_destroy(botan_pk_op_ka_t op);

BOTAN_DLL int botan_pk_op_key_agreement_export_public(botan_privkey_t key,
                                                      uint8_t out[], size_t* out_len);

BOTAN_DLL int botan_pk_op_key_agreement(botan_pk_op_ka_t op,
                                        uint8_t out[], size_t* out_len,
                                        const uint8_t other_key[], size_t other_key_len,
                                        const uint8_t salt[], size_t salt_len);


/*
*
* @param mce_key must be a McEliece key
* ct_len should be pt_len + n/8 + a few?
*/
BOTAN_DLL int botan_mceies_encrypt(botan_pubkey_t mce_key,
                                   botan_rng_t rng,
                                   const char* aead,
                                   const uint8_t pt[], size_t pt_len,
                                   const uint8_t ad[], size_t ad_len,
                                   uint8_t ct[], size_t* ct_len);

BOTAN_DLL int botan_mceies_decrypt(botan_privkey_t mce_key,
                                   const char* aead,
                                   const uint8_t ct[], size_t ct_len,
                                   const uint8_t ad[], size_t ad_len,
                                   uint8_t pt[], size_t* pt_len);



typedef struct botan_x509_cert_struct* botan_x509_cert_t;
BOTAN_DLL int botan_x509_cert_load(botan_x509_cert_t* cert_obj, const uint8_t cert[], size_t cert_len);
BOTAN_DLL int botan_x509_cert_load_file(botan_x509_cert_t* cert_obj, const char* filename);
BOTAN_DLL int botan_x509_cert_destroy(botan_x509_cert_t cert);

BOTAN_DLL int botan_x509_cert_gen_selfsigned(botan_x509_cert_t* cert,
                                             botan_privkey_t key,
                                             botan_rng_t rng,
                                             const char* common_name,
                                             const char* org_name);

// TODO: return botan_time_struct instead
BOTAN_DLL int botan_x509_cert_get_time_starts(botan_x509_cert_t cert, char out[], size_t* out_len);
BOTAN_DLL int botan_x509_cert_get_time_expires(botan_x509_cert_t cert, char out[], size_t* out_len);

BOTAN_DLL int botan_x509_cert_get_fingerprint(botan_x509_cert_t cert, const char* hash, uint8_t out[], size_t* out_len);

BOTAN_DLL int botan_x509_cert_get_serial_number(botan_x509_cert_t cert, uint8_t out[], size_t* out_len);
BOTAN_DLL int botan_x509_cert_get_authority_key_id(botan_x509_cert_t cert, uint8_t out[], size_t* out_len);
BOTAN_DLL int botan_x509_cert_get_subject_key_id(botan_x509_cert_t cert, uint8_t out[], size_t* out_len);

BOTAN_DLL int botan_x509_cert_path_verify(botan_x509_cert_t cert,
                                          const char* ca_dir);

BOTAN_DLL int botan_x509_cert_get_public_key_bits(botan_x509_cert_t cert,
                                                  uint8_t out[], size_t* out_len);

BOTAN_DLL int botan_x509_cert_get_public_key(botan_x509_cert_t cert, botan_pubkey_t* key);

BOTAN_DLL int botan_x509_cert_get_issuer_dn(botan_x509_cert_t cert,
                                            const char* key, size_t index,
                                            uint8_t out[], size_t* out_len);

BOTAN_DLL int botan_x509_cert_get_subject_dn(botan_x509_cert_t cert,
                                             const char* key, size_t index,
                                             uint8_t out[], size_t* out_len);

BOTAN_DLL int botan_x509_cert_to_string(botan_x509_cert_t cert, char out[], size_t* out_len);

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

BOTAN_DLL int botan_x509_cert_allowed_usage(botan_x509_cert_t cert, unsigned int key_usage);

/*
* TLS (WIP)
*/
#if defined(BOTAN_HAS_TLS) && 0

typedef struct botan_tls_session_struct* botan_tls_session_t;

BOTAN_DLL int botan_tls_session_decrypt(botan_tls_session_t* session,
                                        const uint8_t key[], size_t key_len,
                                        const uint8_t blob[], size_t blob_len);

BOTAN_DLL int botan_tls_session_get_version(botan_tls_session_t session, uint16_t* tls_version);
BOTAN_DLL int botan_tls_session_get_ciphersuite(botan_tls_session_t session, uint16_t* ciphersuite);
BOTAN_DLL int botan_tls_session_encrypt(botan_tls_session_t session, botan_rng_t rng, uint8_t key[], size_t* key_len);

BOTAN_DLL int botan_tls_session_get_peer_certs(botan_tls_session_t session, botan_x509_cert_t certs[], size_t* cert_len);

// TODO: peer certs, validation, ...

typedef struct botan_tls_channel_struct* botan_tls_channel_t;

typedef void (*botan_tls_channel_output_fn)(void* application_data, const uint8_t* data, size_t data_len);

typedef void (*botan_tls_channel_data_cb)(void* application_data, const uint8_t* data, size_t data_len);

typedef void (*botan_tls_channel_alert_cb)(void* application_data, uint16_t alert_code);

typedef void (*botan_tls_channel_session_established)(void* application_data,
                                                      botan_tls_channel_t channel,
                                                      botan_tls_session_t session);

BOTAN_DLL int botan_tls_channel_init_client(botan_tls_channel_t* channel,
                                            botan_tls_channel_output_fn output_fn,
                                            botan_tls_channel_data_cb data_cb,
                                            botan_tls_channel_alert_cb alert_cb,
                                            botan_tls_channel_session_established session_cb,
                                            const char* server_name);

BOTAN_DLL int botan_tls_channel_init_server(botan_tls_channel_t* channel,
                                            botan_tls_channel_output_fn output_fn,
                                            botan_tls_channel_data_cb data_cb,
                                            botan_tls_channel_alert_cb alert_cb,
                                            botan_tls_channel_session_established session_cb);

BOTAN_DLL int botan_tls_channel_received_data(botan_tls_channel_t chan,
                                              const uint8_t input[], size_t len);

/**
* Returns 0 for client, 1 for server, negative for error
*/
BOTAN_DLL int botan_tls_channel_type(botan_tls_channel_t chan);

BOTAN_DLL int botan_tls_channel_send(botan_tls_channel_t chan,
                                     const uint8_t input[], size_t len);

BOTAN_DLL int botan_tls_channel_close(botan_tls_channel_t chan);

BOTAN_DLL int botan_tls_channel_destroy(botan_tls_channel_t chan);

#endif
#ifdef __cplusplus
}
#endif

#endif
