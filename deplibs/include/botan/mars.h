/*
* MARS
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_MARS_H__
#define BOTAN_MARS_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* MARS, IBM's candidate for AES
*/
class BOTAN_DLL MARS : public Block_Cipher_Fixed_Params<16, 16, 32, 4>
   {
   public:
      void encrypt_n(const byte in[], byte out[], size_t blocks) const;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const;

      void clear();
      std::string name() const { return "MARS"; }
      BlockCipher* clone() const { return new MARS; }
   private:
      void key_schedule(const byte[], size_t);

      secure_vector<u32bit> EK;
   };

}

#endif
