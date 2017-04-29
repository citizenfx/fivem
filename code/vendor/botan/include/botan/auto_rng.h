/*
* Auto Seeded RNG
* (C) 2008,2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_AUTO_SEEDING_RNG_H__
#define BOTAN_AUTO_SEEDING_RNG_H__

#include <botan/rng.h>

namespace Botan {

class Stateful_RNG;

/**
* A userspace PRNG
*/
class BOTAN_DLL AutoSeeded_RNG final : public RandomNumberGenerator
   {
   public:
      void randomize(uint8_t out[], size_t len) override;

      void randomize_with_input(uint8_t output[], size_t output_len,
                                const uint8_t input[], size_t input_len) override;

      bool is_seeded() const override;

      /**
      * Mark state as requiring a reseed on next use
      */
      void force_reseed();

      size_t reseed(Entropy_Sources& srcs,
                    size_t poll_bits = BOTAN_RNG_RESEED_POLL_BITS,
                    std::chrono::milliseconds poll_timeout = BOTAN_RNG_RESEED_DEFAULT_TIMEOUT) override;

      void add_entropy(const uint8_t in[], size_t len) override;

      std::string name() const override;

      void clear() override;

      /**
      * Uses the system RNG (if available) or else a default group of
      * entropy sources (all other systems) to gather seed material.
      *
      * @param reseed_interval specifies a limit of how many times
      * the RNG will be called before automatic reseeding is performed
      */
      AutoSeeded_RNG(size_t reseed_interval = BOTAN_RNG_DEFAULT_RESEED_INTERVAL);

      /**
      * Uses the BOTAN_AUTO_RNG_DRBG RNG to gather seed material.
      *
      * @param underlying_rng is a reference to some RNG which will be used
      * to perform the periodic reseeding
      * @param reseed_interval specifies a limit of how many times
      * the RNG will be called before automatic reseeding is performed
      */
      AutoSeeded_RNG(RandomNumberGenerator& underlying_rng,
                     size_t reseed_interval = BOTAN_RNG_DEFAULT_RESEED_INTERVAL);

      /**
      * Uses the BOTAN_AUTO_RNG_DRBG RNG to gather seed material.
      *
      * @param entropy_sources will be polled to perform reseeding periodically
      * @param reseed_interval specifies a limit of how many times
      * the RNG will be called before automatic reseeding is performed
      */
      AutoSeeded_RNG(Entropy_Sources& entropy_sources,
                     size_t reseed_interval = BOTAN_RNG_DEFAULT_RESEED_INTERVAL);

      /**
      * Uses the BOTAN_AUTO_RNG_DRBG RNG to gather seed material.
      *
      * @param underlying_rng is a reference to some RNG which will be used
      * to perform the periodic reseeding
      * @param entropy_sources will be polled to perform reseeding periodically
      * @param reseed_interval specifies a limit of how many times
      * the RNG will be called before automatic reseeding is performed
      */
      AutoSeeded_RNG(RandomNumberGenerator& underlying_rng,
                     Entropy_Sources& entropy_sources,
                     size_t reseed_interval = BOTAN_RNG_DEFAULT_RESEED_INTERVAL);

      ~AutoSeeded_RNG();

   private:
      std::unique_ptr<Stateful_RNG> m_rng;
   };

}

#endif
