/*
* HOTP
* (C) 2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_HOTP_H_
#define BOTAN_HOTP_H_

#include <botan/mac.h>

namespace Botan {

/**
* HOTP one time passwords (RFC 4226)
*/
class BOTAN_PUBLIC_API(2,2) HOTP final
   {
   public:
      /**
      * @param key the secret key shared between client and server
      * @param hash_algo the hash algorithm to use, should be SHA-1 or SHA-256
      * @param digits the number of digits in the OTP (must be 6, 7, or 8)
      */
      HOTP(const SymmetricKey& key, const std::string& hash_algo = "SHA-1", size_t digits = 6) :
         HOTP(key.begin(), key.size(), hash_algo, digits) {}

      /**
      * @param key the secret key shared between client and server
      * @param key_len length of key param
      * @param hash_algo the hash algorithm to use, should be SHA-1 or SHA-256
      * @param digits the number of digits in the OTP (must be 6, 7, or 8)
      */
      HOTP(const uint8_t key[], size_t key_len,
           const std::string& hash_algo = "SHA-1",
           size_t digits = 6);

      /**
      * Generate the HOTP for a particular counter value
      * @warning if the counter value is repeated the OTP ceases to be one-time
      */
      uint32_t generate_hotp(uint64_t counter);

      /**
      * Check an OTP value using a starting counter and a resync range
      * @param otp the client provided OTP
      * @param starting_counter the server's guess as to the current counter state
      * @param resync_range if 0 then only HOTP(starting_counter) is accepted
      * If larger than 0, up to resync_range values after HOTP are also checked.
      * @return (valid,next_counter). If the OTP does not validate, always
      * returns (false,starting_counter). Otherwise returns (true,next_counter)
      * where next_counter is at most starting_counter + resync_range + 1
      */
      std::pair<bool,uint64_t> verify_hotp(uint32_t otp, uint64_t starting_counter, size_t resync_range = 0);
   private:
      std::unique_ptr<MessageAuthenticationCode> m_mac;
      uint32_t m_digit_mod;
   };

}

#endif
