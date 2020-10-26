#include <StdInc.h>
#include <Hooking.h>

#include <ScriptEngine.h>

#include <Resource.h>
#include <fxScripting.h>

#include <nutsnbolts.h>

#include <atArray.h>

#include <concurrent_queue.h>

static concurrency::concurrent_queue<std::function<void()>> g_mainThreadQueue;

struct VisualSettingsEntry
{
	uint32_t entryHash;
	float value;

	inline bool operator<(const VisualSettingsEntry& right)
	{
		return (entryHash < right.entryHash);
	}
};

struct VisualSettingsData
{
	bool initialized;
	atArray<VisualSettingsEntry> entries;
};

static VisualSettingsData* g_visualSettings;

static hook::cdecl_stub<void(void*)> _loadVisualSettings([]()
{
#ifdef GTA_FIVE
	return hook::get_call(hook::get_pattern("48 83 25 ? ? ? ? 00 48 8D ? ? ? ? ? E8 ? ? ? ? E8", 15));
#elif IS_RDR3
	return hook::get_call(hook::get_pattern("48 89 2D ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 48 8D 0D", 7));
#endif
});

static std::unordered_map<uint32_t, float> g_visualSettingsOverrides;

static bool(*g_origLoadVisualSettingsDat)(void* vsData, const char* filename);

static bool LoadVisualSettingsDatStub(void* vsData, const char* filename)
{
	bool success = g_origLoadVisualSettingsDat(vsData, filename);

	if (success)
	{
		for (auto& entry : g_visualSettingsOverrides)
		{
			bool found = false;

			// TODO: binary search?
			for (auto& pair : g_visualSettings->entries)
			{
				if (pair.entryHash == entry.first)
				{
					pair.value = entry.second;

					found = true;
					break;
				}
			}

			if (!found)
			{
				VisualSettingsEntry val;
				val.entryHash = entry.first;
				val.value = entry.second;

				g_visualSettings->entries.Set(g_visualSettings->entries.GetCount(), val);
			}
		}

		std::sort(g_visualSettings->entries.begin(), g_visualSettings->entries.end());
	}

	return success;
}

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("SET_VISUAL_SETTING_FLOAT", [](fx::ScriptContext& context)
	{
		const char* name = context.CheckArgument<const char*>(0);
		float value = context.GetArgument<float>(1);

		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			auto keyHash = HashString(name);
			g_visualSettingsOverrides[keyHash] = value;

			_loadVisualSettings(g_visualSettings);

			resource->OnStop.Connect([keyHash]()
			{
				g_mainThreadQueue.push([keyHash]()
				{
					g_visualSettingsOverrides.erase(keyHash);

					_loadVisualSettings(g_visualSettings);
				});
			});
		}
	});

	OnMainGameFrame.Connect([=]()
	{
		std::function<void()> func;

		while (g_mainThreadQueue.try_pop(func))
		{
			func();
		}
	});
});

static HookFunction hookFunction([]()
{
#ifdef GTA_FIVE
	g_visualSettings = hook::get_address<VisualSettingsData*>(hook::get_pattern("48 83 25 ? ? ? ? 00 48 8D ? ? ? ? ? E8 ? ? ? ? E8", 11));
#elif IS_RDR3
	g_visualSettings = hook::get_address<VisualSettingsData*>(hook::get_pattern("48 89 2D ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 48 8D 0D", -11));
#endif

	{
#ifdef GTA_FIVE
		auto location = (char*)hook::get_call(hook::get_pattern("48 83 25 ? ? ? ? 00 48 8D ? ? ? ? ? E8 ? ? ? ? E8", 15));
#elif IS_RDR3
		auto location = (char*)hook::get_call(hook::get_pattern("48 89 2D ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 48 8D 0D", 7));
#endif

		hook::set_call(&g_origLoadVisualSettingsDat, location + 0x12);
		hook::call(location + 0x12, LoadVisualSettingsDatStub);
	}
});
