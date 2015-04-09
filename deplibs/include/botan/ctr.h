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
class BOTAN_DLL CTR_BE : public StreamCipher
   {
   public:
      void cipher(const byte in[], byte out[], size_t length);

      void set_iv(const byte iv[], size_t iv_len);

      bool valid_iv_length(size_t iv_len) const
         { return (iv_len <= m_cipher->block_size()); }

      Key_Length_Specification key_spec() const
         {
         return m_cipher->key_spec();
         }

      std::string name() const;

      CTR_BE* clone() const
         { return new CTR_BE(m_cipher->clone()); }

      void clear();

      static CTR_BE* make(const Spec& spec);

      /**
      * @param cipher the underlying block cipher to use
      */
      CTR_BE(BlockCipher* cipher);
   private:
      void key_schedule(const byte key[], size_t key_len);
      void increment_counter();

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<byte> m_counter, m_pad;
      size_t m_pad_pos;
   };

}

#endif
