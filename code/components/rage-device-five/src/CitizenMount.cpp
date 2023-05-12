/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "fiDevice.h"
#include "Hooking.h"

#include "PureModeState.h"

static int(__cdecl* origSetFunc)(void* extraContentMgr, void* a2, const char* deviceName);
int someFunc(void* a1, void* a2, const char* a3)
{
	int someResult = origSetFunc(a1, a2, a3);

	std::string dlcName = std::string(a3);
	dlcName = dlcName.substr(0, dlcName.find(":"));

	auto exists = [](const std::string& path)
	{
		auto device = rage::fiDevice::GetDevice(path.c_str(), true);

		if (device)
		{
			return (device->GetFileAttributes(path.c_str()) != INVALID_FILE_ATTRIBUTES);
		}

		return false;
	};

	{
		std::string name = "citizen:/dlc/" + dlcName + "/";

		if (exists(name))
		{
			{
				rage::fiDeviceRelative* dlc = new rage::fiDeviceRelative();
				dlc->SetPath(name.c_str(), true);
				dlc->Mount((dlcName + ":/").c_str());
			}

			{
				rage::fiDeviceRelative* dlc = new rage::fiDeviceRelative();
				dlc->SetPath(name.c_str(), true);
				dlc->Mount((dlcName + "CRC:/").c_str());
			}
		}
	}

	{
		std::string name = "citizen:/dlc/" + dlcName + "CRC/";

		if (exists(name))
		{
			rage::fiDeviceRelative* dlc = new rage::fiDeviceRelative();
			dlc->SetPath(name.c_str(), true);
			dlc->Mount((dlcName + "CRC:/").c_str());
		}
	}

	return someResult;
}

static HookFunction hookFunction([]()
{
	if (fx::client::GetPureLevel() >= 1)
	{
		return;
	}

	auto dlcPattern = hook::get_pattern("66 39 79 38 74 06 4C 8B 41 30 EB 07 4C 8D", 19);
	hook::set_call(&origSetFunc, dlcPattern);
	hook::call(dlcPattern, someFunc);
});
