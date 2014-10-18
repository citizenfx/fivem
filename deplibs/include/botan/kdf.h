/*
* Key Derivation Function interfaces
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_KDF_BASE_H__
#define BOTAN_KDF_BASE_H__

#include <botan/secmem.h>
#include <botan/types.h>
#include <string>

namespace Botan {

/**
* Key Derivation Function
*/
class BOTAN_DLL KDF
   {
   public:
      virtual ~KDF() {}

      virtual std::string name() const = 0;

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param salt a diversifier
      */
      secure_vector<byte> derive_key(size_t key_len,
                                    const secure_vector<byte>& secret,
                                    const std::string& salt = "") const
         {
         return derive_key(key_len, &secret[0], secret.size(),
                           reinterpret_cast<const byte*>(salt.data()),
                           salt.length());
         }

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param salt a diversifier
      */
      template<typename Alloc, typename Alloc2>
      secure_vector<byte> derive_key(size_t key_len,
                                     const std::vector<byte, Alloc>& secret,
                                     const std::vector<byte, Alloc2>& salt) const
         {
         return derive_key(key_len,
                           &secret[0], secret.size(),
                           &salt[0], salt.size());
         }

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param salt a diversifier
      * @param salt_len size of salt in bytes
      */
      secure_vector<byte> derive_key(size_t key_len,
                                    const secure_vector<byte>& secret,
                                    const byte salt[],
                                    size_t salt_len) const
         {
         return derive_key(key_len,
                           &secret[0], secret.size(),
                           salt, salt_len);
         }

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param secret_len size of secret in bytes
      * @param salt a diversifier
      */
      secure_vector<byte> derive_key(size_t key_len,
                                    const byte secret[],
                                    size_t secret_len,
                                    const std::string& salt = "") const
         {
         return derive_key(key_len, secret, secret_len,
                           reinterpret_cast<const byte*>(salt.data()),
                           salt.length());
         }

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param secret_len size of secret in bytes
      * @param salt a diversifier
      * @param salt_len size of salt in bytes
      */
      secure_vector<byte> derive_key(size_t key_len,
                                    const byte secret[],
                                    size_t secret_len,
                                    const byte salt[],
                                    size_t salt_len) const
         {
         return derive(key_len, secret, secret_len, salt, salt_len);
         }

      virtual KDF* clone() const = 0;
   private:
      virtual secure_vector<byte>
         derive(size_t key_len,
                const byte secret[], size_t secret_len,
                const byte salt[], size_t salt_len) const = 0;
   };

/**
* Factory method for KDF (key derivation function)
* @param algo_spec the name of the KDF to create
* @return pointer to newly allocated object of that type
*/
BOTAN_DLL KDF*  get_kdf(const std::string& algo_spec);

}

#endif
