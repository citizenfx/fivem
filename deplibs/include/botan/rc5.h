/*
* RC5
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_RC5_H__
#define BOTAN_RC5_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* RC5
*/
class BOTAN_DLL RC5 : public Block_Cipher_Fixed_Params<8, 1, 32>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override;
      BlockCipher* clone() const override { return new RC5(rounds); }

      /**
      * @param rounds the number of RC5 rounds to run. Must be between
      * 8 and 32 and a multiple of 4.
      */
      RC5(size_t rounds);
   private:
      void key_schedule(const byte[], size_t) override;

      size_t rounds;
      secure_vector<u32bit> S;
   };

}

#endif
