/*
* Botan 2.10.0 Amalgamation
* (C) 1999-2018 The Botan Authors
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "botan_all.h"
#include "botan_all_internal.h"

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
      SIMD_8x32()
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
         }

      template<size_t ROT>
      BOTAN_FUNC_ISA("avx2")
      SIMD_8x32 rotr() const
         {
         return this->rotl<32-ROT>();
         }

      SIMD_8x32 operator+(const SIMD_8x32& other) const
         {
         SIMD_8x32 retval(*this);
         retval += other;
         return retval;
         }

      SIMD_8x32 operator-(const SIMD_8x32& other) const
         {
         SIMD_8x32 retval(*this);
         retval -= other;
         return retval;
         }

      SIMD_8x32 operator^(const SIMD_8x32& other) const
         {
         SIMD_8x32 retval(*this);
         retval ^= other;
         return retval;
         }

      SIMD_8x32 operator|(const SIMD_8x32& other) const
         {
         SIMD_8x32 retval(*this);
         retval |= other;
         return retval;
         }

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

      template<int SHIFT> BOTAN_FUNC_ISA("avx2")SIMD_8x32 shr() const
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
      static void reset_registers()
         {
         _mm256_zeroupper();
         }

      BOTAN_FUNC_ISA("avx2")
      static void zero_registers()
         {
         _mm256_zeroall();
         }

      __m256i handle() const { return m_avx2; }

   private:

      BOTAN_FUNC_ISA("avx2")
      SIMD_8x32(__m256i x) : m_avx2(x) {}

      __m256i m_avx2;
   };

}
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC target ("avx2")
#endif
/*
* (C) 2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

//static
BOTAN_FUNC_ISA("avx2")
void ChaCha::chacha_avx2_x8(uint8_t output[64*8], uint32_t state[16], size_t rounds)
   {
   SIMD_8x32::reset_registers();

   BOTAN_ASSERT(rounds % 2 == 0, "Valid rounds");
   const SIMD_8x32 CTR0 = SIMD_8x32(0, 1, 2, 3, 4, 5, 6, 7);

   const uint32_t C = 0xFFFFFFFF - state[12];
   const SIMD_8x32 CTR1 = SIMD_8x32(0, C < 1, C < 2, C < 3, C < 4, C < 5, C < 6, C < 7);

   SIMD_8x32 R00 = SIMD_8x32::splat(state[ 0]);
   SIMD_8x32 R01 = SIMD_8x32::splat(state[ 1]);
   SIMD_8x32 R02 = SIMD_8x32::splat(state[ 2]);
   SIMD_8x32 R03 = SIMD_8x32::splat(state[ 3]);
   SIMD_8x32 R04 = SIMD_8x32::splat(state[ 4]);
   SIMD_8x32 R05 = SIMD_8x32::splat(state[ 5]);
   SIMD_8x32 R06 = SIMD_8x32::splat(state[ 6]);
   SIMD_8x32 R07 = SIMD_8x32::splat(state[ 7]);
   SIMD_8x32 R08 = SIMD_8x32::splat(state[ 8]);
   SIMD_8x32 R09 = SIMD_8x32::splat(state[ 9]);
   SIMD_8x32 R10 = SIMD_8x32::splat(state[10]);
   SIMD_8x32 R11 = SIMD_8x32::splat(state[11]);
   SIMD_8x32 R12 = SIMD_8x32::splat(state[12]) + CTR0;
   SIMD_8x32 R13 = SIMD_8x32::splat(state[13]) + CTR1;
   SIMD_8x32 R14 = SIMD_8x32::splat(state[14]);
   SIMD_8x32 R15 = SIMD_8x32::splat(state[15]);

   for(size_t r = 0; r != rounds / 2; ++r)
      {
      R00 += R04;
      R01 += R05;
      R02 += R06;
      R03 += R07;

      R12 ^= R00;
      R13 ^= R01;
      R14 ^= R02;
      R15 ^= R03;

      R12 = R12.rotl<16>();
      R13 = R13.rotl<16>();
      R14 = R14.rotl<16>();
      R15 = R15.rotl<16>();

      R08 += R12;
      R09 += R13;
      R10 += R14;
      R11 += R15;

      R04 ^= R08;
      R05 ^= R09;
      R06 ^= R10;
      R07 ^= R11;

      R04 = R04.rotl<12>();
      R05 = R05.rotl<12>();
      R06 = R06.rotl<12>();
      R07 = R07.rotl<12>();

      R00 += R04;
      R01 += R05;
      R02 += R06;
      R03 += R07;

      R12 ^= R00;
      R13 ^= R01;
      R14 ^= R02;
      R15 ^= R03;

      R12 = R12.rotl<8>();
      R13 = R13.rotl<8>();
      R14 = R14.rotl<8>();
      R15 = R15.rotl<8>();

      R08 += R12;
      R09 += R13;
      R10 += R14;
      R11 += R15;

      R04 ^= R08;
      R05 ^= R09;
      R06 ^= R10;
      R07 ^= R11;

      R04 = R04.rotl<7>();
      R05 = R05.rotl<7>();
      R06 = R06.rotl<7>();
      R07 = R07.rotl<7>();

      R00 += R05;
      R01 += R06;
      R02 += R07;
      R03 += R04;

      R15 ^= R00;
      R12 ^= R01;
      R13 ^= R02;
      R14 ^= R03;

      R15 = R15.rotl<16>();
      R12 = R12.rotl<16>();
      R13 = R13.rotl<16>();
      R14 = R14.rotl<16>();

      R10 += R15;
      R11 += R12;
      R08 += R13;
      R09 += R14;

      R05 ^= R10;
      R06 ^= R11;
      R07 ^= R08;
      R04 ^= R09;

      R05 = R05.rotl<12>();
      R06 = R06.rotl<12>();
      R07 = R07.rotl<12>();
      R04 = R04.rotl<12>();

      R00 += R05;
      R01 += R06;
      R02 += R07;
      R03 += R04;

      R15 ^= R00;
      R12 ^= R01;
      R13 ^= R02;
      R14 ^= R03;

      R15 = R15.rotl<8>();
      R12 = R12.rotl<8>();
      R13 = R13.rotl<8>();
      R14 = R14.rotl<8>();

      R10 += R15;
      R11 += R12;
      R08 += R13;
      R09 += R14;

      R05 ^= R10;
      R06 ^= R11;
      R07 ^= R08;
      R04 ^= R09;

      R05 = R05.rotl<7>();
      R06 = R06.rotl<7>();
      R07 = R07.rotl<7>();
      R04 = R04.rotl<7>();
      }

   R00 += SIMD_8x32::splat(state[0]);
   R01 += SIMD_8x32::splat(state[1]);
   R02 += SIMD_8x32::splat(state[2]);
   R03 += SIMD_8x32::splat(state[3]);
   R04 += SIMD_8x32::splat(state[4]);
   R05 += SIMD_8x32::splat(state[5]);
   R06 += SIMD_8x32::splat(state[6]);
   R07 += SIMD_8x32::splat(state[7]);
   R08 += SIMD_8x32::splat(state[8]);
   R09 += SIMD_8x32::splat(state[9]);
   R10 += SIMD_8x32::splat(state[10]);
   R11 += SIMD_8x32::splat(state[11]);
   R12 += SIMD_8x32::splat(state[12]) + CTR0;
   R13 += SIMD_8x32::splat(state[13]) + CTR1;
   R14 += SIMD_8x32::splat(state[14]);
   R15 += SIMD_8x32::splat(state[15]);

   SIMD_8x32::transpose(R00, R01, R02, R03);
   SIMD_8x32::transpose(R04, R05, R06, R07);
   SIMD_8x32::transpose(R08, R09, R10, R11);
   SIMD_8x32::transpose(R12, R13, R14, R15);

   __m256i* output_mm = reinterpret_cast<__m256i*>(output);

   _mm256_storeu_si256(output_mm     , _mm256_permute2x128_si256(R00.handle(), R04.handle(), 0 + (2 << 4)));
   _mm256_storeu_si256(output_mm +  1, _mm256_permute2x128_si256(R08.handle(), R12.handle(), 0 + (2 << 4)));
   _mm256_storeu_si256(output_mm +  2, _mm256_permute2x128_si256(R01.handle(), R05.handle(), 0 + (2 << 4)));
   _mm256_storeu_si256(output_mm +  3, _mm256_permute2x128_si256(R09.handle(), R13.handle(), 0 + (2 << 4)));
   _mm256_storeu_si256(output_mm +  4, _mm256_permute2x128_si256(R02.handle(), R06.handle(), 0 + (2 << 4)));
   _mm256_storeu_si256(output_mm +  5, _mm256_permute2x128_si256(R10.handle(), R14.handle(), 0 + (2 << 4)));
   _mm256_storeu_si256(output_mm +  6, _mm256_permute2x128_si256(R03.handle(), R07.handle(), 0 + (2 << 4)));
   _mm256_storeu_si256(output_mm +  7, _mm256_permute2x128_si256(R11.handle(), R15.handle(), 0 + (2 << 4)));

   _mm256_storeu_si256(output_mm +  8, _mm256_permute2x128_si256(R00.handle(), R04.handle(), 1 + (3 << 4)));
   _mm256_storeu_si256(output_mm +  9, _mm256_permute2x128_si256(R08.handle(), R12.handle(), 1 + (3 << 4)));
   _mm256_storeu_si256(output_mm + 10, _mm256_permute2x128_si256(R01.handle(), R05.handle(), 1 + (3 << 4)));
   _mm256_storeu_si256(output_mm + 11, _mm256_permute2x128_si256(R09.handle(), R13.handle(), 1 + (3 << 4)));
   _mm256_storeu_si256(output_mm + 12, _mm256_permute2x128_si256(R02.handle(), R06.handle(), 1 + (3 << 4)));
   _mm256_storeu_si256(output_mm + 13, _mm256_permute2x128_si256(R10.handle(), R14.handle(), 1 + (3 << 4)));
   _mm256_storeu_si256(output_mm + 14, _mm256_permute2x128_si256(R03.handle(), R07.handle(), 1 + (3 << 4)));
   _mm256_storeu_si256(output_mm + 15, _mm256_permute2x128_si256(R11.handle(), R15.handle(), 1 + (3 << 4)));

   SIMD_8x32::zero_registers();

   state[12] += 8;
   if(state[12] < 8)
      state[13]++;
   }
}
/*
* (C) 2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {


#define key_xor(round, B0, B1, B2, B3)                             \
   do {                                                            \
      B0 ^= SIMD_8x32::splat(m_round_key[4*round  ]);              \
      B1 ^= SIMD_8x32::splat(m_round_key[4*round+1]);              \
      B2 ^= SIMD_8x32::splat(m_round_key[4*round+2]);              \
      B3 ^= SIMD_8x32::splat(m_round_key[4*round+3]);              \
   } while(0)

/*
* Serpent's linear transformations
*/
#define transform(B0, B1, B2, B3)                                  \
   do {                                                            \
      B0 = B0.rotl<13>();                                          \
      B2 = B2.rotl<3>();                                           \
      B1 ^= B0 ^ B2;                                               \
      B3 ^= B2 ^ B0.shl<3>();                                      \
      B1 = B1.rotl<1>();                                           \
      B3 = B3.rotl<7>();                                           \
      B0 ^= B1 ^ B3;                                               \
      B2 ^= B3 ^ B1.shl<7>();                                      \
      B0 = B0.rotl<5>();                                           \
      B2 = B2.rotl<22>();                                          \
   } while(0)

#define i_transform(B0, B1, B2, B3)                                \
   do {                                                            \
      B2 = B2.rotr<22>();                                          \
      B0 = B0.rotr<5>();                                           \
      B2 ^= B3 ^ B1.shl<7>();                                      \
      B0 ^= B1 ^ B3;                                               \
      B3 = B3.rotr<7>();                                           \
      B1 = B1.rotr<1>();                                           \
      B3 ^= B2 ^ B0.shl<3>();                                      \
      B1 ^= B0 ^ B2;                                               \
      B2 = B2.rotr<3>();                                           \
      B0 = B0.rotr<13>();                                          \
   } while(0)

BOTAN_FUNC_ISA("avx2")
void Serpent::avx2_encrypt_8(const uint8_t in[128], uint8_t out[128]) const
   {
   SIMD_8x32::reset_registers();

   SIMD_8x32 B0 = SIMD_8x32::load_le(in);
   SIMD_8x32 B1 = SIMD_8x32::load_le(in + 32);
   SIMD_8x32 B2 = SIMD_8x32::load_le(in + 64);
   SIMD_8x32 B3 = SIMD_8x32::load_le(in + 96);

   SIMD_8x32::transpose(B0, B1, B2, B3);

   key_xor( 0,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 1,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 2,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 3,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 4,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 5,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 6,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 7,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 8,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor( 9,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(10,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(11,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(12,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(13,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(14,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(15,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(16,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(17,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(18,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(19,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(20,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(21,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(22,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(23,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(24,B0,B1,B2,B3); SBoxE1(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(25,B0,B1,B2,B3); SBoxE2(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(26,B0,B1,B2,B3); SBoxE3(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(27,B0,B1,B2,B3); SBoxE4(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(28,B0,B1,B2,B3); SBoxE5(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(29,B0,B1,B2,B3); SBoxE6(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(30,B0,B1,B2,B3); SBoxE7(B0,B1,B2,B3); transform(B0,B1,B2,B3);
   key_xor(31,B0,B1,B2,B3); SBoxE8(B0,B1,B2,B3); key_xor(32,B0,B1,B2,B3);

   SIMD_8x32::transpose(B0, B1, B2, B3);
   B0.store_le(out);
   B1.store_le(out + 32);
   B2.store_le(out + 64);
   B3.store_le(out + 96);

   SIMD_8x32::zero_registers();
   }

BOTAN_FUNC_ISA("avx2")
void Serpent::avx2_decrypt_8(const uint8_t in[128], uint8_t out[128]) const
   {
   SIMD_8x32::reset_registers();

   SIMD_8x32 B0 = SIMD_8x32::load_le(in);
   SIMD_8x32 B1 = SIMD_8x32::load_le(in + 32);
   SIMD_8x32 B2 = SIMD_8x32::load_le(in + 64);
   SIMD_8x32 B3 = SIMD_8x32::load_le(in + 96);

   SIMD_8x32::transpose(B0, B1, B2, B3);

   key_xor(32,B0,B1,B2,B3);  SBoxD8(B0,B1,B2,B3); key_xor(31,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor(30,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor(29,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor(28,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor(27,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor(26,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor(25,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor(24,B0,B1,B2,B3);

   i_transform(B0,B1,B2,B3); SBoxD8(B0,B1,B2,B3); key_xor(23,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor(22,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor(21,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor(20,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor(19,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor(18,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor(17,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor(16,B0,B1,B2,B3);

   i_transform(B0,B1,B2,B3); SBoxD8(B0,B1,B2,B3); key_xor(15,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor(14,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor(13,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor(12,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor(11,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor(10,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor( 9,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor( 8,B0,B1,B2,B3);

   i_transform(B0,B1,B2,B3); SBoxD8(B0,B1,B2,B3); key_xor( 7,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD7(B0,B1,B2,B3); key_xor( 6,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD6(B0,B1,B2,B3); key_xor( 5,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD5(B0,B1,B2,B3); key_xor( 4,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD4(B0,B1,B2,B3); key_xor( 3,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD3(B0,B1,B2,B3); key_xor( 2,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD2(B0,B1,B2,B3); key_xor( 1,B0,B1,B2,B3);
   i_transform(B0,B1,B2,B3); SBoxD1(B0,B1,B2,B3); key_xor( 0,B0,B1,B2,B3);

   SIMD_8x32::transpose(B0, B1, B2, B3);

   B0.store_le(out);
   B1.store_le(out + 32);
   B2.store_le(out + 64);
   B3.store_le(out + 96);

   SIMD_8x32::zero_registers();
   }

#undef key_xor
#undef transform
#undef i_transform

}
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
   _mm256_zeroupper();

   const uint64_t* K = m_K.data();
   const uint64_t* T_64 = m_T.data();

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

   _mm256_zeroall();

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
   _mm256_zeroupper();

   const uint64_t* K = m_K.data();
   const uint64_t* T_64 = m_T.data();

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

   _mm256_zeroall();
   }

}
