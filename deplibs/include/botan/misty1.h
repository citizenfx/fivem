/*
* MISTY1
* (C) 1999-2008 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_MISTY1_H__
#define BOTAN_MISTY1_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* MISTY1 with 8 rounds
*/
class BOTAN_DLL MISTY1 : public Block_Cipher_Fixed_Params<8, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "MISTY1"; }
      BlockCipher* clone() const { return new MISTY1; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u16bit> EK, DK;
   };

}

#endif
