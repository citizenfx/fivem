/*
* PBKDF1
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_PBKDF1_H__
#define BOTAN_PBKDF1_H__

#include <botan/pbkdf.h>
#include <botan/hash.h>

namespace Botan {

/**
* PKCS #5 v1 PBKDF, aka PBKDF1
* Can only generate a key up to the size of the hash output.
* Unless needed for backwards compatability, use PKCS5_PBKDF2
*/
class BOTAN_DLL PKCS5_PBKDF1 : public PBKDF
   {
   public:
      /**
      * Create a PKCS #5 instance using the specified hash function.
      * @param hash_in pointer to a hash function object to use
      */
      PKCS5_PBKDF1(HashFunction* hash_in) : hash(hash_in) {}

      std::string name() const
         {
         return "PBKDF1(" + hash->name() + ")";
         }

      PBKDF* clone() const
         {
         return new PKCS5_PBKDF1(hash->clone());
         }

      std::pair<size_t, OctetString>
         key_derivation(size_t output_len,
                        const std::string& passphrase,
                        const byte salt[], size_t salt_len,
                        size_t iterations,
                        std::chrono::milliseconds msec) const override;
   private:
      std::unique_ptr<HashFunction> hash;
   };

}

#endif
