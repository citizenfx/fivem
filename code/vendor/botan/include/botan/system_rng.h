/*
* System RNG interface
* (C) 2014,2015 Jack Lloyd
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

/*
* Instantiable reference to the system RNG.
*/
class BOTAN_DLL System_RNG final : public RandomNumberGenerator
   {
   public:
      std::string name() const override { return system_rng().name(); }

      void randomize(uint8_t out[], size_t len) override { system_rng().randomize(out, len); }

      void add_entropy(const uint8_t in[], size_t length) override { system_rng().add_entropy(in, length); }

      bool is_seeded() const override { return true; }

      void clear() override {}
   };

}

#endif
