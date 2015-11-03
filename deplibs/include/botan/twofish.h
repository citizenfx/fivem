/*
* Twofish
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_TWOFISH_H__
#define BOTAN_TWOFISH_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* Twofish, an AES finalist
*/
class BOTAN_DLL Twofish : public Block_Cipher_Fixed_Params<16, 16, 32, 8>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "Twofish"; }
      BlockCipher* clone() const override { return new Twofish; }
   private:
      void key_schedule(const byte[], size_t) override;

      static void rs_mul(byte[4], byte, size_t);

      static const u32bit MDS0[256];
      static const u32bit MDS1[256];
      static const u32bit MDS2[256];
      static const u32bit MDS3[256];
      static const byte Q0[256];
      static const byte Q1[256];
      static const byte RS[32];
      static const byte EXP_TO_POLY[255];
      static const byte POLY_TO_EXP[255];

      secure_vector<u32bit> SB, RK;
   };

}

#endif
