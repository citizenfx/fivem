/*
* MD4
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_MD4_H__
#define BOTAN_MD4_H__

#include <botan/mdx_hash.h>

namespace Botan {

/**
* MD4
*/
class BOTAN_DLL MD4 final : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "MD4"; }
      size_t output_length() const override { return 16; }
      HashFunction* clone() const override { return new MD4; }

      void clear() override;

      MD4() : MDx_HashFunction(64, false, true), m_M(16), m_digest(4)
         { clear(); }
   protected:
      void compress_n(const uint8_t input[], size_t blocks) override;
      void copy_out(uint8_t[]) override;
   private:

      /**
      * The message buffer
      */
      secure_vector<uint32_t> m_M;

      /**
      * The digest value
      */
      secure_vector<uint32_t> m_digest;
   };

}

#endif
