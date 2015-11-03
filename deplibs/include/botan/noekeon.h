/*
* Noekeon
* (C) 1999-2008 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_NOEKEON_H__
#define BOTAN_NOEKEON_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* Noekeon
*/
class BOTAN_DLL Noekeon : public Block_Cipher_Fixed_Params<16, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "Noekeon"; }
      BlockCipher* clone() const override { return new Noekeon; }
   protected:
      /**
      * The Noekeon round constants
      */
      static const byte RC[17];

      /**
      * @return const reference to encryption subkeys
      */
      const secure_vector<u32bit>& get_EK() const { return EK; }

      /**
      * @return const reference to decryption subkeys
      */
      const secure_vector<u32bit>& get_DK() const { return DK; }

   private:
      void key_schedule(const byte[], size_t) override;
      secure_vector<u32bit> EK, DK;
   };

}

#endif
