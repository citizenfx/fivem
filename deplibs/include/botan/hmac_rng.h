/*
* HMAC RNG
* (C) 2008,2013 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_HMAC_RNG_H__
#define BOTAN_HMAC_RNG_H__

#include <botan/mac.h>
#include <botan/rng.h>
#include <vector>

namespace Botan {

/**
* HMAC_RNG - based on the design described in "On Extract-then-Expand
* Key Derivation Functions and an HMAC-based KDF" by Hugo Krawczyk
* (henceforce, 'E-t-E')
*
* However it actually can be parameterized with any two MAC functions,
* not restricted to HMAC (this variation is also described in
* Krawczyk's paper), for instance one could use HMAC(SHA-512) as the
* extractor and CMAC(AES-256) as the PRF.
*/
class BOTAN_DLL HMAC_RNG : public RandomNumberGenerator
   {
   public:
      void randomize(byte buf[], size_t len);
      bool is_seeded() const;
      void clear();
      std::string name() const;

      void reseed(size_t poll_bits);
      void add_entropy(const byte[], size_t);

      /**
      * @param extractor a MAC used for extracting the entropy
      * @param prf a MAC used as a PRF using HKDF construction
      */
      HMAC_RNG(MessageAuthenticationCode* extractor,
               MessageAuthenticationCode* prf);
   private:
      std::unique_ptr<MessageAuthenticationCode> m_extractor;
      std::unique_ptr<MessageAuthenticationCode> m_prf;

      size_t m_collected_entropy_estimate = 0;
      size_t m_output_since_reseed = 0;

      secure_vector<byte> m_K;
      u32bit m_counter = 0;
   };

}

#endif
