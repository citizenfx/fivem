/**
 * (C) 2014 cryptosource GmbH
 * (C) 2014 Falko Strenzke fstrenzke@cryptosource.de
 *
 * Botan is released under the Simplified BSD License (see license.txt)
 *
 */

#ifndef BOTAN_MCE_KEM_H__
#define BOTAN_MCE_KEM_H__

#include <botan/mceliece.h>
#include <utility>

namespace Botan {

class BOTAN_DLL McEliece_KEM_Encryptor
   {
   public:
      McEliece_KEM_Encryptor(const McEliece_PublicKey& public_key);

      /**
      * returns the pair (mceliece ciphertext, symmetric key)
      */
      std::pair<secure_vector<byte>, secure_vector<byte>> encrypt(RandomNumberGenerator& rng);

   private:
      McEliece_Public_Operation m_raw_pub_op;
   };

class BOTAN_DLL McEliece_KEM_Decryptor
   {
    public:
      McEliece_KEM_Decryptor(const McEliece_PrivateKey& mce_key);

      /**
      * returns the derived 512-bit symmetric key
      */
      secure_vector<Botan::byte> decrypt(const byte msg[], size_t msg_len);

      /**
      * returns the derived 512-bit symmetric key
      */
      template<typename Alloc>
      secure_vector<Botan::byte> decrypt_vec(const std::vector<byte, Alloc>& v)
         {
         return decrypt(&v[0], v.size());

         }
   private:
      McEliece_Private_Operation m_raw_priv_op;
  };
}

#endif
