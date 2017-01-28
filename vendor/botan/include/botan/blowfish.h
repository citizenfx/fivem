/*
* Blowfish
* (C) 1999-2011 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_BLOWFISH_H__
#define BOTAN_BLOWFISH_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* Blowfish
*/
class BOTAN_DLL Blowfish final : public Block_Cipher_Fixed_Params<8, 1, 56>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      /**
      * Modified EKSBlowfish key schedule, used for bcrypt password hashing
      */
      void eks_key_schedule(const uint8_t key[], size_t key_length,
                            const uint8_t salt[16], size_t workfactor);

      void clear() override;
      std::string name() const override { return "Blowfish"; }
      BlockCipher* clone() const override { return new Blowfish; }
   private:
      void key_schedule(const uint8_t key[], size_t length) override;

      void key_expansion(const uint8_t key[],
                         size_t key_length,
                         const uint8_t salt[16]);

      void generate_sbox(secure_vector<uint32_t>& box,
                         uint32_t& L, uint32_t& R,
                         const uint8_t salt[16],
                         size_t salt_off) const;

      secure_vector<uint32_t> m_S, m_P;
   };

}

#endif
