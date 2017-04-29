/*
* PKCS #5 v2.0 PBE
* (C) 1999-2007,2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_PBE_PKCS_v20_H__
#define BOTAN_PBE_PKCS_v20_H__

#include <botan/rng.h>
#include <botan/alg_id.h>
#include <chrono>

namespace Botan {

/**
* Encrypt with PBES2 from PKCS #5 v2.0
* @param key_bits the input
* @param passphrase the passphrase to use for encryption
* @param msec how many milliseconds to run PBKDF2
* @param cipher specifies the block cipher to use to encrypt
* @param digest specifies the PRF to use with PBKDF2 (eg "HMAC(SHA-1)")
* @param rng a random number generator
*/
std::pair<AlgorithmIdentifier, std::vector<uint8_t>>
BOTAN_DLL pbes2_encrypt(const secure_vector<uint8_t>& key_bits,
                        const std::string& passphrase,
                        std::chrono::milliseconds msec,
                        const std::string& cipher,
                        const std::string& digest,
                        RandomNumberGenerator& rng);

/**
* Decrypt a PKCS #5 v2.0 encrypted stream
* @param key_bits the input
* @param passphrase the passphrase to use for decryption
* @param params the PBES2 parameters
*/
secure_vector<uint8_t>
BOTAN_DLL pbes2_decrypt(const secure_vector<uint8_t>& key_bits,
                        const std::string& passphrase,
                        const std::vector<uint8_t>& params);

}

#endif
