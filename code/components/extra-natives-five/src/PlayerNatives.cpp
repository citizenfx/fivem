#include <StdInc.h>
#include <ScriptEngine.h>

#include <ScriptSerialization.h>
#include <NetworkPlayerMgr.h>
#include <scrEngine.h>

#include <Hooking.h>

static int(*netInterface_GetNumPhysicalPlayers)();
static CNetGamePlayer** (*netInterface_GetAllPhysicalPlayers)();

enum NativeIdentifiers : uint64_t
{
	GET_PLAYER_PED = 0x43A66C31C68491C0
};

static void* getAndCheckPlayerInfo(fx::ScriptContext& context)
{
	if (context.GetArgumentCount() < 1)
	{
		return nullptr;
	}

	CNetGamePlayer* player = CNetworkPlayerMgr::GetPlayer(context.GetArgument<int>(0));

	if (!player)
	{
		return nullptr;
	}

	return player->playerInfo();
}

template<typename T, int* offset>
static void readPlayerInfoMemory(fx::ScriptContext& context, T defaultValue)
{
	if (void* playerInfo = getAndCheckPlayerInfo(context))
	{
		context.SetResult<T>(*(T*)((char*)playerInfo + *offset));
		return;
	}

	context.SetResult<T>(defaultValue);
}

static int WeaponDamageModifierOffset;
static int WeaponDefenseModifierOffset;
static int VehicleDamageModifierOffset;
static int VehicleDefenseModifierOffset;
static int WeaponDefenseModifier2Offset;
static int MeleeWeaponDamageModifierOffset;
static int MeleeWeaponDefenseModifierOffset;
static int PlayerStaminaOffset;
static int PlayerMaxStaminaOffset;

static HookFunction hookFunction([]()
{
	WeaponDamageModifierOffset = *hook::get_pattern<int>("F3 41 0F 59 85 ? ? ? ? 74 ? 48 8B 83", 0x16);
	WeaponDefenseModifierOffset = *hook::get_pattern<int>("F3 0F 11 80 ? ? ? ? F3 0F 10 87 ? ? ? ? 48", 0x4);
	VehicleDamageModifierOffset = *hook::get_pattern<int>("74 ? 48 8B 80 ? ? ? ? F3 44 0F 59 88", 0xE);
	VehicleDefenseModifierOffset = *hook::get_pattern<int>("F6 C1 ? 75 ? 48 8B 83 ? ? ? ? 48 8B 4E", 0x13);
	WeaponDefenseModifier2Offset = *hook::get_pattern<int>("F3 0F 11 ? ? ? ? ? 8A 87 ? ? ? ? C0 E0", 0x4);
	MeleeWeaponDamageModifierOffset = *hook::get_pattern<int>("48 8B 85 ? ? ? ? 0F 2F CE 77 ? 0F 28 CE 49", -0xC);
	MeleeWeaponDefenseModifierOffset = *hook::get_pattern<int>("45 84 ? ? 74 ? 48 8B 83", 0x11);

	PlayerStaminaOffset = *hook::get_pattern<int>("F3 0F 58 F2 0F 2F F0 F3 0F", 0xB);
	PlayerMaxStaminaOffset = PlayerStaminaOffset + 4;

	{
		auto location = hook::get_pattern<char>("48 8B F8 E8 ? ? ? ? F3 0F 10 00 F3 0F 10 48 04", -0x5A);
		hook::set_call(&netInterface_GetNumPhysicalPlayers, location + 0x28);
		hook::set_call(&netInterface_GetAllPhysicalPlayers, location + 0x2F);
	}

	fx::ScriptEngine::RegisterNativeHandler("GET_ACTIVE_PLAYERS", [](fx::ScriptContext& context)
	{
		std::vector<int> playerList;

		int playerNum = netInterface_GetNumPhysicalPlayers();
		auto players = netInterface_GetAllPhysicalPlayers();

		for (int i = 0; i < playerNum; i++)
		{
			playerList.push_back(players[i]->physicalPlayerIndex());
		}

		context.SetResult(fx::SerializeObject(playerList));
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_INVINCIBLE_2", [](fx::ScriptContext& context)
	{
		bool result = false;
		
		int playerPedId = NativeInvoke::Invoke<GET_PLAYER_PED, int>(context.GetArgument<int>(0));
		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(playerPedId);

		if (entity && entity->IsOfType<CPed>())
		{
			auto address = (char*)entity;
			DWORD flag = *(DWORD *)(address + 0x188);  
			result = ((flag & (1 << 8)) != 0) || ((flag & (1 << 9)) != 0);
		}

		context.SetResult<bool>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_STAMINA", [](fx::ScriptContext& context)
	{
		readPlayerInfoMemory<float, &PlayerStaminaOffset>(context, 0.0f);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_IS_STREAMERMODE_ACTIVE", [](fx::ScriptContext& context)
	{
		static ConVar<bool> streamerMode("ui_streamerMode", ConVar_None, false);
		context.SetResult<bool>(streamerMode.GetValue());
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_MAX_STAMINA", [](fx::ScriptContext& context)
	{
		readPlayerInfoMemory<float, &PlayerMaxStaminaOffset>(context, 0.0f);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_PLAYER_STAMINA", [](fx::ScriptContext& context)
	{
		bool result = false;

		void* playerInfo = getAndCheckPlayerInfo(context);

		if (playerInfo)
		{
			float newStamina = context.GetArgument<float>(1);
			float maxStamina = *((float*)((char*)playerInfo + PlayerMaxStaminaOffset));
			if (newStamina && newStamina <= maxStamina)
			{
				*((float*)((char*)playerInfo + PlayerStaminaOffset)) = newStamina;
				result = true;
			}
		}

		context.SetResult<bool>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_PLAYER_MAX_STAMINA", [](fx::ScriptContext& context)
	{
		bool result = false;

		void* playerInfo = getAndCheckPlayerInfo(context);

		if (playerInfo)
		{
			float newMaxStamina = context.GetArgument<float>(1);
			if (newMaxStamina && newMaxStamina > 0.0)
			{
				*((float*)((char*)playerInfo + PlayerMaxStaminaOffset)) = newMaxStamina;
				result = true;
			}
		}

		context.SetResult<bool>(result);
	});

	using namespace std::placeholders;

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_WEAPON_DAMAGE_MODIFIER", std::bind(readPlayerInfoMemory<float, &WeaponDamageModifierOffset>, _1, 0.0f));
	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_WEAPON_DEFENSE_MODIFIER", std::bind(readPlayerInfoMemory<float, &WeaponDefenseModifierOffset>, _1, 0.0f));
	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_WEAPON_DEFENSE_MODIFIER_2", std::bind(readPlayerInfoMemory<float, &WeaponDefenseModifier2Offset>, _1, 0.0f));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_VEHICLE_DAMAGE_MODIFIER", std::bind(readPlayerInfoMemory<float, &VehicleDamageModifierOffset>, _1, 0.0f));
	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_VEHICLE_DEFENSE_MODIFIER", std::bind(readPlayerInfoMemory<float, &VehicleDefenseModifierOffset>, _1, 0.0f));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_MELEE_WEAPON_DAMAGE_MODIFIER", std::bind(readPlayerInfoMemory<float, &MeleeWeaponDamageModifierOffset>, _1, 0.0f));
	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_MELEE_WEAPON_DEFENSE_MODIFIER", std::bind(readPlayerInfoMemory<float, &MeleeWeaponDefenseModifierOffset>, _1, 0.0f));
});
