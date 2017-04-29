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
class BOTAN_DLL ChaCha final : public StreamCipher
   {
   public:
      StreamCipher* clone() const override { return new ChaCha(m_rounds); }

      /**
      * @param rounds number of rounds
      * @note Currently only 8, 12 or 20 rounds are supported, all others
      * will throw an exception
      */
      ChaCha(size_t rounds = 20);

      std::string provider() const override;

      void cipher(const uint8_t in[], uint8_t out[], size_t length) override;

      void set_iv(const uint8_t iv[], size_t iv_len) override;

      bool valid_iv_length(size_t iv_len) const override
         { return (iv_len == 8 || iv_len == 12); }

      Key_Length_Specification key_spec() const override
         {
         return Key_Length_Specification(16, 32, 16);
         }

      void clear() override;

      std::string name() const override;

      void seek(uint64_t offset) override;

   private:
      void key_schedule(const uint8_t key[], size_t key_len) override;

      void chacha_x4(uint8_t output[64*4], uint32_t state[16], size_t rounds);

#if defined(BOTAN_HAS_CHACHA_SSE2)
      void chacha_sse2_x4(uint8_t output[64*4], uint32_t state[16], size_t rounds);
#endif

      size_t m_rounds;
      secure_vector<uint32_t> m_state;
      secure_vector<uint8_t> m_buffer;
      size_t m_position = 0;
   };

}

#endif
