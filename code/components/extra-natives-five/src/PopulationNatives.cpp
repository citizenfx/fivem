#include "StdInc.h"

#include "Hooking.h"
#include "ScriptEngine.h"

#include "fxScripting.h"

#include "Resource.h"
#include "ResourceScriptingComponent.h"
#include <Hooking.Stubs.h>

namespace rage
{
	using Vec3V = DirectX::XMVECTOR;
}

static hook::cdecl_stub<void(const char*)> _loadPopGroups([]
{
	return hook::get_call<void*>(hook::get_pattern<char>("74 4D 48 8B C8 E8 ? ? ? ? 33 DB", -0x1E) + 0x23);
});

static hook::cdecl_stub<void()> _reloadPopGroups([]
{
	return hook::get_pattern("74 4D 48 8B C8 E8 ? ? ? ? 33 DB", -0x1E);
});

static hook::cdecl_stub<void(rage::Vec3V*, bool, float, float, float, float, float, bool, int, int)> _GeneratePedsAtScenarioPoints([]
{
	return hook::get_pattern("48 8B C4 48 89 58 ? 48 89 70 ? 57 48 83 EC ? 8B 9C 24");
});


static fx::OMPtr<IScriptHost> GetCurrentScriptHost()
{
	fx::OMPtr<IScriptRuntime> runtime;

	if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
	{
		auto resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());
		auto scriptingComponent = resource->GetComponent<fx::ResourceScriptingComponent>();
		return scriptingComponent->GetScriptHost();
	}

	return {};
}

static void LoadPopGroups(std::string_view filename)
{
	std::vector<uint8_t> data;
	std::string finalFileName{ filename };
	auto scriptHost = GetCurrentScriptHost();

	if (scriptHost.GetRef())
	{
		fx::OMPtr<fxIStream> stream;
		if (FX_FAILED(scriptHost->OpenHostFile(const_cast<char*>(finalFileName.c_str()), stream.GetAddressOf())))
		{
			return;
		}

		uint64_t length;
		if (FX_FAILED(stream->GetLength(&length)))
		{
			return;
		}

		data.resize(length);

		uint32_t bytesRead = 0;
		if (FX_FAILED(stream->Read(data.data(), uint32_t(length), &bytesRead)) || bytesRead != length)
		{
			return;
		}

		finalFileName = fmt::sprintf("memory:$%016llx,%d,0:%s", (uintptr_t)data.data(), length, "popgroups");
	}

	_loadPopGroups(finalFileName.c_str());
}

static void UnloadPopGroups()
{
	_reloadPopGroups();
}

static InitFunction initFunction([]
{
	fx::ScriptEngine::RegisterNativeHandler("OVERRIDE_POP_GROUPS", [](fx::ScriptContext& context)
	{
		if (auto fileName = context.GetArgument<const char*>(0))
		{
			LoadPopGroups(fileName);
		}
		else
		{
			UnloadPopGroups();
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GENERATE_PEDS_AT_SCENARIO_POINTS", [](fx::ScriptContext& context)
	{
		float x = context.GetArgument<float>(0);
		float y = context.GetArgument<float>(1);
		float z = context.GetArgument<float>(2);
		auto allowDeepInteriors = context.GetArgument<bool>(3);
		auto rangeOutOfViewMin = context.GetArgument<float>(4);
		auto rangeOutOfViewMax = context.GetArgument<float>(5);
		auto rangeInViewMin = context.GetArgument<float>(6);
		auto rangeInViewMax = context.GetArgument<float>(7);
		auto rangeFrustumExtra = context.GetArgument<float>(8);
		auto doInFrustumTest = context.GetArgument<bool>(9);
		auto maxPeds = context.GetArgument<int>(10);
		auto maxInteriorPeds = context.GetArgument<int>(11);

		rage::Vec3V popControlCentre = DirectX::XMVectorSet(x, y, z, 0.0f);
		_GeneratePedsAtScenarioPoints(&popControlCentre, allowDeepInteriors, rangeOutOfViewMin, rangeOutOfViewMax, rangeInViewMin, rangeInViewMax,
									 rangeFrustumExtra, doInFrustumTest, maxPeds, maxInteriorPeds);
	});
});
