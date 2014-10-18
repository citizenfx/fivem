/*
* Serpent
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_SERPENT_H__
#define BOTAN_SERPENT_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* Serpent, an AES finalist
*/
class BOTAN_DLL Serpent : public Block_Cipher_Fixed_Params<16, 16, 32, 8>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "Serpent"; }
      BlockCipher* clone() const { return new Serpent; }
   protected:
      /**
      * For use by subclasses using SIMD, asm, etc
      * @return const reference to the key schedule
      */
      const secure_vector<u32bit>& get_round_keys() const
         { return round_key; }

      /**
      * For use by subclasses that implement the key schedule
      * @param ks is the new key schedule value to set
      */
      void set_round_keys(const u32bit ks[132])
         {
         round_key.assign(&ks[0], &ks[132]);
         }

   private:
      void key_schedule(const byte key[], size_t length);
      secure_vector<u32bit> round_key;
   };

}

#endif
