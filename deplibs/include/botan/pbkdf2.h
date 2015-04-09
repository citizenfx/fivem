/*
* PBKDF2
* (C) 1999-2007,2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_PBKDF2_H__
#define BOTAN_PBKDF2_H__

#include <botan/pbkdf.h>
#include <botan/mac.h>
#include <botan/hash.h>

namespace Botan {

BOTAN_DLL size_t pbkdf2(MessageAuthenticationCode& prf,
                        byte out[],
                        size_t out_len,
                        const std::string& passphrase,
                        const byte salt[], size_t salt_len,
                        size_t iterations,
                        std::chrono::milliseconds msec);

/**
* PKCS #5 PBKDF2
*/
class BOTAN_DLL PKCS5_PBKDF2 : public PBKDF
   {
   public:
      std::string name() const override
         {
         return "PBKDF2(" + mac->name() + ")";
         }

      PBKDF* clone() const override
         {
         return new PKCS5_PBKDF2(mac->clone());
         }

      size_t pbkdf(byte output_buf[], size_t output_len,
                   const std::string& passphrase,
                   const byte salt[], size_t salt_len,
                   size_t iterations,
                   std::chrono::milliseconds msec) const override;

      /**
      * Create a PKCS #5 instance using the specified message auth code
      * @param mac_fn the MAC object to use as PRF
      */
      PKCS5_PBKDF2(MessageAuthenticationCode* mac_fn) : mac(mac_fn) {}

      static PKCS5_PBKDF2* make(const Spec& spec);
   private:
      std::unique_ptr<MessageAuthenticationCode> mac;
   };

}

#endif
