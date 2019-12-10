// hook for the initial mountpoint of filesystem stuff
#include "StdInc.h"
#include <fiDevice.h>
#include <Hooking.h>

#include <Error.h>

static void(*originalMount)();

static void CallInitialMount()
{
	// do pre-initial mount
	originalMount();

	rage::fiDevice::OnInitialMount();
}

#if 0
static std::string currentPack;

static bool OpenArchiveWrap(rage::fiPackfile* packfile, const char* archive, bool a3, int a4, intptr_t a5)
{
	currentPack = archive;

	bool retval = packfile->OpenPackfile(archive, a3, a4, a5);

	currentPack = "";

	if (!retval)
	{
		FatalError("Could not open %s. Please try to verify your GTA V files, see http://rsg.ms/verify for more information.\n\nCurrently, the installation directory %s is being used.", archive, ToNarrow(MakeRelativeGamePath(L"")));
	}

	return retval;
}

static void PackfileEncryptionError()
{
	FatalError("Invalid rage::fiPackfile encryption type%s.\n\nIf you have any modified game files, please remove or verify them. See http://rsg.ms/verify for more information on verifying your game files.\n"
		"If using OpenIV, please make sure you have used the 'mods' folder for placing your modified files.\n\n"
		"Currently, the installation directory %s is being used.",
		(!currentPack.empty()) ? fmt::sprintf(" in packfile %s", currentPack) : " specified",
		ToNarrow(MakeRelativeGamePath(L"")));
}
#endif

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
		auto location = hook::get_pattern<int>("B9 ? ? 00 00 C7 05 ? ? ? ? 64 00 00 00", 11);
		hook::put<int>(location, *location * 15); // '1500' mount limit now, instead of '500'
	}

	hook::set_call(&originalMount, hook::pattern("48 03 C3 44 88 34 38 66 01 1D").count(1).get(0).get<void>(0xE));
	hook::call(hook::pattern("48 03 C3 44 88 34 38 66 01 1D").count(1).get(0).get<void>(0xE), CallInitialMount);

	// don't sort update:/ relative devices before ours
	hook::nop(hook::pattern("C6 80 00 01 00 00 01 E8").count(1).get(0).get<void>(12), 5);

	// fail sanely on missing game packfiles
#if 0
	{
		auto matches = hook::pattern("E8 ? ? ? ? 84 C0 75 0A E8 ? ? ? ? 84 C0").count_hint(7);

		for (int i = 0; i < matches.size(); i++)
		{
			hook::call(matches.get(i).get<void>(0), OpenArchiveWrap);
		}
	}

	// wrap err_gen_invalid failures
	hook::call(hook::get_pattern("B9 EA 0A 0E BE E8", 5), PackfileEncryptionError);
#endif
});
