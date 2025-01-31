/*
* AES
* (C) 1999-2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_AES_H_
#define BOTAN_AES_H_

#include <botan/block_cipher.h>

namespace Botan {

/**
* AES-128
*/
class BOTAN_PUBLIC_API(2,0) AES_128 final : public Block_Cipher_Fixed_Params<16, 16>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;

      std::string provider() const override;
      std::string name() const override { return "AES-128"; }
      BlockCipher* clone() const override { return new AES_128; }
      size_t parallelism() const override;

   private:
      void key_schedule(const uint8_t key[], size_t length) override;

#if defined(BOTAN_HAS_AES_SSSE3)
      void ssse3_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void ssse3_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void ssse3_key_schedule(const uint8_t key[], size_t length);
#endif

#if defined(BOTAN_HAS_AES_NI)
      void aesni_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void aesni_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void aesni_key_schedule(const uint8_t key[], size_t length);
#endif

#if defined(BOTAN_HAS_AES_ARMV8)
      void armv8_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void armv8_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
#endif

#if defined(BOTAN_HAS_AES_POWER8)
      void power8_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void power8_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
#endif

      secure_vector<uint32_t> m_EK, m_DK;
      secure_vector<uint8_t> m_ME, m_MD;
   };

/**
* AES-192
*/
class BOTAN_PUBLIC_API(2,0) AES_192 final : public Block_Cipher_Fixed_Params<16, 24>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;

      std::string provider() const override;
      std::string name() const override { return "AES-192"; }
      BlockCipher* clone() const override { return new AES_192; }
      size_t parallelism() const override;

   private:
#if defined(BOTAN_HAS_AES_SSSE3)
      void ssse3_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void ssse3_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void ssse3_key_schedule(const uint8_t key[], size_t length);
#endif

#if defined(BOTAN_HAS_AES_NI)
      void aesni_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void aesni_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void aesni_key_schedule(const uint8_t key[], size_t length);
#endif

#if defined(BOTAN_HAS_AES_ARMV8)
      void armv8_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void armv8_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
#endif

#if defined(BOTAN_HAS_AES_POWER8)
      void power8_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void power8_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
#endif

      void key_schedule(const uint8_t key[], size_t length) override;

      secure_vector<uint32_t> m_EK, m_DK;
      secure_vector<uint8_t> m_ME, m_MD;
   };

/**
* AES-256
*/
class BOTAN_PUBLIC_API(2,0) AES_256 final : public Block_Cipher_Fixed_Params<16, 32>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;

      std::string provider() const override;

      std::string name() const override { return "AES-256"; }
      BlockCipher* clone() const override { return new AES_256; }
      size_t parallelism() const override;

   private:
#if defined(BOTAN_HAS_AES_SSSE3)
      void ssse3_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void ssse3_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void ssse3_key_schedule(const uint8_t key[], size_t length);
#endif

#if defined(BOTAN_HAS_AES_NI)
      void aesni_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void aesni_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void aesni_key_schedule(const uint8_t key[], size_t length);
#endif

#if defined(BOTAN_HAS_AES_ARMV8)
      void armv8_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void armv8_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
#endif

#if defined(BOTAN_HAS_AES_POWER8)
      void power8_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void power8_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
#endif

      void key_schedule(const uint8_t key[], size_t length) override;

      secure_vector<uint32_t> m_EK, m_DK;
      secure_vector<uint8_t> m_ME, m_MD;
   };

}

#endif
