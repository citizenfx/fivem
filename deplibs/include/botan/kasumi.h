/*
* KASUMI
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_KASUMI_H__
#define BOTAN_KASUMI_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* KASUMI, the block cipher used in 3G telephony
*/
class BOTAN_DLL KASUMI : public Block_Cipher_Fixed_Params<8, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "KASUMI"; }
      BlockCipher* clone() const { return new KASUMI; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u16bit> EK;
   };

}

#endif
