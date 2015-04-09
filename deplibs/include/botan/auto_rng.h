/*
* Auto Seeded RNG
* (C) 2008 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_AUTO_SEEDING_RNG_H__
#define BOTAN_AUTO_SEEDING_RNG_H__

#include <botan/rng.h>
#include <string>

namespace Botan {

class BOTAN_DLL AutoSeeded_RNG : public RandomNumberGenerator
   {
   public:
      void randomize(byte out[], size_t len)
         { m_rng->randomize(out, len); }

      bool is_seeded() const { return m_rng->is_seeded(); }

      void clear() { m_rng->clear(); }

      std::string name() const { return m_rng->name(); }

      void reseed(size_t poll_bits = 256) { m_rng->reseed(poll_bits); }

      void add_entropy(const byte in[], size_t len)
         { m_rng->add_entropy(in, len); }

      AutoSeeded_RNG() : m_rng(RandomNumberGenerator::make_rng()) {}
   private:
      std::unique_ptr<RandomNumberGenerator> m_rng;
   };

}

#endif
