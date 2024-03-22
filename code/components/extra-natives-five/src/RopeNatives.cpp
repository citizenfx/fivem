#include "StdInc.h"

#include "ropeManager.h"

#include <Hooking.h>
#include <ScriptEngine.h>
#include <ScriptSerialization.h>
#include <ICoreGameInit.h>
#include <ResourceManager.h>
#include <VFSManager.h>

namespace rage
{
static ropeDataManager* g_ropeDataManager;

ropeDataManager* ropeDataManager::GetInstance()
{
	return g_ropeDataManager;
}

static hook::cdecl_stub<void()> ropeDataManager_Init([]()
{
	return hook::get_pattern("48 83 EC 28 33 D2 8D 4A 07");
});

void ropeDataManager::Init()
{
	ropeDataManager_Init();
}

static hook::cdecl_stub<void()> ropeDataManager_Shutdown([]()
{
	return hook::get_call(hook::get_pattern("48 8D 4E 38 E8 ? ? ? ? 48 8B D8", 37));
});

void ropeDataManager::Shutdown()
{
	ropeDataManager_Shutdown();
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

static hook::thiscall_stub<void(ropeManager* manager)> ropeManager_RemoveAllRopes([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 88 1D ? ? ? ? B0 01"));
});

void ropeManager::RemoveAllRopes()
{
	ropeManager_RemoveAllRopes(this);
}
}

const size_t kRopeDataPathSize = 256;
const char* g_defaultRopeDataPath;
char* g_modifiedRopeDataPath;

static void ChangeAndRestartRopeDataManager(const char* path)
{
	rage::ropeManager* ropeManager = rage::ropeManager::GetInstance();
	if (ropeManager)
	{
		// Invalidates and removes any ropes currently in use
		ropeManager->RemoveAllRopes();
	}
	rage::ropeDataManager::Shutdown();
	strcpy_s(g_modifiedRopeDataPath, kRopeDataPathSize, path);
	rage::ropeDataManager::Init();
}

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

	auto location = hook::get_pattern("4C 8D 05 ? ? ? ? 48 8D 15 ? ? ? ? 48 89 44 24 ? 48 8D 05", 10);
	g_defaultRopeDataPath = hook::get_address<char*>(location);

	g_modifiedRopeDataPath = (char*)hook::AllocateStubMemory(kRopeDataPathSize);
	hook::put<uint32_t>(location, (uintptr_t)g_modifiedRopeDataPath - (uintptr_t)location - 4);
	strcpy_s(g_modifiedRopeDataPath, kRopeDataPathSize, g_defaultRopeDataPath);

	fx::ScriptEngine::RegisterNativeHandler("LOAD_ROPE_DATA_FROM_PATH", [](fx::ScriptContext& context)
	{
		fx::ResourceManager* resourceManager = fx::ResourceManager::GetCurrent();
		const char* resourceName = context.CheckArgument<const char*>(0);
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(resourceName);

		if (!resource.GetRef())
		{
			trace("LOAD_ROPE_DATA_FROM_PATH: resource name %s does not exist\n", resourceName);
			context.SetResult(false);
			return;
		}

		std::string filePath = resource->GetPath();

		if (filePath.back() != '/' && filePath.back() != '\\')
		{
			filePath += '/';
		}

		filePath += context.CheckArgument<const char*>(1);

		fwRefContainer<vfs::Stream> stream = vfs::OpenRead(filePath);
		if (!stream.GetRef())
		{
			trace("LOAD_ROPE_DATA_FROM_PATH: unable to find rope data file at %s\n", filePath.c_str());
			context.SetResult(false);
			return;
		}

		ChangeAndRestartRopeDataManager(filePath.c_str());

		context.SetResult(true);
	});

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		if (strcmp(g_modifiedRopeDataPath, g_defaultRopeDataPath) != 0)
		{
			ChangeAndRestartRopeDataManager(g_defaultRopeDataPath);
		}
	});
});
