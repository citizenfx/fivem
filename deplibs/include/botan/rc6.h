/*
* RC6
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_RC6_H__
#define BOTAN_RC6_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* RC6, Ron Rivest's AES candidate
*/
class BOTAN_DLL RC6 : public Block_Cipher_Fixed_Params<16, 1, 32>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "RC6"; }
      BlockCipher* clone() const { return new RC6; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u32bit> S;
   };

}

#endif
