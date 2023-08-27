/*
* (C) 2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_SIMD_AVX2_H_
#define BOTAN_SIMD_AVX2_H_

#include <botan/types.h>
#include <immintrin.h>

namespace Botan {

class SIMD_8x32 final
   {
   public:

      SIMD_8x32& operator=(const SIMD_8x32& other) = default;
      SIMD_8x32(const SIMD_8x32& other) = default;

      SIMD_8x32& operator=(SIMD_8x32&& other) = default;
      SIMD_8x32(SIMD_8x32&& other) = default;

      BOTAN_FUNC_ISA("avx2")
      BOTAN_FORCE_INLINE SIMD_8x32()
         {
         m_avx2 = _mm256_setzero_si256();
         }

      BOTAN_FUNC_ISA("avx2")
      explicit SIMD_8x32(const uint32_t B[8])
         {
         m_avx2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(B));
         }

      BOTAN_FUNC_ISA("avx2")
      explicit SIMD_8x32(uint32_t B0, uint32_t B1, uint32_t B2, uint32_t B3,
                         uint32_t B4, uint32_t B5, uint32_t B6, uint32_t B7)
         {
         m_avx2 = _mm256_set_epi32(B7, B6, B5, B4, B3, B2, B1, B0);
         }

      BOTAN_FUNC_ISA("avx2")
      static SIMD_8x32 splat(uint32_t B)
         {
         return SIMD_8x32(_mm256_set1_epi32(B));
         }

      BOTAN_FUNC_ISA("avx2")
      static SIMD_8x32 load_le(const uint8_t* in)
         {
         return SIMD_8x32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(in)));
         }

      BOTAN_FUNC_ISA("avx2")
      static SIMD_8x32 load_be(const uint8_t* in)
         {
         return load_le(in).bswap();
         }

      BOTAN_FUNC_ISA("avx2")
      void store_le(uint8_t out[]) const
         {
         _mm256_storeu_si256(reinterpret_cast<__m256i*>(out), m_avx2);
         }

      BOTAN_FUNC_ISA("avx2")
      void store_be(uint8_t out[]) const
         {
         bswap().store_le(out);
         }

      template<size_t ROT>
      BOTAN_FUNC_ISA("avx2")
      SIMD_8x32 rotl() const
         {
         static_assert(ROT > 0 && ROT < 32, "Invalid rotation constant");

#if defined(__AVX512VL__)
         return SIMD_8x32(_mm256_rol_epi32(m_avx2, ROT));
#else
         BOTAN_IF_CONSTEXPR(ROT == 8)
            {
            const __m256i shuf_rotl_8 = _mm256_set_epi8(14, 13, 12, 15, 10, 9, 8, 11, 6, 5, 4, 7, 2, 1, 0, 3,
                                                        14, 13, 12, 15, 10, 9, 8, 11, 6, 5, 4, 7, 2, 1, 0, 3);

            return SIMD_8x32(_mm256_shuffle_epi8(m_avx2, shuf_rotl_8));
            }
         else BOTAN_IF_CONSTEXPR(ROT == 16)
            {
            const __m256i shuf_rotl_16 = _mm256_set_epi8(13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2,
                                                         13, 12, 15, 14, 9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2);

            return SIMD_8x32(_mm256_shuffle_epi8(m_avx2, shuf_rotl_16));
            }
         else
            {
            return SIMD_8x32(_mm256_or_si256(_mm256_slli_epi32(m_avx2, static_cast<int>(ROT)),
                                             _mm256_srli_epi32(m_avx2, static_cast<int>(32-ROT))));
            }
#endif
         }

      template<size_t ROT>
      BOTAN_FUNC_ISA("avx2")
      SIMD_8x32 rotr() const
         {
         return this->rotl<32-ROT>();
         }

      template<size_t ROT1, size_t ROT2, size_t ROT3>
      SIMD_8x32 BOTAN_FUNC_ISA("avx2") rho() const
         {
         SIMD_8x32 res;

         const SIMD_8x32 rot1 = this->rotr<ROT1>();
         const SIMD_8x32 rot2 = this->rotr<ROT2>();
         const SIMD_8x32 rot3 = this->rotr<ROT3>();

         return rot1 ^ rot2 ^ rot3;
         }

      BOTAN_FUNC_ISA("avx2")
      SIMD_8x32 operator+(const SIMD_8x32& other) const
         {
         SIMD_8x32 retval(*this);
         retval += other;
         return retval;
         }

      BOTAN_FUNC_ISA("avx2")
      SIMD_8x32 operator-(const SIMD_8x32& other) const
         {
         SIMD_8x32 retval(*this);
         retval -= other;
         return retval;
         }

      BOTAN_FUNC_ISA("avx2")
      SIMD_8x32 operator^(const SIMD_8x32& other) const
         {
         SIMD_8x32 retval(*this);
         retval ^= other;
         return retval;
         }

      BOTAN_FUNC_ISA("avx2")
      SIMD_8x32 operator|(const SIMD_8x32& other) const
         {
         SIMD_8x32 retval(*this);
         retval |= other;
         return retval;
         }

      BOTAN_FUNC_ISA("avx2")
      SIMD_8x32 operator&(const SIMD_8x32& other) const
         {
         SIMD_8x32 retval(*this);
         retval &= other;
         return retval;
         }

      BOTAN_FUNC_ISA("avx2")
      void operator+=(const SIMD_8x32& other)
         {
         m_avx2 = _mm256_add_epi32(m_avx2, other.m_avx2);
         }

      BOTAN_FUNC_ISA("avx2")
      void operator-=(const SIMD_8x32& other)
         {
         m_avx2 = _mm256_sub_epi32(m_avx2, other.m_avx2);
         }

      BOTAN_FUNC_ISA("avx2")
      void operator^=(const SIMD_8x32& other)
         {
         m_avx2 = _mm256_xor_si256(m_avx2, other.m_avx2);
         }

      BOTAN_FUNC_ISA("avx2")
      void operator|=(const SIMD_8x32& other)
         {
         m_avx2 = _mm256_or_si256(m_avx2, other.m_avx2);
         }

      BOTAN_FUNC_ISA("avx2")
      void operator&=(const SIMD_8x32& other)
         {
         m_avx2 = _mm256_and_si256(m_avx2, other.m_avx2);
         }

      template<int SHIFT> BOTAN_FUNC_ISA("avx2") SIMD_8x32 shl() const
         {
         return SIMD_8x32(_mm256_slli_epi32(m_avx2, SHIFT));
         }

      template<int SHIFT> BOTAN_FUNC_ISA("avx2") SIMD_8x32 shr() const
         {
         return SIMD_8x32(_mm256_srli_epi32(m_avx2, SHIFT));
         }

      BOTAN_FUNC_ISA("avx2")
      SIMD_8x32 operator~() const
         {
         return SIMD_8x32(_mm256_xor_si256(m_avx2, _mm256_set1_epi32(0xFFFFFFFF)));
         }

      // (~reg) & other
      BOTAN_FUNC_ISA("avx2")
      SIMD_8x32 andc(const SIMD_8x32& other) const
         {
         return SIMD_8x32(_mm256_andnot_si256(m_avx2, other.m_avx2));
         }

      BOTAN_FUNC_ISA("avx2")
      SIMD_8x32 bswap() const
         {
         const uint8_t BSWAP_MASK[32] = { 3, 2, 1, 0,
                                          7, 6, 5, 4,
                                          11, 10, 9, 8,
                                          15, 14, 13, 12,
                                          19, 18, 17, 16,
                                          23, 22, 21, 20,
                                          27, 26, 25, 24,
                                          31, 30, 29, 28 };

         const __m256i bswap = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(BSWAP_MASK));

         const __m256i output = _mm256_shuffle_epi8(m_avx2, bswap);

         return SIMD_8x32(output);
         }

      BOTAN_FUNC_ISA("avx2")
      static void transpose(SIMD_8x32& B0, SIMD_8x32& B1,
                            SIMD_8x32& B2, SIMD_8x32& B3)
         {
         const __m256i T0 = _mm256_unpacklo_epi32(B0.m_avx2, B1.m_avx2);
         const __m256i T1 = _mm256_unpacklo_epi32(B2.m_avx2, B3.m_avx2);
         const __m256i T2 = _mm256_unpackhi_epi32(B0.m_avx2, B1.m_avx2);
         const __m256i T3 = _mm256_unpackhi_epi32(B2.m_avx2, B3.m_avx2);

         B0.m_avx2 = _mm256_unpacklo_epi64(T0, T1);
         B1.m_avx2 = _mm256_unpackhi_epi64(T0, T1);
         B2.m_avx2 = _mm256_unpacklo_epi64(T2, T3);
         B3.m_avx2 = _mm256_unpackhi_epi64(T2, T3);
         }

      BOTAN_FUNC_ISA("avx2")
      static void transpose(SIMD_8x32& B0, SIMD_8x32& B1,
                            SIMD_8x32& B2, SIMD_8x32& B3,
                            SIMD_8x32& B4, SIMD_8x32& B5,
                            SIMD_8x32& B6, SIMD_8x32& B7)
         {
         transpose(B0, B1, B2, B3);
         transpose(B4, B5, B6, B7);

         swap_tops(B0, B4);
         swap_tops(B1, B5);
         swap_tops(B2, B6);
         swap_tops(B3, B7);
         }

      BOTAN_FUNC_ISA("avx2")
      static void reset_registers()
         {
         _mm256_zeroupper();
         }

      BOTAN_FUNC_ISA("avx2")
      static void zero_registers()
         {
         _mm256_zeroall();
         }

      __m256i BOTAN_FUNC_ISA("avx2") handle() const { return m_avx2; }

      BOTAN_FUNC_ISA("avx2")
      SIMD_8x32(__m256i x) : m_avx2(x) {}

   private:

      BOTAN_FUNC_ISA("avx2")
      static void swap_tops(SIMD_8x32& A, SIMD_8x32& B)
         {
         SIMD_8x32 T0 = _mm256_permute2x128_si256(A.handle(), B.handle(), 0 + (2 << 4));
         SIMD_8x32 T1 = _mm256_permute2x128_si256(A.handle(), B.handle(), 1 + (3 << 4));
         A = T0;
         B = T1;
         }

      __m256i m_avx2;
   };

}

#endif
