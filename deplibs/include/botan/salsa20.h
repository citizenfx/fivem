/*
* Salsa20 / XSalsa20
* (C) 1999-2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_SALSA20_H__
#define BOTAN_SALSA20_H__

#include <botan/stream_cipher.h>

namespace Botan {

/**
* DJB's Salsa20 (and XSalsa20)
*/
class BOTAN_DLL Salsa20 : public StreamCipher
   {
   public:
      void cipher(const byte in[], byte out[], size_t length);

      void set_iv(const byte iv[], size_t iv_len);

      bool valid_iv_length(size_t iv_len) const
         { return (iv_len == 8 || iv_len == 24); }

      Key_Length_Specification key_spec() const
         {
         return Key_Length_Specification(16, 32, 16);
         }

      void clear();
      std::string name() const;
      StreamCipher* clone() const { return new Salsa20; }
   private:
      void key_schedule(const byte key[], size_t key_len);

      secure_vector<u32bit> m_state;
      secure_vector<byte> m_buffer;
      size_t m_position;
   };

}

#endif
