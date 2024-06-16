/*
* (C) 2010,2016,2021 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_SHA3_ROUND_H_
#define BOTAN_SHA3_ROUND_H_

#include <botan/types.h>
#include <botan/rotate.h>

/*
A bug in Clang 12 and XCode 13 cause a miscompilation of SHA3_round
with optimization levels -O2 and higher.

For details
 https://github.com/randombit/botan/issues/2802 and
 https://bugs.llvm.org/show_bug.cgi?id=51957
*/

#if    defined(__clang__) && \
    (( defined(__apple_build_version__) && __clang_major__ == 13) || \
     (!defined(__apple_build_version__) && __clang_major__ == 12))
  #define BOTAN_HAS_BROKEN_CLANG_SHA3
#endif

#if defined(BOTAN_HAS_BROKEN_CLANG_SHA3)
  #include <tuple>
#endif

namespace Botan {

#if defined(BOTAN_HAS_BROKEN_CLANG_SHA3)

namespace {

__attribute__((noinline)) std::tuple<uint64_t, uint64_t, uint64_t, uint64_t, uint64_t>
   xor_CNs(const uint64_t A[25])
   {
   return std::tuple<uint64_t, uint64_t, uint64_t, uint64_t, uint64_t>(
      A[0] ^ A[5] ^ A[10] ^ A[15] ^ A[20],
      A[1] ^ A[6] ^ A[11] ^ A[16] ^ A[21],
      A[2] ^ A[7] ^ A[12] ^ A[17] ^ A[22],
      A[3] ^ A[8] ^ A[13] ^ A[18] ^ A[23],
      A[4] ^ A[9] ^ A[14] ^ A[19] ^ A[24]);
   }

}

#endif

inline void SHA3_round(uint64_t T[25], const uint64_t A[25], uint64_t RC)
   {
#if defined(BOTAN_HAS_BROKEN_CLANG_SHA3)
   const auto C_tup = xor_CNs(A);
   const uint64_t C0 = std::get<0>(C_tup);
   const uint64_t C1 = std::get<1>(C_tup);
   const uint64_t C2 = std::get<2>(C_tup);
   const uint64_t C3 = std::get<3>(C_tup);
   const uint64_t C4 = std::get<4>(C_tup);
#else
   const uint64_t C0 = A[0] ^ A[5] ^ A[10] ^ A[15] ^ A[20];
   const uint64_t C1 = A[1] ^ A[6] ^ A[11] ^ A[16] ^ A[21];
   const uint64_t C2 = A[2] ^ A[7] ^ A[12] ^ A[17] ^ A[22];
   const uint64_t C3 = A[3] ^ A[8] ^ A[13] ^ A[18] ^ A[23];
   const uint64_t C4 = A[4] ^ A[9] ^ A[14] ^ A[19] ^ A[24];
#endif

   const uint64_t D0 = rotl<1>(C0) ^ C3;
   const uint64_t D1 = rotl<1>(C1) ^ C4;
   const uint64_t D2 = rotl<1>(C2) ^ C0;
   const uint64_t D3 = rotl<1>(C3) ^ C1;
   const uint64_t D4 = rotl<1>(C4) ^ C2;

   const uint64_t B00 =          A[ 0] ^ D1;
   const uint64_t B01 = rotl<44>(A[ 6] ^ D2);
   const uint64_t B02 = rotl<43>(A[12] ^ D3);
   const uint64_t B03 = rotl<21>(A[18] ^ D4);
   const uint64_t B04 = rotl<14>(A[24] ^ D0);
   T[ 0] = B00 ^ (~B01 & B02) ^ RC;
   T[ 1] = B01 ^ (~B02 & B03);
   T[ 2] = B02 ^ (~B03 & B04);
   T[ 3] = B03 ^ (~B04 & B00);
   T[ 4] = B04 ^ (~B00 & B01);

   const uint64_t B05 = rotl<28>(A[ 3] ^ D4);
   const uint64_t B06 = rotl<20>(A[ 9] ^ D0);
   const uint64_t B07 = rotl< 3>(A[10] ^ D1);
   const uint64_t B08 = rotl<45>(A[16] ^ D2);
   const uint64_t B09 = rotl<61>(A[22] ^ D3);
   T[ 5] = B05 ^ (~B06 & B07);
   T[ 6] = B06 ^ (~B07 & B08);
   T[ 7] = B07 ^ (~B08 & B09);
   T[ 8] = B08 ^ (~B09 & B05);
   T[ 9] = B09 ^ (~B05 & B06);

   const uint64_t B10 = rotl< 1>(A[ 1] ^ D2);
   const uint64_t B11 = rotl< 6>(A[ 7] ^ D3);
   const uint64_t B12 = rotl<25>(A[13] ^ D4);
   const uint64_t B13 = rotl< 8>(A[19] ^ D0);
   const uint64_t B14 = rotl<18>(A[20] ^ D1);
   T[10] = B10 ^ (~B11 & B12);
   T[11] = B11 ^ (~B12 & B13);
   T[12] = B12 ^ (~B13 & B14);
   T[13] = B13 ^ (~B14 & B10);
   T[14] = B14 ^ (~B10 & B11);

   const uint64_t B15 = rotl<27>(A[ 4] ^ D0);
   const uint64_t B16 = rotl<36>(A[ 5] ^ D1);
   const uint64_t B17 = rotl<10>(A[11] ^ D2);
   const uint64_t B18 = rotl<15>(A[17] ^ D3);
   const uint64_t B19 = rotl<56>(A[23] ^ D4);
   T[15] = B15 ^ (~B16 & B17);
   T[16] = B16 ^ (~B17 & B18);
   T[17] = B17 ^ (~B18 & B19);
   T[18] = B18 ^ (~B19 & B15);
   T[19] = B19 ^ (~B15 & B16);

   const uint64_t B20 = rotl<62>(A[ 2] ^ D3);
   const uint64_t B21 = rotl<55>(A[ 8] ^ D4);
   const uint64_t B22 = rotl<39>(A[14] ^ D0);
   const uint64_t B23 = rotl<41>(A[15] ^ D1);
   const uint64_t B24 = rotl< 2>(A[21] ^ D2);
   T[20] = B20 ^ (~B21 & B22);
   T[21] = B21 ^ (~B22 & B23);
   T[22] = B22 ^ (~B23 & B24);
   T[23] = B23 ^ (~B24 & B20);
   T[24] = B24 ^ (~B20 & B21);
   }

}

#endif
