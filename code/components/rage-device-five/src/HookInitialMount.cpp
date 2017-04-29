// hook for the initial mountpoint of filesystem stuff
#include "StdInc.h"
#include <fiDevice.h>
#include <Hooking.h>

static hook::cdecl_stub<void()> originalMount([] ()
{
	return hook::pattern("48 81 EC E0 03 00 00 48 B8 63 6F 6D 6D").count(1).get(0).get<void>(-0x1A);
});

static void CallInitialMount()
{
	// do pre-initial mount
	originalMount();

	rage::fiDevice::OnInitialMount();
}

static HookFunction hookFunction([] ()
{
	/*static hook::inject_call<void, int> injectCall(0x7B2E27);

	injectCall.inject([] (int)
	{
		injectCall.call();

		rage::fiDevice::OnInitialMount();
	});*/

	// increase non-DLC fiDevice mount limit
	{
		/*auto location = hook::get_pattern<char>("44 8B 35 ? ? ? ? 45 85 F6 74 1B 48 8D", 3);
		int* limit = (int*)(location + *(int32_t*)location + 4);

		*limit *= 8;*/

		// GTA project initialization code, arxan-obfuscated
		auto location = hook::get_pattern<int>("C7 05 ? ? ? ? 64 00 00 00 48 8B", 6);
		hook::put<int>(location, *location * 5);
	}

	// patch 2 changed register alloc (2015-04-17)
	//hook::call(hook::pattern("0F B7 05 ? ? ? ? 48 03 C3 44 88 3C 38 66").count(1).get(0).get<void>(0x15), CallInitialMount);
	hook::call(hook::pattern("0F B7 05 ? ? ? ? 48 03 C3 44 88 34 38 66").count(1).get(0).get<void>(0x15), CallInitialMount);

	// temporarily located here because of lack of glue/such
	rage::fiDevice::OnInitialMount.Connect([] ()
	{
		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
			std::string narrowPath = converter.to_bytes(MakeRelativeCitPath(L"citizen\\common"));

			rage::fiDeviceRelative* relativeDevice = new rage::fiDeviceRelative();
			relativeDevice->SetPath(narrowPath.c_str(), nullptr, true);
			relativeDevice->Mount("common:/");

			rage::fiDeviceRelative* relativeDeviceCrc = new rage::fiDeviceRelative();
			relativeDeviceCrc->SetPath(narrowPath.c_str(), nullptr, true);
			relativeDeviceCrc->Mount("commoncrc:/");
		}

		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
			std::string narrowPath = converter.to_bytes(MakeRelativeCitPath(L"citizen\\platform"));

			rage::fiDeviceRelative* relativeDevice = new rage::fiDeviceRelative();
			relativeDevice->SetPath(narrowPath.c_str(), nullptr, true);
			relativeDevice->Mount("platform:/");

			rage::fiDeviceRelative* relativeDeviceCrc = new rage::fiDeviceRelative();
			relativeDeviceCrc->SetPath(narrowPath.c_str(), nullptr, true);
			relativeDeviceCrc->Mount("platformcrc:/");
		}

		/*
		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
			std::string narrowPath = converter.to_bytes(MakeRelativeCitPath(L"citizen\\update"));

			rage::fiDeviceRelative* relativeDevice = new rage::fiDeviceRelative();
			relativeDevice->SetPath(narrowPath.c_str(), nullptr, true);
			relativeDevice->Mount("update:/");

			rage::fiDeviceRelative* relativeDeviceCrc = new rage::fiDeviceRelative();
			relativeDeviceCrc->SetPath(narrowPath.c_str(), nullptr, true);
			relativeDeviceCrc->Mount("updatecrc:/");
		}
		*/
	});

	// don't sort update:/ relative devices before ours
	hook::nop(hook::pattern("C6 80 F0 00 00 00 01 E8 ? ? ? ? E8").count(1).get(0).get<void>(12), 5);
});