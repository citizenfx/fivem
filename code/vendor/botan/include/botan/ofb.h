/*
* OFB Mode
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_OUTPUT_FEEDBACK_MODE_H__
#define BOTAN_OUTPUT_FEEDBACK_MODE_H__

#include <botan/stream_cipher.h>
#include <botan/block_cipher.h>

namespace Botan {

/**
* Output Feedback Mode
*/
class BOTAN_DLL OFB final : public StreamCipher
   {
   public:
      void cipher(const uint8_t in[], uint8_t out[], size_t length) override;

      void set_iv(const uint8_t iv[], size_t iv_len) override;

      bool valid_iv_length(size_t iv_len) const override
         { return (iv_len <= m_cipher->block_size()); }

      Key_Length_Specification key_spec() const override
         {
         return m_cipher->key_spec();
         }

      std::string name() const override;

      OFB* clone() const override
         { return new OFB(m_cipher->clone()); }

      void clear() override;

      /**
      * @param cipher the block cipher to use
      */
      explicit OFB(BlockCipher* cipher);

      void seek(uint64_t offset) override;
   private:
      void key_schedule(const uint8_t key[], size_t key_len) override;

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<uint8_t> m_buffer;
      size_t m_buf_pos;
   };

}

#endif
