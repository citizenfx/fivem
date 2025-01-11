#include "StdInc.h"

#include "ropeManager.h"

#include <Hooking.h>
#include <ScriptEngine.h>
#include <ScriptSerialization.h>
#include <ScriptWarnings.h>
#include <Pool.h>
#include <ICoreGameInit.h>

namespace rage
{
static hook::thiscall_stub<void(ropeData* self)> ropeData_ropeData([]()
{
	return hook::get_pattern("48 89 5C 24 ? 48 89 7C 24 ? 41 56 48 83 EC 20 8B 3D ? ? ? ? C7 41");
});

ropeData::ropeData()
{
	ropeData_ropeData(this);
}

void* ropeData::operator new(size_t size)
{
	return PoolAllocate(GetPool<ropeData>("ropeData"));
}

void ropeData::operator delete(void* pointer)
{
	PoolRelease(GetPool<ropeData>("ropeData"), pointer);
}

static hook::thiscall_stub<void(ropeDataManager* self)> ropeDataManager_UnloadRopeTextures([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8B CF E8 ? ? ? ? 48 8B 9F ? ? ? ? 48 8B 8B"));
});

void ropeDataManager::UnloadRopeTextures()
{
	ropeDataManager_UnloadRopeTextures(this);
}

static hook::thiscall_stub<void(ropeDataManager* self)> ropeDataManager_Load([]()
{
	return hook::get_pattern("48 83 EC 48 E8 ? ? ? ? 4C 8B 0D");
});

void ropeDataManager::Load()
{
	ropeDataManager_Load(this);
}

static ropeDataManager* g_ropeDataManager;

ropeDataManager* ropeDataManager::GetInstance()
{
	return g_ropeDataManager;
}

static hook::thiscall_stub<ropeInstance*(ropeManager* manager, int handle)> ropeManager_findRope([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 4C 8B E8 48 89 45 67"));
});

ropeInstance* ropeManager::FindRope(int handle)
{
	return ropeManager_findRope(this, handle);
}

static ropeManager** g_ropeManager;

ropeManager* ropeManager::GetInstance()
{
	return (g_ropeManager) ? *g_ropeManager : nullptr;
}
}

static int* ropeDataManager__txdStatus;
static bool g_hasCustomRopeTypes = false;

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern("48 8B 05 ? ? ? ? 48 8B 0C F8 48 89 71 18", 0x3);
		auto address = hook::get_address<uintptr_t>(location) - 0x8;
		rage::g_ropeDataManager = reinterpret_cast<rage::ropeDataManager*>(address);
	}

	{
		auto location = hook::get_pattern("8B 91 ? ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? 33 DB", 0x9);
		rage::g_ropeManager = hook::get_address<rage::ropeManager**>(location);
	}

	fx::ScriptEngine::RegisterNativeHandler("GET_ALL_ROPES", [](fx::ScriptContext& context)
	{
		std::vector<uint32_t> handles;

		if (auto manager = rage::ropeManager::GetInstance())
		{
			if (manager->numAllocated > 0)
			{
				auto rope = manager->allocated.head;
				while (rope)
				{
					handles.push_back(rope->handle);
					rope = rope->nextRope;
				}
			}
		}

		context.SetResult(fx::SerializeObject(handles));
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_ROPE_LENGTH_CHANGE_RATE", [](fx::ScriptContext& context)
	{
		int handle = context.GetArgument<int>(0);
		float lengthChangeRate = 0.0f;

		if (auto manager = rage::ropeManager::GetInstance())
		{
			if (auto rope = manager->FindRope(handle))
			{
				lengthChangeRate = rope->lengthChangeRate;
			}
		}

		context.SetResult(lengthChangeRate);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_ROPE_LENGTH_CHANGE_RATE", [](fx::ScriptContext& context)
	{
		int handle = context.GetArgument<int>(0);
		float lengthChangeRate = context.GetArgument<float>(1);

		if (auto manager = rage::ropeManager::GetInstance())
		{
			if (auto rope = manager->FindRope(handle))
			{
				rope->lengthChangeRate = lengthChangeRate;
			}
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_ROPE_TIME_MULTIPLIER", [](fx::ScriptContext& context)
	{
		int handle = context.GetArgument<int>(0);
		float multiplier = 0.0f;

		if (auto manager = rage::ropeManager::GetInstance())
		{
			if (auto rope = manager->FindRope(handle))
			{
				multiplier = rope->timeMultiplier;
			}
		}

		context.SetResult(multiplier);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_ROPE_FLAGS", [](fx::ScriptContext& context)
	{
		int handle = context.GetArgument<int>(0);
		rage::eRopeFlags flags{};

		if (auto manager = rage::ropeManager::GetInstance())
		{
			if (auto rope = manager->FindRope(handle))
			{
				flags = rope->flags;
			}
		}

		context.SetResult(flags);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_ROPE_UPDATE_ORDER", [](fx::ScriptContext& context)
	{
		int handle = context.GetArgument<int>(0);
		uint32_t updateOrder = 0;

		if (auto manager = rage::ropeManager::GetInstance())
		{
			if (auto rope = manager->FindRope(handle))
			{
				updateOrder = rope->updateOrder;
			}
		}

		context.SetResult(updateOrder);
	});

	{
		auto location = hook::get_pattern<char>("48 83 EC 28 33 D2 8D 4A 07");
		ropeDataManager__txdStatus = (int*)(hook::get_address<char*>(location + 16) + 1);
	}

	fx::ScriptEngine::RegisterNativeHandler("REGISTER_ROPE_DATA", [](fx::ScriptContext& context)
	{
		int numSections = context.GetArgument<int>(0);
		float radius = context.GetArgument<float>(1);
		const char* diffuseTextureName = context.CheckArgument<const char*>(2);
		const char* normalMapName = context.CheckArgument<const char*>(3);
		float distanceMappingScale = context.GetArgument<float>(4);
		float uvScaleX = context.GetArgument<float>(5);
		float uvScaleY = context.GetArgument<float>(6);
		float specularFresnel = context.GetArgument<float>(7);
		float specularFalloff = context.GetArgument<float>(8);
		float specularIntensity = context.GetArgument<float>(9);
		float bumpiness = context.GetArgument<float>(10);
		int color = context.GetArgument<int>(11);

		context.SetResult(-1);

		if (numSections <= 0)
		{
			fx::scripting::Warningf("natives", "Invalid numSections was passed to REGISTER_ROPE_DATA (%d), should greater than 0\n", numSections);
			return;
		}

		if (radius <= 0.0)
		{
			fx::scripting::Warningf("natives", "Invalid radius was passed to REGISTER_ROPE_DATA (%f), should greater than 0.0\n", radius);
			return;
		}

		rage::ropeDataManager* manager = rage::ropeDataManager::GetInstance();
		if (!manager)
		{
			return;
		}

		atPool<rage::ropeData>* pool = rage::GetPool<rage::ropeData>("ropeData");
		if (pool->GetCount() == pool->GetSize())
		{
			fx::scripting::Warningf("natives", "Unable to allocate rope data in REGISTER_ROPE_DATA, pool is full\n");
			return;
		}

		rage::ropeData* ropeData = new rage::ropeData();

		ropeData->numSections = numSections;
		ropeData->radius = radius;
		ropeData->diffuseTextureNameHash = HashString(diffuseTextureName);
		ropeData->normalMapNameHash = HashString(normalMapName);
		ropeData->distanceMappingScale = distanceMappingScale;
		ropeData->UVScaleX = uvScaleX;
		ropeData->UVScaleY = uvScaleY;
		ropeData->specularFresnel = specularFresnel;
		ropeData->specularFalloff = specularFalloff;
		ropeData->specularIntensity = specularIntensity;
		ropeData->bumpiness = bumpiness;
		ropeData->color = color;

		int ropeType = manager->typeData.GetCount();
		manager->typeData.Set(ropeType, ropeData);

		// If rope textures are loaded then put them back into the loading state
		if (*ropeDataManager__txdStatus == 2)
		{
			manager->UnloadRopeTextures();
			*ropeDataManager__txdStatus = 1;
		}

		g_hasCustomRopeTypes = true;

		context.SetResult(ropeType);
	});

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		if (g_hasCustomRopeTypes)
		{
			rage::ropeDataManager* manager = rage::ropeDataManager::GetInstance();
			if (manager)
			{
				manager->Load();
			}
			g_hasCustomRopeTypes = false;
		}
	});
});
