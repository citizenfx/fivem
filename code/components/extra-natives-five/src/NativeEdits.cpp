#include <StdInc.h>
#include <GameInit.h>
#include <Hooking.h>
#include <ScriptEngine.h>

static bool* g_ropesCreateNetworkWorldState;

static HookFunction hookFunction([]()
{
	g_ropesCreateNetworkWorldState = (bool*)hook::AllocateStubMemory(1);
	*g_ropesCreateNetworkWorldState = false;

	// replace ADD_ROPE native net game check whether to create CNetworkRopeWorldStateData
	auto location = hook::get_pattern<uint32_t>("80 3D ? ? ? ? ? 74 71 E8 ? ? ? ? 48", 2);
	*location = (intptr_t)g_ropesCreateNetworkWorldState - (intptr_t)location - 5;

	fx::ScriptEngine::RegisterNativeHandler("SET_ROPES_CREATE_NETWORK_WORLD_STATE", [](fx::ScriptContext& context)
	{
		bool shouldCreate = context.GetArgument<bool>(0);
		*g_ropesCreateNetworkWorldState = shouldCreate;
	});

	OnKillNetworkDone.Connect([]()
	{
		*g_ropesCreateNetworkWorldState = false;
	});
});
