/*
* Passhash9 Password Hashing
* (C) 2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_PASSHASH9_H__
#define BOTAN_PASSHASH9_H__

#include <botan/rng.h>

namespace Botan {

/**
* Create a password hash using PBKDF2
* @param password the password
* @param rng a random number generator
* @param work_factor how much work to do to slow down guessing attacks
* @param alg_id specifies which PRF to use with PBKDF2
*        0 is HMAC(SHA-1)
*        1 is HMAC(SHA-256)
*        2 is CMAC(Blowfish)
*        3 is HMAC(SHA-384)
*        4 is HMAC(SHA-512)
*        all other values are currently undefined
*/
std::string BOTAN_DLL generate_passhash9(const std::string& password,
                                         RandomNumberGenerator& rng,
                                         uint16_t work_factor = 10,
                                         uint8_t alg_id = 1);

/**
* Check a previously created password hash
* @param password the password to check against
* @param hash the stored hash to check against
*/
bool BOTAN_DLL check_passhash9(const std::string& password,
                               const std::string& hash);

}

#endif
