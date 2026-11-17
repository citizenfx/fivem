/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>

#include <ScriptEngine.h>

#include <NetLibrary.h>

static bool IsLocalPlayerSrc(NetLibrary* netLibrary, fx::ScriptContext& context)
{
	const char* id = context.GetArgument<const char*>(0);

	if (!id)
	{
		return true;
	}

	uint32_t netId = atoi(id);

	return netId != 0 && netId == netLibrary->GetServerNetID();
}

static InitFunction initFunction([]()
{
	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* netLibrary)
	{
		fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_PING", [netLibrary](fx::ScriptContext& context)
		{
			if (!IsLocalPlayerSrc(netLibrary, context))
			{
				context.SetResult<int>(0);
				return;
			}

			uint32_t roundTripTime;

			if (!netLibrary->GetPeerStatistic(NetPeerStatistic::RoundTripTime, &roundTripTime))
			{
				context.SetResult<int>(-1);
				return;
			}

			context.SetResult<int>(int(roundTripTime));
		});

		fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_PEER_STATISTICS", [netLibrary](fx::ScriptContext& context)
		{
			if (!IsLocalPlayerSrc(netLibrary, context))
			{
				context.SetResult<int>(0);
				return;
			}

			const int peerStatistic = context.GetArgument<uint8_t>(1);

			if (peerStatistic >= int(NetPeerStatistic::Max))
			{
				throw std::runtime_error(va("Argument 1 is out of range, max peer statistics is %d, got %d", int(NetPeerStatistic::Max) - 1, peerStatistic));
			}

			uint32_t value = 0;
			netLibrary->GetPeerStatistic(static_cast<NetPeerStatistic>(peerStatistic), &value);

			context.SetResult<int>(static_cast<int>(value));
		});
	});
});
