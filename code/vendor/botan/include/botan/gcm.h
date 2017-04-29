/*
* GCM Mode
* (C) 2013 Jack Lloyd
* (C) 2016 Daniel Neus, Rohde & Schwarz Cybersecurity
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_AEAD_GCM_H__
#define BOTAN_AEAD_GCM_H__

#include <botan/aead.h>
#include <botan/block_cipher.h>
#include <botan/stream_cipher.h>

namespace Botan {

class GHASH;

/**
* GCM Mode
*/
class BOTAN_DLL GCM_Mode : public AEAD_Mode
   {
   public:
      void set_associated_data(const uint8_t ad[], size_t ad_len) override;

      std::string name() const override;

      size_t update_granularity() const override;

      Key_Length_Specification key_spec() const override;

      // GCM supports arbitrary nonce lengths
      bool valid_nonce_length(size_t) const override { return true; }

      size_t tag_size() const override { return m_tag_size; }

      void clear() override;

      void reset() override;

      std::string provider() const override;
   protected:
      GCM_Mode(BlockCipher* cipher, size_t tag_size);

      const size_t m_BS = 16;

      const size_t m_tag_size;
      const std::string m_cipher_name;

      std::unique_ptr<StreamCipher> m_ctr;
      std::unique_ptr<GHASH> m_ghash;
   private:
      void start_msg(const uint8_t nonce[], size_t nonce_len) override;

      void key_schedule(const uint8_t key[], size_t length) override;
   };

/**
* GCM Encryption
*/
class BOTAN_DLL GCM_Encryption final : public GCM_Mode
   {
   public:
      /**
      * @param cipher the 128 bit block cipher to use
      * @param tag_size is how big the auth tag will be
      */
      GCM_Encryption(BlockCipher* cipher, size_t tag_size = 16) :
         GCM_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         { return input_length + tag_size(); }

      size_t minimum_final_size() const override { return 0; }

      size_t process(uint8_t buf[], size_t size) override;

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;
   };

/**
* GCM Decryption
*/
class BOTAN_DLL GCM_Decryption final : public GCM_Mode
   {
   public:
      /**
      * @param cipher the 128 bit block cipher to use
      * @param tag_size is how big the auth tag will be
      */
      GCM_Decryption(BlockCipher* cipher, size_t tag_size = 16) :
         GCM_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         {
         BOTAN_ASSERT(input_length >= tag_size(), "Sufficient input");
         return input_length - tag_size();
         }

      size_t minimum_final_size() const override { return tag_size(); }

      size_t process(uint8_t buf[], size_t size) override;

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;
   };

/**
* GCM's GHASH
* Maybe a Transform?
*/
class BOTAN_DLL GHASH : public SymmetricAlgorithm
   {
   public:
      void set_associated_data(const uint8_t ad[], size_t ad_len);

      secure_vector<uint8_t> nonce_hash(const uint8_t nonce[], size_t len);

      void start(const uint8_t nonce[], size_t len);

      /*
      * Assumes input len is multiple of 16
      */
      void update(const uint8_t in[], size_t len);

      secure_vector<uint8_t> final();

      Key_Length_Specification key_spec() const override
         { return Key_Length_Specification(16); }

      void clear() override;

      void reset();

      std::string name() const override { return "GHASH"; }
   protected:
      void ghash_update(secure_vector<uint8_t>& x,
                        const uint8_t input[], size_t input_len);

      void add_final_block(secure_vector<uint8_t>& x,
                           size_t ad_len, size_t pt_len);

      secure_vector<uint8_t> m_H;
      secure_vector<uint8_t> m_H_ad;
      secure_vector<uint8_t> m_ghash;
      size_t m_ad_len = 0;

   private:
      void key_schedule(const uint8_t key[], size_t key_len) override;

      void gcm_multiply(secure_vector<uint8_t>& x) const;

      secure_vector<uint8_t> m_nonce;
      size_t m_text_len = 0;
   };

}

#endif
