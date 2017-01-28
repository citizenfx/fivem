/*
* SHA-160
* (C) 1999-2007,2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_SHA_160_H__
#define BOTAN_SHA_160_H__

#include <botan/mdx_hash.h>

namespace Botan {

/**
* NIST's SHA-160
*/
class BOTAN_DLL SHA_160 final : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "SHA-160"; }
      size_t output_length() const override { return 20; }
      HashFunction* clone() const override { return new SHA_160; }

      void clear() override;

      SHA_160() : MDx_HashFunction(64, true, true), m_digest(5)
         {
         clear();
         }

   private:
      void compress_n(const uint8_t[], size_t blocks) override;

#if defined(BOTAN_HAS_SHA1_SSE2)
      static void sse2_compress_n(secure_vector<uint32_t>& digest,
                                  const uint8_t blocks[],
                                  size_t block_count);
#endif


      void copy_out(uint8_t[]) override;

      /**
      * The digest value
      */
      secure_vector<uint32_t> m_digest;

      /**
      * The message buffer
      */
      secure_vector<uint32_t> m_W;
   };

typedef SHA_160 SHA_1;

}

#endif
