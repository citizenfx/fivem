/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <ScriptEngine.h>
#include <NetLibrary.h>

static HookFunction hookFunction([]()
{
	static NetLibrary* netLibrary;

	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* lib)
	{
		netLibrary = lib;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_CLIENT_PING", [](fx::ScriptContext& context)
	{
		if (netLibrary)
		{
			context.SetResult<int>(netLibrary->GetPing());
			return;
		}

		context.SetResult<int>(-1);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_CLIENT_PING_VARIANCE", [](fx::ScriptContext& context)
	{
		if (netLibrary)
		{
			context.SetResult<int>(netLibrary->GetVariance());
			return;
		}

		context.SetResult<int>(-1);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_CLIENT_PACKET_LOSS", [](fx::ScriptContext& context)
	{
		if (netLibrary)
		{
			context.SetResult<int>(netLibrary->GetPacketLoss());
			return;
		}

		context.SetResult<int>(-1);
	});
});
