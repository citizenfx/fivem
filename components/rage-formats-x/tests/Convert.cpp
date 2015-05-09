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

#undef RAGE_FORMATS_GAME_NY
#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE
#include <gtaDrawable.h>

#include <convert/gtaDrawable_ny_five.h>

void ConvertDrawable()
{
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
}