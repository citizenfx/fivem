#include <StdInc.h>
#include <Hooking.h>

#include <ScriptEngine.h>

#include <Resource.h>
#include <fxScripting.h>

struct PlayerRecoilModifiers
{
	float recoilModifierMin;
	float recoilModifierMax;
	float recoilCrouchedModifier;
	float blindFireModifierMin;
	float blindFireModifierMax;
	float recentlyDamagedModifier;
};

static PlayerRecoilModifiers* g_playerModifiers;

static HookFunction initFunction([]()
{
	auto location = hook::get_pattern<char>("48 8B C8 E8 ? ? ? ? F3 0F 10 1D ? ? ? ? F3 0F 5C 1D ? ? ? ? 0F 28 C8");
	g_playerModifiers = hook::get_address<PlayerRecoilModifiers*>(location + 20);

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_RECOIL_MODIFIER_MIN", [=](fx::ScriptContext& context)
	{
		context.SetResult<float>(g_playerModifiers->recoilModifierMin);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_PLAYER_RECOIL_MODIFIER_MIN", [=](fx::ScriptContext& context)
	{
		const float value = context.GetArgument<float>(0);
		g_playerModifiers->recoilModifierMin = value;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_RECOIL_MODIFIER_MAX", [=](fx::ScriptContext& context)
	{
		context.SetResult<float>(g_playerModifiers->recoilModifierMax);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_PLAYER_RECOIL_MODIFIER_MAX", [=](fx::ScriptContext& context)
	{
		const float value = context.GetArgument<float>(0);
		g_playerModifiers->recoilModifierMax = value;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_RECOIL_CROUCHED_MODIFIER", [=](fx::ScriptContext& context)
	{
		context.SetResult<float>(g_playerModifiers->recoilCrouchedModifier);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_PLAYER_RECOIL_CROUCHED_MODIFIER", [=](fx::ScriptContext& context)
	{
		const float value = context.GetArgument<float>(0);
		g_playerModifiers->recoilCrouchedModifier = value;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_BLIND_FIRE_MODIFIER_MIN", [=](fx::ScriptContext& context)
	{
		context.SetResult<float>(g_playerModifiers->blindFireModifierMin);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_PLAYER_BLIND_FIRE_MODIFIER_MIN", [=](fx::ScriptContext& context)
	{
		const float value = context.GetArgument<float>(0);
		g_playerModifiers->blindFireModifierMin = value;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_BLIND_FIRE_MODIFIER_MAX", [=](fx::ScriptContext& context)
	{
		context.SetResult<float>(g_playerModifiers->blindFireModifierMax);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_PLAYER_BLIND_FIRE_MODIFIER_MAX", [=](fx::ScriptContext& context)
	{
		const float value = context.GetArgument<float>(0);
		g_playerModifiers->blindFireModifierMax = value;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_RECENTLY_DAMAGED_MODIFIER", [=](fx::ScriptContext& context)
	{
		context.SetResult<float>(g_playerModifiers->recentlyDamagedModifier);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_PLAYER_RECENTLY_DAMAGED_MODIFIER", [=](fx::ScriptContext& context)
	{
		const float value = context.GetArgument<float>(0);
		g_playerModifiers->recentlyDamagedModifier = value;
	});
});
