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
inline int CalculateFlag(size_t size, size_t* outSize)
{
#ifdef RAGE_FORMATS_GAME_NY
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
#else

	//size_t base = 0x2000;

	//uint32_t flag = 0;

	size_t base = 0x2000;

	uint32_t flag = base >> 14;

	intptr_t t = (size % base) ? (size + base) : size;

	if (t >= (base * 16))
	{
		flag |= (1 << 4);
		t -= (base * 16);
	}

	uint8_t a = 0;

	while (t >= (base * 8))
	{
		if (a >= 3)
		{
			break;
		}

		t -= (base * 8);
		a++;
	}

	flag |= (a << 5);

	a = 0;

	while (t >= (base * 4))
	{
		if (a >= 15)
		{
			break;
		}

		t -= (base * 4);
		a++;
	}

	flag |= (a << 7);

	a = 0;

	while (t >= (base * 2))
	{
		if (a >= 0x3F)
		{
			break;
		}

		t -= (base * 2);
		a++;
	}

	flag |= (a << 11);

	a = 0;

	while (t >= (base))
	{
		if (a >= 0x7F)
		{
			break;
		}

		t -= (base);
		a++;
	}

	flag |= (a << 17);

	*outSize = ((((flag >> 17) & 0x7f) + (((flag >> 11) & 0x3f) << 1) + (((flag >> 7) & 0xf) << 2) + (((flag >> 5) & 0x3) << 3) + (((flag >> 4) & 0x1) << 4)) * base);

	return flag;
#endif
}

bool BlockMap::Save(int version, fwAction<const void*, size_t> writer, ResourceFlags* flags)
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

	auto calcFlag = [&] (bool physical, size_t* outSize)
	{
		size_t base = this->baseAllocationSize[physical]; // TODO: pass this from another component

		DWORD bitIdx;
		_BitScanReverse(&bitIdx, base);

#ifdef RAGE_FORMATS_GAME_FIVE
		uint32_t flag = (bitIdx - 13);

		const int8_t maxMults  [] = { 16, 8, 4, 2, 1 };
		const uint8_t maxCounts[] = { 1, 3, 15, 63, 127 };
		const uint8_t valShifts[] = { 4,  5, 7,  11, 17 };
#else
		uint32_t flag = 0;

		const int8_t maxMults  [] = { 1, -2, -4, -8, -16 };
		const uint8_t maxCounts[] = { 0x7F, 1, 1, 1, 1 };
#endif

		int curMult = 0;
		int curCounts[_countof(maxMults)] = { 0 };

		bool remnantBlock = false;

		// capture all full flags
		for (int i = (physical ? this->virtualLen : 0); i < this->virtualLen + (physical ? this->physicalLen : 0); i++)
		{
			size_t curMultSize = (maxMults[curMult] >= 0) ? (base * maxMults[curMult]) : (base / -maxMults[curMult]);

			if (this->blocks[i].size == curMultSize)
			{
				curCounts[curMult]++;

				if (curCounts[curMult] == maxCounts[curMult])
				{
					curMult++;

					remnantBlock = false;
				}
			}
			else
			{
				remnantBlock = true;
			}
		}

		// capture the remnant block
		if (remnantBlock)
		{
			auto block = this->blocks[this->virtualLen - 1 + (physical ? this->physicalLen : 0)];
			bool found = false;

			for (int i = _countof(maxMults) - 1; i >= curMult; i--)
			{
				size_t curMultSize = (maxMults[i] >= 0) ? (base * maxMults[i]) : (base / -maxMults[i]);

				if (block.size <= curMultSize && curCounts[i] <= maxCounts[i])
				{
					curCounts[i]++;
					found = true;
					break;
				}
			}

			assert(found);
		}

#ifdef RAGE_FORMATS_GAME_FIVE
		// set flags
		for (int i = 0; i < _countof(maxMults); i++)
		{
			flag |= (curCounts[i] << valShifts[i]);
		}

		*outSize = ((((flag >> 17) & 0x7f) + (((flag >> 11) & 0x3f) << 1) + (((flag >> 7) & 0xf) << 2) + (((flag >> 5) & 0x3) << 3) + (((flag >> 4) & 0x1) << 4)) * base);
#else
		// / 16 flag
		flag |= curCounts[4];

		// / 8 flag
		flag |= curCounts[3] << 1;

		// / 4 flag
		flag |= curCounts[2] << 2;

		// / 2 flag
		flag |= curCounts[1] << 3;

		// big flag
		flag |= curCounts[0] << 4;

		// base shift
		size_t baseShift = bitIdx - 12;
		flag |= baseShift << 11;

		*outSize = (flag & 0x7FF) << (baseShift + 8);
#endif

		return flag;
	};

#ifdef RAGE_FORMATS_GAME_NY
	uint32_t baseFlags = (1 << 31) | (1 << 30) | calcFlag(false, &virtualOut) | (calcFlag(true, &physicalOut) << 15);

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
#else
	uint32_t virtFlags;
	uint32_t physFlags;

	if (!flags)
	{
		size_t base = 0x2000;
		size_t flag = 0x1890;
		virtualOut = ((((flag >> 17) & 0x7f) + (((flag >> 11) & 0x3f) << 1) + (((flag >> 7) & 0xf) << 2) + (((flag >> 5) & 0x3) << 3) + (((flag >> 4) & 0x1) << 4)) * base);

		virtFlags = (version & 0xF0) << 24 | /*0x1890;*/calcFlag(false, &virtualOut);
		physFlags = (version & 0x0F) << 28 | calcFlag(true, &physicalOut);
	}
	else
	{
		virtFlags = flags->virtualFlag;
		physFlags = flags->physicalFlag;

		version = flags->version;

		// TEMP
		virtualOut = this->blocks[0].size;
		physicalOut = this->blocks[1].size;
	}

	//uint32_t virtFlags = (version & 0xF0) << 24 | 0xE0040;
	//uint32_t physFlags = (version & 0x0F) << 28 | 0x0;

	//virtualOut = 0x2E000;
	//physicalOut = 0;

	uint8_t magic[] = { 'R', 'S', 'C', '7' };

	writer(magic, sizeof(magic));

	writer(&version, sizeof(version));

	writer(&virtFlags, sizeof(virtFlags));

	writer(&physFlags, sizeof(physFlags));

	z_stream strm = { 0 };
	deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
#endif

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

	assert(virtualOut >= virtualSize);
	assert(physicalOut >= physicalSize);

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
}
}