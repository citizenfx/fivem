/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"
#include <BaseResourceScripting.h>

static int __stdcall ConvertPathAxis(float x)
{
	int v = ((x + 8192.0f) / 2048);

	if (v < 0)
	{
		return 0;
	}

	if (v > 7)
	{
		v = 7;
	}

	return v;
}

static float __stdcall ConvertBackPathAxis(int x)
{
	return (x * 2048.0) - 8192.0;
}

static HookFunction hookFunction([] ()
{
	// define global divisor
	// can only be a single divisor for both axes without having to do more complicated code patching in some funcs :(
	static float divisor = 1.0f / (16000 / 120.0f);
	static float idleDivisor = (16000 / 120.0f);
	static float lodDivisor = 1.0f / (16000 / 30.0f);
	static float idleLodDivisor = (16000 / 30.0f);
	static float posExtent = 8000.0f;
	static float negExtent = -8000.0f;

	// 7D7700
	hook::put(0x7D771A, &divisor);

	hook::put(0x7DDDD6, &divisor);

	hook::put(0x7DF166, &divisor);
	hook::put(0x7DF221, &divisor);

	hook::put(0x7DF166, &divisor);

	hook::put(0x8B8E75, &divisor);

	// v is for navmeshes, not relevant here
	//hook::put(0x938F9D, &divisor);

	hook::put(0x9E00AA, &divisor);

	// |
	// v navmeeeeesh
	//hook::put(0xAA5A7E, &divisor);
	//hook::put(0xAA5AFE, &divisor);
	//hook::put(0xAA5B6A, &divisor);
	//hook::put(0xAA5BE2, &divisor);

	// |
	// v more navmesh stuff
	//hook::put(0xAA5F8A, &divisor);
	//hook::put(0xAA6005, &divisor);
	//hook::put(0xAA6079, &divisor);
	//hook::put(0xAA60F4, &divisor);

	hook::put(0xADE6C9, &divisor);

	hook::put(0xAE3B0A, &divisor);
	hook::put(0xAE3AFC, &idleDivisor);

	hook::put(0xB257F1, &divisor);

	hook::put(0xAA5A7E, &divisor);

	hook::put(0xBF5F08, &divisor);
	hook::put(0xBF5F75, &divisor);
	hook::put(0xBF5FB4, &divisor);

	// 818C40
	hook::put(0x818C4B, &divisor);

	// 81B4A0
	hook::put(0x81B4AB, &divisor);
	hook::put(0x81B571, &divisor);
	hook::put(0x81B5A3, &divisor);

	// 81BB90
	hook::put(0x81BBCD, &divisor);
	hook::put(0x81BBE7, &divisor);
	hook::put(0x81BC03, &divisor);
	hook::put(0x81BC59, &divisor);

	// 81BFD0
	hook::put(0x81C00D, &divisor);
	hook::put(0x81C027, &divisor);
	hook::put(0x81C043, &divisor);
	hook::put(0x81C099, &divisor);

	// 81C3D0
	hook::put(0x81C40D, &divisor);
	hook::put(0x81C427, &divisor);
	hook::put(0x81C443, &divisor);
	hook::put(0x81C499, &divisor);

	// 8FC7D0
	hook::put(0x8FC80D, &divisor);
	hook::put(0x8FC827, &divisor);
	hook::put(0x8FC843, &divisor);
	hook::put(0x8FC899, &divisor);

	// CEntity::Add
	hook::put(0x9E9FA6, &divisor);
	hook::put(0x9E9FC6, &divisor);
	hook::put(0x9EA001, &divisor);
	hook::put(0x9EA050, &divisor);

	hook::put(0x9E9E24, &negExtent);
	hook::put(0x9E9E5A, &posExtent);

	// CEntity::something
	hook::put(0x9EA826, &divisor);
	hook::put(0x9EA849, &divisor);
	hook::put(0x9EA892, &divisor);
	hook::put(0x9EA8A2, &divisor);

	hook::put(0x9EA6CF, &negExtent);
	hook::put(0x9EA701, &posExtent);

	// 9FA130
	hook::put(0x9FA16D, &divisor);
	hook::put(0x9FA185, &divisor);
	hook::put(0x9FA1D0, &divisor);
	hook::put(0x9FA1E5, &divisor);

	// A739C0
	hook::put(0xA739ED, &divisor);
	hook::put(0xA73A06, &divisor);
	hook::put(0xA73A50, &divisor);
	hook::put(0xA73A66, &divisor);

	// B257E0
	hook::put(0xB257F1, &divisor);

	// B2B660
	hook::put(0xB2B6A0, &divisor);

	// BBE1A0
	hook::put(0xBBE214, &divisor);

	// BBE440
	hook::put(0xBBE479, &divisor);
	hook::put(0xBBE4BB, &divisor);
	hook::put(0xBBE547, &divisor);

	// repeatsectors only caller
	hook::put(0x9E00AA, &divisor);

	// repeatsectors only, BF7B90
	hook::put(0xBF7C27, &divisor);
	hook::put(0xBF7C8E, &divisor);
	hook::put(0xBF7CCF, &divisor);

	// BF81E0
	hook::put(0xBF82F3, &divisor);

	// FIN... for now

	// lod sectors
	hook::put(0x819000, &lodDivisor);

	hook::put(0x9E9E9E, &lodDivisor);

	hook::put(0x9EA745, &lodDivisor);

	hook::put(0xB2BAAD, &lodDivisor);

	// worldscan
	hook::put(0x7DE008, &lodDivisor);

	hook::put(0x7DF452, &lodDivisor);
	hook::put(0x7DF505, &lodDivisor);

	// iplstore quadtree bounds
	static float minCoord = -8000.0f;
	static float maxCoord = 8000.0f;

	hook::put(0xB2731B, &minCoord);
	hook::put(0xB27335, &maxCoord);

	// iplstore subnodes
	//hook::put<uint8_t>(0xB276EB, 4);

	// boundsstore
	hook::put(0xC0A22B, &minCoord);
	hook::put(0xC0A245, &maxCoord);

	// boundsstore subnodes
	//hook::put<uint8_t>(0xC0A6AB, 4);

	// physicsstore
	hook::put(0x9704EB, &minCoord);
	hook::put(0x970505, &maxCoord);

	// entity sync coordinate divisor?
	static float reverseDivisor = 1.0f / 125.0f;

	hook::put<float>(0xEBE15C, 125.0f);

	hook::put<float*>(0x49600D, &reverseDivisor);
	hook::put<float*>(0x495FD4, &reverseDivisor);

	// paths
	OnSetWorldAssetConfig.Connect([] (fwString name, bool value)
	{
		if (name == "bigger_paths")
		{
			static float floatStart = 1.0 / 4.0; // 0.25
			static float floatZ = 1.0 / 32.0;

			auto quickPatch = [=] (uintptr_t funcStart, uintptr_t funcEnd)
			{
				for (char* ptr = (char*)funcStart; ptr <= (char*)funcEnd; ptr++)
				{
					if (!memcmp(ptr, "\xD8\xFD\xDA\x00", 4) && (*(uint8_t*)(ptr - 1) != 0xE8) && (*(uint8_t*)(ptr - 1) != 0xE9))
					{
						hook::put((uintptr_t)ptr, &floatStart);
					}
					else if (!memcmp(ptr, "\x38\xE0\xEB\x00", 4) && (*(uint8_t*)(ptr - 1) != 0xE8) && (*(uint8_t*)(ptr - 1) != 0xE9))
					{
						hook::put((uintptr_t)ptr, &floatZ);
					}
				}
			};

			// start off with most of CPathFind
			quickPatch(0xA01021, 0xA27EA2);

			quickPatch(0x438820, 0x438860);
			quickPatch(0x43969B, 0x4396AB);
			quickPatch(0x445057, 0x445067);
			quickPatch(0x44A113, 0x456DBF);

			quickPatch(0x8193D9, 0x84E686);
			quickPatch(0x88CA63, 0x8A46A2);

			quickPatch(0x909E17, 0x90A5CF);

			quickPatch(0x9BA6AC, 0x9BA6BC);

			quickPatch(0xA8712E, 0xA87E78);

			quickPatch(0xAA4FE3, 0xAA4FF3);

			quickPatch(0xB6903B, 0xB6A004);

			quickPatch(0xBEB953, 0xBF0D90);

			quickPatch(0xBF354C, 0xBF38F3);

			quickPatch(0xBF9A4D, 0xC406FC);

			// more patching
			static float pathSectorSize = 2048.0f;

			hook::put(0x83CB8E, &pathSectorSize);
			hook::put(0x83CBBC, &pathSectorSize);
			hook::put(0xA03A33, &pathSectorSize);
			hook::put(0xA06F07, &pathSectorSize);

			// mmm
			hook::jump(0xA019C0, ConvertPathAxis);

			hook::jump(0xA01A00, ConvertBackPathAxis);
		}
	});
});