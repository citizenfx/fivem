/*
* HMAC_DRBG (SP800-90A)
* (C) 2014,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_HMAC_DRBG_H__
#define BOTAN_HMAC_DRBG_H__

#include <botan/rng.h>
#include <botan/mac.h>

namespace Botan {

/**
* HMAC_DRBG (SP800-90A)
*/
class BOTAN_DLL HMAC_DRBG : public RandomNumberGenerator
   {
   public:
      void randomize(byte buf[], size_t buf_len) override;
      bool is_seeded() const override;
      void clear() override;
      std::string name() const override;

      void reseed(size_t poll_bits) override;

      void add_entropy(const byte input[], size_t input_len) override;

      /**
      * @param mac the underlying mac function (eg HMAC(SHA-512))
      * @param underlying_rng RNG used generating inputs (eg HMAC_RNG)
      */
      HMAC_DRBG(MessageAuthenticationCode* mac,
                RandomNumberGenerator* underlying_rng = nullptr);

      HMAC_DRBG(const std::string& mac,
                RandomNumberGenerator* underlying_rng = nullptr);

   private:
      void update(const byte input[], size_t input_len);

      std::unique_ptr<MessageAuthenticationCode> m_mac;
      std::unique_ptr<RandomNumberGenerator> m_prng;

      secure_vector<byte> m_V;
      size_t m_reseed_counter;
   };

}

#endif
