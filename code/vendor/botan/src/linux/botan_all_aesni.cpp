/*
* Botan 2.0.1 Amalgamation
* (C) 1999-2013,2014,2015,2016 Jack Lloyd and others
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "botan_all.h"
#include "botan_all_internal.h"

#if defined(__GNUG__)
#pragma GCC target ("aes,ssse3,pclmul")
#endif
/*
* AES using AES-NI instructions
* (C) 2009,2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <wmmintrin.h>

namespace Botan {

namespace {

BOTAN_FUNC_ISA("ssse3")
__m128i aes_128_key_expansion(__m128i key, __m128i key_with_rcon)
   {
   key_with_rcon = _mm_shuffle_epi32(key_with_rcon, _MM_SHUFFLE(3,3,3,3));
   key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
   key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
   key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
   return _mm_xor_si128(key, key_with_rcon);
   }

BOTAN_FUNC_ISA("ssse3")
void aes_192_key_expansion(__m128i* K1, __m128i* K2, __m128i key2_with_rcon,
                           uint32_t out[], bool last)
   {
   __m128i key1 = *K1;
   __m128i key2 = *K2;

   key2_with_rcon  = _mm_shuffle_epi32(key2_with_rcon, _MM_SHUFFLE(1,1,1,1));
   key1 = _mm_xor_si128(key1, _mm_slli_si128(key1, 4));
   key1 = _mm_xor_si128(key1, _mm_slli_si128(key1, 4));
   key1 = _mm_xor_si128(key1, _mm_slli_si128(key1, 4));
   key1 = _mm_xor_si128(key1, key2_with_rcon);

   *K1 = key1;
   _mm_storeu_si128(reinterpret_cast<__m128i*>(out), key1);

   if(last)
      return;

   key2 = _mm_xor_si128(key2, _mm_slli_si128(key2, 4));
   key2 = _mm_xor_si128(key2, _mm_shuffle_epi32(key1, _MM_SHUFFLE(3,3,3,3)));

   *K2 = key2;
   out[4] = _mm_cvtsi128_si32(key2);
   out[5] = _mm_cvtsi128_si32(_mm_srli_si128(key2, 4));
   }

/*
* The second half of the AES-256 key expansion (other half same as AES-128)
*/
BOTAN_FUNC_ISA("ssse3,aes")
__m128i aes_256_key_expansion(__m128i key, __m128i key2)
   {
   __m128i key_with_rcon = _mm_aeskeygenassist_si128(key2, 0x00);
   key_with_rcon = _mm_shuffle_epi32(key_with_rcon, _MM_SHUFFLE(2,2,2,2));

   key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
   key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
   key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
   return _mm_xor_si128(key, key_with_rcon);
   }

}

#define AES_ENC_4_ROUNDS(K)                     \
   do                                           \
      {                                         \
      B0 = _mm_aesenc_si128(B0, K);             \
      B1 = _mm_aesenc_si128(B1, K);             \
      B2 = _mm_aesenc_si128(B2, K);             \
      B3 = _mm_aesenc_si128(B3, K);             \
      } while(0)

#define AES_ENC_4_LAST_ROUNDS(K)                \
   do                                           \
      {                                         \
      B0 = _mm_aesenclast_si128(B0, K);         \
      B1 = _mm_aesenclast_si128(B1, K);         \
      B2 = _mm_aesenclast_si128(B2, K);         \
      B3 = _mm_aesenclast_si128(B3, K);         \
      } while(0)

#define AES_DEC_4_ROUNDS(K)                     \
   do                                           \
      {                                         \
      B0 = _mm_aesdec_si128(B0, K);             \
      B1 = _mm_aesdec_si128(B1, K);             \
      B2 = _mm_aesdec_si128(B2, K);             \
      B3 = _mm_aesdec_si128(B3, K);             \
      } while(0)

#define AES_DEC_4_LAST_ROUNDS(K)                \
   do                                           \
      {                                         \
      B0 = _mm_aesdeclast_si128(B0, K);         \
      B1 = _mm_aesdeclast_si128(B1, K);         \
      B2 = _mm_aesdeclast_si128(B2, K);         \
      B3 = _mm_aesdeclast_si128(B3, K);         \
      } while(0)

/*
* AES-128 Encryption
*/
BOTAN_FUNC_ISA("ssse3,aes")
void AES_128::aesni_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   const __m128i* in_mm = reinterpret_cast<const __m128i*>(in);
   __m128i* out_mm = reinterpret_cast<__m128i*>(out);

   const __m128i* key_mm = reinterpret_cast<const __m128i*>(m_EK.data());

   __m128i K0  = _mm_loadu_si128(key_mm);
   __m128i K1  = _mm_loadu_si128(key_mm + 1);
   __m128i K2  = _mm_loadu_si128(key_mm + 2);
   __m128i K3  = _mm_loadu_si128(key_mm + 3);
   __m128i K4  = _mm_loadu_si128(key_mm + 4);
   __m128i K5  = _mm_loadu_si128(key_mm + 5);
   __m128i K6  = _mm_loadu_si128(key_mm + 6);
   __m128i K7  = _mm_loadu_si128(key_mm + 7);
   __m128i K8  = _mm_loadu_si128(key_mm + 8);
   __m128i K9  = _mm_loadu_si128(key_mm + 9);
   __m128i K10 = _mm_loadu_si128(key_mm + 10);

   while(blocks >= 4)
      {
      __m128i B0 = _mm_loadu_si128(in_mm + 0);
      __m128i B1 = _mm_loadu_si128(in_mm + 1);
      __m128i B2 = _mm_loadu_si128(in_mm + 2);
      __m128i B3 = _mm_loadu_si128(in_mm + 3);

      B0 = _mm_xor_si128(B0, K0);
      B1 = _mm_xor_si128(B1, K0);
      B2 = _mm_xor_si128(B2, K0);
      B3 = _mm_xor_si128(B3, K0);

      AES_ENC_4_ROUNDS(K1);
      AES_ENC_4_ROUNDS(K2);
      AES_ENC_4_ROUNDS(K3);
      AES_ENC_4_ROUNDS(K4);
      AES_ENC_4_ROUNDS(K5);
      AES_ENC_4_ROUNDS(K6);
      AES_ENC_4_ROUNDS(K7);
      AES_ENC_4_ROUNDS(K8);
      AES_ENC_4_ROUNDS(K9);
      AES_ENC_4_LAST_ROUNDS(K10);

      _mm_storeu_si128(out_mm + 0, B0);
      _mm_storeu_si128(out_mm + 1, B1);
      _mm_storeu_si128(out_mm + 2, B2);
      _mm_storeu_si128(out_mm + 3, B3);

      blocks -= 4;
      in_mm += 4;
      out_mm += 4;
      }

   for(size_t i = 0; i != blocks; ++i)
      {
      __m128i B = _mm_loadu_si128(in_mm + i);

      B = _mm_xor_si128(B, K0);

      B = _mm_aesenc_si128(B, K1);
      B = _mm_aesenc_si128(B, K2);
      B = _mm_aesenc_si128(B, K3);
      B = _mm_aesenc_si128(B, K4);
      B = _mm_aesenc_si128(B, K5);
      B = _mm_aesenc_si128(B, K6);
      B = _mm_aesenc_si128(B, K7);
      B = _mm_aesenc_si128(B, K8);
      B = _mm_aesenc_si128(B, K9);
      B = _mm_aesenclast_si128(B, K10);

      _mm_storeu_si128(out_mm + i, B);
      }
   }

/*
* AES-128 Decryption
*/
BOTAN_FUNC_ISA("ssse3,aes")
void AES_128::aesni_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   const __m128i* in_mm = reinterpret_cast<const __m128i*>(in);
   __m128i* out_mm = reinterpret_cast<__m128i*>(out);

   const __m128i* key_mm = reinterpret_cast<const __m128i*>(m_DK.data());

   __m128i K0  = _mm_loadu_si128(key_mm);
   __m128i K1  = _mm_loadu_si128(key_mm + 1);
   __m128i K2  = _mm_loadu_si128(key_mm + 2);
   __m128i K3  = _mm_loadu_si128(key_mm + 3);
   __m128i K4  = _mm_loadu_si128(key_mm + 4);
   __m128i K5  = _mm_loadu_si128(key_mm + 5);
   __m128i K6  = _mm_loadu_si128(key_mm + 6);
   __m128i K7  = _mm_loadu_si128(key_mm + 7);
   __m128i K8  = _mm_loadu_si128(key_mm + 8);
   __m128i K9  = _mm_loadu_si128(key_mm + 9);
   __m128i K10 = _mm_loadu_si128(key_mm + 10);

   while(blocks >= 4)
      {
      __m128i B0 = _mm_loadu_si128(in_mm + 0);
      __m128i B1 = _mm_loadu_si128(in_mm + 1);
      __m128i B2 = _mm_loadu_si128(in_mm + 2);
      __m128i B3 = _mm_loadu_si128(in_mm + 3);

      B0 = _mm_xor_si128(B0, K0);
      B1 = _mm_xor_si128(B1, K0);
      B2 = _mm_xor_si128(B2, K0);
      B3 = _mm_xor_si128(B3, K0);

      AES_DEC_4_ROUNDS(K1);
      AES_DEC_4_ROUNDS(K2);
      AES_DEC_4_ROUNDS(K3);
      AES_DEC_4_ROUNDS(K4);
      AES_DEC_4_ROUNDS(K5);
      AES_DEC_4_ROUNDS(K6);
      AES_DEC_4_ROUNDS(K7);
      AES_DEC_4_ROUNDS(K8);
      AES_DEC_4_ROUNDS(K9);
      AES_DEC_4_LAST_ROUNDS(K10);

      _mm_storeu_si128(out_mm + 0, B0);
      _mm_storeu_si128(out_mm + 1, B1);
      _mm_storeu_si128(out_mm + 2, B2);
      _mm_storeu_si128(out_mm + 3, B3);

      blocks -= 4;
      in_mm += 4;
      out_mm += 4;
      }

   for(size_t i = 0; i != blocks; ++i)
      {
      __m128i B = _mm_loadu_si128(in_mm + i);

      B = _mm_xor_si128(B, K0);

      B = _mm_aesdec_si128(B, K1);
      B = _mm_aesdec_si128(B, K2);
      B = _mm_aesdec_si128(B, K3);
      B = _mm_aesdec_si128(B, K4);
      B = _mm_aesdec_si128(B, K5);
      B = _mm_aesdec_si128(B, K6);
      B = _mm_aesdec_si128(B, K7);
      B = _mm_aesdec_si128(B, K8);
      B = _mm_aesdec_si128(B, K9);
      B = _mm_aesdeclast_si128(B, K10);

      _mm_storeu_si128(out_mm + i, B);
      }
   }

/*
* AES-128 Key Schedule
*/
BOTAN_FUNC_ISA("ssse3,aes")
void AES_128::aesni_key_schedule(const uint8_t key[], size_t)
   {
   m_EK.resize(44);
   m_DK.resize(44);

   #define AES_128_key_exp(K, RCON) \
      aes_128_key_expansion(K, _mm_aeskeygenassist_si128(K, RCON))

   __m128i K0  = _mm_loadu_si128(reinterpret_cast<const __m128i*>(key));
   __m128i K1  = AES_128_key_exp(K0, 0x01);
   __m128i K2  = AES_128_key_exp(K1, 0x02);
   __m128i K3  = AES_128_key_exp(K2, 0x04);
   __m128i K4  = AES_128_key_exp(K3, 0x08);
   __m128i K5  = AES_128_key_exp(K4, 0x10);
   __m128i K6  = AES_128_key_exp(K5, 0x20);
   __m128i K7  = AES_128_key_exp(K6, 0x40);
   __m128i K8  = AES_128_key_exp(K7, 0x80);
   __m128i K9  = AES_128_key_exp(K8, 0x1B);
   __m128i K10 = AES_128_key_exp(K9, 0x36);

   __m128i* EK_mm = reinterpret_cast<__m128i*>(m_EK.data());
   _mm_storeu_si128(EK_mm     , K0);
   _mm_storeu_si128(EK_mm +  1, K1);
   _mm_storeu_si128(EK_mm +  2, K2);
   _mm_storeu_si128(EK_mm +  3, K3);
   _mm_storeu_si128(EK_mm +  4, K4);
   _mm_storeu_si128(EK_mm +  5, K5);
   _mm_storeu_si128(EK_mm +  6, K6);
   _mm_storeu_si128(EK_mm +  7, K7);
   _mm_storeu_si128(EK_mm +  8, K8);
   _mm_storeu_si128(EK_mm +  9, K9);
   _mm_storeu_si128(EK_mm + 10, K10);

   // Now generate decryption keys

   __m128i* DK_mm = reinterpret_cast<__m128i*>(m_DK.data());
   _mm_storeu_si128(DK_mm     , K10);
   _mm_storeu_si128(DK_mm +  1, _mm_aesimc_si128(K9));
   _mm_storeu_si128(DK_mm +  2, _mm_aesimc_si128(K8));
   _mm_storeu_si128(DK_mm +  3, _mm_aesimc_si128(K7));
   _mm_storeu_si128(DK_mm +  4, _mm_aesimc_si128(K6));
   _mm_storeu_si128(DK_mm +  5, _mm_aesimc_si128(K5));
   _mm_storeu_si128(DK_mm +  6, _mm_aesimc_si128(K4));
   _mm_storeu_si128(DK_mm +  7, _mm_aesimc_si128(K3));
   _mm_storeu_si128(DK_mm +  8, _mm_aesimc_si128(K2));
   _mm_storeu_si128(DK_mm +  9, _mm_aesimc_si128(K1));
   _mm_storeu_si128(DK_mm + 10, K0);
   }

/*
* AES-192 Encryption
*/
BOTAN_FUNC_ISA("ssse3,aes")
void AES_192::aesni_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   const __m128i* in_mm = reinterpret_cast<const __m128i*>(in);
   __m128i* out_mm = reinterpret_cast<__m128i*>(out);

   const __m128i* key_mm = reinterpret_cast<const __m128i*>(m_EK.data());

   __m128i K0  = _mm_loadu_si128(key_mm);
   __m128i K1  = _mm_loadu_si128(key_mm + 1);
   __m128i K2  = _mm_loadu_si128(key_mm + 2);
   __m128i K3  = _mm_loadu_si128(key_mm + 3);
   __m128i K4  = _mm_loadu_si128(key_mm + 4);
   __m128i K5  = _mm_loadu_si128(key_mm + 5);
   __m128i K6  = _mm_loadu_si128(key_mm + 6);
   __m128i K7  = _mm_loadu_si128(key_mm + 7);
   __m128i K8  = _mm_loadu_si128(key_mm + 8);
   __m128i K9  = _mm_loadu_si128(key_mm + 9);
   __m128i K10 = _mm_loadu_si128(key_mm + 10);
   __m128i K11 = _mm_loadu_si128(key_mm + 11);
   __m128i K12 = _mm_loadu_si128(key_mm + 12);

   while(blocks >= 4)
      {
      __m128i B0 = _mm_loadu_si128(in_mm + 0);
      __m128i B1 = _mm_loadu_si128(in_mm + 1);
      __m128i B2 = _mm_loadu_si128(in_mm + 2);
      __m128i B3 = _mm_loadu_si128(in_mm + 3);

      B0 = _mm_xor_si128(B0, K0);
      B1 = _mm_xor_si128(B1, K0);
      B2 = _mm_xor_si128(B2, K0);
      B3 = _mm_xor_si128(B3, K0);

      AES_ENC_4_ROUNDS(K1);
      AES_ENC_4_ROUNDS(K2);
      AES_ENC_4_ROUNDS(K3);
      AES_ENC_4_ROUNDS(K4);
      AES_ENC_4_ROUNDS(K5);
      AES_ENC_4_ROUNDS(K6);
      AES_ENC_4_ROUNDS(K7);
      AES_ENC_4_ROUNDS(K8);
      AES_ENC_4_ROUNDS(K9);
      AES_ENC_4_ROUNDS(K10);
      AES_ENC_4_ROUNDS(K11);
      AES_ENC_4_LAST_ROUNDS(K12);

      _mm_storeu_si128(out_mm + 0, B0);
      _mm_storeu_si128(out_mm + 1, B1);
      _mm_storeu_si128(out_mm + 2, B2);
      _mm_storeu_si128(out_mm + 3, B3);

      blocks -= 4;
      in_mm += 4;
      out_mm += 4;
      }

   for(size_t i = 0; i != blocks; ++i)
      {
      __m128i B = _mm_loadu_si128(in_mm + i);

      B = _mm_xor_si128(B, K0);

      B = _mm_aesenc_si128(B, K1);
      B = _mm_aesenc_si128(B, K2);
      B = _mm_aesenc_si128(B, K3);
      B = _mm_aesenc_si128(B, K4);
      B = _mm_aesenc_si128(B, K5);
      B = _mm_aesenc_si128(B, K6);
      B = _mm_aesenc_si128(B, K7);
      B = _mm_aesenc_si128(B, K8);
      B = _mm_aesenc_si128(B, K9);
      B = _mm_aesenc_si128(B, K10);
      B = _mm_aesenc_si128(B, K11);
      B = _mm_aesenclast_si128(B, K12);

      _mm_storeu_si128(out_mm + i, B);
      }
   }

/*
* AES-192 Decryption
*/
BOTAN_FUNC_ISA("ssse3,aes")
void AES_192::aesni_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   const __m128i* in_mm = reinterpret_cast<const __m128i*>(in);
   __m128i* out_mm = reinterpret_cast<__m128i*>(out);

   const __m128i* key_mm = reinterpret_cast<const __m128i*>(m_DK.data());

   __m128i K0  = _mm_loadu_si128(key_mm);
   __m128i K1  = _mm_loadu_si128(key_mm + 1);
   __m128i K2  = _mm_loadu_si128(key_mm + 2);
   __m128i K3  = _mm_loadu_si128(key_mm + 3);
   __m128i K4  = _mm_loadu_si128(key_mm + 4);
   __m128i K5  = _mm_loadu_si128(key_mm + 5);
   __m128i K6  = _mm_loadu_si128(key_mm + 6);
   __m128i K7  = _mm_loadu_si128(key_mm + 7);
   __m128i K8  = _mm_loadu_si128(key_mm + 8);
   __m128i K9  = _mm_loadu_si128(key_mm + 9);
   __m128i K10 = _mm_loadu_si128(key_mm + 10);
   __m128i K11 = _mm_loadu_si128(key_mm + 11);
   __m128i K12 = _mm_loadu_si128(key_mm + 12);

   while(blocks >= 4)
      {
      __m128i B0 = _mm_loadu_si128(in_mm + 0);
      __m128i B1 = _mm_loadu_si128(in_mm + 1);
      __m128i B2 = _mm_loadu_si128(in_mm + 2);
      __m128i B3 = _mm_loadu_si128(in_mm + 3);

      B0 = _mm_xor_si128(B0, K0);
      B1 = _mm_xor_si128(B1, K0);
      B2 = _mm_xor_si128(B2, K0);
      B3 = _mm_xor_si128(B3, K0);

      AES_DEC_4_ROUNDS(K1);
      AES_DEC_4_ROUNDS(K2);
      AES_DEC_4_ROUNDS(K3);
      AES_DEC_4_ROUNDS(K4);
      AES_DEC_4_ROUNDS(K5);
      AES_DEC_4_ROUNDS(K6);
      AES_DEC_4_ROUNDS(K7);
      AES_DEC_4_ROUNDS(K8);
      AES_DEC_4_ROUNDS(K9);
      AES_DEC_4_ROUNDS(K10);
      AES_DEC_4_ROUNDS(K11);
      AES_DEC_4_LAST_ROUNDS(K12);

      _mm_storeu_si128(out_mm + 0, B0);
      _mm_storeu_si128(out_mm + 1, B1);
      _mm_storeu_si128(out_mm + 2, B2);
      _mm_storeu_si128(out_mm + 3, B3);

      blocks -= 4;
      in_mm += 4;
      out_mm += 4;
      }

   for(size_t i = 0; i != blocks; ++i)
      {
      __m128i B = _mm_loadu_si128(in_mm + i);

      B = _mm_xor_si128(B, K0);

      B = _mm_aesdec_si128(B, K1);
      B = _mm_aesdec_si128(B, K2);
      B = _mm_aesdec_si128(B, K3);
      B = _mm_aesdec_si128(B, K4);
      B = _mm_aesdec_si128(B, K5);
      B = _mm_aesdec_si128(B, K6);
      B = _mm_aesdec_si128(B, K7);
      B = _mm_aesdec_si128(B, K8);
      B = _mm_aesdec_si128(B, K9);
      B = _mm_aesdec_si128(B, K10);
      B = _mm_aesdec_si128(B, K11);
      B = _mm_aesdeclast_si128(B, K12);

      _mm_storeu_si128(out_mm + i, B);
      }
   }

/*
* AES-192 Key Schedule
*/
BOTAN_FUNC_ISA("ssse3,aes")
void AES_192::aesni_key_schedule(const uint8_t key[], size_t)
   {
   m_EK.resize(52);
   m_DK.resize(52);

   __m128i K0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(key));
   __m128i K1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(key + 8));
   K1 = _mm_srli_si128(K1, 8);

   load_le(m_EK.data(), key, 6);

   #define AES_192_key_exp(RCON, EK_OFF)                         \
     aes_192_key_expansion(&K0, &K1,                             \
                           _mm_aeskeygenassist_si128(K1, RCON),  \
                           &m_EK[EK_OFF], EK_OFF == 48)

   AES_192_key_exp(0x01, 6);
   AES_192_key_exp(0x02, 12);
   AES_192_key_exp(0x04, 18);
   AES_192_key_exp(0x08, 24);
   AES_192_key_exp(0x10, 30);
   AES_192_key_exp(0x20, 36);
   AES_192_key_exp(0x40, 42);
   AES_192_key_exp(0x80, 48);

   #undef AES_192_key_exp

   // Now generate decryption keys
   const __m128i* EK_mm = reinterpret_cast<const __m128i*>(m_EK.data());

   __m128i* DK_mm = reinterpret_cast<__m128i*>(m_DK.data());
   _mm_storeu_si128(DK_mm     , _mm_loadu_si128(EK_mm + 12));
   _mm_storeu_si128(DK_mm +  1, _mm_aesimc_si128(_mm_loadu_si128(EK_mm + 11)));
   _mm_storeu_si128(DK_mm +  2, _mm_aesimc_si128(_mm_loadu_si128(EK_mm + 10)));
   _mm_storeu_si128(DK_mm +  3, _mm_aesimc_si128(_mm_loadu_si128(EK_mm + 9)));
   _mm_storeu_si128(DK_mm +  4, _mm_aesimc_si128(_mm_loadu_si128(EK_mm + 8)));
   _mm_storeu_si128(DK_mm +  5, _mm_aesimc_si128(_mm_loadu_si128(EK_mm + 7)));
   _mm_storeu_si128(DK_mm +  6, _mm_aesimc_si128(_mm_loadu_si128(EK_mm + 6)));
   _mm_storeu_si128(DK_mm +  7, _mm_aesimc_si128(_mm_loadu_si128(EK_mm + 5)));
   _mm_storeu_si128(DK_mm +  8, _mm_aesimc_si128(_mm_loadu_si128(EK_mm + 4)));
   _mm_storeu_si128(DK_mm +  9, _mm_aesimc_si128(_mm_loadu_si128(EK_mm + 3)));
   _mm_storeu_si128(DK_mm + 10, _mm_aesimc_si128(_mm_loadu_si128(EK_mm + 2)));
   _mm_storeu_si128(DK_mm + 11, _mm_aesimc_si128(_mm_loadu_si128(EK_mm + 1)));
   _mm_storeu_si128(DK_mm + 12, _mm_loadu_si128(EK_mm + 0));
   }

/*
* AES-256 Encryption
*/
BOTAN_FUNC_ISA("ssse3,aes")
void AES_256::aesni_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   const __m128i* in_mm = reinterpret_cast<const __m128i*>(in);
   __m128i* out_mm = reinterpret_cast<__m128i*>(out);

   const __m128i* key_mm = reinterpret_cast<const __m128i*>(m_EK.data());

   __m128i K0  = _mm_loadu_si128(key_mm);
   __m128i K1  = _mm_loadu_si128(key_mm + 1);
   __m128i K2  = _mm_loadu_si128(key_mm + 2);
   __m128i K3  = _mm_loadu_si128(key_mm + 3);
   __m128i K4  = _mm_loadu_si128(key_mm + 4);
   __m128i K5  = _mm_loadu_si128(key_mm + 5);
   __m128i K6  = _mm_loadu_si128(key_mm + 6);
   __m128i K7  = _mm_loadu_si128(key_mm + 7);
   __m128i K8  = _mm_loadu_si128(key_mm + 8);
   __m128i K9  = _mm_loadu_si128(key_mm + 9);
   __m128i K10 = _mm_loadu_si128(key_mm + 10);
   __m128i K11 = _mm_loadu_si128(key_mm + 11);
   __m128i K12 = _mm_loadu_si128(key_mm + 12);
   __m128i K13 = _mm_loadu_si128(key_mm + 13);
   __m128i K14 = _mm_loadu_si128(key_mm + 14);

   while(blocks >= 4)
      {
      __m128i B0 = _mm_loadu_si128(in_mm + 0);
      __m128i B1 = _mm_loadu_si128(in_mm + 1);
      __m128i B2 = _mm_loadu_si128(in_mm + 2);
      __m128i B3 = _mm_loadu_si128(in_mm + 3);

      B0 = _mm_xor_si128(B0, K0);
      B1 = _mm_xor_si128(B1, K0);
      B2 = _mm_xor_si128(B2, K0);
      B3 = _mm_xor_si128(B3, K0);

      AES_ENC_4_ROUNDS(K1);
      AES_ENC_4_ROUNDS(K2);
      AES_ENC_4_ROUNDS(K3);
      AES_ENC_4_ROUNDS(K4);
      AES_ENC_4_ROUNDS(K5);
      AES_ENC_4_ROUNDS(K6);
      AES_ENC_4_ROUNDS(K7);
      AES_ENC_4_ROUNDS(K8);
      AES_ENC_4_ROUNDS(K9);
      AES_ENC_4_ROUNDS(K10);
      AES_ENC_4_ROUNDS(K11);
      AES_ENC_4_ROUNDS(K12);
      AES_ENC_4_ROUNDS(K13);
      AES_ENC_4_LAST_ROUNDS(K14);

      _mm_storeu_si128(out_mm + 0, B0);
      _mm_storeu_si128(out_mm + 1, B1);
      _mm_storeu_si128(out_mm + 2, B2);
      _mm_storeu_si128(out_mm + 3, B3);

      blocks -= 4;
      in_mm += 4;
      out_mm += 4;
      }

   for(size_t i = 0; i != blocks; ++i)
      {
      __m128i B = _mm_loadu_si128(in_mm + i);

      B = _mm_xor_si128(B, K0);

      B = _mm_aesenc_si128(B, K1);
      B = _mm_aesenc_si128(B, K2);
      B = _mm_aesenc_si128(B, K3);
      B = _mm_aesenc_si128(B, K4);
      B = _mm_aesenc_si128(B, K5);
      B = _mm_aesenc_si128(B, K6);
      B = _mm_aesenc_si128(B, K7);
      B = _mm_aesenc_si128(B, K8);
      B = _mm_aesenc_si128(B, K9);
      B = _mm_aesenc_si128(B, K10);
      B = _mm_aesenc_si128(B, K11);
      B = _mm_aesenc_si128(B, K12);
      B = _mm_aesenc_si128(B, K13);
      B = _mm_aesenclast_si128(B, K14);

      _mm_storeu_si128(out_mm + i, B);
      }
   }

/*
* AES-256 Decryption
*/
BOTAN_FUNC_ISA("ssse3,aes")
void AES_256::aesni_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   const __m128i* in_mm = reinterpret_cast<const __m128i*>(in);
   __m128i* out_mm = reinterpret_cast<__m128i*>(out);

   const __m128i* key_mm = reinterpret_cast<const __m128i*>(m_DK.data());

   __m128i K0  = _mm_loadu_si128(key_mm);
   __m128i K1  = _mm_loadu_si128(key_mm + 1);
   __m128i K2  = _mm_loadu_si128(key_mm + 2);
   __m128i K3  = _mm_loadu_si128(key_mm + 3);
   __m128i K4  = _mm_loadu_si128(key_mm + 4);
   __m128i K5  = _mm_loadu_si128(key_mm + 5);
   __m128i K6  = _mm_loadu_si128(key_mm + 6);
   __m128i K7  = _mm_loadu_si128(key_mm + 7);
   __m128i K8  = _mm_loadu_si128(key_mm + 8);
   __m128i K9  = _mm_loadu_si128(key_mm + 9);
   __m128i K10 = _mm_loadu_si128(key_mm + 10);
   __m128i K11 = _mm_loadu_si128(key_mm + 11);
   __m128i K12 = _mm_loadu_si128(key_mm + 12);
   __m128i K13 = _mm_loadu_si128(key_mm + 13);
   __m128i K14 = _mm_loadu_si128(key_mm + 14);

   while(blocks >= 4)
      {
      __m128i B0 = _mm_loadu_si128(in_mm + 0);
      __m128i B1 = _mm_loadu_si128(in_mm + 1);
      __m128i B2 = _mm_loadu_si128(in_mm + 2);
      __m128i B3 = _mm_loadu_si128(in_mm + 3);

      B0 = _mm_xor_si128(B0, K0);
      B1 = _mm_xor_si128(B1, K0);
      B2 = _mm_xor_si128(B2, K0);
      B3 = _mm_xor_si128(B3, K0);

      AES_DEC_4_ROUNDS(K1);
      AES_DEC_4_ROUNDS(K2);
      AES_DEC_4_ROUNDS(K3);
      AES_DEC_4_ROUNDS(K4);
      AES_DEC_4_ROUNDS(K5);
      AES_DEC_4_ROUNDS(K6);
      AES_DEC_4_ROUNDS(K7);
      AES_DEC_4_ROUNDS(K8);
      AES_DEC_4_ROUNDS(K9);
      AES_DEC_4_ROUNDS(K10);
      AES_DEC_4_ROUNDS(K11);
      AES_DEC_4_ROUNDS(K12);
      AES_DEC_4_ROUNDS(K13);
      AES_DEC_4_LAST_ROUNDS(K14);

      _mm_storeu_si128(out_mm + 0, B0);
      _mm_storeu_si128(out_mm + 1, B1);
      _mm_storeu_si128(out_mm + 2, B2);
      _mm_storeu_si128(out_mm + 3, B3);

      blocks -= 4;
      in_mm += 4;
      out_mm += 4;
      }

   for(size_t i = 0; i != blocks; ++i)
      {
      __m128i B = _mm_loadu_si128(in_mm + i);

      B = _mm_xor_si128(B, K0);

      B = _mm_aesdec_si128(B, K1);
      B = _mm_aesdec_si128(B, K2);
      B = _mm_aesdec_si128(B, K3);
      B = _mm_aesdec_si128(B, K4);
      B = _mm_aesdec_si128(B, K5);
      B = _mm_aesdec_si128(B, K6);
      B = _mm_aesdec_si128(B, K7);
      B = _mm_aesdec_si128(B, K8);
      B = _mm_aesdec_si128(B, K9);
      B = _mm_aesdec_si128(B, K10);
      B = _mm_aesdec_si128(B, K11);
      B = _mm_aesdec_si128(B, K12);
      B = _mm_aesdec_si128(B, K13);
      B = _mm_aesdeclast_si128(B, K14);

      _mm_storeu_si128(out_mm + i, B);
      }
   }

/*
* AES-256 Key Schedule
*/
BOTAN_FUNC_ISA("ssse3,aes")
void AES_256::aesni_key_schedule(const uint8_t key[], size_t)
   {
   m_EK.resize(60);
   m_DK.resize(60);

   __m128i K0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(key));
   __m128i K1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(key + 16));

   __m128i K2 = aes_128_key_expansion(K0, _mm_aeskeygenassist_si128(K1, 0x01));
   __m128i K3 = aes_256_key_expansion(K1, K2);

   __m128i K4 = aes_128_key_expansion(K2, _mm_aeskeygenassist_si128(K3, 0x02));
   __m128i K5 = aes_256_key_expansion(K3, K4);

   __m128i K6 = aes_128_key_expansion(K4, _mm_aeskeygenassist_si128(K5, 0x04));
   __m128i K7 = aes_256_key_expansion(K5, K6);

   __m128i K8 = aes_128_key_expansion(K6, _mm_aeskeygenassist_si128(K7, 0x08));
   __m128i K9 = aes_256_key_expansion(K7, K8);

   __m128i K10 = aes_128_key_expansion(K8, _mm_aeskeygenassist_si128(K9, 0x10));
   __m128i K11 = aes_256_key_expansion(K9, K10);

   __m128i K12 = aes_128_key_expansion(K10, _mm_aeskeygenassist_si128(K11, 0x20));
   __m128i K13 = aes_256_key_expansion(K11, K12);

   __m128i K14 = aes_128_key_expansion(K12, _mm_aeskeygenassist_si128(K13, 0x40));

   __m128i* EK_mm = reinterpret_cast<__m128i*>(m_EK.data());
   _mm_storeu_si128(EK_mm     , K0);
   _mm_storeu_si128(EK_mm +  1, K1);
   _mm_storeu_si128(EK_mm +  2, K2);
   _mm_storeu_si128(EK_mm +  3, K3);
   _mm_storeu_si128(EK_mm +  4, K4);
   _mm_storeu_si128(EK_mm +  5, K5);
   _mm_storeu_si128(EK_mm +  6, K6);
   _mm_storeu_si128(EK_mm +  7, K7);
   _mm_storeu_si128(EK_mm +  8, K8);
   _mm_storeu_si128(EK_mm +  9, K9);
   _mm_storeu_si128(EK_mm + 10, K10);
   _mm_storeu_si128(EK_mm + 11, K11);
   _mm_storeu_si128(EK_mm + 12, K12);
   _mm_storeu_si128(EK_mm + 13, K13);
   _mm_storeu_si128(EK_mm + 14, K14);

   // Now generate decryption keys
   __m128i* DK_mm = reinterpret_cast<__m128i*>(m_DK.data());
   _mm_storeu_si128(DK_mm     , K14);
   _mm_storeu_si128(DK_mm +  1, _mm_aesimc_si128(K13));
   _mm_storeu_si128(DK_mm +  2, _mm_aesimc_si128(K12));
   _mm_storeu_si128(DK_mm +  3, _mm_aesimc_si128(K11));
   _mm_storeu_si128(DK_mm +  4, _mm_aesimc_si128(K10));
   _mm_storeu_si128(DK_mm +  5, _mm_aesimc_si128(K9));
   _mm_storeu_si128(DK_mm +  6, _mm_aesimc_si128(K8));
   _mm_storeu_si128(DK_mm +  7, _mm_aesimc_si128(K7));
   _mm_storeu_si128(DK_mm +  8, _mm_aesimc_si128(K6));
   _mm_storeu_si128(DK_mm +  9, _mm_aesimc_si128(K5));
   _mm_storeu_si128(DK_mm + 10, _mm_aesimc_si128(K4));
   _mm_storeu_si128(DK_mm + 11, _mm_aesimc_si128(K3));
   _mm_storeu_si128(DK_mm + 12, _mm_aesimc_si128(K2));
   _mm_storeu_si128(DK_mm + 13, _mm_aesimc_si128(K1));
   _mm_storeu_si128(DK_mm + 14, K0);
   }

#undef AES_ENC_4_ROUNDS
#undef AES_ENC_4_LAST_ROUNDS
#undef AES_DEC_4_ROUNDS
#undef AES_DEC_4_LAST_ROUNDS

}
/*
* CLMUL hook
* (C) 2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <immintrin.h>

namespace Botan {

BOTAN_FUNC_ISA("pclmul,ssse3")
void gcm_multiply_clmul(uint8_t x[16], const uint8_t H[16])
   {
   /*
   * Algorithms 1 and 5 from Intel's CLMUL guide
   */
   const __m128i BSWAP_MASK = _mm_set_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);

   __m128i a = _mm_loadu_si128(reinterpret_cast<const __m128i*>(x));
   __m128i b = _mm_loadu_si128(reinterpret_cast<const __m128i*>(H));

   a = _mm_shuffle_epi8(a, BSWAP_MASK);
   b = _mm_shuffle_epi8(b, BSWAP_MASK);

   __m128i T0, T1, T2, T3, T4, T5;

   T0 = _mm_clmulepi64_si128(a, b, 0x00);
   T1 = _mm_clmulepi64_si128(a, b, 0x01);
   T2 = _mm_clmulepi64_si128(a, b, 0x10);
   T3 = _mm_clmulepi64_si128(a, b, 0x11);

   T1 = _mm_xor_si128(T1, T2);
   T2 = _mm_slli_si128(T1, 8);
   T1 = _mm_srli_si128(T1, 8);
   T0 = _mm_xor_si128(T0, T2);
   T3 = _mm_xor_si128(T3, T1);

   T4 = _mm_srli_epi32(T0, 31);
   T0 = _mm_slli_epi32(T0, 1);

   T5 = _mm_srli_epi32(T3, 31);
   T3 = _mm_slli_epi32(T3, 1);

   T2 = _mm_srli_si128(T4, 12);
   T5 = _mm_slli_si128(T5, 4);
   T4 = _mm_slli_si128(T4, 4);
   T0 = _mm_or_si128(T0, T4);
   T3 = _mm_or_si128(T3, T5);
   T3 = _mm_or_si128(T3, T2);

   T4 = _mm_slli_epi32(T0, 31);
   T5 = _mm_slli_epi32(T0, 30);
   T2 = _mm_slli_epi32(T0, 25);

   T4 = _mm_xor_si128(T4, T5);
   T4 = _mm_xor_si128(T4, T2);
   T5 = _mm_srli_si128(T4, 4);
   T3 = _mm_xor_si128(T3, T5);
   T4 = _mm_slli_si128(T4, 12);
   T0 = _mm_xor_si128(T0, T4);
   T3 = _mm_xor_si128(T3, T0);

   T4 = _mm_srli_epi32(T0, 1);
   T1 = _mm_srli_epi32(T0, 2);
   T2 = _mm_srli_epi32(T0, 7);
   T3 = _mm_xor_si128(T3, T1);
   T3 = _mm_xor_si128(T3, T2);
   T3 = _mm_xor_si128(T3, T4);

   T3 = _mm_shuffle_epi8(T3, BSWAP_MASK);

   _mm_storeu_si128(reinterpret_cast<__m128i*>(x), T3);
   }

}
