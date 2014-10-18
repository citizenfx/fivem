/*
* HMAC_DRBG (SP800-90A)
* (C) 2014 Jack Lloyd
*
* Distributed under the terms of the Botan license
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
      void randomize(byte buf[], size_t buf_len);
      bool is_seeded() const;
      void clear();
      std::string name() const;

      void reseed(size_t poll_bits);

      void add_entropy(const byte input[], size_t input_len);

      /**
      * @param mac the underlying mac function (eg HMAC(SHA-512))
      * @param underlying_rng RNG used generating inputs (eg HMAC_RNG)
      */
      HMAC_DRBG(MessageAuthenticationCode* mac,
                RandomNumberGenerator* underlying_rng);

   private:
      void update(const byte input[], size_t input_len);

      std::unique_ptr<MessageAuthenticationCode> m_mac;
      std::unique_ptr<RandomNumberGenerator> m_prng;

      secure_vector<byte> m_V;
      size_t m_reseed_counter;
   };

}

#endif
