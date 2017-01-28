/*
* SIV Mode
* (C) 2013 Jack Lloyd
* (C) 2016 Daniel Neus, Rohde & Schwarz Cybersecurity
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_AEAD_SIV_H__
#define BOTAN_AEAD_SIV_H__

#include <botan/aead.h>
#include <botan/block_cipher.h>
#include <botan/stream_cipher.h>
#include <botan/mac.h>

namespace Botan {

/**
* Base class for SIV encryption and decryption (@see RFC 5297)
*/
class BOTAN_DLL SIV_Mode : public AEAD_Mode
   {
   public:
      size_t process(uint8_t buf[], size_t size) override;

      /**
      * Sets the nth element of the vector of associated data
      * @param n index into the AD vector
      * @param ad associated data
      * @param ad_len length of associated data in bytes
      */
      void set_associated_data_n(size_t n, const uint8_t ad[], size_t ad_len);

      void set_associated_data(const uint8_t ad[], size_t ad_len) override
         {
         set_associated_data_n(0, ad, ad_len);
         }

      std::string name() const override;

      size_t update_granularity() const override;

      Key_Length_Specification key_spec() const override;

      bool valid_nonce_length(size_t) const override;

      void clear() override;

      void reset() override;

      size_t tag_size() const override { return 16; }

   protected:
      explicit SIV_Mode(BlockCipher* cipher);

      StreamCipher& ctr() { return *m_ctr; }

      void set_ctr_iv(secure_vector<uint8_t> V);

      secure_vector<uint8_t>& msg_buf() { return m_msg_buf; }

      secure_vector<uint8_t> S2V(const uint8_t text[], size_t text_len);
   private:
      void start_msg(const uint8_t nonce[], size_t nonce_len) override;

      void key_schedule(const uint8_t key[], size_t length) override;

      const std::string m_name;
      std::unique_ptr<StreamCipher> m_ctr;
      std::unique_ptr<MessageAuthenticationCode> m_cmac;
      secure_vector<uint8_t> m_nonce, m_msg_buf;
      std::vector<secure_vector<uint8_t>> m_ad_macs;
   };

/**
* SIV Encryption
*/
class BOTAN_DLL SIV_Encryption final : public SIV_Mode
   {
   public:
      /**
      * @param cipher a block cipher
      */
      explicit SIV_Encryption(BlockCipher* cipher) : SIV_Mode(cipher) {}

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override
         { return input_length + tag_size(); }

      size_t minimum_final_size() const override { return 0; }
   };

/**
* SIV Decryption
*/
class BOTAN_DLL SIV_Decryption final : public SIV_Mode
   {
   public:
      /**
      * @param cipher a 128-bit block cipher
      */
      explicit SIV_Decryption(BlockCipher* cipher) : SIV_Mode(cipher) {}

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override
         {
         BOTAN_ASSERT(input_length >= tag_size(), "Sufficient input");
         return input_length - tag_size();
         }

      size_t minimum_final_size() const override { return tag_size(); }
   };

}

#endif
