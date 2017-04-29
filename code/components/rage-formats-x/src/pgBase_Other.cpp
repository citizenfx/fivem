/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#ifdef GTA_FIVE
#define RAGE_FORMATS_GAME ny
#define RAGE_FORMATS_GAME_NY
#else
#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE
#endif

#include "pgBase.cpp"

#ifdef RAGE_FORMATS_GAME_NY
#include <zlib.h>

 // temporary wrapper function
FORMATS_EXPORT rage::ny::BlockMap* UnwrapRSC5(const wchar_t* fileName)
{
	FILE* f = _wfopen(fileName, L"rb");

	if (!f)
	{
		return nullptr;
	}

	fseek(f, 0, SEEK_END);
	size_t fileLength = ftell(f) - 12;
	fseek(f, 0, SEEK_SET);

	uint32_t magic;
	fread(&magic, 1, sizeof(magic), f);

	if (magic != 0x05435352)
	{
		printf("that's not a RSC5, you silly goose...\n");

		fclose(f);
		return nullptr;
	}

	uint32_t version;
	fread(&version, 1, sizeof(version), f);

	if (version != 110 && version != 32 && version != 8)
	{
		printf("not actually a supported file...\n");

		fclose(f);
		return nullptr;
	}

	uint32_t flags;
	fread(&flags, 1, sizeof(flags), f);

	uint32_t virtualSize = (flags & 0x7FF) << (((flags >> 11) & 0xF) + 8);
	uint32_t physicalSize = ((flags >> 15) & 0x7FF) << (((flags >> 26) & 0xF) + 8);

	std::vector<uint8_t> tempBytes(virtualSize + physicalSize);

	{
		std::vector<uint8_t> tempInBytes(fileLength);
		fread(&tempInBytes[0], 1, fileLength, f);

		size_t destLength = tempBytes.size();
		uncompress(&tempBytes[0], (uLongf*)&destLength, &tempInBytes[0], tempInBytes.size());
	}

	char* virtualData = new char[virtualSize];
	memcpy(virtualData, &tempBytes[0], virtualSize);

	char* physicalData = new char[physicalSize];
	memcpy(physicalData, &tempBytes[virtualSize], physicalSize);

	rage::ny::BlockMap* bm = new rage::ny::BlockMap();
	bm->physicalLen = 1;
	bm->virtualLen = 1;

	bm->blocks[0].data = virtualData;
	bm->blocks[0].offset = 0;
	bm->blocks[0].size = virtualSize;

	bm->blocks[1].data = physicalData;
	bm->blocks[1].offset = 0;
	bm->blocks[1].size = physicalSize;

	return bm;
}
#endif