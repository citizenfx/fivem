/*
* XTS mode, from IEEE P1619
* (C) 2009,2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_MODE_XTS_H__
#define BOTAN_MODE_XTS_H__

#include <botan/cipher_mode.h>
#include <botan/block_cipher.h>

namespace Botan {

/**
* IEEE P1619 XTS Mode
*/
class BOTAN_DLL XTS_Mode : public Cipher_Mode
   {
   public:
      std::string name() const override;

      size_t update_granularity() const override;

      size_t minimum_final_size() const override;

      Key_Length_Specification key_spec() const override;

      size_t default_nonce_length() const override;

      bool valid_nonce_length(size_t n) const override;

      void clear() override;
   protected:
      XTS_Mode(BlockCipher* cipher);

      const byte* tweak() const { return &m_tweak[0]; }

      const BlockCipher& cipher() const { return *m_cipher; }

      void update_tweak(size_t last_used);

   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override;
      void key_schedule(const byte key[], size_t length) override;

      std::unique_ptr<BlockCipher> m_cipher, m_tweak_cipher;
      secure_vector<byte> m_tweak;
   };

/**
* IEEE P1619 XTS Encryption
*/
class BOTAN_DLL XTS_Encryption : public XTS_Mode
   {
   public:
      XTS_Encryption(BlockCipher* cipher) : XTS_Mode(cipher) {}

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override;
   };

/**
* IEEE P1619 XTS Decryption
*/
class BOTAN_DLL XTS_Decryption : public XTS_Mode
   {
   public:
      XTS_Decryption(BlockCipher* cipher) : XTS_Mode(cipher) {}

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override;
   };

}

#endif
