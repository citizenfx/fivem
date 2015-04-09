/*
* CFB mode
* (C) 1999-2007,2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_MODE_CFB_H__
#define BOTAN_MODE_CFB_H__

#include <botan/cipher_mode.h>
#include <botan/block_cipher.h>
#include <botan/mode_pad.h>

namespace Botan {

/**
* CFB Mode
*/
class BOTAN_DLL CFB_Mode : public Cipher_Mode
   {
   public:
      std::string name() const override;

      size_t update_granularity() const override;

      size_t minimum_final_size() const override;

      Key_Length_Specification key_spec() const override;

      size_t output_length(size_t input_length) const override;

      size_t default_nonce_length() const override;

      bool valid_nonce_length(size_t n) const override;

      void clear() override;
   protected:
      CFB_Mode(BlockCipher* cipher, size_t feedback_bits);

      const BlockCipher& cipher() const { return *m_cipher; }

      size_t feedback() const { return m_feedback_bytes; }

      secure_vector<byte>& shift_register() { return m_shift_register; }

      secure_vector<byte>& keystream_buf() { return m_keystream_buf; }

   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override;
      void key_schedule(const byte key[], size_t length) override;

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<byte> m_shift_register;
      secure_vector<byte> m_keystream_buf;
      size_t m_feedback_bytes;
   };

/**
* CFB Encryption
*/
class BOTAN_DLL CFB_Encryption : public CFB_Mode
   {
   public:
      CFB_Encryption(BlockCipher* cipher, size_t feedback_bits) :
         CFB_Mode(cipher, feedback_bits) {}

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   };

/**
* CFB Decryption
*/
class BOTAN_DLL CFB_Decryption : public CFB_Mode
   {
   public:
      CFB_Decryption(BlockCipher* cipher, size_t feedback_bits) :
         CFB_Mode(cipher, feedback_bits) {}

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   };

}

#endif
