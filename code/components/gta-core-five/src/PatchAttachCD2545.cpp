#include <StdInc.h>
#include <Hooking.h>
#include "CrossBuildRuntime.h"
#include <ScriptEngine.h>
#include "ICoreGameInit.h"

//
// This patches multiple checks that prevent the application of entity-to-ped
// attachment data to networked peds so it can be dynamically toggled.
//

static bool ReturnFalse()
{
	return false;
}

static bool g_disableRemotePedAttachment = false;

static bool ReturnState()
{
	return g_disableRemotePedAttachment;
}

static HookFunction hookFunction([]()
{
	if (xbr::IsGameBuildOrGreater<2545>())
	{
		const auto location = hook::get_pattern<char>("BA E7 8F A5 1E B9 BD C5 AF E3 E8");
		hook::call(location + 23, ReturnState);

		// This call was removed in 2802.0
		if (!xbr::IsGameBuildOrGreater<2802>())
		{
			hook::call(location + 61, ReturnFalse);
		}

		const auto attachEntityToEntityTunable = hook::get_pattern<char>("BA 37 89 3D 8A B9 BD C5 AF E3");
		hook::call(attachEntityToEntityTunable + 23, ReturnFalse);
	}
});

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("ONESYNC_ENABLE_REMOTE_ATTACHMENT_SANITIZATION", [](fx::ScriptContext& context)
	{
		const bool enable = context.GetArgument<bool>(0);
		g_disableRemotePedAttachment = enable;
	});

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		g_disableRemotePedAttachment = false;
	});
});
