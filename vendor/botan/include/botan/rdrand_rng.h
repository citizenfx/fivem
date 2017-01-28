/*
* RDRAND RNG
* (C) 2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_RNG_RDRAND_H__
#define BOTAN_RNG_RDRAND_H__

#include <botan/rng.h>

namespace Botan {

class BOTAN_DLL RDRAND_RNG : public Hardware_RNG
   {
   public:
      /**
      * On correctly working hardware, RDRAND is always supposed to
      * succeed within a set number of retries. If after that many
      * retries RDRAND has still not suceeded, sets ok = false and
      * returns 0.
      */
      static uint32_t rdrand_status(bool& ok);

      /*
      * Calls RDRAND until it succeeds, this could hypothetically
      * loop forever on broken hardware.
      */
      static uint32_t rdrand();

      /**
      * Constructor will throw if CPU does not have RDRAND bit set
      */
      RDRAND_RNG();

      /**
      * Uses RDRAND to produce output
      */
      void randomize(uint8_t out[], size_t out_len) override;

      /*
      * No way to provide entropy to RDRAND generator, so add_entropy is ignored
      */
      void add_entropy(const uint8_t[], size_t) override
         { /* no op */ }

      /*
      * No way to reseed RDRAND generator, so reseed is ignored
      */
      size_t reseed(Entropy_Sources&, size_t, std::chrono::milliseconds) override
         { return 0; /* no op */ }

      std::string name() const override { return "RDRAND"; }

      bool is_seeded() const override { return true; }

      void clear() override {}
   };

}

#endif
