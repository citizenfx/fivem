/*
* OCB Mode
* (C) 2013,2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_AEAD_OCB_H__
#define BOTAN_AEAD_OCB_H__

#include <botan/aead.h>
#include <botan/block_cipher.h>

namespace Botan {

class L_computer;

/**
* OCB Mode (base class for OCB_Encryption and OCB_Decryption). Note
* that OCB is patented, but is freely licensed in some circumstances.
*
* @see "The OCB Authenticated-Encryption Algorithm" internet draft
        http://tools.ietf.org/html/draft-irtf-cfrg-ocb-03
* @see Free Licenses http://www.cs.ucdavis.edu/~rogaway/ocb/license.htm
* @see OCB home page http://www.cs.ucdavis.edu/~rogaway/ocb
*/
class BOTAN_DLL OCB_Mode : public AEAD_Mode
   {
   public:
      void set_associated_data(const byte ad[], size_t ad_len) override;

      std::string name() const override;

      size_t update_granularity() const override;

      Key_Length_Specification key_spec() const override;

      bool valid_nonce_length(size_t) const override;

      size_t tag_size() const override { return m_tag_size; }

      void clear() override;

      ~OCB_Mode();
   protected:
      /**
      * @param cipher the 128-bit block cipher to use
      * @param tag_size is how big the auth tag will be
      */
      OCB_Mode(BlockCipher* cipher, size_t tag_size);

      size_t BS() const { return m_BS; }

      // fixme make these private
      std::unique_ptr<BlockCipher> m_cipher;
      std::unique_ptr<L_computer> m_L;

      size_t m_BS;
      size_t m_block_index = 0;

      secure_vector<byte> m_checksum;
      secure_vector<byte> m_offset;
      secure_vector<byte> m_ad_hash;
   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override;

      void key_schedule(const byte key[], size_t length) override;

      secure_vector<byte> update_nonce(const byte nonce[], size_t nonce_len);

      size_t m_tag_size = 0;
      secure_vector<byte> m_last_nonce;
      secure_vector<byte> m_stretch;
   };

class BOTAN_DLL OCB_Encryption : public OCB_Mode
   {
   public:
      /**
      * @param cipher the 128-bit block cipher to use
      * @param tag_size is how big the auth tag will be
      */
      OCB_Encryption(BlockCipher* cipher, size_t tag_size = 16) :
         OCB_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         { return input_length + tag_size(); }

      size_t minimum_final_size() const override { return 0; }

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   private:
      void encrypt(byte input[], size_t blocks);
   };

class BOTAN_DLL OCB_Decryption : public OCB_Mode
   {
   public:
      /**
      * @param cipher the 128-bit block cipher to use
      * @param tag_size is how big the auth tag will be
      */
      OCB_Decryption(BlockCipher* cipher, size_t tag_size = 16) :
         OCB_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         {
         BOTAN_ASSERT(input_length > tag_size(), "Sufficient input");
         return input_length - tag_size();
         }

      size_t minimum_final_size() const override { return tag_size(); }

      void update(secure_vector<byte>& blocks, size_t offset = 0) override;

      void finish(secure_vector<byte>& final_block, size_t offset = 0) override;
   private:
      void decrypt(byte input[], size_t blocks);
   };

}

#endif
