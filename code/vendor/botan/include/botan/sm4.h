/*
* SM4
* (C) 2017 Ribose Inc
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_SM4_H_
#define BOTAN_SM4_H_

#include <botan/block_cipher.h>

namespace Botan {

/**
* SM4
*/
class BOTAN_PUBLIC_API(2,2) SM4 final : public Block_Cipher_Fixed_Params<16, 16>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "SM4"; }
      BlockCipher* clone() const override { return new SM4; }
   private:
      void key_schedule(const uint8_t[], size_t) override;

      secure_vector<uint32_t> m_RK;
   };

}

#endif
