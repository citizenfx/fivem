/*
* HMAC_DRBG (SP800-90A)
* (C) 2014,2015,2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_HMAC_DRBG_H__
#define BOTAN_HMAC_DRBG_H__

#include <botan/stateful_rng.h>
#include <botan/mac.h>

namespace Botan {

class Entropy_Sources;

/**
* HMAC_DRBG from NIST SP800-90A
*/
class BOTAN_DLL HMAC_DRBG final : public Stateful_RNG
   {
   public:
      /**
      * Initialize an HMAC_DRBG instance with the given MAC as PRF (normally HMAC)
      *
      * Automatic reseeding is disabled completely, as it has no access to
      * any source for seed material.
      *
      * If a fork is detected, the RNG will be unable to reseed itself
      * in response. In this case, an exception will be thrown rather
      * than generating duplicated output.
      */
      HMAC_DRBG(std::unique_ptr<MessageAuthenticationCode> prf);

      /**
      * Initialize an HMAC_DRBG instance with the given MAC as PRF (normally HMAC)
      *
      * Automatic reseeding from @p underlying_rng will take place after
      * @p reseed_interval many requests or after a fork was detected.
      *
      * @param prf MAC to use as a PRF
      * @param underlying_rng is a reference to some RNG which will be used
      * to perform the periodic reseeding
      * @param reseed_interval specifies a limit of how many times
      * the RNG will be called before automatic reseeding is performed
      * @param max_number_of_bytes_per_request requests that are in size higher 
      * than max_number_of_bytes_per_request are treated as if multiple single 
      * requests of max_number_of_bytes_per_request size had been made.
      * In theory SP 800-90A requires that we reject any request for a DRBG
      * output longer than max_number_of_bytes_per_request. To avoid inconveniencing
      * the caller who wants an output larger than max_number_of_bytes_per_request, 
      * instead treat these requests as if multiple requests of 
      * max_number_of_bytes_per_request size had been made. NIST requires for 
      * HMAC_DRBG that every implementation set a value no more than 2**19 bits 
      * (or 64 KiB). Together with @p reseed_interval = 1 you can enforce that for 
      * example every 512 bit automatic reseeding occurs.
      */
      HMAC_DRBG(std::unique_ptr<MessageAuthenticationCode> prf,
                RandomNumberGenerator& underlying_rng,
                size_t reseed_interval = BOTAN_RNG_DEFAULT_RESEED_INTERVAL,
                size_t max_number_of_bytes_per_request = 64 * 1024);

      /**
      * Initialize an HMAC_DRBG instance with the given MAC as PRF (normally HMAC)
      *
      * Automatic reseeding from @p entropy_sources will take place after
      * @p reseed_interval many requests or after a fork was detected.
      *
      * @param prf MAC to use as a PRF
      * @param entropy_sources will be polled to perform reseeding periodically
      * @param reseed_interval specifies a limit of how many times
      * the RNG will be called before automatic reseeding is performed.
      * @param max_number_of_bytes_per_request requests that are in size higher 
      * than max_number_of_bytes_per_request are treated as if multiple single 
      * requests of max_number_of_bytes_per_request size had been made.
      * In theory SP 800-90A requires that we reject any request for a DRBG
      * output longer than max_number_of_bytes_per_request. To avoid inconveniencing
      * the caller who wants an output larger than max_number_of_bytes_per_request, 
      * instead treat these requests as if multiple requests of 
      * max_number_of_bytes_per_request size had been made. NIST requires for 
      * HMAC_DRBG that every implementation set a value no more than 2**19 bits 
      * (or 64 KiB). Together with @p reseed_interval = 1 you can enforce that for 
      * example every 512 bit automatic reseeding occurs.
      */
      HMAC_DRBG(std::unique_ptr<MessageAuthenticationCode> prf,
                Entropy_Sources& entropy_sources,
                size_t reseed_interval = BOTAN_RNG_DEFAULT_RESEED_INTERVAL,
                size_t max_number_of_bytes_per_request = 64 * 1024);

      /**
      * Initialize an HMAC_DRBG instance with the given MAC as PRF (normally HMAC)
      *
      * Automatic reseeding from @p underlying_rng and @p entropy_sources
      * will take place after @p reseed_interval many requests or after
      * a fork was detected.
      *
      * @param prf MAC to use as a PRF
      * @param underlying_rng is a reference to some RNG which will be used
      * to perform the periodic reseeding
      * @param entropy_sources will be polled to perform reseeding periodically
      * @param reseed_interval specifies a limit of how many times
      * the RNG will be called before automatic reseeding is performed.
      * @param max_number_of_bytes_per_request requests that are in size higher 
      * than max_number_of_bytes_per_request are treated as if multiple single 
      * requests of max_number_of_bytes_per_request size had been made.
      * In theory SP 800-90A requires that we reject any request for a DRBG
      * output longer than max_number_of_bytes_per_request. To avoid inconveniencing
      * the caller who wants an output larger than max_number_of_bytes_per_request, 
      * instead treat these requests as if multiple requests of 
      * max_number_of_bytes_per_request size had been made. NIST requires for 
      * HMAC_DRBG that every implementation set a value no more than 2**19 bits 
      * (or 64 KiB). Together with @p reseed_interval = 1 you can enforce that for 
      * example every 512 bit automatic reseeding occurs.
      */
      HMAC_DRBG(std::unique_ptr<MessageAuthenticationCode> prf,
                RandomNumberGenerator& underlying_rng,
                Entropy_Sources& entropy_sources,
                size_t reseed_interval = BOTAN_RNG_DEFAULT_RESEED_INTERVAL,
                size_t max_number_of_bytes_per_request = 64 * 1024);

      /**
      * Constructor taking a string for the hash
      */
      HMAC_DRBG(const std::string& hmac_hash) :
         Stateful_RNG(),
         m_mac(MessageAuthenticationCode::create_or_throw("HMAC(" + hmac_hash + ")")),
         m_max_number_of_bytes_per_request(64 * 1024)
         {
         clear();
         }

      std::string name() const override;

      void clear() override;

      void randomize(uint8_t output[], size_t output_len) override;

      void randomize_with_input(uint8_t output[], size_t output_len,
                                const uint8_t input[], size_t input_len) override;

      void add_entropy(const uint8_t input[], size_t input_len) override;

      size_t security_level() const override;

   private:
      void update(const uint8_t input[], size_t input_len);

      std::unique_ptr<MessageAuthenticationCode> m_mac;
      secure_vector<uint8_t> m_V;
      const size_t m_max_number_of_bytes_per_request;
   };

}

#endif
