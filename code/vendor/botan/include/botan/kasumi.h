/*
* KASUMI
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_KASUMI_H__
#define BOTAN_KASUMI_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* KASUMI, the block cipher used in 3G telephony
*/
class BOTAN_DLL KASUMI final : public Block_Cipher_Fixed_Params<8, 16>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "KASUMI"; }
      BlockCipher* clone() const override { return new KASUMI; }
   private:
      void key_schedule(const uint8_t[], size_t) override;

      secure_vector<uint16_t> m_EK;
   };

}

#endif
