/*
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_FFI_H__
#define BOTAN_FFI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <botan/build.h>
#include <stdint.h>
#include <stddef.h>

/*
* TODO:
* - Better error reporting
* - User callback for exception logging
* - Doxygen comments for all functions/params
* - X.509 certs and PKIX path validation goo
* - TLS
*/

/*
* Versioning
*/
BOTAN_DLL uint32_t botan_ffi_api_version();

BOTAN_DLL const char* botan_version_string();
BOTAN_DLL uint32_t botan_version_major();
BOTAN_DLL uint32_t botan_version_minor();
BOTAN_DLL uint32_t botan_version_patch();
BOTAN_DLL uint32_t botan_version_datestamp();

/*
* Error handling
*/
#define BOTAN_FFI_ERROR_EXCEPTION_THROWN (-20)
#define BOTAN_FFI_ERROR_BAD_FLAG (-30)
#define BOTAN_FFI_ERROR_NULL_POINTER (-31)
#define BOTAN_FFI_ERROR_NULL_POINTER (-31)

//const char* botan_error_description(int err);

/*
* Utility
*/
BOTAN_DLL int botan_same_mem(const uint8_t* x, const uint8_t* y, size_t len);

/*
* RNG
*/
typedef struct botan_rng_struct* botan_rng_t;

BOTAN_DLL int botan_rng_init(botan_rng_t* rng, const char* rng_type);
BOTAN_DLL int botan_rng_get(botan_rng_t rng, uint8_t* out, size_t out_len);
BOTAN_DLL int botan_rng_reseed(botan_rng_t rng, size_t bits);
BOTAN_DLL int botan_rng_destroy(botan_rng_t rng);

/*
* Hashing
*/
typedef struct botan_hash_struct* botan_hash_t;

BOTAN_DLL int botan_hash_init(botan_hash_t* hash, const char* hash_name, uint32_t flags);
BOTAN_DLL int botan_hash_output_length(botan_hash_t hash, size_t* output_length);
BOTAN_DLL int botan_hash_update(botan_hash_t hash, const uint8_t* in, size_t in_len);
BOTAN_DLL int botan_hash_final(botan_hash_t hash, uint8_t out[]);
BOTAN_DLL int botan_hash_clear(botan_hash_t hash);
BOTAN_DLL int botan_hash_destroy(botan_hash_t hash);

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
                        const uint8_t salt[], size_t salt_len);

/*
* Bcrypt
*/
#if defined(BOTAN_HAS_BCRYPT)

BOTAN_DLL int botan_bcrypt_generate(uint8_t* out, size_t* out_len,
                                    const char* pass,
                                    botan_rng_t rng,
                                    size_t work_factor,
                                    uint32_t flags);

/**
* Returns 0 if if this password/hash combination is valid
* Returns 1 if the combination is not valid (but otherwise well formed)
* Returns negative on error
*/
BOTAN_DLL int botan_bcrypt_is_valid(const char* pass, const char* hash);

#endif

/*
* Public/private key creation, import, ...
*/
typedef struct botan_privkey_struct* botan_privkey_t;

BOTAN_DLL int botan_privkey_create_rsa(botan_privkey_t* key, botan_rng_t rng, size_t n_bits);
//BOTAN_DLL int botan_privkey_create_dsa(botan_privkey_t* key, botan_rng_t rng, size_t p_bits, size_t q_bits);
//BOTAN_DLL int botan_privkey_create_dh(botan_privkey_t* key, botan_rng_t rng, size_t p_bits);
BOTAN_DLL int botan_privkey_create_ecdsa(botan_privkey_t* key, botan_rng_t rng, const char* params);
BOTAN_DLL int botan_privkey_create_ecdh(botan_privkey_t* key, botan_rng_t rng, const char* params);
//BOTAN_DLL int botan_privkey_create_mceliece(botan_privkey_t* key, botan_rng_t rng, size_t n, size_t t);

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

BOTAN_DLL int botan_pubkey_destroy(botan_privkey_t key);


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
* TLS (WIP)
*/
#if defined(BOTAN_HAS_TLS) && 0

typedef struct botan_tls_session_struct* botan_tls_session_t;

BOTAN_DLL int botan_tls_session_get_version(botan_tls_session_t* session, uint16_t* tls_version);
BOTAN_DLL int botan_tls_session_get_ciphersuite(botan_tls_session_t* session, uint16_t* ciphersuite);
// TODO: peer certs, validation, ...

typedef struct botan_tls_channel_struct* botan_tls_channel_t;

typedef void (*botan_tls_channel_output_fn)(void*, const uint8_t*, size_t);
typedef void (*botan_tls_channel_data_cb)(void*, const uint8_t*, size_t);
typedef void (*botan_tls_channel_alert_cb)(void*, uint16_t, const char*);
typedef void (*botan_tls_channel_session_established)(void*, botan_tls_session_t);

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

BOTAN_DLL int botan_tls_channel_send(botan_tls_channel_t chan,
                                     const uint8_t input[], size_t len);

BOTAN_DLL int botan_tls_channel_close(botan_tls_channel_t chan);

BOTAN_DLL int botan_tls_channel_destroy(botan_tls_channel_t chan);

#endif

#ifdef __cplusplus
}
#endif

#endif
