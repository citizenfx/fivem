/*
* CTR-BE Mode
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_CTR_BE_H__
#define BOTAN_CTR_BE_H__

#include <botan/block_cipher.h>
#include <botan/stream_cipher.h>

namespace Botan {

/**
* CTR-BE (Counter mode, big-endian)
*/
class BOTAN_DLL CTR_BE final : public StreamCipher
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

      CTR_BE* clone() const override
         { return new CTR_BE(m_cipher->clone()); }

      void clear() override;

      /**
      * @param cipher the block cipher to use
      */
      explicit CTR_BE(BlockCipher* cipher);

      CTR_BE(BlockCipher* cipher, size_t ctr_size);

      void seek(uint64_t offset) override;
   private:
      void key_schedule(const uint8_t key[], size_t key_len) override;
      void increment_counter();

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<uint8_t> m_counter, m_pad;
      size_t m_ctr_size;
      size_t m_pad_pos;
   };

}

#endif
