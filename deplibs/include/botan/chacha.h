/*
* ChaCha20
* (C) 2014 Jack Lloyd
*
* Distributed under the terms of the Botan license
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
      void cipher(const byte in[], byte out[], size_t length);

      void set_iv(const byte iv[], size_t iv_len);

      bool valid_iv_length(size_t iv_len) const
         { return (iv_len == 8); }

      Key_Length_Specification key_spec() const
         {
         return Key_Length_Specification(16, 32, 16);
         }

      void clear();
      std::string name() const;

      StreamCipher* clone() const { return new ChaCha; }
   protected:
      virtual void chacha(byte output[64], const u32bit input[16]);
   private:
      void key_schedule(const byte key[], size_t key_len);

      secure_vector<u32bit> m_state;
      secure_vector<byte> m_buffer;
      size_t m_position = 0;
   };

}

#endif
