/*
* Format Preserving Encryption (FE1 scheme)
* (C) 2009 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_FPE_FE1_H__
#define BOTAN_FPE_FE1_H__

#include <botan/bigint.h>
#include <botan/symkey.h>

namespace Botan {

namespace FPE {

/**
* Format Preserving Encryption using the scheme FE1 from the paper
* "Format-Preserving Encryption" by Bellare, Rogaway, et al
* (http://eprint.iacr.org/2009/251)
*
* Encrypt X from and onto the group Z_n using key and tweak
* @param n the modulus
* @param X the plaintext as a BigInt
* @param key a random key
* @param tweak will modify the ciphertext (think of as an IV)
*/
BigInt BOTAN_DLL fe1_encrypt(const BigInt& n, const BigInt& X,
                             const SymmetricKey& key,
                             const std::vector<byte>& tweak);

/**
* Decrypt X from and onto the group Z_n using key and tweak
* @param n the modulus
* @param X the ciphertext as a BigInt
* @param key is the key used for encryption
* @param tweak the same tweak used for encryption
*/
BigInt BOTAN_DLL fe1_decrypt(const BigInt& n, const BigInt& X,
                             const SymmetricKey& key,
                             const std::vector<byte>& tweak);

}

}

#endif
