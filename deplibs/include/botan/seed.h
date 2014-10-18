/*
* SEED
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_SEED_H__
#define BOTAN_SEED_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* SEED, a Korean block cipher
*/
class BOTAN_DLL SEED : public Block_Cipher_Fixed_Params<16, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "SEED"; }
      BlockCipher* clone() const { return new SEED; }
   private:
      void key_schedule(const byte[], size_t);

      class G_FUNC
         {
         public:
            u32bit operator()(u32bit) const;
         private:
            static const u32bit S0[256], S1[256], S2[256], S3[256];
         };

      secure_vector<u32bit> K;
   };

}

#endif
