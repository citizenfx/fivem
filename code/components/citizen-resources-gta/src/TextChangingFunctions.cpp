/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>

#if defined(GTA_FIVE)
#include <ScriptEngine.h>

#include <Resource.h>
#include <fxScripting.h>

#include <CustomText.h>
#include <sfFontStuff.h>

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

	fx::ScriptEngine::RegisterNativeHandler("REGISTER_FONT_ID", [](fx::ScriptContext& context)
	{
		context.SetResult(sf::RegisterFontIndex(context.GetArgument<const char*>(0)));
	});

	fx::ScriptEngine::RegisterNativeHandler("REGISTER_FONT_FILE", [](fx::ScriptContext& context)
	{
		sf::RegisterFontLib(context.GetArgument<const char*>(0));
	});

	fx::ScriptEngine::RegisterNativeHandler("ADD_MINIMAP_OVERLAY", [](fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());
			int overlayIdx = sf::AddMinimapOverlay(resource->GetPath() + "/" + context.GetArgument<const char*>(0));

			resource->OnStop.Connect([=]()
			{
				sf::RemoveMinimapOverlay(overlayIdx);
			});

			context.SetResult(overlayIdx);
			return;
		}

		context.SetResult(-1);
	});

	fx::ScriptEngine::RegisterNativeHandler("HAS_MINIMAP_OVERLAY_LOADED", [](fx::ScriptContext& context)
	{
		context.SetResult(sf::HasMinimapLoaded(context.GetArgument<int>(0)));
	});

	fx::ScriptEngine::RegisterNativeHandler("CALL_MINIMAP_SCALEFORM_FUNCTION", [](fx::ScriptContext& context)
	{
		context.SetResult(sf::CallMinimapOverlay(context.GetArgument<int>(0), context.GetArgument<const char*>(1)));
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_MINIMAP_OVERLAY_DISPLAY", [](fx::ScriptContext& context)
	{
		sf::SetMinimapOverlayDisplay(context.GetArgument<int>(0),
			context.GetArgument<float>(1), context.GetArgument<float>(2),
			context.GetArgument<float>(3), context.GetArgument<float>(4),
			context.GetArgument<float>(5));
	});
});
#endif
