/*
* DLIES
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_DLIES_H__
#define BOTAN_DLIES_H__

#include <botan/pubkey.h>
#include <botan/mac.h>
#include <botan/kdf.h>

namespace Botan {

/**
* DLIES Encryption
*/
class BOTAN_DLL DLIES_Encryptor : public PK_Encryptor
   {
   public:
      DLIES_Encryptor(const PK_Key_Agreement_Key&,
                      KDF* kdf,
                      MessageAuthenticationCode* mac,
                      size_t mac_key_len = 20);

      void set_other_key(const std::vector<byte>&);
   private:
      std::vector<byte> enc(const byte[], size_t,
                            RandomNumberGenerator&) const;

      size_t maximum_input_size() const;

      std::vector<byte> other_key, my_key;

      PK_Key_Agreement ka;
      std::unique_ptr<KDF> kdf;
      std::unique_ptr<MessageAuthenticationCode> mac;
      size_t mac_keylen;
   };

/**
* DLIES Decryption
*/
class BOTAN_DLL DLIES_Decryptor : public PK_Decryptor
   {
   public:
      DLIES_Decryptor(const PK_Key_Agreement_Key&,
                      KDF* kdf,
                      MessageAuthenticationCode* mac,
                      size_t mac_key_len = 20);

   private:
      secure_vector<byte> dec(const byte[], size_t) const;

      std::vector<byte> my_key;

      PK_Key_Agreement ka;
      std::unique_ptr<KDF> kdf;
      std::unique_ptr<MessageAuthenticationCode> mac;
      size_t mac_keylen;
   };

}

#endif
