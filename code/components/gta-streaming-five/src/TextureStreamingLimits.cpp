#include <StdInc.h>
#include <Hooking.h>

#include <EntitySystem.h>

#include <CoreConsole.h>

extern std::string GetCurrentStreamingName();

static int(*g_origCalculateMipLevel)(uint8_t type, uint16_t width, uint16_t height, uint8_t levels, uint32_t format);

fwArchetype* GetArchetypeSafe(uint32_t archetypeHash, uint64_t* archetypeUnk);

static ConVar<int>* g_maxVehicleTextureRes;
static ConVar<int>* g_maxVehicleTextureResRgba;
static uintptr_t g_vtbl_CVehicleModelInfo;

static int CalculateMipLevelHook(uint8_t type, uint16_t width, uint16_t height, uint8_t levels, uint32_t format)
{
	auto targetLevel = g_origCalculateMipLevel(type, width, height, levels, format);
	auto oldTargetLevel = targetLevel;

	if (levels > 1 && targetLevel < levels)
	{
		auto strName = GetCurrentStreamingName();
		auto baseName = strName.substr(0, strName.find('.'));

		// try getting the relevant archetype, and see if it's a vehicle
		uint64_t archetypeUnk = 0xFFFFFFF;
		auto archetype = GetArchetypeSafe(HashString(baseName.c_str()), &archetypeUnk);

		if (archetype)
		{
			if (*(uintptr_t*)archetype == g_vtbl_CVehicleModelInfo)
			{
				auto rvar = (format == 21 || format == 32) ? g_maxVehicleTextureResRgba : g_maxVehicleTextureRes;

				// get width after applying stuff
				auto newWidth = width;
				auto newHeight = height;

				for (int r = 0; r < targetLevel; r++)
				{
					newWidth /= 2;
					newHeight /= 2;
				}

				// limit width/height
				while (newWidth > rvar->GetValue() || newHeight > rvar->GetValue())
				{
					if (targetLevel >= levels)
					{
						break;
					}

					newWidth /= 2;
					newHeight /= 2;

					targetLevel++;
				}

#ifdef _DEBUG
				if (targetLevel != oldTargetLevel)
				{
					trace("limited texture level in %s from %dx%d to %dx%d\n", strName, width, height, newWidth, newHeight);
				}
#endif
			}
		}
	}

	return targetLevel;
}

static HookFunction hookFunction([]()
{
	g_maxVehicleTextureRes = new ConVar<int>("str_maxVehicleTextureRes", ConVar_Archive, 1024);
	g_maxVehicleTextureResRgba = new ConVar<int>("str_maxVehicleTextureResRgba", ConVar_Archive, 512);

	auto location = hook::get_pattern("8B 47 58 0F B6 49 5C 89 44 24 20 E8", 11);
	hook::set_call(&g_origCalculateMipLevel, location);
	hook::call(location, CalculateMipLevelHook);

	g_vtbl_CVehicleModelInfo = hook::get_address<uintptr_t>(hook::get_pattern("45 33 C0 48 8D 05 ? ? ? ? 48 8D BB C0 00 00 00", 6));
});
