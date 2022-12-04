#include "StdInc.h"

#include "Hooking.h"
#include "ScriptEngine.h"

#include "fxScripting.h"

#include "Resource.h"
#include "ResourceScriptingComponent.h"

static hook::cdecl_stub<void(const char*)> _loadPopGroups([]
{
	return hook::get_call<void*>(hook::get_pattern<char>("74 4D 48 8B C8 E8 ? ? ? ? 33 DB", -0x1E) + 0x23);
});

static hook::cdecl_stub<void()> _reloadPopGroups([]
{
	return hook::get_pattern("74 4D 48 8B C8 E8 ? ? ? ? 33 DB", -0x1E);
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
});
