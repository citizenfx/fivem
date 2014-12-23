/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

/*
 * Flag algorithm for NY based on:
 *
 * RageLib - Copyright (C) 2008 Arushan/Aru <oneforaru at gmail.com>
 */

#include "StdInc.h"
#include "pgBase.h"

#include <zlib.h>

namespace rage
{
namespace RAGE_FORMATS_GAME
{
#if defined(RAGE_FORMATS_GAME_NY)
inline int CalculateFlag(size_t size, size_t* outSize)
{
	int maxBase = (1 << 7) - 1;

	int base = size >> 8;
	int shift = 0;

	while (base > maxBase)
	{
		if (base & 1)
		{
			base += 2;
		}

		base >>= 1;
		shift++;
	}

	// to pad non-even sizes
	shift++;

	if (outSize)
	{
		*outSize = base << (shift + 8);
	}

	return base | (shift << 11);
}

bool BlockMap::Save(int version, fwAction<const void*, size_t> writer)
{
	// calculate physical/virtual sizes
	size_t virtualSize = 0;
	size_t physicalSize = 0;

	for (int i = 0; i < virtualLen; i++)
	{
		virtualSize += blocks[i].size;
	}

	for (int i = virtualLen; i < (physicalLen + virtualLen); i++)
	{
		physicalSize += blocks[i].size;
	}

	// calculate stored sizes
	size_t virtualOut;
	size_t physicalOut;

	uint32_t baseFlags = (1 << 31) | (1 << 30) | CalculateFlag(virtualSize, &virtualOut) | (CalculateFlag(physicalSize, &physicalOut) << 15);

	// write out data to the buffer

	// magic
	uint8_t magic[] = { 'R', 'S', 'C', 0x05 };

	writer(magic, sizeof(magic));

	// version
	writer(&version, sizeof(version));

	// flags
	writer(&baseFlags, sizeof(baseFlags));

	// initialize zlib
	z_stream strm = { 0 };
	deflateInit(&strm, Z_BEST_COMPRESSION);

	auto zwriter = [&] (const void* data, size_t size)
	{
		strm.avail_in = size;
		strm.next_in = (Bytef*)data;

		do 
		{
			uint8_t out[32768];

			strm.avail_out = 32768;
			strm.next_out = out;

			deflate(&strm, Z_NO_FLUSH);

			writer(out, sizeof(out) - strm.avail_out);
		} while (strm.avail_in > 0);
	};

	auto zflush = [&] ()
	{
		uint8_t out[32768];

		do
		{
			strm.avail_out = 32768;
			strm.next_out = out;

			deflate(&strm, Z_FINISH);

			writer(out, sizeof(out) - strm.avail_out);
		} while (strm.avail_out == 0);
	};

	// virtual blocks, start
	for (int i = 0; i < virtualLen; i++)
	{
		zwriter(blocks[i].data, blocks[i].size);
	}

	// virtual blocks, padding
	const char nullChar = 0xCF;

	for (int i = 0; i < (virtualOut - virtualSize); i++)
	{
		zwriter(&nullChar, sizeof(nullChar));
	}

	// physical blocks, start
	for (int i = virtualLen; i < (virtualLen + physicalLen); i++)
	{
		zwriter(blocks[i].data, blocks[i].size);
	}

	// physical blocks, padding
	for (int i = 0; i < (physicalOut - physicalSize); i++)
	{
		zwriter(&nullChar, sizeof(nullChar));
	}

	zflush();

	deflateEnd(&strm);

	// fin!
	return true;
}
#endif
}
}