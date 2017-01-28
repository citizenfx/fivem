/*
* Cryptobox Message Routines
* (C) 2009 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_CRYPTOBOX_H__
#define BOTAN_CRYPTOBOX_H__

#include <string>
#include <botan/rng.h>
#include <botan/symkey.h>

namespace Botan {

/**
* This namespace holds various high-level crypto functions
*/
namespace CryptoBox {

/**
* Encrypt a message using a passphrase
* @param input the input data
* @param input_len the length of input in bytes
* @param passphrase the passphrase used to encrypt the message
* @param rng a ref to a random number generator, such as AutoSeeded_RNG
*/
BOTAN_DLL std::string encrypt(const uint8_t input[], size_t input_len,
                              const std::string& passphrase,
                              RandomNumberGenerator& rng);


/**
* Decrypt a message encrypted with CryptoBox::encrypt
* @param input the input data
* @param input_len the length of input in bytes
* @param passphrase the passphrase used to encrypt the message
*/
BOTAN_DLL std::string decrypt(const uint8_t input[], size_t input_len,
                              const std::string& passphrase);

/**
* Decrypt a message encrypted with CryptoBox::encrypt
* @param input the input data
* @param passphrase the passphrase used to encrypt the message
*/
BOTAN_DLL std::string decrypt(const std::string& input,
                              const std::string& passphrase);

}

}

#endif
