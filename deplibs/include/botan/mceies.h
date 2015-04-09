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
secure_vector<byte>
BOTAN_DLL mceies_encrypt(const McEliece_PublicKey& pubkey,
                         const secure_vector<byte>& pt,
                         byte ad[], size_t ad_len,
                         RandomNumberGenerator& rng);

/**
* McEliece Integrated Encryption System
* Derive a shared key using MCE KEM and decrypt/authenticate the
* ciphertext and AD using AES-256 in OCB mode.
*/
secure_vector<byte>
BOTAN_DLL mceies_decrypt(const McEliece_PrivateKey& privkey,
                         const secure_vector<byte>& ct,
                         byte ad[], size_t ad_len);


}

#endif
