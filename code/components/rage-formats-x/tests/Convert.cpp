/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#define RAGE_FORMATS_GAME ny
#define RAGE_FORMATS_GAME_NY
#include <gtaDrawable.h>
#include <phBound.h>

#undef RAGE_FORMATS_GAME_NY
#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE
#include <gtaDrawable.h>
#include <phBound.h>

#include <convert/gtaDrawable_ny_five.h>
#include <convert/phBound_ny_five.h>

rage::ny::BlockMap* UnwrapRSC5(const wchar_t* fileName);

void ConvertDrawableInternal(const wchar_t* fileName)
{
	rage::ny::BlockMap* bm = UnwrapRSC5(fileName);

	if (!bm)
	{
		trace("couldn't open input file...\n");
		return;
	}

	std::wstring fileExt = std::wstring(wcsrchr(fileName, L'.'));

	rage::ny::pgStreamManager::SetBlockInfo(bm);
	auto bm2 = rage::five::pgStreamManager::BeginPacking();

	int fileVersion = 0;

	if (fileExt == L".wbn")
	{
		trace("converting bound...\n");

		auto bound = (rage::ny::datOwner<rage::ny::phBound>*)bm->blocks[0].data;
		rage::convert<rage::five::phBound*>(bound->GetChild());

		fileVersion = 43;
	}
	else if (fileExt == L".wdr")
	{
		trace("converting drawable...\n");

		auto drawable = (rage::ny::gtaDrawable*)bm->blocks[0].data;
		rage::convert<rage::five::gtaDrawable*>(drawable);

		fileVersion = 165;
	}
	else if (fileExt == L".wtd")
	{
		trace("converting txd...\n");

		auto txd = (rage::ny::pgDictionary<rage::ny::grcTexturePC>*)bm->blocks[0].data;
		rage::convert<rage::five::pgDictionary<rage::five::grcTexturePC>*>(txd);

		fileVersion = 13;
	}
	else
	{
		trace("unknown file extension...\n");

		return;
	}

	rage::five::pgStreamManager::EndPacking();

	std::wstring outFileName(fileName);
	outFileName = outFileName.substr(0, outFileName.length() - 3) + L"y" + fileExt.substr(2);

	FILE* f = _wfopen(outFileName.c_str(), L"wb");

	if (!f)
	{
		trace("... couldn't open output file for writing.\n");
		return;
	}

	size_t outputSize = 0;

	bm2->Save(fileVersion, [&] (const void* d, size_t s)
	{
		fwrite(d, 1, s, f);

		outputSize += s;
	});

	trace("written successfully - compressed size %d\n", outputSize);

	fclose(f);

	for (int i = 0; i < bm->physicalLen + bm->virtualLen; i++)
	{
		delete bm->blocks[i].data;
	}

	delete bm;
}

void ConvertDrawable(const wchar_t* from)
{
	ConvertDrawableInternal(from);

#if 0
	FILE* f = fopen("Y:/dev/ydr/stat_hilberty01.wdr.sys", "rb");
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	fseek(f, 0, SEEK_SET);

	char* buffer = new char[len];
	fread(buffer, 1, len, f);
	fclose(f);

	long vlen = len;

	f = fopen("Y:/dev/ydr/stat_hilberty01.wdr.gfx", "rb");
	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, 0, SEEK_SET);

	char* buffer2 = new char[len];
	fread(buffer2, 1, len, f);
	fclose(f);

	rage::ny::BlockMap bm;
	bm.virtualLen = 1;
	bm.physicalLen = 1;
	bm.blocks[0].data = buffer;
	bm.blocks[0].offset = 0;
	bm.blocks[0].size = vlen;

	bm.blocks[1].data = buffer2;
	bm.blocks[1].offset = 0;
	bm.blocks[1].size = len;

	rage::ny::pgStreamManager::SetBlockInfo(&bm);
	rage::ny::gtaDrawable* drawable = (rage::ny::gtaDrawable*)buffer;
	//ddrawable->Resolve(&bm);

	//rage::ny::pgStreamManager::BeginPacking(&bm);
	auto bm2 = rage::five::pgStreamManager::BeginPacking();
	auto cdrawable = rage::convert<rage::five::gtaDrawable*>(drawable);
	rage::five::pgStreamManager::EndPacking();

	f = fopen("Y:\\common\\lovely.ydr", "wb");

	bm2->Save(165, [&] (const void* d, size_t s)
	{
		fwrite(d, 1, s, f);
	});

	fclose(f);
#endif
}