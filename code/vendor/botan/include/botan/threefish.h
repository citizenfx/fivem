/*
* Threefish
* (C) 2013,2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_THREEFISH_H__
#define BOTAN_THREEFISH_H__

#include <botan/block_cipher.h>

namespace Botan {

/**
* Threefish-512
*/
class BOTAN_DLL Threefish_512 final : public Block_Cipher_Fixed_Params<64, 64>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void set_tweak(const uint8_t tweak[], size_t len);

      void clear() override;
      std::string provider() const override;
      std::string name() const override { return "Threefish-512"; }
      BlockCipher* clone() const override { return new Threefish_512; }
   protected:
      const secure_vector<uint64_t>& get_T() const { return m_T; }
      const secure_vector<uint64_t>& get_K() const { return m_K; }
   private:

#if defined(BOTAN_HAS_THREEFISH_512_AVX2)
      void avx2_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void avx2_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
#endif

      void key_schedule(const uint8_t key[], size_t key_len) override;

      // Interface for Skein
      friend class Skein_512;

      virtual void skein_feedfwd(const secure_vector<uint64_t>& M,
                                 const secure_vector<uint64_t>& T);

      // Private data
      secure_vector<uint64_t> m_T;
      secure_vector<uint64_t> m_K;
   };

}

#endif
