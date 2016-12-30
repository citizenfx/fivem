/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <ScriptEngine.h>

#include <Resource.h>
#include <fxScripting.h>

#include <CustomText.h>

static bool AddTextEntryForResource(fx::Resource* resource, uint32_t hashKey, const char* textValue)
{
	static std::unordered_map<uint32_t, std::string> g_owners;

	std::string resourceName = resource->GetName();

	auto it = g_owners.find(hashKey);

	if (it == g_owners.end())
	{
		g_owners.insert({ hashKey, resource->GetName() });
		game::AddCustomText(hashKey, textValue);

		// on-stop handler, as well, to remove from the list when the resource stops
		resource->OnStop.Connect([=] ()
		{
			if (g_owners[hashKey] == resource->GetName())
			{
				game::RemoveCustomText(hashKey);

				g_owners.erase(hashKey);
			}
		});

		return true;
	}

	if (it->second == resource->GetName())
	{
		game::AddCustomText(hashKey, textValue);
	}
}

static InitFunction initFunction([] ()
{
	fx::ScriptEngine::RegisterNativeHandler("ADD_TEXT_ENTRY", [] (fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			const char* textKey = context.GetArgument<const char*>(0);
			const char* textValue = context.GetArgument<const char*>(1);

			uint32_t hashKey = HashString(textKey);

			if (_strnicmp(textKey, "0x", 2) == 0)
			{
				hashKey = strtoul(&textKey[2], nullptr, 16);
			}

			context.SetResult<bool>(AddTextEntryForResource(resource, hashKey, textValue));
			return;
		}

		context.SetResult<bool>(false);
	});

	fx::ScriptEngine::RegisterNativeHandler("ADD_TEXT_ENTRY_BY_HASH", [](fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			uint32_t textKey = context.GetArgument<uint32_t>(0);
			const char* textValue = context.GetArgument<const char*>(1);

			context.SetResult<bool>(AddTextEntryForResource(resource, textKey, textValue));
			return;
		}

		context.SetResult<bool>(false);
	});
});