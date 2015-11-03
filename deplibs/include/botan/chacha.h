/*
* ChaCha20
* (C) 2014 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_CHACHA_H__
#define BOTAN_CHACHA_H__

#include <botan/stream_cipher.h>

namespace Botan {

/**
* DJB's ChaCha (http://cr.yp.to/chacha.html)
*/
class BOTAN_DLL ChaCha : public StreamCipher
   {
   public:
      void cipher(const byte in[], byte out[], size_t length) override;

      void set_iv(const byte iv[], size_t iv_len) override;

      bool valid_iv_length(size_t iv_len) const override
         { return (iv_len == 8 || iv_len == 12); }

      Key_Length_Specification key_spec() const override
         {
         return Key_Length_Specification(16, 32, 16);
         }

      void clear() override;
      std::string name() const override { return "ChaCha"; }

      StreamCipher* clone() const override { return new ChaCha; }
   protected:
      virtual void chacha(byte output[64], const u32bit input[16]);
   private:
      void key_schedule(const byte key[], size_t key_len) override;

      secure_vector<u32bit> m_state;
      secure_vector<byte> m_buffer;
      size_t m_position = 0;
   };

}

#endif
