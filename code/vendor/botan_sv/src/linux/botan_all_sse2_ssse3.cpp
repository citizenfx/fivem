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
#pragma GCC target ("sse2")
#endif
/*
* AES using vector permutes (SSSE3, NEON)
* (C) 2010,2016,2019 Jack Lloyd
*
* Based on public domain x86-64 assembly written by Mike Hamburg,
* described in "Accelerating AES with Vector Permute Instructions"
* (CHES 2009). His original code is available at
* https://crypto.stanford.edu/vpaes/
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_SIMD_USE_SSE2)
  #include <tmmintrin.h>
#endif

namespace Botan {

namespace {

inline SIMD_4x32 BOTAN_FUNC_ISA(BOTAN_VPERM_ISA) shuffle(SIMD_4x32 a, SIMD_4x32 b)
   {
#if defined(BOTAN_SIMD_USE_SSE2)
   return SIMD_4x32(_mm_shuffle_epi8(a.raw(), b.raw()));
#elif defined(BOTAN_SIMD_USE_NEON)
   const uint8x16_t tbl = vreinterpretq_u8_u32(a.raw());
   const uint8x16_t idx = vreinterpretq_u8_u32(b.raw());

#if defined(BOTAN_TARGET_ARCH_IS_ARM32)
   const uint8x8x2_t tbl2 = { vget_low_u8(tbl), vget_high_u8(tbl) };

   return SIMD_4x32(vreinterpretq_u32_u8(
                       vcombine_u8(vtbl2_u8(tbl2, vget_low_u8(idx)),
                                   vtbl2_u8(tbl2, vget_high_u8(idx)))));

#else
   return SIMD_4x32(vreinterpretq_u32_u8(vqtbl1q_u8(tbl, idx)));
#endif

#elif defined(BOTAN_SIMD_USE_ALTIVEC)
   __vector unsigned char bv = (__vector unsigned char)b.raw();

   const auto high_bit = vec_sl(vec_sr(bv, vec_splat_u8(7)), vec_splat_u8(4));
   bv = vec_and(bv, vec_splat_u8(0x0F));
   bv = vec_add(bv, high_bit);

   const __vector unsigned int zero = vec_splat_u32(0);
   return SIMD_4x32(vec_perm(a.raw(), zero, bv));
#else
   #error "No shuffle implementation available"
#endif
   }

inline SIMD_4x32 BOTAN_FUNC_ISA(BOTAN_VPERM_ISA) alignr8(SIMD_4x32 a, SIMD_4x32 b)
   {
#if defined(BOTAN_SIMD_USE_SSE2)
   return SIMD_4x32(_mm_alignr_epi8(a.raw(), b.raw(), 8));
#elif defined(BOTAN_SIMD_USE_NEON)
   return SIMD_4x32(vextq_u32(b.raw(), a.raw(), 2));
#elif defined(BOTAN_SIMD_USE_ALTIVEC)
   const __vector unsigned char mask = {8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
   return SIMD_4x32(vec_perm(b.raw(), a.raw(), mask));
#else
   #error "No alignr8 implementation available"
#endif
   }

const SIMD_4x32 k_ipt1 = SIMD_4x32(0x5A2A7000, 0xC2B2E898, 0x52227808, 0xCABAE090);
const SIMD_4x32 k_ipt2 = SIMD_4x32(0x317C4D00, 0x4C01307D, 0xB0FDCC81, 0xCD80B1FC);

const SIMD_4x32 k_inv1 = SIMD_4x32(0x0D080180, 0x0E05060F, 0x0A0B0C02, 0x04070309);
const SIMD_4x32 k_inv2 = SIMD_4x32(0x0F0B0780, 0x01040A06, 0x02050809, 0x030D0E0C);

const SIMD_4x32 sb1u = SIMD_4x32(0xCB503E00, 0xB19BE18F, 0x142AF544, 0xA5DF7A6E);
const SIMD_4x32 sb1t = SIMD_4x32(0xFAE22300, 0x3618D415, 0x0D2ED9EF, 0x3BF7CCC1);

const SIMD_4x32 mc_forward[4] = {
   SIMD_4x32(0x00030201, 0x04070605, 0x080B0A09, 0x0C0F0E0D),
   SIMD_4x32(0x04070605, 0x080B0A09, 0x0C0F0E0D, 0x00030201),
   SIMD_4x32(0x080B0A09, 0x0C0F0E0D, 0x00030201, 0x04070605),
   SIMD_4x32(0x0C0F0E0D, 0x00030201, 0x04070605, 0x080B0A09)
};

const SIMD_4x32 sr[4] = {
   SIMD_4x32(0x03020100, 0x07060504, 0x0B0A0908, 0x0F0E0D0C),
   SIMD_4x32(0x0F0A0500, 0x030E0904, 0x07020D08, 0x0B06010C),
   SIMD_4x32(0x0B020900, 0x0F060D04, 0x030A0108, 0x070E050C),
   SIMD_4x32(0x070A0D00, 0x0B0E0104, 0x0F020508, 0x0306090C),
};

const SIMD_4x32 rcon[10] = {
   SIMD_4x32(0x00000070, 0x00000000, 0x00000000, 0x00000000),
   SIMD_4x32(0x0000002A, 0x00000000, 0x00000000, 0x00000000),
   SIMD_4x32(0x00000098, 0x00000000, 0x00000000, 0x00000000),
   SIMD_4x32(0x00000008, 0x00000000, 0x00000000, 0x00000000),
   SIMD_4x32(0x0000004D, 0x00000000, 0x00000000, 0x00000000),
   SIMD_4x32(0x0000007C, 0x00000000, 0x00000000, 0x00000000),
   SIMD_4x32(0x0000007D, 0x00000000, 0x00000000, 0x00000000),
   SIMD_4x32(0x00000081, 0x00000000, 0x00000000, 0x00000000),
   SIMD_4x32(0x0000001F, 0x00000000, 0x00000000, 0x00000000),
   SIMD_4x32(0x00000083, 0x00000000, 0x00000000, 0x00000000),
};

inline SIMD_4x32 low_nibs(SIMD_4x32 x)
   {
   const SIMD_4x32 lo_nibs_mask = SIMD_4x32::splat_u8(0x0F);
   return lo_nibs_mask & x;
   }

inline SIMD_4x32 high_nibs(SIMD_4x32 x)
   {
   const SIMD_4x32 hi_nibs_mask = SIMD_4x32::splat_u8(0xF0);
   return (hi_nibs_mask & x).shr<4>();
   }

inline SIMD_4x32 aes_enc_first_round(SIMD_4x32 B, SIMD_4x32 K)
   {
   return shuffle(k_ipt1, low_nibs(B)) ^ shuffle(k_ipt2, high_nibs(B)) ^ K;
   }

inline SIMD_4x32 aes_enc_round(SIMD_4x32 B, SIMD_4x32 K, size_t r)
   {
   const SIMD_4x32 sb2u = SIMD_4x32(0x0B712400, 0xE27A93C6, 0xBC982FCD, 0x5EB7E955);
   const SIMD_4x32 sb2t = SIMD_4x32(0x0AE12900, 0x69EB8840, 0xAB82234A, 0xC2A163C8);

   const SIMD_4x32 mc_backward[4] = {
      SIMD_4x32(0x02010003, 0x06050407, 0x0A09080B, 0x0E0D0C0F),
      SIMD_4x32(0x0E0D0C0F, 0x02010003, 0x06050407, 0x0A09080B),
      SIMD_4x32(0x0A09080B, 0x0E0D0C0F, 0x02010003, 0x06050407),
      SIMD_4x32(0x06050407, 0x0A09080B, 0x0E0D0C0F, 0x02010003),
   };

   const SIMD_4x32 Bh = high_nibs(B);
   SIMD_4x32 Bl = low_nibs(B);
   const SIMD_4x32 t2 = shuffle(k_inv2, Bl);
   Bl ^= Bh;

   const SIMD_4x32 t5 = Bl ^ shuffle(k_inv1, t2 ^ shuffle(k_inv1, Bh));
   const SIMD_4x32 t6 = Bh ^ shuffle(k_inv1, t2 ^ shuffle(k_inv1, Bl));

   const SIMD_4x32 t7 = shuffle(sb1t, t6) ^ shuffle(sb1u, t5) ^ K;
   const SIMD_4x32 t8 = shuffle(sb2t, t6) ^ shuffle(sb2u, t5) ^ shuffle(t7, mc_forward[r % 4]);

   return shuffle(t8, mc_forward[r % 4]) ^ shuffle(t7, mc_backward[r % 4]) ^ t8;
   }

inline SIMD_4x32 aes_enc_last_round(SIMD_4x32 B, SIMD_4x32 K, size_t r)
   {
   const SIMD_4x32 sbou = SIMD_4x32(0x6FBDC700, 0xD0D26D17, 0xC502A878, 0x15AABF7A);
   const SIMD_4x32 sbot = SIMD_4x32(0x5FBB6A00, 0xCFE474A5, 0x412B35FA, 0x8E1E90D1);

   const SIMD_4x32 Bh = high_nibs(B);
   SIMD_4x32 Bl = low_nibs(B);
   const SIMD_4x32 t2 = shuffle(k_inv2, Bl);
   Bl ^= Bh;

   const SIMD_4x32 t5 = Bl ^ shuffle(k_inv1, t2 ^ shuffle(k_inv1, Bh));
   const SIMD_4x32 t6 = Bh ^ shuffle(k_inv1, t2 ^ shuffle(k_inv1, Bl));

   return shuffle(shuffle(sbou, t5) ^ shuffle(sbot, t6) ^ K, sr[r % 4]);
   }

inline SIMD_4x32 aes_dec_first_round(SIMD_4x32 B, SIMD_4x32 K)
   {
   const SIMD_4x32 k_dipt1 = SIMD_4x32(0x0B545F00, 0x0F505B04, 0x114E451A, 0x154A411E);
   const SIMD_4x32 k_dipt2 = SIMD_4x32(0x60056500, 0x86E383E6, 0xF491F194, 0x12771772);

   return shuffle(k_dipt1, low_nibs(B)) ^ shuffle(k_dipt2, high_nibs(B)) ^ K;
   }

inline SIMD_4x32 aes_dec_round(SIMD_4x32 B, SIMD_4x32 K, size_t r)
   {
   const SIMD_4x32 sb9u = SIMD_4x32(0x9A86D600, 0x851C0353, 0x4F994CC9, 0xCAD51F50);
   const SIMD_4x32 sb9t = SIMD_4x32(0xECD74900, 0xC03B1789, 0xB2FBA565, 0x725E2C9E);

   const SIMD_4x32 sbeu = SIMD_4x32(0x26D4D000, 0x46F29296, 0x64B4F6B0, 0x22426004);
   const SIMD_4x32 sbet = SIMD_4x32(0xFFAAC100, 0x0C55A6CD, 0x98593E32, 0x9467F36B);

   const SIMD_4x32 sbdu = SIMD_4x32(0xE6B1A200, 0x7D57CCDF, 0x882A4439, 0xF56E9B13);
   const SIMD_4x32 sbdt = SIMD_4x32(0x24C6CB00, 0x3CE2FAF7, 0x15DEEFD3, 0x2931180D);

   const SIMD_4x32 sbbu = SIMD_4x32(0x96B44200, 0xD0226492, 0xB0F2D404, 0x602646F6);
   const SIMD_4x32 sbbt = SIMD_4x32(0xCD596700, 0xC19498A6, 0x3255AA6B, 0xF3FF0C3E);

   const SIMD_4x32 mcx[4] = {
      SIMD_4x32(0x0C0F0E0D, 0x00030201, 0x04070605, 0x080B0A09),
      SIMD_4x32(0x080B0A09, 0x0C0F0E0D, 0x00030201, 0x04070605),
      SIMD_4x32(0x04070605, 0x080B0A09, 0x0C0F0E0D, 0x00030201),
      SIMD_4x32(0x00030201, 0x04070605, 0x080B0A09, 0x0C0F0E0D),
   };

   const SIMD_4x32 Bh = high_nibs(B);
   B = low_nibs(B);
   const SIMD_4x32 t2 = shuffle(k_inv2, B);

   B ^= Bh;

   const SIMD_4x32 t5 = B ^ shuffle(k_inv1, t2 ^ shuffle(k_inv1, Bh));
   const SIMD_4x32 t6 = Bh ^ shuffle(k_inv1, t2 ^ shuffle(k_inv1, B));

   const SIMD_4x32 mc = mcx[(r-1)%4];

   const SIMD_4x32 t8 = shuffle(sb9t, t6) ^ shuffle(sb9u, t5) ^ K;
   const SIMD_4x32 t9 = shuffle(t8, mc) ^ shuffle(sbdu, t5) ^ shuffle(sbdt, t6);
   const SIMD_4x32 t12 = shuffle(t9, mc) ^ shuffle(sbbu, t5) ^ shuffle(sbbt, t6);
   return shuffle(t12, mc) ^ shuffle(sbeu, t5) ^ shuffle(sbet, t6);
   }

inline SIMD_4x32 aes_dec_last_round(SIMD_4x32 B, SIMD_4x32 K, size_t r)
   {
   const SIMD_4x32 sbou = SIMD_4x32(0x7EF94000, 0x1387EA53, 0xD4943E2D, 0xC7AA6DB9);
   const SIMD_4x32 sbot = SIMD_4x32(0x93441D00, 0x12D7560F, 0xD8C58E9C, 0xCA4B8159);

   const uint32_t which_sr = ((((r - 1) << 4) ^ 48) & 48) / 16;

   const SIMD_4x32 Bh = high_nibs(B);
   B = low_nibs(B);
   const SIMD_4x32 t2 = shuffle(k_inv2, B);

   B ^= Bh;

   const SIMD_4x32 t5 = B ^ shuffle(k_inv1, t2 ^ shuffle(k_inv1, Bh));
   const SIMD_4x32 t6 = Bh ^ shuffle(k_inv1, t2 ^ shuffle(k_inv1, B));

   const SIMD_4x32 x = shuffle(sbou, t5) ^ shuffle(sbot, t6) ^ K;
   return shuffle(x, sr[which_sr]);
   }

void vperm_encrypt_blocks(const uint8_t in[], uint8_t out[], size_t blocks,
                          const SIMD_4x32 K[], size_t rounds)
   {
   CT::poison(in, blocks * 16);

   const size_t blocks2 = blocks - (blocks % 2);

   for(size_t i = 0; i != blocks2; i += 2)
      {
      SIMD_4x32 B0 = SIMD_4x32::load_le(in + i*16);
      SIMD_4x32 B1 = SIMD_4x32::load_le(in + (i+1)*16);

      B0 = aes_enc_first_round(B0, K[0]);
      B1 = aes_enc_first_round(B1, K[0]);

      for(size_t r = 1; r != rounds; ++r)
         {
         B0 = aes_enc_round(B0, K[r], r);
         B1 = aes_enc_round(B1, K[r], r);
         }

      B0 = aes_enc_last_round(B0, K[rounds], rounds);
      B1 = aes_enc_last_round(B1, K[rounds], rounds);

      B0.store_le(out + i*16);
      B1.store_le(out + (i+1)*16);
      }

   for(size_t i = blocks2; i < blocks; ++i)
      {
      SIMD_4x32 B = SIMD_4x32::load_le(in + i*16); // ???

      B = aes_enc_first_round(B, K[0]);

      for(size_t r = 1; r != rounds; ++r)
         {
         B = aes_enc_round(B, K[r], r);
         }

      B = aes_enc_last_round(B, K[rounds], rounds);
      B.store_le(out + i*16);
      }

   CT::unpoison(in,  blocks * 16);
   CT::unpoison(out, blocks * 16);
   }

void vperm_decrypt_blocks(const uint8_t in[], uint8_t out[], size_t blocks,
                          const SIMD_4x32 K[], size_t rounds)
   {
   CT::poison(in, blocks * 16);

   const size_t blocks2 = blocks - (blocks % 2);

   for(size_t i = 0; i != blocks2; i += 2)
      {
      SIMD_4x32 B0 = SIMD_4x32::load_le(in + i*16);
      SIMD_4x32 B1 = SIMD_4x32::load_le(in + (i+1)*16);

      B0 = aes_dec_first_round(B0, K[0]);
      B1 = aes_dec_first_round(B1, K[0]);

      for(size_t r = 1; r != rounds; ++r)
         {
         B0 = aes_dec_round(B0, K[r], r);
         B1 = aes_dec_round(B1, K[r], r);
         }

      B0 = aes_dec_last_round(B0, K[rounds], rounds);
      B1 = aes_dec_last_round(B1, K[rounds], rounds);

      B0.store_le(out + i*16);
      B1.store_le(out + (i+1)*16);
      }

   for(size_t i = blocks2; i < blocks; ++i)
      {
      SIMD_4x32 B = SIMD_4x32::load_le(in + i*16); // ???

      B = aes_dec_first_round(B, K[0]);

      for(size_t r = 1; r != rounds; ++r)
         {
         B = aes_dec_round(B, K[r], r);
         }

      B = aes_dec_last_round(B, K[rounds], rounds);
      B.store_le(out + i*16);
      }

   CT::unpoison(in,  blocks * 16);
   CT::unpoison(out, blocks * 16);
   }

}

void AES_128::vperm_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   const SIMD_4x32 K[11] = {
      SIMD_4x32(&m_EK[4* 0]), SIMD_4x32(&m_EK[4* 1]), SIMD_4x32(&m_EK[4* 2]),
      SIMD_4x32(&m_EK[4* 3]), SIMD_4x32(&m_EK[4* 4]), SIMD_4x32(&m_EK[4* 5]),
      SIMD_4x32(&m_EK[4* 6]), SIMD_4x32(&m_EK[4* 7]), SIMD_4x32(&m_EK[4* 8]),
      SIMD_4x32(&m_EK[4* 9]), SIMD_4x32(&m_EK[4*10]),
   };

   return vperm_encrypt_blocks(in, out, blocks, K, 10);
   }

void AES_128::vperm_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   const SIMD_4x32 K[11] = {
      SIMD_4x32(&m_DK[4* 0]), SIMD_4x32(&m_DK[4* 1]), SIMD_4x32(&m_DK[4* 2]),
      SIMD_4x32(&m_DK[4* 3]), SIMD_4x32(&m_DK[4* 4]), SIMD_4x32(&m_DK[4* 5]),
      SIMD_4x32(&m_DK[4* 6]), SIMD_4x32(&m_DK[4* 7]), SIMD_4x32(&m_DK[4* 8]),
      SIMD_4x32(&m_DK[4* 9]), SIMD_4x32(&m_DK[4*10]),
   };

   return vperm_decrypt_blocks(in, out, blocks, K, 10);
   }

void AES_192::vperm_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   const SIMD_4x32 K[13] = {
      SIMD_4x32(&m_EK[4* 0]), SIMD_4x32(&m_EK[4* 1]), SIMD_4x32(&m_EK[4* 2]),
      SIMD_4x32(&m_EK[4* 3]), SIMD_4x32(&m_EK[4* 4]), SIMD_4x32(&m_EK[4* 5]),
      SIMD_4x32(&m_EK[4* 6]), SIMD_4x32(&m_EK[4* 7]), SIMD_4x32(&m_EK[4* 8]),
      SIMD_4x32(&m_EK[4* 9]), SIMD_4x32(&m_EK[4*10]), SIMD_4x32(&m_EK[4*11]),
      SIMD_4x32(&m_EK[4*12]),
   };

   return vperm_encrypt_blocks(in, out, blocks, K, 12);
   }

void AES_192::vperm_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   const SIMD_4x32 K[13] = {
      SIMD_4x32(&m_DK[4* 0]), SIMD_4x32(&m_DK[4* 1]), SIMD_4x32(&m_DK[4* 2]),
      SIMD_4x32(&m_DK[4* 3]), SIMD_4x32(&m_DK[4* 4]), SIMD_4x32(&m_DK[4* 5]),
      SIMD_4x32(&m_DK[4* 6]), SIMD_4x32(&m_DK[4* 7]), SIMD_4x32(&m_DK[4* 8]),
      SIMD_4x32(&m_DK[4* 9]), SIMD_4x32(&m_DK[4*10]), SIMD_4x32(&m_DK[4*11]),
      SIMD_4x32(&m_DK[4*12]),
   };

   return vperm_decrypt_blocks(in, out, blocks, K, 12);
   }

void AES_256::vperm_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   const SIMD_4x32 K[15] = {
      SIMD_4x32(&m_EK[4* 0]), SIMD_4x32(&m_EK[4* 1]), SIMD_4x32(&m_EK[4* 2]),
      SIMD_4x32(&m_EK[4* 3]), SIMD_4x32(&m_EK[4* 4]), SIMD_4x32(&m_EK[4* 5]),
      SIMD_4x32(&m_EK[4* 6]), SIMD_4x32(&m_EK[4* 7]), SIMD_4x32(&m_EK[4* 8]),
      SIMD_4x32(&m_EK[4* 9]), SIMD_4x32(&m_EK[4*10]), SIMD_4x32(&m_EK[4*11]),
      SIMD_4x32(&m_EK[4*12]), SIMD_4x32(&m_EK[4*13]), SIMD_4x32(&m_EK[4*14]),
   };

   return vperm_encrypt_blocks(in, out, blocks, K, 14);
   }

void AES_256::vperm_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const
   {
   const SIMD_4x32 K[15] = {
      SIMD_4x32(&m_DK[4* 0]), SIMD_4x32(&m_DK[4* 1]), SIMD_4x32(&m_DK[4* 2]),
      SIMD_4x32(&m_DK[4* 3]), SIMD_4x32(&m_DK[4* 4]), SIMD_4x32(&m_DK[4* 5]),
      SIMD_4x32(&m_DK[4* 6]), SIMD_4x32(&m_DK[4* 7]), SIMD_4x32(&m_DK[4* 8]),
      SIMD_4x32(&m_DK[4* 9]), SIMD_4x32(&m_DK[4*10]), SIMD_4x32(&m_DK[4*11]),
      SIMD_4x32(&m_DK[4*12]), SIMD_4x32(&m_DK[4*13]), SIMD_4x32(&m_DK[4*14]),
   };

   return vperm_decrypt_blocks(in, out, blocks, K, 14);
   }

namespace {

SIMD_4x32 aes_schedule_transform(SIMD_4x32 input,
                                 SIMD_4x32 table_1,
                                 SIMD_4x32 table_2)
   {
   return shuffle(table_1, low_nibs(input)) ^ shuffle(table_2, high_nibs(input));
   }

SIMD_4x32 aes_schedule_mangle(SIMD_4x32 k, uint8_t round_no)
   {
   const SIMD_4x32 mc_forward0(0x00030201, 0x04070605, 0x080B0A09, 0x0C0F0E0D);
   const SIMD_4x32 srx(sr[round_no % 4]);

   SIMD_4x32 t = shuffle(k ^ SIMD_4x32::splat_u8(0x5B), mc_forward0);
   SIMD_4x32 t2 = t;
   t = shuffle(t, mc_forward0);
   t2 = t ^ t2 ^ shuffle(t, mc_forward0);
   return shuffle(t2, srx);
   }

SIMD_4x32 aes_schedule_mangle_dec(SIMD_4x32 k, uint8_t round_no)
   {
   const SIMD_4x32 mc_forward0(0x00030201, 0x04070605, 0x080B0A09, 0x0C0F0E0D);

   const SIMD_4x32 dsk[8] = {
      SIMD_4x32(0x7ED9A700, 0xB6116FC8, 0x82255BFC, 0x4AED9334),
      SIMD_4x32(0x27143300, 0x45765162, 0xE9DAFDCE, 0x8BB89FAC),
      SIMD_4x32(0xCCA86400, 0x27438FEB, 0xADC90561, 0x4622EE8A),
      SIMD_4x32(0x4F92DD00, 0x815C13CE, 0xBD602FF2, 0x73AEE13C),
      SIMD_4x32(0x01C6C700, 0x03C4C502, 0xFA3D3CFB, 0xF83F3EF9),
      SIMD_4x32(0x38CFF700, 0xEE1921D6, 0x7384BC4B, 0xA5526A9D),
      SIMD_4x32(0x53732000, 0xE3C390B0, 0x10306343, 0xA080D3F3),
      SIMD_4x32(0x036982E8, 0xA0CA214B, 0x8CE60D67, 0x2F45AEC4),
   };

   SIMD_4x32 t = aes_schedule_transform(k, dsk[0], dsk[1]);
   SIMD_4x32 output = shuffle(t, mc_forward0);

   t = aes_schedule_transform(t, dsk[2], dsk[3]);
   output = shuffle(t ^ output, mc_forward0);

   t = aes_schedule_transform(t, dsk[4], dsk[5]);
   output = shuffle(t ^ output, mc_forward0);

   t = aes_schedule_transform(t, dsk[6], dsk[7]);
   output = shuffle(t ^ output, mc_forward0);

   return shuffle(output, sr[round_no % 4]);
   }

SIMD_4x32 aes_schedule_mangle_last(SIMD_4x32 k, uint8_t round_no)
   {
   const SIMD_4x32 out_tr1(0xD6B66000, 0xFF9F4929, 0xDEBE6808, 0xF7974121);
   const SIMD_4x32 out_tr2(0x50BCEC00, 0x01EDBD51, 0xB05C0CE0, 0xE10D5DB1);

   k = shuffle(k, sr[round_no % 4]);
   k ^= SIMD_4x32::splat_u8(0x5B);
   return aes_schedule_transform(k, out_tr1, out_tr2);
   }

SIMD_4x32 aes_schedule_mangle_last_dec(SIMD_4x32 k)
   {
   const SIMD_4x32 deskew1(0x47A4E300, 0x07E4A340, 0x5DBEF91A, 0x1DFEB95A);
   const SIMD_4x32 deskew2(0x83EA6900, 0x5F36B5DC, 0xF49D1E77, 0x2841C2AB);

   k ^= SIMD_4x32::splat_u8(0x5B);
   return aes_schedule_transform(k, deskew1, deskew2);
   }

SIMD_4x32 aes_schedule_round(SIMD_4x32 input1, SIMD_4x32 input2)
   {
   SIMD_4x32 smeared = input2 ^ input2.shift_elems_left<1>();
   smeared ^= smeared.shift_elems_left<2>();
   smeared ^= SIMD_4x32::splat_u8(0x5B);

   const SIMD_4x32 Bh = high_nibs(input1);
   SIMD_4x32 Bl = low_nibs(input1);

   const SIMD_4x32 t2 = shuffle(k_inv2, Bl);

   Bl ^= Bh;

   SIMD_4x32 t5 = Bl ^ shuffle(k_inv1, t2 ^ shuffle(k_inv1, Bh));
   SIMD_4x32 t6 = Bh ^ shuffle(k_inv1, t2 ^ shuffle(k_inv1, Bl));

   return smeared ^ shuffle(sb1u, t5) ^ shuffle(sb1t, t6);
   }

SIMD_4x32 aes_schedule_round(SIMD_4x32 rc, SIMD_4x32 input1, SIMD_4x32 input2)
   {
   // This byte shuffle is equivalent to alignr<1>(shuffle32(input1, (3,3,3,3)));
   const SIMD_4x32 shuffle3333_15 = SIMD_4x32::splat(0x0C0F0E0D);
   return aes_schedule_round(shuffle(input1, shuffle3333_15), input2 ^ rc);
   }

SIMD_4x32 aes_schedule_192_smear(SIMD_4x32 x, SIMD_4x32 y)
   {
   const SIMD_4x32 shuffle3332 =
      SIMD_4x32(0x0B0A0908, 0x0F0E0D0C, 0x0F0E0D0C, 0x0F0E0D0C);
   const SIMD_4x32 shuffle2000 =
      SIMD_4x32(0x03020100, 0x03020100, 0x03020100, 0x0B0A0908);

   const SIMD_4x32 zero_top_half(0, 0, ~0, ~0);
   y &= zero_top_half;
   return y ^ shuffle(x, shuffle3332) ^ shuffle(y, shuffle2000);
   }

}

void AES_128::vperm_key_schedule(const uint8_t keyb[], size_t)
   {
   m_EK.resize(11*4);
   m_DK.resize(11*4);

   SIMD_4x32 key = SIMD_4x32::load_le(keyb);

   shuffle(key, sr[2]).store_le(&m_DK[4*10]);

   key = aes_schedule_transform(key, k_ipt1, k_ipt2);
   key.store_le(&m_EK[0]);

   for(size_t i = 1; i != 10; ++i)
      {
      key = aes_schedule_round(rcon[i-1], key, key);

      aes_schedule_mangle(key, (12-i) % 4).store_le(&m_EK[4*i]);

      aes_schedule_mangle_dec(key, (10-i)%4).store_le(&m_DK[4*(10-i)]);
      }

   key = aes_schedule_round(rcon[9], key, key);
   aes_schedule_mangle_last(key, 2).store_le(&m_EK[4*10]);
   aes_schedule_mangle_last_dec(key).store_le(&m_DK[0]);
   }

void AES_192::vperm_key_schedule(const uint8_t keyb[], size_t)
   {
   m_EK.resize(13*4);
   m_DK.resize(13*4);

   SIMD_4x32 key1 = SIMD_4x32::load_le(keyb);
   SIMD_4x32 key2 = SIMD_4x32::load_le(keyb + 8);

   shuffle(key1, sr[0]).store_le(&m_DK[12*4]);

   key1 = aes_schedule_transform(key1, k_ipt1, k_ipt2);
   key2 = aes_schedule_transform(key2, k_ipt1, k_ipt2);

   key1.store_le(&m_EK[0]);

   for(size_t i = 0; i != 4; ++i)
      {
      // key2 with 8 high bytes masked off
      SIMD_4x32 t = key2;
      key2 = aes_schedule_round(rcon[2*i], key2, key1);
      const SIMD_4x32 key2t = alignr8(key2, t);
      aes_schedule_mangle(key2t, (i+3)%4).store_le(&m_EK[4*(3*i+1)]);
      aes_schedule_mangle_dec(key2t, (i+3)%4).store_le(&m_DK[4*(11-3*i)]);

      t = aes_schedule_192_smear(key2, t);

      aes_schedule_mangle(t, (i+2)%4).store_le(&m_EK[4*(3*i+2)]);
      aes_schedule_mangle_dec(t, (i+2)%4).store_le(&m_DK[4*(10-3*i)]);

      key2 = aes_schedule_round(rcon[2*i+1], t, key2);

      if(i == 3)
         {
         aes_schedule_mangle_last(key2, (i+1)%4).store_le(&m_EK[4*(3*i+3)]);
         aes_schedule_mangle_last_dec(key2).store_le(&m_DK[4*(9-3*i)]);
         }
      else
         {
         aes_schedule_mangle(key2, (i+1)%4).store_le(&m_EK[4*(3*i+3)]);
         aes_schedule_mangle_dec(key2, (i+1)%4).store_le(&m_DK[4*(9-3*i)]);
         }

      key1 = key2;
      key2 = aes_schedule_192_smear(key2, t);
      }
   }

void AES_256::vperm_key_schedule(const uint8_t keyb[], size_t)
   {
   m_EK.resize(15*4);
   m_DK.resize(15*4);

   SIMD_4x32 key1 = SIMD_4x32::load_le(keyb);
   SIMD_4x32 key2 = SIMD_4x32::load_le(keyb + 16);

   shuffle(key1, sr[2]).store_le(&m_DK[4*14]);

   key1 = aes_schedule_transform(key1, k_ipt1, k_ipt2);
   key2 = aes_schedule_transform(key2, k_ipt1, k_ipt2);

   key1.store_le(&m_EK[0]);
   aes_schedule_mangle(key2, 3).store_le(&m_EK[4]);

   aes_schedule_mangle_dec(key2, 1).store_le(&m_DK[4*13]);

   const SIMD_4x32 shuffle3333 = SIMD_4x32::splat(0x0F0E0D0C);

   for(size_t i = 2; i != 14; i += 2)
      {
      const SIMD_4x32 k_t = key2;
      key1 = key2 = aes_schedule_round(rcon[(i/2)-1], key2, key1);

      aes_schedule_mangle(key2, i % 4).store_le(&m_EK[4*i]);
      aes_schedule_mangle_dec(key2, (i+2)%4).store_le(&m_DK[4*(14-i)]);

      key2 = aes_schedule_round(shuffle(key2, shuffle3333), k_t);

      aes_schedule_mangle(key2, (i-1)%4).store_le(&m_EK[4*(i+1)]);
      aes_schedule_mangle_dec(key2, (i+1)%4).store_le(&m_DK[4*(13-i)]);
      }

   key2 = aes_schedule_round(rcon[6], key2, key1);

   aes_schedule_mangle_last(key2, 2).store_le(&m_EK[4*14]);
   aes_schedule_mangle_last_dec(key2).store_le(&m_DK[0]);
   }

}
/*
* (C) 2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <immintrin.h>

namespace Botan {

BOTAN_FUNC_ISA("ssse3")
void gcm_multiply_ssse3(uint8_t x[16],
                        const uint64_t HM[256],
                        const uint8_t input_bytes[], size_t blocks)
   {
   const __m128i BSWAP_MASK = _mm_set_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);

   const __m128i* HM_mm = reinterpret_cast<const __m128i*>(HM);

   __m128i X = _mm_loadu_si128(reinterpret_cast<__m128i*>(x));
   X = _mm_shuffle_epi8(X, BSWAP_MASK);

   const __m128i ones = _mm_set1_epi8(-1);

   for(size_t b = 0; b != blocks; ++b)
      {
      __m128i M = _mm_loadu_si128(reinterpret_cast<const __m128i*>(input_bytes) + b);
      M = _mm_shuffle_epi8(M, BSWAP_MASK);

      X = _mm_xor_si128(X, M);

      __m128i Z = _mm_setzero_si128();

      for(size_t i = 0; i != 64; i += 2)
         {
         const __m128i HM0 = _mm_loadu_si128(HM_mm + 2*i);
         const __m128i HM1 = _mm_loadu_si128(HM_mm + 2*i + 1);
         const __m128i HM2 = _mm_loadu_si128(HM_mm + 2*i + 2);
         const __m128i HM3 = _mm_loadu_si128(HM_mm + 2*i + 3);

         const __m128i XMASK1 = _mm_add_epi64(_mm_srli_epi64(X, 63), ones);
         X = _mm_slli_epi64(X, 1);
         const __m128i XMASK2 = _mm_add_epi64(_mm_srli_epi64(X, 63), ones);
         X = _mm_slli_epi64(X, 1);

         Z = _mm_xor_si128(Z, _mm_andnot_si128(_mm_unpackhi_epi64(XMASK1, XMASK1), HM0));
         Z = _mm_xor_si128(Z, _mm_andnot_si128(_mm_unpacklo_epi64(XMASK1, XMASK1), HM1));
         Z = _mm_xor_si128(Z, _mm_andnot_si128(_mm_unpackhi_epi64(XMASK2, XMASK2), HM2));
         Z = _mm_xor_si128(Z, _mm_andnot_si128(_mm_unpacklo_epi64(XMASK2, XMASK2), HM3));
         }

      X = _mm_shuffle_epi32(Z, _MM_SHUFFLE(1, 0, 3, 2));
      }

   X = _mm_shuffle_epi8(X, BSWAP_MASK);
   _mm_storeu_si128(reinterpret_cast<__m128i*>(x), X);
   }

}
