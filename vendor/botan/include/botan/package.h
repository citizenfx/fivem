/*
* Rivest's Package Tranform
* (C) 2009 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_AONT_PACKAGE_TRANSFORM_H__
#define BOTAN_AONT_PACKAGE_TRANSFORM_H__

#include <botan/block_cipher.h>
#include <botan/rng.h>

namespace Botan {

/**
* Rivest's Package Tranform
* @param rng the random number generator to use
* @param cipher the block cipher to use (aont_package takes ownership)
* @param input the input data buffer
* @param input_len the length of the input data in bytes
* @param output the output data buffer (must be at least
*        input_len + cipher->BLOCK_SIZE bytes long)
*/
void BOTAN_DLL aont_package(RandomNumberGenerator& rng,
                            BlockCipher* cipher,
                            const uint8_t input[], size_t input_len,
                            uint8_t output[]);

/**
* Rivest's Package Tranform (Inversion)
* @param cipher the block cipher to use (aont_package takes ownership)
* @param input the input data buffer
* @param input_len the length of the input data in bytes
* @param output the output data buffer (must be at least
*        input_len - cipher->BLOCK_SIZE bytes long)
*/
void BOTAN_DLL aont_unpackage(BlockCipher* cipher,
                              const uint8_t input[], size_t input_len,
                              uint8_t output[]);

}

#endif
