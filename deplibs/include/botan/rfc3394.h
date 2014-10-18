/*
* AES Key Wrap (RFC 3394)
* (C) 2011 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_AES_KEY_WRAP_H__
#define BOTAN_AES_KEY_WRAP_H__

#include <botan/symkey.h>

namespace Botan {

class Algorithm_Factory;

/**
* Encrypt a key under a key encryption key using the algorithm
* described in RFC 3394
*
* @param key the plaintext key to encrypt
* @param kek the key encryption key
* @param af an algorithm factory
* @return key encrypted under kek
*/
secure_vector<byte> BOTAN_DLL rfc3394_keywrap(const secure_vector<byte>& key,
                                             const SymmetricKey& kek,
                                             Algorithm_Factory& af);

/**
* Decrypt a key under a key encryption key using the algorithm
* described in RFC 3394
*
* @param key the encrypted key to decrypt
* @param kek the key encryption key
* @param af an algorithm factory
* @return key decrypted under kek
*/
secure_vector<byte> BOTAN_DLL rfc3394_keyunwrap(const secure_vector<byte>& key,
                                               const SymmetricKey& kek,
                                               Algorithm_Factory& af);

}

#endif
