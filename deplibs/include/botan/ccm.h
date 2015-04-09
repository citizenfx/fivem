/*
* CCM Mode
* (C) 2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_AEAD_CCM_H__
#define BOTAN_AEAD_CCM_H__

#include <botan/aead.h>
#include <botan/block_cipher.h>
#include <botan/stream_cipher.h>
#include <botan/mac.h>

namespace Botan {

/**
* Base class for CCM encryption and decryption
* @see RFC 3610
*/
class BOTAN_DLL CCM_Mode : public AEAD_Mode
   {
   public:
      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void set_associated_data(const byte ad[], size_t ad_len) override;

      std::string name() const override;

      size_t update_granularity() const;

      Key_Length_Specification key_spec() const override;

      bool valid_nonce_length(size_t) const override;

      size_t default_nonce_length() const override;

      void clear() override;

      size_t tag_size() const { return m_tag_size; }

   protected:
      const size_t BS = 16; // intrinsic to CCM definition

      CCM_Mode(BlockCipher* cipher, size_t tag_size, size_t L);

      size_t L() const { return m_L; }

      const BlockCipher& cipher() const { return *m_cipher; }

      void encode_length(size_t len, byte out[]);

      void inc(secure_vector<byte>& C);

      const secure_vector<byte>& ad_buf() const { return m_ad_buf; }

      secure_vector<byte>& msg_buf() { return m_msg_buf; }

      secure_vector<byte> format_b0(size_t msg_size);
      secure_vector<byte> format_c0();
   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override;

      void key_schedule(const byte key[], size_t length) override;

      const size_t m_tag_size;
      const size_t m_L;

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<byte> m_nonce, m_msg_buf, m_ad_buf;
   };

/**
* CCM Encryption
*/
class BOTAN_DLL CCM_Encryption : public CCM_Mode
   {
   public:
      /**
      * @param cipher a 128-bit block cipher
      * @param tag_size is how big the auth tag will be (even values
      *                 between 4 and 16 are accepted)
      * @param L length of L parameter. The total message length
      *           must be less than 2**L bytes, and the nonce is 15-L bytes.
      */
      CCM_Encryption(BlockCipher* cipher, size_t tag_size = 16, size_t L = 3) :
         CCM_Mode(cipher, tag_size, L) {}

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override
         { return input_length + tag_size(); }

      size_t minimum_final_size() const override { return 0; }
   };

/**
* CCM Decryption
*/
class BOTAN_DLL CCM_Decryption : public CCM_Mode
   {
   public:
      /**
      * @param cipher a 128-bit block cipher
      * @param tag_size is how big the auth tag will be (even values
      *                 between 4 and 16 are accepted)
      * @param L length of L parameter. The total message length
      *           must be less than 2**L bytes, and the nonce is 15-L bytes.
      */
      CCM_Decryption(BlockCipher* cipher, size_t tag_size = 16, size_t L = 3) :
         CCM_Mode(cipher, tag_size, L) {}

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override
         {
         BOTAN_ASSERT(input_length > tag_size(), "Sufficient input");
         return input_length - tag_size();
         }

      size_t minimum_final_size() const override { return tag_size(); }
   };

}

#endif
