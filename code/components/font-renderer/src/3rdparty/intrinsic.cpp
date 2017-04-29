/******************************************************************************
* Fast DXT - a realtime DXT compression tool
*
* Author : Luc Renambot
*
* Copyright (C) 2007 Electronic Visualization Laboratory,
* University of Illinois at Chicago
*
* This library is free software; you can redistribute it and/or modify it
* under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either Version 2.1 of the License, or
* (at your option) any later version.
*
* This library is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
* License for more details.
*
* You should have received a copy of the GNU Lesser Public License along
* with this library; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
*****************************************************************************/

/*
Code convert from asm to intrinsics from:

Copyright (C) 2006 Id Software, Inc.
Written by J.M.P. van Waveren
This code is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.
This code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.
*/

#include "StdInc.h"
#include "dxt.h"

#include <emmintrin.h>  // sse2


void ExtractBlock_Intrinsics(const byte *inPtr, int width, byte *colorBlock)
{
	__m128i t0, t1, t2, t3;
	register int w = width << 2;  // width*4

	t0 = _mm_load_si128((__m128i*) inPtr);
	_mm_store_si128((__m128i*) &colorBlock[0], t0);   // copy first row, 16bytes

	t1 = _mm_load_si128((__m128i*) (inPtr + w));
	_mm_store_si128((__m128i*) &colorBlock[16], t1);   // copy second row

	t2 = _mm_load_si128((__m128i*) (inPtr + 2 * w));
	_mm_store_si128((__m128i*) &colorBlock[32], t2);   // copy third row

	inPtr = inPtr + w;     // add width, intead of *3

	t3 = _mm_load_si128((__m128i*) (inPtr + 2 * w));
	_mm_store_si128((__m128i*) &colorBlock[48], t3);   // copy last row
}

#define R_SHUFFLE_D( x, y, z, w ) (( (w) & 3 ) << 6 | ( (z) & 3 ) << 4 | ( (y) & 3 ) << 2 | ( (x) & 3 ))

ALIGN16(static byte SIMD_SSE2_byte_0[16]) = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

void GetMinMaxColors_Intrinsics(const byte *colorBlock, byte *minColor, byte *maxColor)
{
	__m128i t0, t1, t3, t4, t6, t7;

	// get bounding box
	// ----------------

	// load the first row
	t0 = _mm_load_si128((__m128i*) colorBlock);
	t1 = _mm_load_si128((__m128i*) colorBlock);

	__m128i t16 = _mm_load_si128((__m128i*) (colorBlock + 16));
	// Minimum of Packed Unsigned Byte Integers
	t0 = _mm_min_epu8(t0, t16);
	// Maximum of Packed Unsigned Byte Integers
	t1 = _mm_max_epu8(t1, t16);

	__m128i t32 = _mm_load_si128((__m128i*) (colorBlock + 32));
	t0 = _mm_min_epu8(t0, t32);
	t1 = _mm_max_epu8(t1, t32);

	__m128i t48 = _mm_load_si128((__m128i*) (colorBlock + 48));
	t0 = _mm_min_epu8(t0, t48);
	t1 = _mm_max_epu8(t1, t48);

	// Shuffle Packed Doublewords
	t3 = _mm_shuffle_epi32(t0, R_SHUFFLE_D(2, 3, 2, 3));
	t4 = _mm_shuffle_epi32(t1, R_SHUFFLE_D(2, 3, 2, 3));

	t0 = _mm_min_epu8(t0, t3);
	t1 = _mm_max_epu8(t1, t4);

	// Shuffle Packed Low Words
	t6 = _mm_shufflelo_epi16(t0, R_SHUFFLE_D(2, 3, 2, 3));
	t7 = _mm_shufflelo_epi16(t1, R_SHUFFLE_D(2, 3, 2, 3));

	t0 = _mm_min_epu8(t0, t6);
	t1 = _mm_max_epu8(t1, t7);

	// inset the bounding box
	// ----------------------

	// Unpack Low Data
	//__m128i t66 = _mm_set1_epi8( 0 );
	__m128i t66 = _mm_load_si128((__m128i*) SIMD_SSE2_byte_0);
	t0 = _mm_unpacklo_epi8(t0, t66);
	t1 = _mm_unpacklo_epi8(t1, t66);

	// copy (movdqa)
	//__m128i t2 = _mm_load_si128 ( &t1 );
	__m128i t2 = t1;

	// Subtract Packed Integers
	t2 = _mm_sub_epi16(t2, t0);

	// Shift Packed Data Right Logical 
	t2 = _mm_srli_epi16(t2, INSET_SHIFT);

	// Add Packed Integers
	t0 = _mm_add_epi16(t0, t2);

	t1 = _mm_sub_epi16(t1, t2);

	// Pack with Unsigned Saturation
	t0 = _mm_packus_epi16(t0, t0);
	t1 = _mm_packus_epi16(t1, t1);

	// store bounding box extents
	// --------------------------
	_mm_store_si128((__m128i*) minColor, t0);
	_mm_store_si128((__m128i*) maxColor, t1);
}


ALIGN16(static word SIMD_SSE2_word_0[8]) = { 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
ALIGN16(static word SIMD_SSE2_word_1[8]) = { 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001 };
ALIGN16(static word SIMD_SSE2_word_2[8]) = { 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002 };
ALIGN16(static word SIMD_SSE2_word_div_by_3[8]) = { (1 << 16) / 3 + 1, (1 << 16) / 3 + 1, (1 << 16) / 3 + 1, (1 << 16) / 3 + 1, (1 << 16) / 3 + 1, (1 << 16) / 3 + 1, (1 << 16) / 3 + 1, (1 << 16) / 3 + 1 };
ALIGN16(static byte SIMD_SSE2_byte_colorMask[16]) = { C565_5_MASK, C565_6_MASK, C565_5_MASK, 0x00, 0x00, 0x00, 0x00, 0x00, C565_5_MASK, C565_6_MASK, C565_5_MASK, 0x00, 0x00, 0x00, 0x00, 0x00 };

void EmitColorIndices_Intrinsics(const byte *colorBlock, const byte *minColor, const byte *maxColor, byte *&outData)
{
	ALIGN16(byte color0[16]);
	ALIGN16(byte color1[16]);
	ALIGN16(byte color2[16]);
	ALIGN16(byte color3[16]);
	ALIGN16(byte result[16]);

	// mov esi, maxColor
	// mov edi, minColor

	__m128i t0, t1, t2, t3, t4, t5, t6, t7;

	t7 = _mm_setzero_si128();
	//t7 = _mm_xor_si128(t7, t7);
	_mm_store_si128((__m128i*) &result, t7);


	//t0 = _mm_load_si128 ( (__m128i*)  maxColor );
	t0 = _mm_cvtsi32_si128(*(int*)maxColor);

	// Bitwise AND
	__m128i tt = _mm_load_si128((__m128i*) SIMD_SSE2_byte_colorMask);
	t0 = _mm_and_si128(t0, tt);

	t0 = _mm_unpacklo_epi8(t0, t7);

	t4 = _mm_shufflelo_epi16(t0, R_SHUFFLE_D(0, 3, 2, 3));
	t5 = _mm_shufflelo_epi16(t0, R_SHUFFLE_D(3, 1, 3, 3));

	t4 = _mm_srli_epi16(t4, 5);
	t5 = _mm_srli_epi16(t5, 6);

	// Bitwise Logical OR
	t0 = _mm_or_si128(t0, t4);
	t0 = _mm_or_si128(t0, t5);   // t0 contains color0 in 565




	//t1 = _mm_load_si128 ( (__m128i*)  minColor );
	t1 = _mm_cvtsi32_si128(*(int*)minColor);

	t1 = _mm_and_si128(t1, tt);

	t1 = _mm_unpacklo_epi8(t1, t7);

	t4 = _mm_shufflelo_epi16(t1, R_SHUFFLE_D(0, 3, 2, 3));
	t5 = _mm_shufflelo_epi16(t1, R_SHUFFLE_D(3, 1, 3, 3));

	t4 = _mm_srli_epi16(t4, 5);
	t5 = _mm_srli_epi16(t5, 6);

	t1 = _mm_or_si128(t1, t4);
	t1 = _mm_or_si128(t1, t5);  // t1 contains color1 in 565



	t2 = t0;

	t2 = _mm_packus_epi16(t2, t7);

	t2 = _mm_shuffle_epi32(t2, R_SHUFFLE_D(0, 1, 0, 1));

	_mm_store_si128((__m128i*) &color0, t2);

	t6 = t0;
	t6 = _mm_add_epi16(t6, t0);
	t6 = _mm_add_epi16(t6, t1);

	// Multiply Packed Signed Integers and Store High Result
	__m128i tw3 = _mm_load_si128((__m128i*) SIMD_SSE2_word_div_by_3);
	t6 = _mm_mulhi_epi16(t6, tw3);
	t6 = _mm_packus_epi16(t6, t7);

	t6 = _mm_shuffle_epi32(t6, R_SHUFFLE_D(0, 1, 0, 1));

	_mm_store_si128((__m128i*) &color2, t6);

	t3 = t1;
	t3 = _mm_packus_epi16(t3, t7);
	t3 = _mm_shuffle_epi32(t3, R_SHUFFLE_D(0, 1, 0, 1));

	_mm_store_si128((__m128i*) &color1, t3);

	t1 = _mm_add_epi16(t1, t1);
	t0 = _mm_add_epi16(t0, t1);

	t0 = _mm_mulhi_epi16(t0, tw3);
	t0 = _mm_packus_epi16(t0, t7);

	t0 = _mm_shuffle_epi32(t0, R_SHUFFLE_D(0, 1, 0, 1));
	_mm_store_si128((__m128i*) &color3, t0);

	__m128i w0 = _mm_load_si128((__m128i*) SIMD_SSE2_word_0);
	__m128i w1 = _mm_load_si128((__m128i*) SIMD_SSE2_word_1);
	__m128i w2 = _mm_load_si128((__m128i*) SIMD_SSE2_word_2);

	// mov eax, 32
	// mov esi, colorBlock
	int x = 32;
	//const byte *c = colorBlock;
	while (x >= 0)
	{
		t3 = _mm_loadl_epi64((__m128i*) (colorBlock + x + 0));
		t3 = _mm_shuffle_epi32(t3, R_SHUFFLE_D(0, 2, 1, 3));

		t5 = _mm_loadl_epi64((__m128i*) (colorBlock + x + 8));
		t5 = _mm_shuffle_epi32(t5, R_SHUFFLE_D(0, 2, 1, 3));

		t0 = t3;
		t6 = t5;
		// Compute Sum of Absolute Difference
		__m128i c0 = _mm_load_si128((__m128i*)  color0);
		t0 = _mm_sad_epu8(t0, c0);
		t6 = _mm_sad_epu8(t6, c0);
		// Pack with Signed Saturation 
		t0 = _mm_packs_epi32(t0, t6);

		t1 = t3;
		t6 = t5;
		__m128i c1 = _mm_load_si128((__m128i*)  color1);
		t1 = _mm_sad_epu8(t1, c1);
		t6 = _mm_sad_epu8(t6, c1);
		t1 = _mm_packs_epi32(t1, t6);

		t2 = t3;
		t6 = t5;
		__m128i c2 = _mm_load_si128((__m128i*)  color2);
		t2 = _mm_sad_epu8(t2, c2);
		t6 = _mm_sad_epu8(t6, c2);
		t2 = _mm_packs_epi32(t2, t6);

		__m128i c3 = _mm_load_si128((__m128i*)  color3);
		t3 = _mm_sad_epu8(t3, c3);
		t5 = _mm_sad_epu8(t5, c3);
		t3 = _mm_packs_epi32(t3, t5);


		t4 = _mm_loadl_epi64((__m128i*) (colorBlock + x + 16));
		t4 = _mm_shuffle_epi32(t4, R_SHUFFLE_D(0, 2, 1, 3));

		t5 = _mm_loadl_epi64((__m128i*) (colorBlock + x + 24));
		t5 = _mm_shuffle_epi32(t5, R_SHUFFLE_D(0, 2, 1, 3));

		t6 = t4;
		t7 = t5;
		t6 = _mm_sad_epu8(t6, c0);
		t7 = _mm_sad_epu8(t7, c0);
		t6 = _mm_packs_epi32(t6, t7);
		t0 = _mm_packs_epi32(t0, t6);  // d0

		t6 = t4;
		t7 = t5;
		t6 = _mm_sad_epu8(t6, c1);
		t7 = _mm_sad_epu8(t7, c1);
		t6 = _mm_packs_epi32(t6, t7);
		t1 = _mm_packs_epi32(t1, t6);  // d1

		t6 = t4;
		t7 = t5;
		t6 = _mm_sad_epu8(t6, c2);
		t7 = _mm_sad_epu8(t7, c2);
		t6 = _mm_packs_epi32(t6, t7);
		t2 = _mm_packs_epi32(t2, t6);  // d2

		t4 = _mm_sad_epu8(t4, c3);
		t5 = _mm_sad_epu8(t5, c3);
		t4 = _mm_packs_epi32(t4, t5);
		t3 = _mm_packs_epi32(t3, t4);  // d3

		t7 = _mm_load_si128((__m128i*) result);

		t7 = _mm_slli_epi32(t7, 16);

		t4 = t0;
		t5 = t1;
		// Compare Packed Signed Integers for Greater Than
		t0 = _mm_cmpgt_epi16(t0, t3); // b0
		t1 = _mm_cmpgt_epi16(t1, t2); // b1
		t4 = _mm_cmpgt_epi16(t4, t2); // b2
		t5 = _mm_cmpgt_epi16(t5, t3); // b3
		t2 = _mm_cmpgt_epi16(t2, t3); // b4

		t4 = _mm_and_si128(t4, t1); // x0
		t5 = _mm_and_si128(t5, t0); // x1
		t2 = _mm_and_si128(t2, t0); // x2

		t4 = _mm_or_si128(t4, t5);
		t2 = _mm_and_si128(t2, w1);
		t4 = _mm_and_si128(t4, w2);
		t2 = _mm_or_si128(t2, t4);

		t5 = _mm_shuffle_epi32(t2, R_SHUFFLE_D(2, 3, 0, 1));

		// Unpack Low Data
		t2 = _mm_unpacklo_epi16(t2, w0);
		t5 = _mm_unpacklo_epi16(t5, w0);

		//t5 = _mm_slli_si128 ( t5, 8);
		t5 = _mm_slli_epi32(t5, 8);

		t7 = _mm_or_si128(t7, t5);
		t7 = _mm_or_si128(t7, t2);

		_mm_store_si128((__m128i*) &result, t7);

		x -= 32;
	}

	t4 = _mm_shuffle_epi32(t7, R_SHUFFLE_D(1, 2, 3, 0));
	t5 = _mm_shuffle_epi32(t7, R_SHUFFLE_D(2, 3, 0, 1));
	t6 = _mm_shuffle_epi32(t7, R_SHUFFLE_D(3, 0, 1, 2));

	t4 = _mm_slli_epi32(t4, 2);
	t5 = _mm_slli_epi32(t5, 4);
	t6 = _mm_slli_epi32(t6, 6);

	t7 = _mm_or_si128(t7, t4);
	t7 = _mm_or_si128(t7, t5);
	t7 = _mm_or_si128(t7, t6);

	//_mm_store_si128 ( (__m128i*) outData, t7 );

	int r = _mm_cvtsi128_si32(t7);
	memcpy(outData, &r, 4);   // Anything better ?

	outData += 4;
}



ALIGN16(static byte SIMD_SSE2_byte_1[16]) = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 };
ALIGN16(static byte SIMD_SSE2_byte_2[16]) = { 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02 };
ALIGN16(static byte SIMD_SSE2_byte_7[16]) = { 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07 };
ALIGN16(static word SIMD_SSE2_word_div_by_7[8]) = { (1 << 16) / 7 + 1, (1 << 16) / 7 + 1, (1 << 16) / 7 + 1, (1 << 16) / 7 + 1, (1 << 16) / 7 + 1, (1 << 16) / 7 + 1, (1 << 16) / 7 + 1, (1 << 16) / 7 + 1 };
ALIGN16(static word SIMD_SSE2_word_div_by_14[8]) = { (1 << 16) / 14 + 1, (1 << 16) / 14 + 1, (1 << 16) / 14 + 1, (1 << 16) / 14 + 1, (1 << 16) / 14 + 1, (1 << 16) / 14 + 1, (1 << 16) / 14 + 1, (1 << 16) / 14 + 1 };
ALIGN16(static word SIMD_SSE2_word_scale66554400[8]) = { 6, 6, 5, 5, 4, 4, 0, 0 };
ALIGN16(static word SIMD_SSE2_word_scale11223300[8]) = { 1, 1, 2, 2, 3, 3, 0, 0 };
ALIGN16(static dword SIMD_SSE2_dword_alpha_bit_mask0[4]) = { 7 << 0, 0, 7 << 0, 0 };
ALIGN16(static dword SIMD_SSE2_dword_alpha_bit_mask1[4]) = { 7 << 3, 0, 7 << 3, 0 };
ALIGN16(static dword SIMD_SSE2_dword_alpha_bit_mask2[4]) = { 7 << 6, 0, 7 << 6, 0 };
ALIGN16(static dword SIMD_SSE2_dword_alpha_bit_mask3[4]) = { 7 << 9, 0, 7 << 9, 0 };
ALIGN16(static dword SIMD_SSE2_dword_alpha_bit_mask4[4]) = { 7 << 12, 0, 7 << 12, 0 };
ALIGN16(static dword SIMD_SSE2_dword_alpha_bit_mask5[4]) = { 7 << 15, 0, 7 << 15, 0 };
ALIGN16(static dword SIMD_SSE2_dword_alpha_bit_mask6[4]) = { 7 << 18, 0, 7 << 18, 0 };
ALIGN16(static dword SIMD_SSE2_dword_alpha_bit_mask7[4]) = { 7 << 21, 0, 7 << 21, 0 };


void EmitAlphaIndices_Intrinsics(const byte *colorBlock, const byte minAlpha, const byte maxAlpha, byte *&outData)
{
	/*
	__asm {
	mov esi, colorBlock
	movdqa xmm0, [esi+ 0]
	movdqa xmm5, [esi+16]
	psrld xmm0, 24
	psrld xmm5, 24
	packuswb xmm0, xmm5

	movdqa xmm6, [esi+32]
	movdqa xmm4, [esi+48]
	psrld xmm6, 24
	psrld xmm4, 24
	packuswb xmm6, xmm4

	movzx ecx, maxAlpha
	movd xmm5, ecx
	pshuflw xmm5, xmm5, R_SHUFFLE_D( 0, 0, 0, 0 )
	pshufd xmm5, xmm5, R_SHUFFLE_D( 0, 0, 0, 0 )
	movdqa xmm7, xmm5

	movzx edx, minAlpha
	movd xmm2, edx
	pshuflw xmm2, xmm2, R_SHUFFLE_D( 0, 0, 0, 0 )
	pshufd xmm2, xmm2, R_SHUFFLE_D( 0, 0, 0, 0 )
	movdqa xmm3, xmm2

	movdqa xmm4, xmm5
	psubw xmm4, xmm2
	pmulhw xmm4, SIMD_SSE2_word_div_by_14    // * ( ( 1 << 16 ) / 14 + 1 ) ) >> 16
	movdqa xmm1, xmm2
	paddw xmm1, xmm4
	packuswb xmm1, xmm1                      // ab1

	pmullw xmm5, SIMD_SSE2_word_scale66554400
	pmullw xmm7, SIMD_SSE2_word_scale11223300
	pmullw xmm2, SIMD_SSE2_word_scale11223300
	pmullw xmm3, SIMD_SSE2_word_scale66554400
	paddw xmm5, xmm2
	paddw xmm7, xmm3
	pmulhw xmm5, SIMD_SSE2_word_div_by_7 // * ( ( 1 << 16 ) / 7 + 1 ) ) >> 16
	pmulhw xmm7, SIMD_SSE2_word_div_by_7 // * ( ( 1 << 16 ) / 7 + 1 ) ) >> 16
	paddw xmm5, xmm4
	paddw xmm7, xmm4

	pshufd xmm2, xmm5, R_SHUFFLE_D( 0, 0, 0, 0 )
	pshufd xmm3, xmm5, R_SHUFFLE_D( 1, 1, 1, 1 )
	pshufd xmm4, xmm5, R_SHUFFLE_D( 2, 2, 2, 2 )
	packuswb xmm2, xmm2 // ab2
	packuswb xmm3, xmm3 // ab3
	packuswb xmm4, xmm4 // ab4

	packuswb xmm0, xmm6 // alpha values

	pshufd xmm5, xmm7, R_SHUFFLE_D( 2, 2, 2, 2 )
	pshufd xmm6, xmm7, R_SHUFFLE_D( 1, 1, 1, 1 )
	pshufd xmm7, xmm7, R_SHUFFLE_D( 0, 0, 0, 0 )
	packuswb xmm5, xmm5 // ab5
	packuswb xmm6, xmm6 // ab6
	packuswb xmm7, xmm7 // ab7

	pminub xmm1, xmm0
	pminub xmm2, xmm0
	pminub xmm3, xmm0
	pcmpeqb xmm1, xmm0
	pcmpeqb xmm2, xmm0
	pcmpeqb xmm3, xmm0
	pminub xmm4, xmm0
	pminub xmm5, xmm0
	pminub xmm6, xmm0
	pminub xmm7, xmm0
	pcmpeqb xmm4, xmm0
	pcmpeqb xmm5, xmm0
	pcmpeqb xmm6, xmm0
	pcmpeqb xmm7, xmm0
	pand xmm1, SIMD_SSE2_byte_1
	pand xmm2, SIMD_SSE2_byte_1
	pand xmm3, SIMD_SSE2_byte_1
	pand xmm4, SIMD_SSE2_byte_1
	pand xmm5, SIMD_SSE2_byte_1
	pand xmm6, SIMD_SSE2_byte_1
	pand xmm7, SIMD_SSE2_byte_1
	movdqa xmm0, SIMD_SSE2_byte_1
	paddusb xmm0, xmm1
	paddusb xmm2, xmm3
	paddusb xmm4, xmm5
	paddusb xmm6, xmm7
	paddusb xmm0, xmm2
	paddusb xmm4, xmm6
	paddusb xmm0, xmm4
	pand xmm0, SIMD_SSE2_byte_7
	movdqa xmm1, SIMD_SSE2_byte_2
	pcmpgtb xmm1, xmm0
	pand xmm1, SIMD_SSE2_byte_1
	pxor xmm0, xmm1
	movdqa xmm1, xmm0
	movdqa xmm2, xmm0
	movdqa xmm3, xmm0
	movdqa xmm4, xmm0
	movdqa xmm5, xmm0
	movdqa xmm6, xmm0
	movdqa xmm7, xmm0
	psrlq xmm1, 8- 3
	psrlq xmm2, 16- 6
	psrlq xmm3, 24- 9

	psrlq xmm4, 32-12
	psrlq xmm5, 40-15
	psrlq xmm6, 48-18
	psrlq xmm7, 56-21
	pand xmm0, SIMD_SSE2_dword_alpha_bit_mask0
	pand xmm1, SIMD_SSE2_dword_alpha_bit_mask1
	pand xmm2, SIMD_SSE2_dword_alpha_bit_mask2
	pand xmm3, SIMD_SSE2_dword_alpha_bit_mask3
	pand xmm4, SIMD_SSE2_dword_alpha_bit_mask4
	pand xmm5, SIMD_SSE2_dword_alpha_bit_mask5
	pand xmm6, SIMD_SSE2_dword_alpha_bit_mask6
	pand xmm7, SIMD_SSE2_dword_alpha_bit_mask7
	por xmm0, xmm1
	por xmm2, xmm3
	por xmm4, xmm5
	por xmm6, xmm7
	por xmm0, xmm2
	por xmm4, xmm6
	por xmm0, xmm4
	mov esi, outData
	movd [esi+0], xmm0
	pshufd xmm1, xmm0, R_SHUFFLE_D( 2, 3, 0, 1 )
	movd [esi+3], xmm1
	}
	outData += 6;
	*/
}

