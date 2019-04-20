/*
* Lightweight wrappers for SIMD operations
* (C) 2009,2011,2016,2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_SIMD_32_H_
#define BOTAN_SIMD_32_H_

#include <botan/types.h>
#include <botan/loadstor.h>
#include <botan/bswap.h>
#include <botan/cpuid.h>

#if defined(BOTAN_TARGET_SUPPORTS_SSE2)
  #include <emmintrin.h>
  #define BOTAN_SIMD_USE_SSE2

#elif defined(BOTAN_TARGET_SUPPORTS_ALTIVEC)
  #include <altivec.h>
  #undef vector
  #undef bool
  #define BOTAN_SIMD_USE_ALTIVEC

#elif defined(BOTAN_TARGET_SUPPORTS_NEON)
  #include <arm_neon.h>
  #define BOTAN_SIMD_USE_NEON

#else
  #include <botan/rotate.h>
#endif

namespace Botan {

/**
* 4x32 bit SIMD register
*
* This class is not a general purpose SIMD type, and only offers
* instructions needed for evaluation of specific crypto primitives.
* For example it does not currently have equality operators of any
* kind.
*
* Implemented for SSE2, VMX (Altivec), and NEON.
*/
class SIMD_4x32 final
   {
   public:

      SIMD_4x32& operator=(const SIMD_4x32& other) = default;
      SIMD_4x32(const SIMD_4x32& other) = default;

      SIMD_4x32& operator=(SIMD_4x32&& other) = default;
      SIMD_4x32(SIMD_4x32&& other) = default;

      /**
      * Zero initialize SIMD register with 4 32-bit elements
      */
      SIMD_4x32() // zero initialized
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_setzero_si128();
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = vec_splat_u32(0);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = vdupq_n_u32(0);
#else
         m_scalar[0] = 0;
         m_scalar[1] = 0;
         m_scalar[2] = 0;
         m_scalar[3] = 0;
#endif
         }

      /**
      * Load SIMD register with 4 32-bit elements
      */
      explicit SIMD_4x32(const uint32_t B[4])
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_loadu_si128(reinterpret_cast<const __m128i*>(B));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = (__vector unsigned int){B[0], B[1], B[2], B[3]};
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = vld1q_u32(B);
#else
         m_scalar[0] = B[0];
         m_scalar[1] = B[1];
         m_scalar[2] = B[2];
         m_scalar[3] = B[3];
#endif
         }

      /**
      * Load SIMD register with 4 32-bit elements
      */
      SIMD_4x32(uint32_t B0, uint32_t B1, uint32_t B2, uint32_t B3)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_set_epi32(B3, B2, B1, B0);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = (__vector unsigned int){B0, B1, B2, B3};
#elif defined(BOTAN_SIMD_USE_NEON)
         // Better way to do this?
         const uint32_t B[4] = { B0, B1, B2, B3 };
         m_neon = vld1q_u32(B);
#else
         m_scalar[0] = B0;
         m_scalar[1] = B1;
         m_scalar[2] = B2;
         m_scalar[3] = B3;
#endif
         }

      /**
      * Load SIMD register with one 32-bit element repeated
      */
      static SIMD_4x32 splat(uint32_t B)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_set1_epi32(B));
#elif defined(BOTAN_SIMD_USE_ARM)
         return SIMD_4x32(vdupq_n_u32(B));
#else
         return SIMD_4x32(B, B, B, B);
#endif
         }

      /**
      * Load a SIMD register with little-endian convention
      */
      static SIMD_4x32 load_le(const void* in)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(in)));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         uint32_t R[4];
         Botan::load_le(R, static_cast<const uint8_t*>(in), 4);
         return SIMD_4x32(R);

#elif defined(BOTAN_SIMD_USE_NEON)

         SIMD_4x32 l(vld1q_u32(static_cast<const uint32_t*>(in)));
         return CPUID::is_big_endian() ? l.bswap() : l;
#else
         SIMD_4x32 out;
         Botan::load_le(out.m_scalar, static_cast<const uint8_t*>(in), 4);
         return out;
#endif
         }

      /**
      * Load a SIMD register with big-endian convention
      */
      static SIMD_4x32 load_be(const void* in)
         {
#if defined(BOTAN_SIMD_USE_SSE2)

         return load_le(in).bswap();

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         uint32_t R[4];
         Botan::load_be(R, static_cast<const uint8_t*>(in), 4);
         return SIMD_4x32(R);

#elif defined(BOTAN_SIMD_USE_NEON)

         SIMD_4x32 l(vld1q_u32(static_cast<const uint32_t*>(in)));
         return CPUID::is_little_endian() ? l.bswap() : l;

#else
         SIMD_4x32 out;
         Botan::load_be(out.m_scalar, static_cast<const uint8_t*>(in), 4);
         return out;
#endif
         }

      /**
      * Load a SIMD register with little-endian convention
      */
      void store_le(uint8_t out[]) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)

         _mm_storeu_si128(reinterpret_cast<__m128i*>(out), m_sse);

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         union {
            __vector unsigned int V;
            uint32_t R[4];
            } vec;
         vec.V = m_vmx;
         Botan::store_le(out, vec.R[0], vec.R[1], vec.R[2], vec.R[3]);

#elif defined(BOTAN_SIMD_USE_NEON)

         if(CPUID::is_big_endian())
            {
            bswap().store_le(out);
            }
         else
            {
            vst1q_u8(out, vreinterpretq_u8_u32(m_neon));
            }
#else
         Botan::store_le(out, m_scalar[0], m_scalar[1], m_scalar[2], m_scalar[3]);
#endif
         }

      /**
      * Load a SIMD register with big-endian convention
      */
      void store_be(uint8_t out[]) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)

         bswap().store_le(out);

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         union {
            __vector unsigned int V;
            uint32_t R[4];
            } vec;
         vec.V = m_vmx;
         Botan::store_be(out, vec.R[0], vec.R[1], vec.R[2], vec.R[3]);

#elif defined(BOTAN_SIMD_USE_NEON)

         if(CPUID::is_little_endian())
            {
            bswap().store_le(out);
            }
         else
            {
            vst1q_u8(out, vreinterpretq_u8_u32(m_neon));
            }

#else
         Botan::store_be(out, m_scalar[0], m_scalar[1], m_scalar[2], m_scalar[3]);
#endif
         }


      /*
      * This is used for SHA-2/SHACAL2
      * Return rotr(ROT1) ^ rotr(ROT2) ^ rotr(ROT3)
      */
      template<size_t ROT1, size_t ROT2, size_t ROT3>
      SIMD_4x32 rho() const
         {
         const SIMD_4x32 rot1 = this->rotr<ROT1>();
         const SIMD_4x32 rot2 = this->rotr<ROT2>();
         const SIMD_4x32 rot3 = this->rotr<ROT3>();
         return (rot1 ^ rot2 ^ rot3);
         }

      /**
      * Left rotation by a compile time constant
      */
      template<size_t ROT>
      SIMD_4x32 rotl() const
         {
         static_assert(ROT > 0 && ROT < 32, "Invalid rotation constant");

#if defined(BOTAN_SIMD_USE_SSE2)

         return SIMD_4x32(_mm_or_si128(_mm_slli_epi32(m_sse, static_cast<int>(ROT)),
                                       _mm_srli_epi32(m_sse, static_cast<int>(32-ROT))));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         const unsigned int r = static_cast<unsigned int>(ROT);
         return SIMD_4x32(vec_rl(m_vmx, (__vector unsigned int){r, r, r, r}));

#elif defined(BOTAN_SIMD_USE_NEON)

         #if defined(BOTAN_TARGET_ARCH_IS_ARM32)

         return SIMD_4x32(vorrq_u32(vshlq_n_u32(m_neon, static_cast<int>(ROT)),
                                    vshrq_n_u32(m_neon, static_cast<int>(32-ROT))));

         #else

         BOTAN_IF_CONSTEXPR(ROT == 8)
            {
            const uint8_t maskb[16] = { 3,0,1,2, 7,4,5,6, 11,8,9,10, 15,12,13,14 };
            const uint8x16_t mask = vld1q_u8(maskb);
            return SIMD_4x32(vreinterpretq_u32_u8(vqtbl1q_u8(vreinterpretq_u8_u32(m_neon), mask)));
            }
         else BOTAN_IF_CONSTEXPR(ROT == 16)
            {
            return SIMD_4x32(vreinterpretq_u32_u16(vrev32q_u16(vreinterpretq_u16_u32(m_neon))));
            }
         else
            {
            return SIMD_4x32(vorrq_u32(vshlq_n_u32(m_neon, static_cast<int>(ROT)),
                                       vshrq_n_u32(m_neon, static_cast<int>(32-ROT))));
            }

         #endif

#else
         return SIMD_4x32(Botan::rotl<ROT>(m_scalar[0]),
                          Botan::rotl<ROT>(m_scalar[1]),
                          Botan::rotl<ROT>(m_scalar[2]),
                          Botan::rotl<ROT>(m_scalar[3]));
#endif
         }

      /**
      * Right rotation by a compile time constant
      */
      template<size_t ROT>
      SIMD_4x32 rotr() const
         {
         return this->rotl<32-ROT>();
         }

      /**
      * Add elements of a SIMD vector
      */
      SIMD_4x32 operator+(const SIMD_4x32& other) const
         {
         SIMD_4x32 retval(*this);
         retval += other;
         return retval;
         }

      /**
      * Subtract elements of a SIMD vector
      */
      SIMD_4x32 operator-(const SIMD_4x32& other) const
         {
         SIMD_4x32 retval(*this);
         retval -= other;
         return retval;
         }

      /**
      * XOR elements of a SIMD vector
      */
      SIMD_4x32 operator^(const SIMD_4x32& other) const
         {
         SIMD_4x32 retval(*this);
         retval ^= other;
         return retval;
         }

      /**
      * Binary OR elements of a SIMD vector
      */
      SIMD_4x32 operator|(const SIMD_4x32& other) const
         {
         SIMD_4x32 retval(*this);
         retval |= other;
         return retval;
         }

      /**
      * Binary AND elements of a SIMD vector
      */
      SIMD_4x32 operator&(const SIMD_4x32& other) const
         {
         SIMD_4x32 retval(*this);
         retval &= other;
         return retval;
         }

      void operator+=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_add_epi32(m_sse, other.m_sse);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = vec_add(m_vmx, other.m_vmx);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = vaddq_u32(m_neon, other.m_neon);
#else
         m_scalar[0] += other.m_scalar[0];
         m_scalar[1] += other.m_scalar[1];
         m_scalar[2] += other.m_scalar[2];
         m_scalar[3] += other.m_scalar[3];
#endif
         }

      void operator-=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_sub_epi32(m_sse, other.m_sse);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = vec_sub(m_vmx, other.m_vmx);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = vsubq_u32(m_neon, other.m_neon);
#else
         m_scalar[0] -= other.m_scalar[0];
         m_scalar[1] -= other.m_scalar[1];
         m_scalar[2] -= other.m_scalar[2];
         m_scalar[3] -= other.m_scalar[3];
#endif
         }

      void operator^=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_xor_si128(m_sse, other.m_sse);

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = vec_xor(m_vmx, other.m_vmx);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = veorq_u32(m_neon, other.m_neon);
#else
         m_scalar[0] ^= other.m_scalar[0];
         m_scalar[1] ^= other.m_scalar[1];
         m_scalar[2] ^= other.m_scalar[2];
         m_scalar[3] ^= other.m_scalar[3];
#endif
         }

      void operator|=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_or_si128(m_sse, other.m_sse);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = vec_or(m_vmx, other.m_vmx);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = vorrq_u32(m_neon, other.m_neon);
#else
         m_scalar[0] |= other.m_scalar[0];
         m_scalar[1] |= other.m_scalar[1];
         m_scalar[2] |= other.m_scalar[2];
         m_scalar[3] |= other.m_scalar[3];
#endif
         }

      void operator&=(const SIMD_4x32& other)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         m_sse = _mm_and_si128(m_sse, other.m_sse);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         m_vmx = vec_and(m_vmx, other.m_vmx);
#elif defined(BOTAN_SIMD_USE_NEON)
         m_neon = vandq_u32(m_neon, other.m_neon);
#else
         m_scalar[0] &= other.m_scalar[0];
         m_scalar[1] &= other.m_scalar[1];
         m_scalar[2] &= other.m_scalar[2];
         m_scalar[3] &= other.m_scalar[3];
#endif
         }


      template<int SHIFT> SIMD_4x32 shl() const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_slli_epi32(m_sse, SHIFT));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const unsigned int s = static_cast<unsigned int>(SHIFT);
         return SIMD_4x32(vec_sl(m_vmx, (__vector unsigned int){s, s, s, s}));
#elif defined(BOTAN_SIMD_USE_NEON)
         return SIMD_4x32(vshlq_n_u32(m_neon, SHIFT));
#else
         return SIMD_4x32(m_scalar[0] << SHIFT,
                          m_scalar[1] << SHIFT,
                          m_scalar[2] << SHIFT,
                          m_scalar[3] << SHIFT);
#endif
         }

      template<int SHIFT> SIMD_4x32 shr() const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_srli_epi32(m_sse, SHIFT));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const unsigned int s = static_cast<unsigned int>(SHIFT);
         return SIMD_4x32(vec_sr(m_vmx, (__vector unsigned int){s, s, s, s}));
#elif defined(BOTAN_SIMD_USE_NEON)
         return SIMD_4x32(vshrq_n_u32(m_neon, SHIFT));
#else
         return SIMD_4x32(m_scalar[0] >> SHIFT, m_scalar[1] >> SHIFT,
                          m_scalar[2] >> SHIFT, m_scalar[3] >> SHIFT);

#endif
         }

      SIMD_4x32 operator~() const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_xor_si128(m_sse, _mm_set1_epi32(0xFFFFFFFF)));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         return SIMD_4x32(vec_nor(m_vmx, m_vmx));
#elif defined(BOTAN_SIMD_USE_NEON)
         return SIMD_4x32(vmvnq_u32(m_neon));
#else
         return SIMD_4x32(~m_scalar[0], ~m_scalar[1], ~m_scalar[2], ~m_scalar[3]);
#endif
         }

      // (~reg) & other
      SIMD_4x32 andc(const SIMD_4x32& other) const
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         return SIMD_4x32(_mm_andnot_si128(m_sse, other.m_sse));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         /*
         AltiVec does arg1 & ~arg2 rather than SSE's ~arg1 & arg2
         so swap the arguments
         */
         return SIMD_4x32(vec_andc(other.m_vmx, m_vmx));
#elif defined(BOTAN_SIMD_USE_NEON)
         // NEON is also a & ~b
         return SIMD_4x32(vbicq_u32(other.m_neon, m_neon));
#else
         return SIMD_4x32((~m_scalar[0]) & other.m_scalar[0],
                          (~m_scalar[1]) & other.m_scalar[1],
                          (~m_scalar[2]) & other.m_scalar[2],
                          (~m_scalar[3]) & other.m_scalar[3]);
#endif
         }

      /**
      * Return copy *this with each word byte swapped
      */
      SIMD_4x32 bswap() const
         {
#if defined(BOTAN_SIMD_USE_SSE2)

         __m128i T = m_sse;
         T = _mm_shufflehi_epi16(T, _MM_SHUFFLE(2, 3, 0, 1));
         T = _mm_shufflelo_epi16(T, _MM_SHUFFLE(2, 3, 0, 1));
         return SIMD_4x32(_mm_or_si128(_mm_srli_epi16(T, 8), _mm_slli_epi16(T, 8)));

#elif defined(BOTAN_SIMD_USE_ALTIVEC)

         union {
            __vector unsigned int V;
            uint32_t R[4];
            } vec;

         vec.V = m_vmx;
         bswap_4(vec.R);
         return SIMD_4x32(vec.R[0], vec.R[1], vec.R[2], vec.R[3]);

#elif defined(BOTAN_SIMD_USE_NEON)

         return SIMD_4x32(vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(m_neon))));

#else
         // scalar
         return SIMD_4x32(reverse_bytes(m_scalar[0]),
                          reverse_bytes(m_scalar[1]),
                          reverse_bytes(m_scalar[2]),
                          reverse_bytes(m_scalar[3]));
#endif
         }

      /**
      * 4x4 Transposition on SIMD registers
      */
      static void transpose(SIMD_4x32& B0, SIMD_4x32& B1,
                            SIMD_4x32& B2, SIMD_4x32& B3)
         {
#if defined(BOTAN_SIMD_USE_SSE2)
         const __m128i T0 = _mm_unpacklo_epi32(B0.m_sse, B1.m_sse);
         const __m128i T1 = _mm_unpacklo_epi32(B2.m_sse, B3.m_sse);
         const __m128i T2 = _mm_unpackhi_epi32(B0.m_sse, B1.m_sse);
         const __m128i T3 = _mm_unpackhi_epi32(B2.m_sse, B3.m_sse);

         B0.m_sse = _mm_unpacklo_epi64(T0, T1);
         B1.m_sse = _mm_unpackhi_epi64(T0, T1);
         B2.m_sse = _mm_unpacklo_epi64(T2, T3);
         B3.m_sse = _mm_unpackhi_epi64(T2, T3);
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
         const __vector unsigned int T0 = vec_mergeh(B0.m_vmx, B2.m_vmx);
         const __vector unsigned int T1 = vec_mergeh(B1.m_vmx, B3.m_vmx);
         const __vector unsigned int T2 = vec_mergel(B0.m_vmx, B2.m_vmx);
         const __vector unsigned int T3 = vec_mergel(B1.m_vmx, B3.m_vmx);

         B0.m_vmx = vec_mergeh(T0, T1);
         B1.m_vmx = vec_mergel(T0, T1);
         B2.m_vmx = vec_mergeh(T2, T3);
         B3.m_vmx = vec_mergel(T2, T3);
#elif defined(BOTAN_SIMD_USE_NEON)

#if defined(BOTAN_TARGET_ARCH_IS_ARM32)

         const uint32x4x2_t T0 = vzipq_u32(B0.m_neon, B2.m_neon);
         const uint32x4x2_t T1 = vzipq_u32(B1.m_neon, B3.m_neon);
         const uint32x4x2_t O0 = vzipq_u32(T0.val[0], T1.val[0]);
         const uint32x4x2_t O1 = vzipq_u32(T0.val[1], T1.val[1]);

         B0.m_neon = O0.val[0];
         B1.m_neon = O0.val[1];
         B2.m_neon = O1.val[0];
         B3.m_neon = O1.val[1];

#elif defined(BOTAN_TARGET_ARCH_IS_ARM64)
         const uint32x4_t T0 = vzip1q_u32(B0.m_neon, B2.m_neon);
         const uint32x4_t T2 = vzip2q_u32(B0.m_neon, B2.m_neon);

         const uint32x4_t T1 = vzip1q_u32(B1.m_neon, B3.m_neon);
         const uint32x4_t T3 = vzip2q_u32(B1.m_neon, B3.m_neon);

         B0.m_neon = vzip1q_u32(T0, T1);
         B1.m_neon = vzip2q_u32(T0, T1);

         B2.m_neon = vzip1q_u32(T2, T3);
         B3.m_neon = vzip2q_u32(T2, T3);
#endif

#else
         // scalar
         SIMD_4x32 T0(B0.m_scalar[0], B1.m_scalar[0], B2.m_scalar[0], B3.m_scalar[0]);
         SIMD_4x32 T1(B0.m_scalar[1], B1.m_scalar[1], B2.m_scalar[1], B3.m_scalar[1]);
         SIMD_4x32 T2(B0.m_scalar[2], B1.m_scalar[2], B2.m_scalar[2], B3.m_scalar[2]);
         SIMD_4x32 T3(B0.m_scalar[3], B1.m_scalar[3], B2.m_scalar[3], B3.m_scalar[3]);

         B0 = T0;
         B1 = T1;
         B2 = T2;
         B3 = T3;
#endif
         }

   private:

#if defined(BOTAN_SIMD_USE_SSE2)
      explicit SIMD_4x32(__m128i in) : m_sse(in) {}
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
      explicit SIMD_4x32(__vector unsigned int in) : m_vmx(in) {}
#elif defined(BOTAN_SIMD_USE_NEON)
      explicit SIMD_4x32(uint32x4_t in) : m_neon(in) {}
#endif

#if defined(BOTAN_SIMD_USE_SSE2)
      __m128i m_sse;
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
      __vector unsigned int m_vmx;
#elif defined(BOTAN_SIMD_USE_NEON)
      uint32x4_t m_neon;
#else
      uint32_t m_scalar[4];
#endif
   };

}

#endif
