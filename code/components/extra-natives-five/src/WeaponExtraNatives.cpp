#include <StdInc.h>
#include <Hooking.h>

#include <ScriptEngine.h>

#include <Resource.h>
#include <fxScripting.h>

static int WeaponDamageModifierOffset;

static uint16_t* g_weaponCount;
static uint64_t** g_weaponList;

static HookFunction hookFunction([]()
{
	{
		// _getWeaponInfoByName
		auto location = hook::get_pattern<char>("45 33 D2 85 C0 74 ? 44 0F B7 05");
		g_weaponCount = hook::get_address<uint16_t*>(location + 11);
		g_weaponList = hook::get_address<uint64_t**>(location + 26);
	}

	{
		WeaponDamageModifierOffset = *hook::get_pattern<int>("48 85 C9 74 ? F3 0F 10 81 ? ? ? ? F3 0F 59 81", 9);
	}

	fx::ScriptEngine::RegisterNativeHandler("GET_WEAPON_DAMAGE_MODIFIER", [](fx::ScriptContext& context)
	{
		auto weaponHash = context.GetArgument<int>(0);
		float result = 0.0f;

		for (int i = 0; i < *g_weaponCount - 1; i++)
		{
			auto weapon = (*g_weaponList)[i];
			auto hash = *(uint32_t*)(weapon + 16);

			if (hash == weaponHash)
			{
				result = *(float*)(weapon + WeaponDamageModifierOffset);
				break;
			}
		}

		context.SetResult<float>(result);
	});
});
