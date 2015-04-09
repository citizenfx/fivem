/*
* CAST-128
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_CAST128_H__
#define BOTAN_CAST128_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* CAST-128
*/
class BOTAN_DLL CAST_128 : public Block_Cipher_Fixed_Params<8, 11, 16>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "CAST-128"; }
      BlockCipher* clone() const { return new CAST_128; }

   private:
      void key_schedule(const byte[], size_t);

      static void cast_ks(secure_vector<u32bit>& ks,
                          secure_vector<u32bit>& user_key);

      secure_vector<u32bit> MK;
      secure_vector<byte> RK;
   };

}

#endif
