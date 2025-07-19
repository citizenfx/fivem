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
#include <deque>

#include <sfFontStuff.h>

static bool AddTextEntryForResource(fx::Resource* resource, uint32_t hashKey, const char* textValueRaw)
{
	std::string textValue = textValueRaw;
	static std::unordered_map<uint32_t, std::deque<std::tuple<std::string /* resource */, std::string /* str */>>> g_hashStack;
	const auto& resourceName = resource->GetName();
	auto& selfStack = g_hashStack[hashKey];

	static auto find = [](auto& selfStack, std::string_view resourceName)
	{
		return std::find_if(selfStack.begin(), selfStack.end(), [resourceName](const auto& pair)
		{
			return std::get<0>(pair) == resourceName;
		});
	};

	// find the resource
	auto resourceIt = find(selfStack, resourceName);

	if (resourceIt == selfStack.end())
	{
		resource->OnStop.Connect([resource, hashKey]()
		{
			auto& selfStack = g_hashStack[hashKey];
			bool matched = false;

			do
			{
				matched = false;

				if (auto it = find(selfStack, resource->GetName()); it != selfStack.end())
				{
					selfStack.erase(it);
					matched = true;
				}
			} while (matched);

			if (!selfStack.empty())
			{
				game::AddCustomText(hashKey, std::get<1>(selfStack.front()));
			}
			else
			{
				game::RemoveCustomText(hashKey);
			}
		});

		selfStack.emplace_front(resourceName, textValue);
	}
	else
	{
		*resourceIt = { std::get<0>(*resourceIt), textValue };
		std::iter_swap(selfStack.begin(), resourceIt);
	}

	game::AddCustomText(hashKey, textValue);
	return true;
}

static InitFunction initFunction([] ()
{
	fx::ScriptEngine::RegisterNativeHandler("ADD_TEXT_ENTRY", [] (fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			const char* textKey = context.CheckArgument<const char*>(0);
			const char* textValue = context.CheckArgument<const char*>(1);

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
			const char* textValue = context.CheckArgument<const char*>(1);

			context.SetResult<bool>(AddTextEntryForResource(resource, textKey, textValue));
			return;
		}

		context.SetResult<bool>(false);
	});

	fx::ScriptEngine::RegisterNativeHandler("REGISTER_FONT_ID", [](fx::ScriptContext& context)
	{
		context.SetResult(sf::RegisterFontIndex(context.CheckArgument<const char*>(0)));
	});

	fx::ScriptEngine::RegisterNativeHandler("REGISTER_FONT_FILE", [](fx::ScriptContext& context)
	{
		sf::RegisterFontLib(context.CheckArgument<const char*>(0));
	});
	
#if defined(GTA_FIVE)

	fx::ScriptEngine::RegisterNativeHandler("ADD_MINIMAP_OVERLAY", [](fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());
			int overlayIdx = sf::AddMinimapOverlay(resource->GetPath() + "/" + context.CheckArgument<const char*>(0), -1);

			resource->OnStop.Connect([=]()
			{
				sf::RemoveMinimapOverlay(overlayIdx);
			});

			context.SetResult(overlayIdx);
			return;
		}

		context.SetResult(-1);
	});

	fx::ScriptEngine::RegisterNativeHandler("ADD_MINIMAP_OVERLAY_WITH_DEPTH", [](fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_FAILED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			context.SetResult(-1);
			return;
		}

		fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

		auto gfxFileName = context.CheckArgument<const char*>(0);
		auto depth = context.GetArgument<int>(1);

		int overlayIdx = sf::AddMinimapOverlay(resource->GetPath() + "/" + gfxFileName, depth);

		resource->OnStop.Connect([=]()
		{
			sf::RemoveMinimapOverlay(overlayIdx);
		});

		context.SetResult(overlayIdx);
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
#endif
});
