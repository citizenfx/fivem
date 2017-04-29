/*
* Lightweight wrappers for SIMD operations
* (C) 2009,2011,2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_SIMD_32_H__
#define BOTAN_SIMD_32_H__

#include <botan/types.h>
#include <botan/loadstor.h>
#include <botan/bswap.h>

#if defined(BOTAN_TARGET_SUPPORTS_SSE2)
  #include <emmintrin.h>
  #define BOTAN_SIMD_USE_SSE2

#elif defined(BOTAN_TARGET_SUPPORTS_ALTIVEC)
  #include <altivec.h>
  #undef vector
  #undef bool
  #define BOTAN_SIMD_USE_ALTIVEC
#endif

// TODO: NEON support

namespace Botan {

/**
* This class is not a general purpose SIMD type, and only offers
* instructions needed for evaluation of specific crypto primitives.
* For example it does not currently have equality operators of any
* kind.
*/
class SIMD_4x32
   {
   public:

      SIMD_4x32() // zero initialized
         {
#if defined(BOTAN_SIMD_USE_SSE2) || defined(BOTAN_SIMD_USE_ALTIVEC)
         ::memset(&m_reg, 0, sizeof(m_reg));
#else
         ::memset(m_reg, 0, sizeof(m_reg));
#endif
         }

      explicit SIMD_4x32(const uint32_t B[4])
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_reg = _mm_loadu_si128(reinterpret_cast<const __m128i*>(B));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_reg = (__vector unsigned int){B[0], B[1], B[2], B[3]};
#else
         m_reg[0] = B[0];
         m_reg[1] = B[1];
         m_reg[2] = B[2];
         m_reg[3] = B[3];
#endif
         }

      SIMD_4x32(uint32_t B0, uint32_t B1, uint32_t B2, uint32_t B3)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_reg = _mm_set_epi32(B0, B1, B2, B3);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_reg = (__vector unsigned int){B0, B1, B2, B3};
#else
         m_reg[0] = B0;
         m_reg[1] = B1;
         m_reg[2] = B2;
         m_reg[3] = B3;
#endif
         }

      explicit SIMD_4x32(uint32_t B)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_reg = _mm_set1_epi32(B);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_reg = (__vector unsigned int){B, B, B, B};
#else
         m_reg[0] = B;
         m_reg[1] = B;
         m_reg[2] = B;
         m_reg[3] = B;
#endif
         }

      static SIMD_4x32 load_le(const void* in)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(in)));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const uint32_t* in_32 = static_cast<const uint32_t*>(in);

         __vector unsigned int R0 = vec_ld(0, in_32);
         __vector unsigned int R1 = vec_ld(12, in_32);

         __vector unsigned char perm = vec_lvsl(0, in_32);

#if defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
         perm = vec_xor(perm, vec_splat_u8(3)); // bswap vector
#endif

         R0 = vec_perm(R0, R1, perm);

         return SIMD_4x32(R0);
#else
         SIMD_4x32 out;
         Botan::load_le(out.m_reg, static_cast<const uint8_t*>(in), 4);
         return out;
#endif
         }

      static SIMD_4x32 load_be(const void* in)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return load_le(in).bswap();
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const uint32_t* in_32 = static_cast<const uint32_t*>(in);

         __vector unsigned int R0 = vec_ld(0, in_32);
         __vector unsigned int R1 = vec_ld(12, in_32);

         __vector unsigned char perm = vec_lvsl(0, in_32);

#if defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN)
         perm = vec_xor(perm, vec_splat_u8(3)); // bswap vector
#endif

         R0 = vec_perm(R0, R1, perm);

         return SIMD_4x32(R0);

#else
         SIMD_4x32 out;
         Botan::load_be(out.m_reg, static_cast<const uint8_t*>(in), 4);
         return out;
#endif
         }

      void store_le(uint8_t out[]) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         _mm_storeu_si128(reinterpret_cast<__m128i*>(out), m_reg);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         __vector unsigned char perm = vec_lvsl(0, static_cast<uint32_t*>(nullptr));

#if defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
         perm = vec_xor(perm, vec_splat_u8(3)); // bswap vector
#endif

         union {
            __vector unsigned int V;
            uint32_t R[4];
            } vec;

         vec.V = vec_perm(m_reg, m_reg, perm);

         Botan::store_be(out, vec.R[0], vec.R[1], vec.R[2], vec.R[3]);
#else
         Botan::store_le(out, m_reg[0], m_reg[1], m_reg[2], m_reg[3]);
#endif
         }

      void store_be(uint8_t out[]) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         bswap().store_le(out);

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         union {
            __vector unsigned int V;
            uint32_t R[4];
            } vec;

         vec.V = m_reg;

         Botan::store_be(out, vec.R[0], vec.R[1], vec.R[2], vec.R[3]);
#else
         Botan::store_be(out, m_reg[0], m_reg[1], m_reg[2], m_reg[3]);
#endif
         }

      void rotate_left(size_t rot)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_reg = _mm_or_si128(_mm_slli_epi32(m_reg, static_cast<int>(rot)),
                              _mm_srli_epi32(m_reg, static_cast<int>(32-rot)));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const unsigned int r = static_cast<unsigned int>(rot);
         m_reg = vec_rl(m_reg, (__vector unsigned int){r, r, r, r});

#else
         m_reg[0] = Botan::rotate_left(m_reg[0], rot);
         m_reg[1] = Botan::rotate_left(m_reg[1], rot);
         m_reg[2] = Botan::rotate_left(m_reg[2], rot);
         m_reg[3] = Botan::rotate_left(m_reg[3], rot);
#endif
         }

      void rotate_right(size_t rot)
         {
         rotate_left(32 - rot);
         }

      void operator+=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_reg = _mm_add_epi32(m_reg, other.m_reg);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_reg = vec_add(m_reg, other.m_reg);
#else
         m_reg[0] += other.m_reg[0];
         m_reg[1] += other.m_reg[1];
         m_reg[2] += other.m_reg[2];
         m_reg[3] += other.m_reg[3];
#endif
         }

      SIMD_4x32 operator+(const SIMD_4x32& other) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_add_epi32(m_reg, other.m_reg));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         return SIMD_4x32(vec_add(m_reg, other.m_reg));
#else
         return SIMD_4x32(m_reg[0] + other.m_reg[0],
                          m_reg[1] + other.m_reg[1],
                          m_reg[2] + other.m_reg[2],
                          m_reg[3] + other.m_reg[3]);
#endif
         }

      void operator-=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_reg = _mm_sub_epi32(m_reg, other.m_reg);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_reg = vec_sub(m_reg, other.m_reg);
#else
         m_reg[0] -= other.m_reg[0];
         m_reg[1] -= other.m_reg[1];
         m_reg[2] -= other.m_reg[2];
         m_reg[3] -= other.m_reg[3];
#endif
         }

      SIMD_4x32 operator-(const SIMD_4x32& other) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_sub_epi32(m_reg, other.m_reg));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         return SIMD_4x32(vec_sub(m_reg, other.m_reg));
#else
         return SIMD_4x32(m_reg[0] - other.m_reg[0],
                          m_reg[1] - other.m_reg[1],
                          m_reg[2] - other.m_reg[2],
                          m_reg[3] - other.m_reg[3]);
#endif
         }

      void operator^=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_reg = _mm_xor_si128(m_reg, other.m_reg);

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_reg = vec_xor(m_reg, other.m_reg);
#else
         m_reg[0] ^= other.m_reg[0];
         m_reg[1] ^= other.m_reg[1];
         m_reg[2] ^= other.m_reg[2];
         m_reg[3] ^= other.m_reg[3];
#endif
         }

      SIMD_4x32 operator^(const SIMD_4x32& other) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_xor_si128(m_reg, other.m_reg));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         return SIMD_4x32(vec_xor(m_reg, other.m_reg));
#else
         return SIMD_4x32(m_reg[0] ^ other.m_reg[0],
                          m_reg[1] ^ other.m_reg[1],
                          m_reg[2] ^ other.m_reg[2],
                          m_reg[3] ^ other.m_reg[3]);
#endif
         }

      void operator|=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_reg = _mm_or_si128(m_reg, other.m_reg);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_reg = vec_or(m_reg, other.m_reg);
#else
         m_reg[0] |= other.m_reg[0];
         m_reg[1] |= other.m_reg[1];
         m_reg[2] |= other.m_reg[2];
         m_reg[3] |= other.m_reg[3];
#endif
         }

      SIMD_4x32 operator&(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_and_si128(m_reg, other.m_reg));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         return SIMD_4x32(vec_and(m_reg, other.m_reg));
#else
         return SIMD_4x32(m_reg[0] & other.m_reg[0],
                          m_reg[1] & other.m_reg[1],
                          m_reg[2] & other.m_reg[2],
                          m_reg[3] & other.m_reg[3]);
#endif
         }

      void operator&=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_reg = _mm_and_si128(m_reg, other.m_reg);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_reg = vec_and(m_reg, other.m_reg);
#else
         m_reg[0] &= other.m_reg[0];
         m_reg[1] &= other.m_reg[1];
         m_reg[2] &= other.m_reg[2];
         m_reg[3] &= other.m_reg[3];
#endif
         }

      SIMD_4x32 operator<<(size_t shift) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_slli_epi32(m_reg, static_cast<int>(shift)));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const unsigned int s = static_cast<unsigned int>(shift);
         return SIMD_4x32(vec_sl(m_reg, (__vector unsigned int){s, s, s, s}));
#else
         return SIMD_4x32(m_reg[0] << shift,
                          m_reg[1] << shift,
                          m_reg[2] << shift,
                          m_reg[3] << shift);
#endif
         }

      SIMD_4x32 operator>>(size_t shift) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_srli_epi32(m_reg, static_cast<int>(shift)));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const unsigned int s = static_cast<unsigned int>(shift);
         return SIMD_4x32(vec_sr(m_reg, (__vector unsigned int){s, s, s, s}));
#else
         return SIMD_4x32(m_reg[0] >> shift,
                          m_reg[1] >> shift,
                          m_reg[2] >> shift,
                          m_reg[3] >> shift);

#endif
         }

      SIMD_4x32 operator~() const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_xor_si128(m_reg, _mm_set1_epi32(0xFFFFFFFF)));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         return SIMD_4x32(vec_nor(m_reg, m_reg));
#else
         return SIMD_4x32(~m_reg[0],
                          ~m_reg[1],
                          ~m_reg[2],
                          ~m_reg[3]);
#endif
         }

      // (~reg) & other
      SIMD_4x32 andc(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_andnot_si128(m_reg, other.m_reg));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         /*
         AltiVec does arg1 & ~arg2 rather than SSE's ~arg1 & arg2
         so swap the arguments
         */
         return SIMD_4x32(vec_andc(other.m_reg, m_reg));
#else
         return SIMD_4x32((~m_reg[0]) & other.m_reg[0],
                          (~m_reg[1]) & other.m_reg[1],
                          (~m_reg[2]) & other.m_reg[2],
                          (~m_reg[3]) & other.m_reg[3]);
#endif
         }

      SIMD_4x32 bswap() const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         __m128i T = m_reg;

         T = _mm_shufflehi_epi16(T, _MM_SHUFFLE(2, 3, 0, 1));
         T = _mm_shufflelo_epi16(T, _MM_SHUFFLE(2, 3, 0, 1));

         return SIMD_4x32(_mm_or_si128(_mm_srli_epi16(T, 8),
                                       _mm_slli_epi16(T, 8)));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         __vector unsigned char perm = vec_lvsl(0, static_cast<uint32_t*>(nullptr));

         perm = vec_xor(perm, vec_splat_u8(3));

         return SIMD_4x32(vec_perm(m_reg, m_reg, perm));
#else
         return SIMD_4x32(reverse_bytes(m_reg[0]),
                          reverse_bytes(m_reg[1]),
                          reverse_bytes(m_reg[2]),
                          reverse_bytes(m_reg[3]));
#endif
         }

      static void transpose(SIMD_4x32& B0, SIMD_4x32& B1,
                            SIMD_4x32& B2, SIMD_4x32& B3)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         __m128i T0 = _mm_unpacklo_epi32(B0.m_reg, B1.m_reg);
         __m128i T1 = _mm_unpacklo_epi32(B2.m_reg, B3.m_reg);
         __m128i T2 = _mm_unpackhi_epi32(B0.m_reg, B1.m_reg);
         __m128i T3 = _mm_unpackhi_epi32(B2.m_reg, B3.m_reg);
         B0.m_reg = _mm_unpacklo_epi64(T0, T1);
         B1.m_reg = _mm_unpackhi_epi64(T0, T1);
         B2.m_reg = _mm_unpacklo_epi64(T2, T3);
         B3.m_reg = _mm_unpackhi_epi64(T2, T3);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         __vector unsigned int T0 = vec_mergeh(B0.m_reg, B2.m_reg);
         __vector unsigned int T1 = vec_mergel(B0.m_reg, B2.m_reg);
         __vector unsigned int T2 = vec_mergeh(B1.m_reg, B3.m_reg);
         __vector unsigned int T3 = vec_mergel(B1.m_reg, B3.m_reg);

         B0.m_reg = vec_mergeh(T0, T2);
         B1.m_reg = vec_mergel(T0, T2);
         B2.m_reg = vec_mergeh(T1, T3);
         B3.m_reg = vec_mergel(T1, T3);
#else
         SIMD_4x32 T0(B0.m_reg[0], B1.m_reg[0], B2.m_reg[0], B3.m_reg[0]);
         SIMD_4x32 T1(B0.m_reg[1], B1.m_reg[1], B2.m_reg[1], B3.m_reg[1]);
         SIMD_4x32 T2(B0.m_reg[2], B1.m_reg[2], B2.m_reg[2], B3.m_reg[2]);
         SIMD_4x32 T3(B0.m_reg[3], B1.m_reg[3], B2.m_reg[3], B3.m_reg[3]);

         B0 = T0;
         B1 = T1;
         B2 = T2;
         B3 = T3;
#endif
         }

   private:
#if defined(BOTAN_SIMD_USE_SSE2)
      explicit SIMD_4x32(__m128i in) { m_reg = in; }
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
      explicit SIMD_4x32(__vector unsigned int input) { m_reg = input; }
#endif

#if defined(BOTAN_SIMD_USE_SSE2)
      __m128i m_reg;
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
      __vector unsigned int m_reg;
#else
      uint32_t m_reg[4];
#endif
   };

typedef SIMD_4x32 SIMD_32;

}

#endif
