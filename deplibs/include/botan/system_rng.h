/*
* System RNG interface
* (C) 2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_SYSTEM_RNG_H__
#define BOTAN_SYSTEM_RNG_H__

#include <botan/rng.h>

namespace Botan {

/**
* Return a shared reference to a global PRNG instance provided by the
* operating system. For instance might be instantiated by /dev/urandom
* or CryptGenRandom.
*/
BOTAN_DLL RandomNumberGenerator& system_rng();

}

#endif
