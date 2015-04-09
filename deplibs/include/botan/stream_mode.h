/*
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_STREAM_MODE_H__
#define BOTAN_STREAM_MODE_H__

#include <botan/cipher_mode.h>
#include <botan/stream_cipher.h>

namespace Botan {

class BOTAN_DLL Stream_Cipher_Mode : public Cipher_Mode
   {
   public:
      Stream_Cipher_Mode(StreamCipher* cipher) : m_cipher(cipher) {}

      void update(secure_vector<byte>& buf, size_t offset) override
         {
         if(offset < buf.size())
            m_cipher->cipher1(&buf[offset], buf.size() - offset);
         }

      void finish(secure_vector<byte>& buf, size_t offset) override
         { return update(buf, offset); }

      size_t output_length(size_t input_length) const override { return input_length; }

      size_t update_granularity() const override { return 64; /* arbitrary */ }

      size_t minimum_final_size() const override { return 0; }

      size_t default_nonce_length() const override { return 0; }

      bool valid_nonce_length(size_t nonce_len) const override
         { return m_cipher->valid_iv_length(nonce_len); }

      Key_Length_Specification key_spec() const override { return m_cipher->key_spec(); }

      std::string name() const override { return m_cipher->name(); }

      void clear() override { return m_cipher->clear(); }

   private:
      secure_vector<byte> start_raw(const byte nonce[], size_t nonce_len) override
         {
         m_cipher->set_iv(nonce, nonce_len);
         return secure_vector<byte>();
         }

      void key_schedule(const byte key[], size_t length)
         {
         m_cipher->set_key(key, length);
         }

      std::unique_ptr<StreamCipher> m_cipher;
   };

}

#endif
