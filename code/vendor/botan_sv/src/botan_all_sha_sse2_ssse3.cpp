/*
* Botan 2.12.1 Amalgamation
* (C) 1999-2018 The Botan Authors
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "botan_all.h"
#include "botan_all_internal.h"

#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC target ("ssse3")
#endif
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC target ("sha")
#endif
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC target ("sse2")
#endif
/*
* SHACAL-2 using x86 SHA extensions
* (C) 2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <immintrin.h>

namespace Botan {

/*
Only encryption is supported since the inverse round function would
require a different instruction
*/

BOTAN_FUNC_ISA("sha,ssse3")
void SHACAL2::x86_encrypt_blocks(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   const __m128i MASK1 = _mm_set_epi8(8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);
   const __m128i MASK2 = _mm_set_epi8(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);

   const __m128i* RK_mm = reinterpret_cast<const __m128i*>(m_RK.data());
   const __m128i* in_mm = reinterpret_cast<const __m128i*>(in);
   __m128i* out_mm = reinterpret_cast<__m128i*>(out);

   while(blocks >= 2)
      {
      __m128i B0_0 = _mm_loadu_si128(in_mm);
      __m128i B0_1 = _mm_loadu_si128(in_mm+1);
      __m128i B1_0 = _mm_loadu_si128(in_mm+2);
      __m128i B1_1 = _mm_loadu_si128(in_mm+3);

      __m128i TMP = _mm_shuffle_epi8(_mm_unpacklo_epi64(B0_0, B0_1), MASK2);
      B0_1 = _mm_shuffle_epi8(_mm_unpackhi_epi64(B0_0, B0_1), MASK2);
      B0_0 = TMP;

      TMP = _mm_shuffle_epi8(_mm_unpacklo_epi64(B1_0, B1_1), MASK2);
      B1_1 = _mm_shuffle_epi8(_mm_unpackhi_epi64(B1_0, B1_1), MASK2);
      B1_0 = TMP;

      for(size_t i = 0; i != 8; ++i)
         {
         const __m128i RK0 = _mm_loadu_si128(RK_mm + 2*i);
         const __m128i RK2 = _mm_loadu_si128(RK_mm + 2*i+1);
         const __m128i RK1 = _mm_srli_si128(RK0, 8);
         const __m128i RK3 = _mm_srli_si128(RK2, 8);

         B0_1 = _mm_sha256rnds2_epu32(B0_1, B0_0, RK0);
         B1_1 = _mm_sha256rnds2_epu32(B1_1, B1_0, RK0);

         B0_0 = _mm_sha256rnds2_epu32(B0_0, B0_1, RK1);
         B1_0 = _mm_sha256rnds2_epu32(B1_0, B1_1, RK1);

         B0_1 = _mm_sha256rnds2_epu32(B0_1, B0_0, RK2);
         B1_1 = _mm_sha256rnds2_epu32(B1_1, B1_0, RK2);

         B0_0 = _mm_sha256rnds2_epu32(B0_0, B0_1, RK3);
         B1_0 = _mm_sha256rnds2_epu32(B1_0, B1_1, RK3);
         }

      TMP = _mm_shuffle_epi8(_mm_unpackhi_epi64(B0_0, B0_1), MASK1);
      B0_1 = _mm_shuffle_epi8(_mm_unpacklo_epi64(B0_0, B0_1), MASK1);
      B0_0 = TMP;

      TMP = _mm_shuffle_epi8(_mm_unpackhi_epi64(B1_0, B1_1), MASK1);
      B1_1 = _mm_shuffle_epi8(_mm_unpacklo_epi64(B1_0, B1_1), MASK1);
      B1_0 = TMP;

      // Save state
      _mm_storeu_si128(out_mm + 0, B0_0);
      _mm_storeu_si128(out_mm + 1, B0_1);
      _mm_storeu_si128(out_mm + 2, B1_0);
      _mm_storeu_si128(out_mm + 3, B1_1);

      blocks -= 2;
      in_mm += 4;
      out_mm += 4;
      }

   while(blocks)
      {
      __m128i B0 = _mm_loadu_si128(in_mm);
      __m128i B1 = _mm_loadu_si128(in_mm+1);

      __m128i TMP = _mm_shuffle_epi8(_mm_unpacklo_epi64(B0, B1), MASK2);
      B1 = _mm_shuffle_epi8(_mm_unpackhi_epi64(B0, B1), MASK2);
      B0 = TMP;

      for(size_t i = 0; i != 8; ++i)
         {
         const __m128i RK0 = _mm_loadu_si128(RK_mm + 2*i);
         const __m128i RK2 = _mm_loadu_si128(RK_mm + 2*i+1);
         const __m128i RK1 = _mm_srli_si128(RK0, 8);
         const __m128i RK3 = _mm_srli_si128(RK2, 8);

         B1 = _mm_sha256rnds2_epu32(B1, B0, RK0);
         B0 = _mm_sha256rnds2_epu32(B0, B1, RK1);
         B1 = _mm_sha256rnds2_epu32(B1, B0, RK2);
         B0 = _mm_sha256rnds2_epu32(B0, B1, RK3);
         }

      TMP = _mm_shuffle_epi8(_mm_unpackhi_epi64(B0, B1), MASK1);
      B1 = _mm_shuffle_epi8(_mm_unpacklo_epi64(B0, B1), MASK1);
      B0 = TMP;

      // Save state
      _mm_storeu_si128(out_mm, B0);
      _mm_storeu_si128(out_mm + 1, B1);

      blocks--;
      in_mm += 2;
      out_mm += 2;
      }
   }

}
