/*
* CAST-256
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_CAST256_H__
#define BOTAN_CAST256_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* CAST-256
*/
class BOTAN_DLL CAST_256 : public Block_Cipher_Fixed_Params<16, 4, 32, 4>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "CAST-256"; }
      BlockCipher* clone() const { return new CAST_256; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u32bit> MK;
      secure_vector<byte> RK;
   };

}

#endif
