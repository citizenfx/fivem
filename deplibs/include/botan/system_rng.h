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
* Instantiatable reference to the system RNG.
*/
class BOTAN_DLL System_RNG : public RandomNumberGenerator
   {
   public:
      System_RNG() : m_rng(system_rng()) {}

      void randomize(Botan::byte out[], size_t len) override { m_rng.randomize(out, len); }

      bool is_seeded() const override { return m_rng.is_seeded(); }

      void clear() override { m_rng.clear(); }

      std::string name() const override { return m_rng.name(); }

      void reseed(size_t poll_bits = 256) override { m_rng.reseed(poll_bits); }

      void add_entropy(const byte in[], size_t len) override { m_rng.add_entropy(in, len); }
   private:
      Botan::RandomNumberGenerator& m_rng;
   };

}

#endif
