//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
//

#pragma once

#include <intrin.h>
#include <emmintrin.h>

static void ConvertImageDataRGBA_BGRA(int xoffset, int yoffset, int width, int height,
							          int inputPitch, const void *input, size_t outputPitch, void *output)
{
	const unsigned int *source = NULL;
	unsigned int *dest = NULL;
	__m128i brMask = _mm_set1_epi32(0x00ff00ff);

	for (int y = 0; y < height; y++)
	{
		source = reinterpret_cast<const unsigned int*>(static_cast<const unsigned char*>(input)+(y + yoffset) * inputPitch + xoffset * 4);
		dest = reinterpret_cast<unsigned int*>(static_cast<unsigned char*>(output)+(y + yoffset) * outputPitch + xoffset * 4);
		int x = 0;

		// Make output writes aligned
		for (x = 0; ((reinterpret_cast<intptr_t>(&dest[x]) & 15) != 0) && x < width; x++)
		{
			unsigned int rgba = source[x];
			dest[x] = (_rotl(rgba, 16) & 0x00ff00ff) | (rgba & 0xff00ff00);
		}

		for (; x + 3 < width; x += 4)
		{
			__m128i sourceData = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&source[x]));
			// Mask out g and a, which don't change
			__m128i gaComponents = _mm_andnot_si128(brMask, sourceData);
			// Mask out b and r
			__m128i brComponents = _mm_and_si128(sourceData, brMask);
			// Swap b and r
			__m128i brSwapped = _mm_shufflehi_epi16(_mm_shufflelo_epi16(brComponents, _MM_SHUFFLE(2, 3, 0, 1)), _MM_SHUFFLE(2, 3, 0, 1));
			__m128i result = _mm_or_si128(gaComponents, brSwapped);
			_mm_store_si128(reinterpret_cast<__m128i*>(&dest[x]), result);
		}

		// Perform leftover writes
		for (; x < width; x++)
		{
			unsigned int rgba = source[x];
			dest[x] = (_rotl(rgba, 16) & 0x00ff00ff) | (rgba & 0xff00ff00);
		}
	}
}