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
class BOTAN_DLL OFB : public StreamCipher
   {
   public:
      void cipher(const byte in[], byte out[], size_t length) override;

      void set_iv(const byte iv[], size_t iv_len) override;

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

      static OFB* make(const Spec& spec);

      /**
      * @param cipher the underlying block cipher to use
      */
      OFB(BlockCipher* cipher);
   private:
      void key_schedule(const byte key[], size_t key_len) override;

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<byte> m_buffer;
      size_t m_buf_pos;
   };

}

#endif
