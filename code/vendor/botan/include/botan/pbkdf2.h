/*
* PBKDF2
* (C) 1999-2007,2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_PBKDF2_H_
#define BOTAN_PBKDF2_H_

#include <botan/pbkdf.h>
#include <botan/mac.h>

namespace Botan {

BOTAN_PUBLIC_API(2,0) size_t pbkdf2(MessageAuthenticationCode& prf,
                        uint8_t out[],
                        size_t out_len,
                        const std::string& passphrase,
                        const uint8_t salt[], size_t salt_len,
                        size_t iterations,
                        std::chrono::milliseconds msec);

/**
* PKCS #5 PBKDF2
*/
class BOTAN_PUBLIC_API(2,0) PKCS5_PBKDF2 final : public PBKDF
   {
   public:
      std::string name() const override
         {
         return "PBKDF2(" + m_mac->name() + ")";
         }

      PBKDF* clone() const override
         {
         return new PKCS5_PBKDF2(m_mac->clone());
         }

      size_t pbkdf(uint8_t output_buf[], size_t output_len,
                   const std::string& passphrase,
                   const uint8_t salt[], size_t salt_len,
                   size_t iterations,
                   std::chrono::milliseconds msec) const override;

      /**
      * Create a PKCS #5 instance using the specified message auth code
      * @param mac_fn the MAC object to use as PRF
      */
      explicit PKCS5_PBKDF2(MessageAuthenticationCode* mac_fn) : m_mac(mac_fn) {}
   private:
      std::unique_ptr<MessageAuthenticationCode> m_mac;
   };

}

#endif
