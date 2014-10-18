/*
* PBKDF2
* (C) 1999-2007,2012 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_PBKDF2_H__
#define BOTAN_PBKDF2_H__

#include <botan/pbkdf.h>
#include <botan/mac.h>

namespace Botan {

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

      std::pair<size_t, OctetString>
         key_derivation(size_t output_len,
                        const std::string& passphrase,
                        const byte salt[], size_t salt_len,
                        size_t iterations,
                        std::chrono::milliseconds msec) const override;

      /**
      * Create a PKCS #5 instance using the specified message auth code
      * @param mac_fn the MAC object to use as PRF
      */
      PKCS5_PBKDF2(MessageAuthenticationCode* mac_fn) : mac(mac_fn) {}
   private:
      std::unique_ptr<MessageAuthenticationCode> mac;
   };

}

#endif
