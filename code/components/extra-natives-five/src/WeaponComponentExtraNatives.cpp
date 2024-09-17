#include <StdInc.h>
#include <Hooking.h>

#include <ScriptEngine.h>
#include <MinHook.h>

#include <CrossBuildRuntime.h>

enum class WeaponComponentType
{
	Info,
	Clip,
	FlashLight,
	LaserSight,
	ProgrammableTargeting,
	Scope,
	Suppressor,
	VariantModel,
	Group,
};

struct CWeaponAccuracyModifier
{
	float AccuracyModifier;
};

struct CWeaponDamageModifier
{
	float DamageModifier;
};

struct CWeaponFallOffModifier
{
	float RangeModifier;
	float DamageModifier;
};

class CWeaponComponentInfo : public CItemInfo
{
public:
	// #TODO2802: hack to sanitize weapon components by identifier.
	bool IsOfType(WeaponComponentType type) const
	{
		using GetClassId = WeaponComponentType (*)(const CWeaponComponentInfo*);

		size_t offset = xbr::IsGameBuildOrGreater<2802>() ? 0x38 : 0x10;
		const char* vtable = *(const char**)this;

		return (*(GetClassId*)(vtable + offset))(this) == type;
	}
};

static int32_t* g_weaponComponentCount;
static CWeaponComponentInfo** g_weaponComponentList;

// CWeaponComponentInfo
static int AccuracyModifierPtrOffset = 0x28;
static int DamageModifierPtrOffset = 0x30;
static int RangeModifierPtrOffset = 0x38;

// CWeaponComponentClipInfo
static int ClipSizeOffset = 0x48;

// CWeaponComponentScopeInfo
static int CameraHashOffset = 0x48;
static int ReticuleHashOffset = 0x54;

static CWeaponComponentInfo* GetWeaponComponent(fx::ScriptContext& context)
{
	if (context.GetArgumentCount() < 1)
	{
		return nullptr;
	}

	uint32_t componentHash = context.GetArgument<uint32_t>(0);

	int32_t low = 0;
	int32_t high = *g_weaponComponentCount - 1;
	while (low <= high)
	{
		int32_t mid = low + (high - low) / 2;
		CWeaponComponentInfo* component = g_weaponComponentList[mid];
		uint32_t hash = component->GetName();

		if (hash == componentHash)
		{
			return component;
		}
		else if (hash > componentHash)
		{
			high = mid - 1;
		}
		else
		{
			low = mid + 1;
		}
	}

	return nullptr;
}

// From VehicleExtraNatives.cpp
template<typename T>
static inline T readValue(CWeaponComponentInfo* ptr, int offset)
{
	return *(T*)((const char*)ptr + offset);
}

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern<char>("44 8B D9 85 C9 74 6C");
		g_weaponComponentCount = hook::get_address<int32_t*>(location + 0x9);
		g_weaponComponentList = hook::get_address<CWeaponComponentInfo**>(location + 0x20);
	}

	fx::ScriptEngine::RegisterNativeHandler("GET_WEAPON_COMPONENT_CLIP_SIZE", [](fx::ScriptContext& context)
	{
		uint16_t clipSize = 0;

		if (auto component = GetWeaponComponent(context); component && component->IsOfType(WeaponComponentType::Clip))
		{
			clipSize = readValue<uint16_t>(component, ClipSizeOffset);
		}

		context.SetResult(clipSize);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WEAPON_COMPONENT_CAMERA_HASH", [](fx::ScriptContext& context)
	{
		uint32_t cameraHash = 0;

		if (auto component = GetWeaponComponent(context); component && component->IsOfType(WeaponComponentType::Scope))
		{
			cameraHash = readValue<uint32_t>(component, CameraHashOffset);
		}

		context.SetResult(cameraHash);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WEAPON_COMPONENT_RETICULE_HASH", [](fx::ScriptContext& context)
	{
		uint32_t reticuleHash = 0;

		if (auto component = GetWeaponComponent(context); component && component->IsOfType(WeaponComponentType::Scope))
		{
			reticuleHash = readValue<uint32_t>(component, ReticuleHashOffset);
		}

		context.SetResult(reticuleHash);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WEAPON_COMPONENT_ACCURACY_MODIFIER", [](fx::ScriptContext& context)
	{
		float result = 0.0f;

		if (auto component = GetWeaponComponent(context))
		{
			if (auto modifier = readValue<CWeaponAccuracyModifier*>(component, AccuracyModifierPtrOffset))
			{
				result = modifier->AccuracyModifier;
			}
		}

		context.SetResult(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WEAPON_COMPONENT_DAMAGE_MODIFIER", [](fx::ScriptContext& context)
	{
		float result = 0.0f;

		if (auto component = GetWeaponComponent(context))
		{
			if (auto modifier = readValue<CWeaponDamageModifier*>(component, DamageModifierPtrOffset))
			{
				result = modifier->DamageModifier;
			}
		}

		context.SetResult(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WEAPON_COMPONENT_RANGE_MODIFIER", [](fx::ScriptContext& context)
	{
		float result = 0.0f;

		if (auto component = GetWeaponComponent(context))
		{
			if (auto modifier = readValue<CWeaponFallOffModifier*>(component, RangeModifierPtrOffset))
			{
				result = modifier->RangeModifier;
			}
		}

		context.SetResult(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WEAPON_COMPONENT_RANGE_DAMAGE_MODIFIER", [](fx::ScriptContext& context)
	{
		float result = 0.0f;

		if (auto component = GetWeaponComponent(context))
		{
			if (auto modifier = readValue<CWeaponFallOffModifier*>(component, RangeModifierPtrOffset))
			{
				result = modifier->DamageModifier;
			}
		}

		context.SetResult(result);
	});
});
