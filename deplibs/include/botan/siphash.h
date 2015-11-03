/*
* SipHash
* (C) 2014,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_SIPHASH_H__
#define BOTAN_SIPHASH_H__

#include <botan/mac.h>

namespace Botan {

class BOTAN_DLL SipHash : public MessageAuthenticationCode
   {
   public:
      SipHash(size_t c = 2, size_t d = 4) : m_C(c), m_D(d) {}

      void clear() override;
      std::string name() const override;

      MessageAuthenticationCode* clone() const override;

      size_t output_length() const override { return 8; }

      Key_Length_Specification key_spec() const override
         {
         return Key_Length_Specification(16);
         }
   private:
      void add_data(const byte[], size_t) override;
      void final_result(byte[]) override;
      void key_schedule(const byte[], size_t) override;

      const size_t m_C, m_D;
      secure_vector<u64bit> m_V;
      u64bit m_mbuf = 0;
      size_t m_mbuf_pos = 0;
      byte m_words = 0;
   };

}

#endif
