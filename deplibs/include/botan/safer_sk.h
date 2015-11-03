/*
* SAFER-SK
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_SAFER_SK_H__
#define BOTAN_SAFER_SK_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* SAFER-SK
*/
class BOTAN_DLL SAFER_SK : public Block_Cipher_Fixed_Params<8, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override;
      BlockCipher* clone() const override;

      /**
      * @param rounds the number of rounds to use - must be between 1
      * and 13
      */
      SAFER_SK(size_t rounds);
   private:
      void key_schedule(const byte[], size_t) override;

      size_t rounds;
      secure_vector<byte> EK;
   };

}

#endif
