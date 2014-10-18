/*
* Cryptobox Message Routines
* (C) 2009,2013 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_CRYPTOBOX_PSK_H__
#define BOTAN_CRYPTOBOX_PSK_H__

#include <string>
#include <botan/rng.h>
#include <botan/symkey.h>

namespace Botan {

/**
* This namespace holds various high-level crypto functions
*/
namespace CryptoBox {

/**
* Encrypt a message using a shared secret key
* @param input the input data
* @param input_len the length of input in bytes
* @param key the key used to encrypt the message
* @param rng a ref to a random number generator, such as AutoSeeded_RNG
*/
BOTAN_DLL std::vector<byte> encrypt(const byte input[], size_t input_len,
                                    const SymmetricKey& key,
                                    RandomNumberGenerator& rng);

/**
* Encrypt a message using a shared secret key
* @param input the input data
* @param input_len the length of input in bytes
* @param key the key used to encrypt the message
* @param rng a ref to a random number generator, such as AutoSeeded_RNG
*/
BOTAN_DLL secure_vector<byte> decrypt(const byte input[], size_t input_len,
                                      const SymmetricKey& key);

}

}

#endif
