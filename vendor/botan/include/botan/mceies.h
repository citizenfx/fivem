/*
* McEliece Integrated Encryption System
* (C) 2014,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_MCEIES_H__
#define BOTAN_MCEIES_H__

#include <botan/secmem.h>
#include <botan/rng.h>

namespace Botan {

class McEliece_PublicKey;
class McEliece_PrivateKey;

/**
* McEliece Integrated Encryption System
* Derive a shared key using MCE KEM and encrypt/authenticate the
* plaintext and AD using AES-256 in OCB mode.
*/
secure_vector<uint8_t>
BOTAN_DLL mceies_encrypt(const McEliece_PublicKey& pubkey,
                         const uint8_t pt[], size_t pt_len,
                         const uint8_t ad[], size_t ad_len,
                         RandomNumberGenerator& rng,
                         const std::string& aead = "AES-256/OCB");

/**
* McEliece Integrated Encryption System
* Derive a shared key using MCE KEM and decrypt/authenticate the
* ciphertext and AD using AES-256 in OCB mode.
*/
secure_vector<uint8_t>
BOTAN_DLL mceies_decrypt(const McEliece_PrivateKey& privkey,
                         const uint8_t ct[], size_t ct_len,
                         const uint8_t ad[], size_t ad_len,
                         const std::string& aead = "AES-256/OCB");


}

#endif
