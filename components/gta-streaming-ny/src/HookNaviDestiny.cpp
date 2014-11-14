#include "StdInc.h"
#include <BaseResourceScripting.h>
#include "Hooking.h"

bool g_naviDestiny = false;

static InitFunction initFunction([] ()
{
	OnSetWorldAssetConfig.Connect([] (fwString type, bool value)
	{
		if (value && type == "definitely_more_navigable")
		{
			g_naviDestiny = true;

			hook::put(0xF2A10C, 3);
			hook::put(0xF2A110, 120 / 3);
			hook::put(0xF2A114, 120 / 3);
			hook::put(0x1568B78, (120 / 3) * (120 / 3));
		}
	});
});

#if 0
static void PutSectorCounts()
{
	hook::put(0xF2A10C, 3);
	hook::put(0xF2A110, 300 / 3);
	hook::put(0xF2A114, 300 / 3);
	hook::put(0x1568B78, (300 / 3) * (300 / 3));
}

static int ParseNavFileName(const char* filename)
{
	if (_strnicmp(filename, "navmesh", 7))
	{
		return 8192;
	}

	if (strlen(filename) > 100)
	{
		return 8192;
	}

	char filename2[128];
	strcpy(filename2, filename);
	
	char* firstStr = strchr(filename2, '[');
	firstStr[0] = '\0';
	firstStr++;

	int x = atoi(firstStr);

	char* end = strchr(firstStr, ']');
	end[0] = '\0';
	end++;

	end++;

	firstStr = end;

	end = strchr(firstStr, ']');
	end[0] = '\0';
	end++;

	int y = atoi(firstStr);

	if (y >= 291)
	{
		return 8192;
	}

	return ((y / 3) * (300 / 3)) + (x / 3);
}

static InitFunction initFunction([] ()
{
	OnSetWorldAssetConfig.Connect([] (fwString type, bool value)
	{
		if (value && type == "definitely_more_navigable")
		{
			g_naviDestiny = true;

			// shut down and reinitialize CPathServer
			hook::nop(0x93CBCE, 5); // registering 'file type'
			hook::nop(0x93CBDA, 5); // and storing it

			hook::nop(0x93CA72, 6); // zeroing out type value

			hook::put<uint32_t>(0x93CBCE, 0x904CC483);
			hook::put<uint8_t>(0x93CBCE + 4, 0x90);

			// some freer
			hook::nop(0x93B285, 5);

			// destruct
			((void(*)())0x93B270)();

			// patch
			int maxCount = (300 * 300) / 9;

			//hook::put(0x93AD21, 240); // amount of sectors
			hook::nop(0x93AD20, 35);
			hook::call(0x93AD20, PutSectorCounts);
			 
			hook::put(0x938D77, maxCount - 1); // max id for 'world' type
			hook::put(0x938DC6, maxCount - 1);
			hook::put(0x93AEBB, maxCount - 1);
			hook::put(0x93AE6B, maxCount - 1);
			hook::put(0x93B116, maxCount - 1);
			hook::put(0x93B1A3, maxCount);
			hook::put(0x93B250, maxCount);

			// runtime
			hook::put(0xC9AD37, maxCount + 100);
			hook::put(0xC9B145, maxCount + 100);

			hook::put(0xCA09F2, maxCount + 100);
			hook::put(0xCA0A56, maxCount + 100);
			hook::put(0xCA370F, maxCount + 100);

			hook::put(0x939179, -(maxCount + 20));
			hook::put(0x939185, -(maxCount + 20) * 5);
			
			hook::put(0xC9AD1D, 16384);
			hook::put(0xC9B120, 16384);
			hook::put(0xC9C602, 16384);
			hook::put(0xC9C778, 16384);

			// ignore distance check
			hook::nop(0x93CEB5, 2);

			// sector getter, as these aren't yer regular CWorld sectors here
			// - shouldn't be needed as these are 50.0?

			// at least increase the center of the sector getter... 60 isn't enough if we have 240
			static float flt120 = 120.0f;
			hook::put(0x938FA5, &flt120);

			// some bounds
			static float negBound = -4000.0f;
			static float posBound = 8000.0f;

			hook::put(0x93CD57, &posBound);
			hook::put(0x93CDD4, &negBound);
			
			hook::put(0x93CE04, &negBound);

			static float maxSecNum = 4000.0f;
			hook::put(0x93CE43, &maxSecNum);

			// mhm
			hook::jump(0x939040, ParseNavFileName);

			// construct
			((void(*)(int, int, const char*, const char*))0x93CA00)(2, 0, "navmeshes.img", "citizen:/");
		}
	});
});

static void __declspec(naked) RegisterNavMeshesHook()
{
	__asm
	{
		mov dword ptr [esp + 0Ch], 10020 // new count

		push 0BCD480h
		retn
	}
}

static HookFunction hookFunction([] ()
{
	hook::call(0x93CBCE, RegisterNavMeshesHook);
});
#endif