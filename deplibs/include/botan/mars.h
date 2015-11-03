/*
* MARS
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
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
      void encrypt_n(const byte in[], byte out[], size_t blocks) const override;
      void decrypt_n(const byte in[], byte out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "MARS"; }
      BlockCipher* clone() const override { return new MARS; }
   private:
      void key_schedule(const byte[], size_t) override;

      secure_vector<u32bit> EK;
   };

}

#endif
