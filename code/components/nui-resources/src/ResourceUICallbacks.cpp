/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <ResourceUI.h>

#include <fxScripting.h>
#include <ScriptEngine.h>

#include "ResourceCallbackComponent.h"
#include <ResourceEventComponent.h>
#include <ResourceScriptingComponent.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <MsgpackJson.h>

static ResUICallback MakeUICallback(fx::Resource* resource, const std::string& type, const std::string& ref = {})
{
	return [resource, type, ref](const std::string& path, const std::string&, const std::multimap<std::string, std::string>& headers, const std::string& postData, const std::optional<std::string>& originResource, ResUIResultCallback cb)
	{
		// get the event component
		fwRefContainer<ResourceUI> ui = resource->GetComponent<ResourceUI>();

		if (!ui->HasCallbacks())
		{
			ui->SetHasCallbacks(true);
		}

		// generate a reference for the result
		auto cbComponent = resource->GetManager()->GetComponent<fx::ResourceCallbackComponent>();
		auto reference = cbComponent->CreateCallback([cb = std::move(cb)](const msgpack::unpacked& unpacked)
		{
			// convert the argument array from msgpack to JSON
			std::vector<msgpack::object> arguments;

			// convert
			unpacked.get().convert(arguments);

			// assume there's one argument
			if (arguments.size() == 1)
			{
				// get the object
				const msgpack::object& object = arguments[0];

				// and convert to a rapidjson object
				rapidjson::Document document;
				ConvertToJSON(object, document, document.GetAllocator());

				// write as a json string
				rapidjson::StringBuffer sb;
				rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

				if (document.Accept(writer))
				{
					if (sb.GetString() && sb.GetSize())
					{
						cb(200, { { "Content-Type", "application/json; charset=utf-8" } }, std::string{ sb.GetString(), sb.GetSize() });
					}
					else
					{
						cb(200, { { "Content-Type", "application/json; charset=utf-8" } }, "null");
					}
				}
			}
		});

		// serialize the post object JSON as a msgpack object
		rapidjson::Document postJSON;

		if (postData.empty())
		{
			postJSON.SetNull();
		}
		else
		{
			postJSON.Parse(postData.c_str());
		}

		if (!postJSON.HasParseError())
		{
			msgpack::object postObject;
			msgpack::zone zone;
			ConvertToMsgPack(postJSON, postObject, zone);

			// 'legacy' __cfx_nui: event
			if (ref.empty())
			{
				fwRefContainer<fx::ResourceEventComponent> eventComponent = resource->GetComponent<fx::ResourceEventComponent>();
				eventComponent->QueueEvent2("__cfx_nui:" + type, "nui", postObject, reference);
			}
			else
			{
				resource->GetManager()->CallReference<void>(ref, postObject, reference);
			}
		}
	};
}

template<bool IsRef>
static void RegisterNuiCallback(fx::ScriptContext& context)
{
	fx::OMPtr<IScriptRuntime> runtime;

	if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
	{
		fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

		if (resource)
		{
			fwRefContainer<ResourceUI> resourceUI = resource->GetComponent<ResourceUI>();

			if (resourceUI.GetRef())
			{
				std::string type = context.CheckArgument<const char*>(0);
				std::string ref;

				if constexpr (IsRef)
				{
					ref = context.CheckArgument<const char*>(1);
				}

				resourceUI->AddCallback(type, MakeUICallback(resource, type, ref));
			}
		}
	}
}

static InitFunction initFunction([] ()
{
	fx::ScriptEngine::RegisterNativeHandler("REGISTER_NUI_CALLBACK", [](fx::ScriptContext& context)
	{
		RegisterNuiCallback<true>(context);
	});

	fx::ScriptEngine::RegisterNativeHandler("REGISTER_NUI_CALLBACK_TYPE", [] (fx::ScriptContext& context)
	{
		RegisterNuiCallback<false>(context);
	});
});
