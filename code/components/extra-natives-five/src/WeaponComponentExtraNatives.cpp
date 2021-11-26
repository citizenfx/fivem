#include <StdInc.h>
#include <Hooking.h>

#include <ScriptEngine.h>
#include <MinHook.h>

static uint16_t g_weaponComponentCount;
static uint64_t* g_weaponComponentList;

static int ClipSizeOffset = 0x48;
static int CameraHashOffset = 0x48;
static int ReticuleHashOffset = 0x54;

static int AccuracyModifierPtrOffset = 0x28;
static int DamageModifierPtrOffset = 0x30;

static int RangeModifierPtrOffset = 0x38;
static int RangeModifierDamageModifierOffset = 0x04;

static uint64_t getWeaponComponent(fx::ScriptContext& context)
{
	if (context.GetArgumentCount() < 1)
	{
		return 0;
	}

	int componentHash = context.GetArgument<int>(0);

	for (int i = 0; i < g_weaponComponentCount - 1; i++)
	{
		uint64_t component = g_weaponComponentList[i];
		auto hash = *(int*)(component + 16);

		if (hash == componentHash)
		{
			return component;
		}
	}

	return 0;
}

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern<char>("44 8B 05 ? ? ? ? 45 33 C9 41 FF C8");
		g_weaponComponentCount = *(uint16_t*)(location + 5);
		g_weaponComponentList = hook::get_address<uint64_t*>(location + 22);
	}

	fx::ScriptEngine::RegisterNativeHandler("GET_WEAPON_COMPONENT_CLIP_SIZE", [](fx::ScriptContext& context)
	{
		uint16_t clipSize = 0;

		if (auto component = getWeaponComponent(context))
		{
			clipSize = *(uint16_t*)(component + ClipSizeOffset);
		}

		context.SetResult(clipSize);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WEAPON_COMPONENT_CAMERA_HASH", [](fx::ScriptContext& context)
	{
		uint32_t cameraHash = 0;

		if (auto component = getWeaponComponent(context))
		{
			cameraHash = *(uint32_t*)(component + CameraHashOffset);
		}

		context.SetResult(cameraHash);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WEAPON_COMPONENT_RETICULE_HASH", [](fx::ScriptContext& context)
	{
		uint32_t reticuleHash = 0;

		if (auto component = getWeaponComponent(context))
		{
			reticuleHash = *(uint32_t*)(component + ReticuleHashOffset);
		}

		context.SetResult(reticuleHash);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WEAPON_COMPONENT_ACCURACY_MODIFIER", [](fx::ScriptContext& context)
	{
		float result = 0.0f;

		if (auto component = getWeaponComponent(context))
		{
			auto accuracyModifier = *reinterpret_cast<uint64_t*>((uint64_t)component + AccuracyModifierPtrOffset);
			result = *(float*)(accuracyModifier);
		}

		context.SetResult(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WEAPON_COMPONENT_DAMAGE_MODIFIER", [](fx::ScriptContext& context)
	{
		float result = 0.0f;

		if (auto component = getWeaponComponent(context))
		{
			auto damageModifier = *reinterpret_cast<uint64_t*>((uint64_t)component + DamageModifierPtrOffset);
			result = *(float*)(damageModifier);
		}

		context.SetResult(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WEAPON_COMPONENT_RANGE_MODIFIER", [](fx::ScriptContext& context)
	{
		float result = 0.0f;

		if (auto component = getWeaponComponent(context))
		{
			auto rangeModifier = *reinterpret_cast<uint64_t*>((uint64_t)component + RangeModifierPtrOffset);
			result = *(float*)(rangeModifier);
		}

		context.SetResult(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WEAPON_COMPONENT_RANGE_DAMAGE_MODIFIER", [](fx::ScriptContext& context)
	{
		float result = 0.0f;

		if (auto component = getWeaponComponent(context))
		{
			auto rangeModifier = *reinterpret_cast<uint64_t*>((uint64_t)component + RangeModifierPtrOffset);
			result = *(float*)(rangeModifier + RangeModifierDamageModifierOffset);
		}

		context.SetResult(result);
	});
});
