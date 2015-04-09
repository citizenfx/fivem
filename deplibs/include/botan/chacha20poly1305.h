/*
* ChaCha20Poly1305 AEAD
* (C) 2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_AEAD_CHACHA20_POLY1305_H__
#define BOTAN_AEAD_CHACHA20_POLY1305_H__

#include <botan/aead.h>
#include <botan/stream_cipher.h>
#include <botan/mac.h>

namespace Botan {

/**
* Base class
* See draft-irtf-cfrg-chacha20-poly1305-03 for specification
* If a nonce of 64 bits is used the older version described in
* draft-agl-tls-chacha20poly1305-04 is used instead.
*/
class BOTAN_DLL ChaCha20Poly1305_Mode : public AEAD_Mode
   {
   public:
      void set_associated_data(const byte ad[], size_t ad_len) override;

      std::string name() const override { return "ChaCha20Poly1305"; }

      size_t update_granularity() const override { return 64; }

      Key_Length_Specification key_spec() const override
         { return Key_Length_Specification(32); }

      bool valid_nonce_length(size_t n) const override;

      size_t tag_size() const override { return 16; }

      void clear() override;
   protected:
      std::unique_ptr<StreamCipher> m_chacha;
      std::unique_ptr<MessageAuthenticationCode> m_poly1305;

      ChaCha20Poly1305_Mode();

      secure_vector<byte> m_ad;
      size_t m_nonce_len = 0;
      size_t m_ctext_len = 0;

      bool cfrg_version() const { return m_nonce_len == 12; }
      void update_len(size_t len);
   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override;

      void key_schedule(const byte key[], size_t length) override;
   };

/**
* ChaCha20Poly1305 Encryption
*/
class BOTAN_DLL ChaCha20Poly1305_Encryption : public ChaCha20Poly1305_Mode
   {
   public:
      size_t output_length(size_t input_length) const override
         { return input_length + tag_size(); }

      size_t minimum_final_size() const override { return 0; }

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   };

/**
* ChaCha20Poly1305 Decryption
*/
class BOTAN_DLL ChaCha20Poly1305_Decryption : public ChaCha20Poly1305_Mode
   {
   public:
      size_t output_length(size_t input_length) const override
         {
         BOTAN_ASSERT(input_length > tag_size(), "Sufficient input");
         return input_length - tag_size();
         }

      size_t minimum_final_size() const override { return tag_size(); }

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   };

}

#endif
