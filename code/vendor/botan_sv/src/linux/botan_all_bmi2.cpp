/*
* Botan 2.12.1 Amalgamation
* (C) 1999-2018 The Botan Authors
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "botan_all.h"
#include "botan_all_internal.h"

#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC target ("bmi2")
#endif
/*
* (C) 2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
Your eyes do not decieve you; this is currently just a copy of the
baseline SHA-256 implementation. Because we compile it with BMI2
flags, GCC and Clang use the BMI2 instructions without further help.

Likely instruction scheduling could be improved by using inline asm.
*/

#define SHA2_32_F(A, B, C, D, E, F, G, H, M1, M2, M3, M4, magic) do {   \
   uint32_t A_rho = rotr<2>(A) ^ rotr<13>(A) ^ rotr<22>(A);             \
   uint32_t E_rho = rotr<6>(E) ^ rotr<11>(E) ^ rotr<25>(E);             \
   uint32_t M2_sigma = rotr<17>(M2) ^ rotr<19>(M2) ^ (M2 >> 10);        \
   uint32_t M4_sigma = rotr<7>(M4) ^ rotr<18>(M4) ^ (M4 >> 3);          \
   H += magic + E_rho + ((E & F) ^ (~E & G)) + M1;                      \
   D += H;                                                              \
   H += A_rho + ((A & B) | ((A | B) & C));                              \
   M1 += M2_sigma + M3 + M4_sigma;                                      \
   } while(0);

void SHA_256::compress_digest_x86_bmi2(secure_vector<uint32_t>& digest,
                                       const uint8_t input[],
                                       size_t blocks)
   {
   uint32_t A = digest[0], B = digest[1], C = digest[2],
            D = digest[3], E = digest[4], F = digest[5],
            G = digest[6], H = digest[7];

   for(size_t i = 0; i != blocks; ++i)
      {
      uint32_t W00 = load_be<uint32_t>(input,  0);
      uint32_t W01 = load_be<uint32_t>(input,  1);
      uint32_t W02 = load_be<uint32_t>(input,  2);
      uint32_t W03 = load_be<uint32_t>(input,  3);
      uint32_t W04 = load_be<uint32_t>(input,  4);
      uint32_t W05 = load_be<uint32_t>(input,  5);
      uint32_t W06 = load_be<uint32_t>(input,  6);
      uint32_t W07 = load_be<uint32_t>(input,  7);
      uint32_t W08 = load_be<uint32_t>(input,  8);
      uint32_t W09 = load_be<uint32_t>(input,  9);
      uint32_t W10 = load_be<uint32_t>(input, 10);
      uint32_t W11 = load_be<uint32_t>(input, 11);
      uint32_t W12 = load_be<uint32_t>(input, 12);
      uint32_t W13 = load_be<uint32_t>(input, 13);
      uint32_t W14 = load_be<uint32_t>(input, 14);
      uint32_t W15 = load_be<uint32_t>(input, 15);

      SHA2_32_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x428A2F98);
      SHA2_32_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x71374491);
      SHA2_32_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0xB5C0FBCF);
      SHA2_32_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0xE9B5DBA5);
      SHA2_32_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x3956C25B);
      SHA2_32_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x59F111F1);
      SHA2_32_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x923F82A4);
      SHA2_32_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0xAB1C5ED5);
      SHA2_32_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0xD807AA98);
      SHA2_32_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x12835B01);
      SHA2_32_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x243185BE);
      SHA2_32_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x550C7DC3);
      SHA2_32_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x72BE5D74);
      SHA2_32_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0x80DEB1FE);
      SHA2_32_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x9BDC06A7);
      SHA2_32_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0xC19BF174);

      SHA2_32_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0xE49B69C1);
      SHA2_32_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0xEFBE4786);
      SHA2_32_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x0FC19DC6);
      SHA2_32_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x240CA1CC);
      SHA2_32_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x2DE92C6F);
      SHA2_32_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x4A7484AA);
      SHA2_32_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x5CB0A9DC);
      SHA2_32_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x76F988DA);
      SHA2_32_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x983E5152);
      SHA2_32_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0xA831C66D);
      SHA2_32_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0xB00327C8);
      SHA2_32_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0xBF597FC7);
      SHA2_32_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0xC6E00BF3);
      SHA2_32_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xD5A79147);
      SHA2_32_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x06CA6351);
      SHA2_32_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x14292967);

      SHA2_32_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x27B70A85);
      SHA2_32_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x2E1B2138);
      SHA2_32_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x4D2C6DFC);
      SHA2_32_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x53380D13);
      SHA2_32_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x650A7354);
      SHA2_32_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x766A0ABB);
      SHA2_32_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x81C2C92E);
      SHA2_32_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x92722C85);
      SHA2_32_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0xA2BFE8A1);
      SHA2_32_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0xA81A664B);
      SHA2_32_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0xC24B8B70);
      SHA2_32_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0xC76C51A3);
      SHA2_32_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0xD192E819);
      SHA2_32_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xD6990624);
      SHA2_32_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0xF40E3585);
      SHA2_32_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x106AA070);

      SHA2_32_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x19A4C116);
      SHA2_32_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x1E376C08);
      SHA2_32_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x2748774C);
      SHA2_32_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x34B0BCB5);
      SHA2_32_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x391C0CB3);
      SHA2_32_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x4ED8AA4A);
      SHA2_32_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x5B9CCA4F);
      SHA2_32_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x682E6FF3);
      SHA2_32_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x748F82EE);
      SHA2_32_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x78A5636F);
      SHA2_32_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x84C87814);
      SHA2_32_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x8CC70208);
      SHA2_32_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x90BEFFFA);
      SHA2_32_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xA4506CEB);
      SHA2_32_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0xBEF9A3F7);
      SHA2_32_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0xC67178F2);

      A = (digest[0] += A);
      B = (digest[1] += B);
      C = (digest[2] += C);
      D = (digest[3] += D);
      E = (digest[4] += E);
      F = (digest[5] += F);
      G = (digest[6] += G);
      H = (digest[7] += H);

      input += 64;
      }
   }

}
/*
* (C) 2019 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* SHA-512 F1 Function
*
* Use a macro as many compilers won't inline a function this big,
* even though it is much faster if inlined.
*/
#define SHA2_64_F(A, B, C, D, E, F, G, H, M1, M2, M3, M4, magic)         \
   do {                                                                  \
      const uint64_t E_rho = rotr<14>(E) ^ rotr<18>(E) ^ rotr<41>(E);    \
      const uint64_t A_rho = rotr<28>(A) ^ rotr<34>(A) ^ rotr<39>(A);    \
      const uint64_t M2_sigma = rotr<19>(M2) ^ rotr<61>(M2) ^ (M2 >> 6); \
      const uint64_t M4_sigma = rotr<1>(M4) ^ rotr<8>(M4) ^ (M4 >> 7);   \
      H += magic + E_rho + ((E & F) ^ (~E & G)) + M1;                    \
      D += H;                                                            \
      H += A_rho + ((A & B) | ((A | B) & C));                            \
      M1 += M2_sigma + M3 + M4_sigma;                                    \
   } while(0);

void SHA_512::compress_digest_bmi2(secure_vector<uint64_t>& digest,
                                   const uint8_t input[], size_t blocks)
   {
   uint64_t A = digest[0], B = digest[1], C = digest[2],
          D = digest[3], E = digest[4], F = digest[5],
          G = digest[6], H = digest[7];

   for(size_t i = 0; i != blocks; ++i)
      {
      uint64_t W00 = load_be<uint64_t>(input,  0);
      uint64_t W01 = load_be<uint64_t>(input,  1);
      uint64_t W02 = load_be<uint64_t>(input,  2);
      uint64_t W03 = load_be<uint64_t>(input,  3);
      uint64_t W04 = load_be<uint64_t>(input,  4);
      uint64_t W05 = load_be<uint64_t>(input,  5);
      uint64_t W06 = load_be<uint64_t>(input,  6);
      uint64_t W07 = load_be<uint64_t>(input,  7);
      uint64_t W08 = load_be<uint64_t>(input,  8);
      uint64_t W09 = load_be<uint64_t>(input,  9);
      uint64_t W10 = load_be<uint64_t>(input, 10);
      uint64_t W11 = load_be<uint64_t>(input, 11);
      uint64_t W12 = load_be<uint64_t>(input, 12);
      uint64_t W13 = load_be<uint64_t>(input, 13);
      uint64_t W14 = load_be<uint64_t>(input, 14);
      uint64_t W15 = load_be<uint64_t>(input, 15);

      SHA2_64_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x428A2F98D728AE22);
      SHA2_64_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x7137449123EF65CD);
      SHA2_64_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0xB5C0FBCFEC4D3B2F);
      SHA2_64_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0xE9B5DBA58189DBBC);
      SHA2_64_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x3956C25BF348B538);
      SHA2_64_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x59F111F1B605D019);
      SHA2_64_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x923F82A4AF194F9B);
      SHA2_64_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0xAB1C5ED5DA6D8118);
      SHA2_64_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0xD807AA98A3030242);
      SHA2_64_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x12835B0145706FBE);
      SHA2_64_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x243185BE4EE4B28C);
      SHA2_64_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x550C7DC3D5FFB4E2);
      SHA2_64_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x72BE5D74F27B896F);
      SHA2_64_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0x80DEB1FE3B1696B1);
      SHA2_64_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x9BDC06A725C71235);
      SHA2_64_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0xC19BF174CF692694);
      SHA2_64_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0xE49B69C19EF14AD2);
      SHA2_64_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0xEFBE4786384F25E3);
      SHA2_64_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x0FC19DC68B8CD5B5);
      SHA2_64_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x240CA1CC77AC9C65);
      SHA2_64_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x2DE92C6F592B0275);
      SHA2_64_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x4A7484AA6EA6E483);
      SHA2_64_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x5CB0A9DCBD41FBD4);
      SHA2_64_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x76F988DA831153B5);
      SHA2_64_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x983E5152EE66DFAB);
      SHA2_64_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0xA831C66D2DB43210);
      SHA2_64_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0xB00327C898FB213F);
      SHA2_64_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0xBF597FC7BEEF0EE4);
      SHA2_64_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0xC6E00BF33DA88FC2);
      SHA2_64_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xD5A79147930AA725);
      SHA2_64_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x06CA6351E003826F);
      SHA2_64_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x142929670A0E6E70);
      SHA2_64_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x27B70A8546D22FFC);
      SHA2_64_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x2E1B21385C26C926);
      SHA2_64_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x4D2C6DFC5AC42AED);
      SHA2_64_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x53380D139D95B3DF);
      SHA2_64_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x650A73548BAF63DE);
      SHA2_64_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x766A0ABB3C77B2A8);
      SHA2_64_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x81C2C92E47EDAEE6);
      SHA2_64_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x92722C851482353B);
      SHA2_64_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0xA2BFE8A14CF10364);
      SHA2_64_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0xA81A664BBC423001);
      SHA2_64_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0xC24B8B70D0F89791);
      SHA2_64_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0xC76C51A30654BE30);
      SHA2_64_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0xD192E819D6EF5218);
      SHA2_64_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xD69906245565A910);
      SHA2_64_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0xF40E35855771202A);
      SHA2_64_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x106AA07032BBD1B8);
      SHA2_64_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x19A4C116B8D2D0C8);
      SHA2_64_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x1E376C085141AB53);
      SHA2_64_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x2748774CDF8EEB99);
      SHA2_64_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x34B0BCB5E19B48A8);
      SHA2_64_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x391C0CB3C5C95A63);
      SHA2_64_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x4ED8AA4AE3418ACB);
      SHA2_64_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x5B9CCA4F7763E373);
      SHA2_64_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x682E6FF3D6B2B8A3);
      SHA2_64_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x748F82EE5DEFB2FC);
      SHA2_64_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x78A5636F43172F60);
      SHA2_64_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x84C87814A1F0AB72);
      SHA2_64_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x8CC702081A6439EC);
      SHA2_64_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x90BEFFFA23631E28);
      SHA2_64_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xA4506CEBDE82BDE9);
      SHA2_64_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0xBEF9A3F7B2C67915);
      SHA2_64_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0xC67178F2E372532B);
      SHA2_64_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0xCA273ECEEA26619C);
      SHA2_64_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0xD186B8C721C0C207);
      SHA2_64_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0xEADA7DD6CDE0EB1E);
      SHA2_64_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0xF57D4F7FEE6ED178);
      SHA2_64_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x06F067AA72176FBA);
      SHA2_64_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x0A637DC5A2C898A6);
      SHA2_64_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x113F9804BEF90DAE);
      SHA2_64_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x1B710B35131C471B);
      SHA2_64_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x28DB77F523047D84);
      SHA2_64_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x32CAAB7B40C72493);
      SHA2_64_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x3C9EBE0A15C9BEBC);
      SHA2_64_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x431D67C49C100D4C);
      SHA2_64_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x4CC5D4BECB3E42B6);
      SHA2_64_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0x597F299CFC657E2A);
      SHA2_64_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x5FCB6FAB3AD6FAEC);
      SHA2_64_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x6C44198C4A475817);

      A = (digest[0] += A);
      B = (digest[1] += B);
      C = (digest[2] += C);
      D = (digest[3] += D);
      E = (digest[4] += E);
      F = (digest[5] += F);
      G = (digest[6] += G);
      H = (digest[7] += H);

      input += 128;
      }
   }

#undef SHA2_64_F

}
/*
* SHA-3
* (C) 2019 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

inline void SHA3_BMI2_round(uint64_t T[25], const uint64_t A[25], uint64_t RC)
   {
   const uint64_t C0 = A[0] ^ A[5] ^ A[10] ^ A[15] ^ A[20];
   const uint64_t C1 = A[1] ^ A[6] ^ A[11] ^ A[16] ^ A[21];
   const uint64_t C2 = A[2] ^ A[7] ^ A[12] ^ A[17] ^ A[22];
   const uint64_t C3 = A[3] ^ A[8] ^ A[13] ^ A[18] ^ A[23];
   const uint64_t C4 = A[4] ^ A[9] ^ A[14] ^ A[19] ^ A[24];

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

void SHA_3::permute_bmi2(uint64_t A[25])
   {
   static const uint64_t RC[24] = {
      0x0000000000000001, 0x0000000000008082, 0x800000000000808A,
      0x8000000080008000, 0x000000000000808B, 0x0000000080000001,
      0x8000000080008081, 0x8000000000008009, 0x000000000000008A,
      0x0000000000000088, 0x0000000080008009, 0x000000008000000A,
      0x000000008000808B, 0x800000000000008B, 0x8000000000008089,
      0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
      0x000000000000800A, 0x800000008000000A, 0x8000000080008081,
      0x8000000000008080, 0x0000000080000001, 0x8000000080008008
   };

   uint64_t T[25];

   for(size_t i = 0; i != 24; i += 2)
      {
      SHA3_BMI2_round(T, A, RC[i+0]);
      SHA3_BMI2_round(A, T, RC[i+1]);
      }
   }

}
