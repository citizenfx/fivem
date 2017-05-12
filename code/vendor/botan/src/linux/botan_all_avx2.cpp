/*
* Botan 2.0.1 Amalgamation
* (C) 1999-2013,2014,2015,2016 Jack Lloyd and others
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "botan_all.h"
#include "botan_all_internal.h"

#if defined(__GNUG__)
#pragma GCC target ("avx2")
#endif
/*
* Threefish-512 using AVX2
* (C) 2013,2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <immintrin.h>

namespace Botan {

namespace {

BOTAN_FUNC_ISA("avx2")
inline void interleave_epi64(__m256i& X0, __m256i& X1)
   {
   // interleave X0 and X1 qwords
   // (X0,X1,X2,X3),(X4,X5,X6,X7) -> (X0,X2,X4,X6),(X1,X3,X5,X7)

   const __m256i T0 = _mm256_unpacklo_epi64(X0, X1);
   const __m256i T1 = _mm256_unpackhi_epi64(X0, X1);

   X0 = _mm256_permute4x64_epi64(T0, _MM_SHUFFLE(3,1,2,0));
   X1 = _mm256_permute4x64_epi64(T1, _MM_SHUFFLE(3,1,2,0));
   }

BOTAN_FUNC_ISA("avx2")
inline void deinterleave_epi64(__m256i& X0, __m256i& X1)
   {
   const __m256i T0 = _mm256_permute4x64_epi64(X0, _MM_SHUFFLE(3,1,2,0));
   const __m256i T1 = _mm256_permute4x64_epi64(X1, _MM_SHUFFLE(3,1,2,0));

   X0 = _mm256_unpacklo_epi64(T0, T1);
   X1 = _mm256_unpackhi_epi64(T0, T1);
   }

BOTAN_FUNC_ISA("avx2")
inline void rotate_keys(__m256i& R0, __m256i& R1, __m256i R2)
   {
   /*
   Behold. The key schedule progresses like so. The values
   loop back to the originals after the rounds are complete
   so we don't need to reload for starting the next block.

                 R0        R1        R2
     K1,K2,K3 (7,5,3,1),(8,6,4,2),(0,7,5,3)
     K3,K4,K5 (0,7,5,3),(1,8,6,4),(2,0,7,5)
     K5,K6,K7 (2,0,7,5),(3,1,8,6),(4,2,0,7)

     K7,K8,K0 (4,2,0,7),(5,3,1,8),(6,4,2,0)
     K0,K1,K2 (6,4,2,0),(7,5,3,1),(8,6,4,2)
     K2,K3,K4 (8,6,4,2),(0,7,5,3),(1,8,6,4)

     K4,K5,K6 (1,8,6,4),(2,0,7,5),(3,1,8,6)
     K6,K7,K8 (3,1,8,6),(4,2,0,7),(5,3,1,8)
     K8,K0,K1 (5,3,1,8),(6,4,2,0),(7,5,3,1)

   To compute the values for the next round:
     X0 is X2 from the last round
     X1 becomes (X0[4],X1[1:3])
     X2 becomes (X1[4],X2[1:3])
 
   Uses 3 permutes and 2 blends, is there a faster way?   
   */
   __m256i T0 = _mm256_permute4x64_epi64(R0, _MM_SHUFFLE(0,0,0,0));
   __m256i T1 = _mm256_permute4x64_epi64(R1, _MM_SHUFFLE(0,3,2,1));
   __m256i T2 = _mm256_permute4x64_epi64(R2, _MM_SHUFFLE(0,3,2,1));
 
   R0 = _mm256_blend_epi32(T1, T0, 0xC0);
   R1 = _mm256_blend_epi32(T2, T1, 0xC0);
   }


}

BOTAN_FUNC_ISA("avx2")
void Threefish_512::avx2_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   const uint64_t* K = &get_K()[0];
   const uint64_t* T_64 = &get_T()[0];

   const __m256i ROTATE_1 = _mm256_set_epi64x(37,19,36,46);
   const __m256i ROTATE_2 = _mm256_set_epi64x(42,14,27,33);
   const __m256i ROTATE_3 = _mm256_set_epi64x(39,36,49,17);
   const __m256i ROTATE_4 = _mm256_set_epi64x(56,54, 9,44);
   const __m256i ROTATE_5 = _mm256_set_epi64x(24,34,30,39);
   const __m256i ROTATE_6 = _mm256_set_epi64x(17,10,50,13);
   const __m256i ROTATE_7 = _mm256_set_epi64x(43,39,29,25);
   const __m256i ROTATE_8 = _mm256_set_epi64x(22,56,35, 8);

#define THREEFISH_ROUND(X0, X1, SHL)                                                \
   do {                                                                             \
      const __m256i SHR = _mm256_sub_epi64(_mm256_set1_epi64x(64), SHL);            \
      X0 = _mm256_add_epi64(X0, X1);                                                \
      X1 = _mm256_or_si256(_mm256_sllv_epi64(X1, SHL), _mm256_srlv_epi64(X1, SHR)); \
      X1 = _mm256_xor_si256(X1, X0);                                                \
      X0 = _mm256_permute4x64_epi64(X0, _MM_SHUFFLE(0, 3, 2, 1));                   \
      X1 = _mm256_permute4x64_epi64(X1, _MM_SHUFFLE(1, 2, 3, 0));                   \
   } while(0)

#define THREEFISH_ROUND_2(X0, X1, X2, X3, SHL)                           \
   do {                                                                             \
      const __m256i SHR = _mm256_sub_epi64(_mm256_set1_epi64x(64), SHL);            \
      X0 = _mm256_add_epi64(X0, X1);                                                \
      X2 = _mm256_add_epi64(X2, X3);                                                \
      X1 = _mm256_or_si256(_mm256_sllv_epi64(X1, SHL), _mm256_srlv_epi64(X1, SHR)); \
      X3 = _mm256_or_si256(_mm256_sllv_epi64(X3, SHL), _mm256_srlv_epi64(X3, SHR)); \
      X1 = _mm256_xor_si256(X1, X0);                                                \
      X3 = _mm256_xor_si256(X3, X2);                                                \
      X0 = _mm256_permute4x64_epi64(X0, _MM_SHUFFLE(0, 3, 2, 1));                   \
      X2 = _mm256_permute4x64_epi64(X2, _MM_SHUFFLE(0, 3, 2, 1));                   \
      X1 = _mm256_permute4x64_epi64(X1, _MM_SHUFFLE(1, 2, 3, 0));                   \
      X3 = _mm256_permute4x64_epi64(X3, _MM_SHUFFLE(1, 2, 3, 0));                   \
   } while(0)

#define THREEFISH_INJECT_KEY(X0, X1, R, K0, K1, T0I, T1I)                        \
   do {                                                                          \
      const __m256i T0 = _mm256_permute4x64_epi64(T, _MM_SHUFFLE(T0I, 0, 0, 0)); \
      const __m256i T1 = _mm256_permute4x64_epi64(T, _MM_SHUFFLE(0, T1I, 0, 0)); \
      X0 = _mm256_add_epi64(X0, K0);                                             \
      X1 = _mm256_add_epi64(X1, K1);                                             \
      X1 = _mm256_add_epi64(X1, _mm256_set_epi64x(R,0,0,0));                     \
      X0 = _mm256_add_epi64(X0, T0);                                             \
      X1 = _mm256_add_epi64(X1, T1);                                             \
   } while(0)

#define THREEFISH_INJECT_KEY_2(X0, X1, X2, X3, R, K0, K1, T0I, T1I)              \
   do {                                                                          \
      const __m256i T0 = _mm256_permute4x64_epi64(T, _MM_SHUFFLE(T0I, 0, 0, 0)); \
      __m256i T1 = _mm256_permute4x64_epi64(T, _MM_SHUFFLE(0, T1I, 0, 0)); \
      X0 = _mm256_add_epi64(X0, K0);                                             \
      X2 = _mm256_add_epi64(X2, K0);                                             \
      X1 = _mm256_add_epi64(X1, K1);                                             \
      X3 = _mm256_add_epi64(X3, K1);                                             \
      T1 = _mm256_add_epi64(T1, _mm256_set_epi64x(R,0,0,0));                     \
      X0 = _mm256_add_epi64(X0, T0);                                             \
      X2 = _mm256_add_epi64(X2, T0);                                             \
      X1 = _mm256_add_epi64(X1, T1);                                             \
      X3 = _mm256_add_epi64(X3, T1);                                             \
   } while(0)

#define THREEFISH_ENC_8_ROUNDS(X0, X1, R, K0, K1, K2, T0, T1, T2)        \
   do {                                                        \
      rotate_keys(K1, K2, K0);                                 \
      THREEFISH_ROUND(X0, X1, ROTATE_1);                       \
      THREEFISH_ROUND(X0, X1, ROTATE_2);                       \
      THREEFISH_ROUND(X0, X1, ROTATE_3);                       \
      THREEFISH_ROUND(X0, X1, ROTATE_4);                       \
      THREEFISH_INJECT_KEY(X0, X1, R, K0, K1, T0, T1);         \
                                                               \
      THREEFISH_ROUND(X0, X1, ROTATE_5);                       \
      THREEFISH_ROUND(X0, X1, ROTATE_6);                       \
      THREEFISH_ROUND(X0, X1, ROTATE_7);                       \
      THREEFISH_ROUND(X0, X1, ROTATE_8);                       \
      THREEFISH_INJECT_KEY(X0, X1, R+1, K1, K2, T2, T0);       \
   } while(0)

#define THREEFISH_ENC_2_8_ROUNDS(X0, X1, X2, X3, R, K0, K1, K2, T0, T1, T2) \
   do {                                                                  \
      rotate_keys(K1, K2, K0);                                 \
      THREEFISH_ROUND_2(X0, X1, X2, X3, ROTATE_1);                       \
      THREEFISH_ROUND_2(X0, X1, X2, X3, ROTATE_2);                       \
      THREEFISH_ROUND_2(X0, X1, X2, X3, ROTATE_3);                       \
      THREEFISH_ROUND_2(X0, X1, X2, X3, ROTATE_4);                       \
      THREEFISH_INJECT_KEY_2(X0, X1, X2, X3, R, K0, K1, T0, T1);         \
                                                                         \
      THREEFISH_ROUND_2(X0, X1, X2, X3, ROTATE_5);                       \
      THREEFISH_ROUND_2(X0, X1, X2, X3, ROTATE_6);                       \
      THREEFISH_ROUND_2(X0, X1, X2, X3, ROTATE_7);                       \
      THREEFISH_ROUND_2(X0, X1, X2, X3, ROTATE_8);                       \
      THREEFISH_INJECT_KEY_2(X0, X1, X2, X3, R+1, K1, K2, T2, T0);       \
   } while(0)

   __m256i K0 = _mm256_set_epi64x(K[5], K[3], K[1], K[8]);
   __m256i K1 = _mm256_set_epi64x(K[6], K[4], K[2], K[0]);
   __m256i K2 = _mm256_set_epi64x(K[7], K[5], K[3], K[1]);

   const __m256i* in_mm = reinterpret_cast<const __m256i*>(in);
   __m256i* out_mm = reinterpret_cast<__m256i*>(out);
   
   while(blocks >= 2)
      {
      __m256i X0 = _mm256_loadu_si256(in_mm++);
      __m256i X1 = _mm256_loadu_si256(in_mm++);
      __m256i X2 = _mm256_loadu_si256(in_mm++);
      __m256i X3 = _mm256_loadu_si256(in_mm++);

      const __m256i T = _mm256_set_epi64x(T_64[0], T_64[1], T_64[2], 0);

      interleave_epi64(X0, X1);
      interleave_epi64(X2, X3);

      THREEFISH_INJECT_KEY_2(X0, X1, X2, X3, 0, K1, K2, 2, 3);

      THREEFISH_ENC_2_8_ROUNDS(X0, X1, X2, X3,  1, K2,K0,K1, 1, 2, 3);
      THREEFISH_ENC_2_8_ROUNDS(X0, X1, X2, X3,  3, K1,K2,K0, 2, 3, 1);
      THREEFISH_ENC_2_8_ROUNDS(X0, X1, X2, X3,  5, K0,K1,K2, 3, 1, 2);
      THREEFISH_ENC_2_8_ROUNDS(X0, X1, X2, X3,  7, K2,K0,K1, 1, 2, 3);
      THREEFISH_ENC_2_8_ROUNDS(X0, X1, X2, X3,  9, K1,K2,K0, 2, 3, 1);
      THREEFISH_ENC_2_8_ROUNDS(X0, X1, X2, X3, 11, K0,K1,K2, 3, 1, 2);
      THREEFISH_ENC_2_8_ROUNDS(X0, X1, X2, X3, 13, K2,K0,K1, 1, 2, 3);
      THREEFISH_ENC_2_8_ROUNDS(X0, X1, X2, X3, 15, K1,K2,K0, 2, 3, 1);
      THREEFISH_ENC_2_8_ROUNDS(X0, X1, X2, X3, 17, K0,K1,K2, 3, 1, 2);

      deinterleave_epi64(X0, X1);
      deinterleave_epi64(X2, X3);

      _mm256_storeu_si256(out_mm++, X0);
      _mm256_storeu_si256(out_mm++, X1);
      _mm256_storeu_si256(out_mm++, X2);
      _mm256_storeu_si256(out_mm++, X3);

      blocks -= 2;
      }
   
   for(size_t i = 0; i != blocks; ++i)
      {
      __m256i X0 = _mm256_loadu_si256(in_mm++);
      __m256i X1 = _mm256_loadu_si256(in_mm++);

      const __m256i T = _mm256_set_epi64x(T_64[0], T_64[1], T_64[2], 0);

      interleave_epi64(X0, X1);

      THREEFISH_INJECT_KEY(X0, X1, 0, K1, K2, 2, 3);

      THREEFISH_ENC_8_ROUNDS(X0, X1,  1, K2,K0,K1, 1, 2, 3);
      THREEFISH_ENC_8_ROUNDS(X0, X1,  3, K1,K2,K0, 2, 3, 1);
      THREEFISH_ENC_8_ROUNDS(X0, X1,  5, K0,K1,K2, 3, 1, 2);
      THREEFISH_ENC_8_ROUNDS(X0, X1,  7, K2,K0,K1, 1, 2, 3);
      THREEFISH_ENC_8_ROUNDS(X0, X1,  9, K1,K2,K0, 2, 3, 1);
      THREEFISH_ENC_8_ROUNDS(X0, X1, 11, K0,K1,K2, 3, 1, 2);
      THREEFISH_ENC_8_ROUNDS(X0, X1, 13, K2,K0,K1, 1, 2, 3);
      THREEFISH_ENC_8_ROUNDS(X0, X1, 15, K1,K2,K0, 2, 3, 1);
      THREEFISH_ENC_8_ROUNDS(X0, X1, 17, K0,K1,K2, 3, 1, 2);

      deinterleave_epi64(X0, X1);

      _mm256_storeu_si256(out_mm++, X0);
      _mm256_storeu_si256(out_mm++, X1);
      }

#undef THREEFISH_ENC_8_ROUNDS
#undef THREEFISH_ROUND
#undef THREEFISH_INJECT_KEY
#undef THREEFISH_DEC_2_8_ROUNDS
#undef THREEFISH_ROUND_2
#undef THREEFISH_INJECT_KEY_2
   }

BOTAN_FUNC_ISA("avx2")
void Threefish_512::avx2_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   const uint64_t* K = &get_K()[0];
   const uint64_t* T_64 = &get_T()[0];

   const __m256i ROTATE_1 = _mm256_set_epi64x(37,19,36,46);
   const __m256i ROTATE_2 = _mm256_set_epi64x(42,14,27,33);
   const __m256i ROTATE_3 = _mm256_set_epi64x(39,36,49,17);
   const __m256i ROTATE_4 = _mm256_set_epi64x(56,54, 9,44);
   const __m256i ROTATE_5 = _mm256_set_epi64x(24,34,30,39);
   const __m256i ROTATE_6 = _mm256_set_epi64x(17,10,50,13);
   const __m256i ROTATE_7 = _mm256_set_epi64x(43,39,29,25);
   const __m256i ROTATE_8 = _mm256_set_epi64x(22,56,35, 8);

#define THREEFISH_ROUND(X0, X1, SHR)                                                \
   do {                                                                             \
      const __m256i SHL = _mm256_sub_epi64(_mm256_set1_epi64x(64), SHR);            \
      X0 = _mm256_permute4x64_epi64(X0, _MM_SHUFFLE(2, 1, 0, 3));                   \
      X1 = _mm256_permute4x64_epi64(X1, _MM_SHUFFLE(1, 2, 3, 0));                   \
      X1 = _mm256_xor_si256(X1, X0);                                                \
      X1 = _mm256_or_si256(_mm256_sllv_epi64(X1, SHL), _mm256_srlv_epi64(X1, SHR)); \
      X0 = _mm256_sub_epi64(X0, X1);                                                \
   } while(0)

#define THREEFISH_ROUND_2(X0, X1, X2, X3, SHR)                                                \
   do {                                                                             \
      const __m256i SHL = _mm256_sub_epi64(_mm256_set1_epi64x(64), SHR);            \
      X0 = _mm256_permute4x64_epi64(X0, _MM_SHUFFLE(2, 1, 0, 3));                   \
      X2 = _mm256_permute4x64_epi64(X2, _MM_SHUFFLE(2, 1, 0, 3));                   \
      X1 = _mm256_permute4x64_epi64(X1, _MM_SHUFFLE(1, 2, 3, 0));                   \
      X3 = _mm256_permute4x64_epi64(X3, _MM_SHUFFLE(1, 2, 3, 0));                   \
      X1 = _mm256_xor_si256(X1, X0);                                                \
      X3 = _mm256_xor_si256(X3, X2);                                                \
      X1 = _mm256_or_si256(_mm256_sllv_epi64(X1, SHL), _mm256_srlv_epi64(X1, SHR)); \
      X3 = _mm256_or_si256(_mm256_sllv_epi64(X3, SHL), _mm256_srlv_epi64(X3, SHR)); \
      X0 = _mm256_sub_epi64(X0, X1);                                                \
      X2 = _mm256_sub_epi64(X2, X3);                                                \
   } while(0)

#define THREEFISH_INJECT_KEY(X0, X1, R, K0, K1, T0I, T1I)                \
   do {                                                                          \
      const __m256i T0 = _mm256_permute4x64_epi64(T, _MM_SHUFFLE(T0I, 0, 0, 0)); \
      const __m256i T1 = _mm256_permute4x64_epi64(T, _MM_SHUFFLE(0, T1I, 0, 0)); \
      X0 = _mm256_sub_epi64(X0, K0);                                             \
      X1 = _mm256_sub_epi64(X1, K1);                                             \
      X1 = _mm256_sub_epi64(X1, _mm256_set_epi64x(R, 0, 0, 0));                  \
      X0 = _mm256_sub_epi64(X0, T0);                                             \
      X1 = _mm256_sub_epi64(X1, T1);                                             \
   } while(0)

#define THREEFISH_DEC_8_ROUNDS(X0, X1, R, K1, K2, K3, T0, T1, T2)   \
   do {                                                      \
      THREEFISH_INJECT_KEY(X0, X1, R+1, K2, K3, T2, T0);     \
      THREEFISH_ROUND(X0, X1, ROTATE_8);                     \
      THREEFISH_ROUND(X0, X1, ROTATE_7);                     \
      THREEFISH_ROUND(X0, X1, ROTATE_6);                     \
      THREEFISH_ROUND(X0, X1, ROTATE_5);                     \
                                                             \
      THREEFISH_INJECT_KEY(X0, X1, R, K1, K2, T0, T1);       \
      THREEFISH_ROUND(X0, X1, ROTATE_4);                     \
      THREEFISH_ROUND(X0, X1, ROTATE_3);                     \
      THREEFISH_ROUND(X0, X1, ROTATE_2);                     \
      THREEFISH_ROUND(X0, X1, ROTATE_1);                     \
   } while(0)

#define THREEFISH_INJECT_KEY_2(X0, X1, X2, X3, R, K0, K1, T0I, T1I)              \
   do {                                                                          \
      const __m256i T0 = _mm256_permute4x64_epi64(T, _MM_SHUFFLE(T0I, 0, 0, 0)); \
      __m256i T1 = _mm256_permute4x64_epi64(T, _MM_SHUFFLE(0, T1I, 0, 0)); \
      X0 = _mm256_sub_epi64(X0, K0);                                             \
      X2 = _mm256_sub_epi64(X2, K0);                                             \
      X1 = _mm256_sub_epi64(X1, K1);                                             \
      X3 = _mm256_sub_epi64(X3, K1);                                             \
      T1 = _mm256_add_epi64(T1, _mm256_set_epi64x(R,0,0,0));                     \
      X0 = _mm256_sub_epi64(X0, T0);                                             \
      X2 = _mm256_sub_epi64(X2, T0);                                             \
      X1 = _mm256_sub_epi64(X1, T1);                                             \
      X3 = _mm256_sub_epi64(X3, T1);                                             \
   } while(0)

#define THREEFISH_DEC_2_8_ROUNDS(X0, X1, X2, X3, R, K1, K2, K3, T0, T1, T2) \
   do {                                                                  \
      THREEFISH_INJECT_KEY_2(X0, X1, X2, X3, R+1, K2, K3, T2, T0);         \
      THREEFISH_ROUND_2(X0, X1, X2, X3, ROTATE_8);                       \
      THREEFISH_ROUND_2(X0, X1, X2, X3, ROTATE_7);                       \
      THREEFISH_ROUND_2(X0, X1, X2, X3, ROTATE_6);                       \
      THREEFISH_ROUND_2(X0, X1, X2, X3, ROTATE_5);                       \
                                                                         \
      THREEFISH_INJECT_KEY_2(X0, X1, X2, X3, R, K1, K2, T0, T1);       \
      THREEFISH_ROUND_2(X0, X1, X2, X3, ROTATE_4);                       \
      THREEFISH_ROUND_2(X0, X1, X2, X3, ROTATE_3);                       \
      THREEFISH_ROUND_2(X0, X1, X2, X3, ROTATE_2);                       \
      THREEFISH_ROUND_2(X0, X1, X2, X3, ROTATE_1);                       \
   } while(0)

   /*
   v1.0 key schedule: 9 ymm registers (only need 2 or 3)
   (0,1,2,3),(4,5,6,7) [8]
   then mutating with vpermq
   */
   const __m256i K0 = _mm256_set_epi64x(K[6], K[4], K[2], K[0]);
   const __m256i K1 = _mm256_set_epi64x(K[7], K[5], K[3], K[1]);
   const __m256i K2 = _mm256_set_epi64x(K[8], K[6], K[4], K[2]);
   const __m256i K3 = _mm256_set_epi64x(K[0], K[7], K[5], K[3]);
   const __m256i K4 = _mm256_set_epi64x(K[1], K[8], K[6], K[4]);
   const __m256i K5 = _mm256_set_epi64x(K[2], K[0], K[7], K[5]);
   const __m256i K6 = _mm256_set_epi64x(K[3], K[1], K[8], K[6]);
   const __m256i K7 = _mm256_set_epi64x(K[4], K[2], K[0], K[7]);
   const __m256i K8 = _mm256_set_epi64x(K[5], K[3], K[1], K[8]);

   const __m256i* in_mm = reinterpret_cast<const __m256i*>(in);
   __m256i* out_mm = reinterpret_cast<__m256i*>(out);

   while(blocks >= 2)
      {
      __m256i X0 = _mm256_loadu_si256(in_mm++);
      __m256i X1 = _mm256_loadu_si256(in_mm++);
      __m256i X2 = _mm256_loadu_si256(in_mm++);
      __m256i X3 = _mm256_loadu_si256(in_mm++);

      const __m256i T = _mm256_set_epi64x(T_64[0], T_64[1], T_64[2], 0);

      interleave_epi64(X0, X1);
      interleave_epi64(X2, X3);

      THREEFISH_DEC_2_8_ROUNDS(X0, X1, X2, X3, 17, K8,K0,K1, 3, 1, 2);
      THREEFISH_DEC_2_8_ROUNDS(X0, X1, X2, X3, 15, K6,K7,K8, 2, 3, 1);
      THREEFISH_DEC_2_8_ROUNDS(X0, X1, X2, X3, 13, K4,K5,K6, 1, 2, 3);
      THREEFISH_DEC_2_8_ROUNDS(X0, X1, X2, X3, 11, K2,K3,K4, 3, 1, 2);
      THREEFISH_DEC_2_8_ROUNDS(X0, X1, X2, X3, 9, K0,K1,K2, 2, 3, 1);
      THREEFISH_DEC_2_8_ROUNDS(X0, X1, X2, X3, 7, K7,K8,K0, 1, 2, 3);
      THREEFISH_DEC_2_8_ROUNDS(X0, X1, X2, X3, 5, K5,K6,K7, 3, 1, 2);
      THREEFISH_DEC_2_8_ROUNDS(X0, X1, X2, X3, 3, K3,K4,K5, 2, 3, 1);
      THREEFISH_DEC_2_8_ROUNDS(X0, X1, X2, X3, 1, K1,K2,K3, 1, 2, 3);

      THREEFISH_INJECT_KEY_2(X0, X1, X2, X3, 0, K0, K1, 2, 3);

      deinterleave_epi64(X0, X1);
      deinterleave_epi64(X2, X3);

      _mm256_storeu_si256(out_mm++, X0);
      _mm256_storeu_si256(out_mm++, X1);
      _mm256_storeu_si256(out_mm++, X2);
      _mm256_storeu_si256(out_mm++, X3);

      blocks -= 2;
      }
   
   for(size_t i = 0; i != blocks; ++i)
      {
      __m256i X0 = _mm256_loadu_si256(in_mm++);
      __m256i X1 = _mm256_loadu_si256(in_mm++);

      const __m256i T = _mm256_set_epi64x(T_64[0], T_64[1], T_64[2], 0);

      interleave_epi64(X0, X1);

      THREEFISH_DEC_8_ROUNDS(X0, X1, 17, K8,K0,K1, 3, 1, 2);
      THREEFISH_DEC_8_ROUNDS(X0, X1, 15, K6,K7,K8, 2, 3, 1);
      THREEFISH_DEC_8_ROUNDS(X0, X1, 13, K4,K5,K6, 1, 2, 3);
      THREEFISH_DEC_8_ROUNDS(X0, X1, 11, K2,K3,K4, 3, 1, 2);
      THREEFISH_DEC_8_ROUNDS(X0, X1, 9, K0,K1,K2, 2, 3, 1);
      THREEFISH_DEC_8_ROUNDS(X0, X1, 7, K7,K8,K0, 1, 2, 3);
      THREEFISH_DEC_8_ROUNDS(X0, X1, 5, K5,K6,K7, 3, 1, 2);
      THREEFISH_DEC_8_ROUNDS(X0, X1, 3, K3,K4,K5, 2, 3, 1);
      THREEFISH_DEC_8_ROUNDS(X0, X1, 1, K1,K2,K3, 1, 2, 3);

      THREEFISH_INJECT_KEY(X0, X1, 0, K0, K1, 2, 3);

      deinterleave_epi64(X0, X1);

      _mm256_storeu_si256(out_mm++, X0);
      _mm256_storeu_si256(out_mm++, X1);
      }

#undef THREEFISH_DEC_8_ROUNDS
#undef THREEFISH_ROUND
#undef THREEFISH_INJECT_KEY
#undef THREEFISH_DEC_2_8_ROUNDS
#undef THREEFISH_ROUND_2
#undef THREEFISH_INJECT_KEY_2
   }

}
