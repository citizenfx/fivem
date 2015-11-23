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

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <sstream>

static InitFunction initFunction([] ()
{
	fx::ScriptEngine::RegisterNativeHandler("SEND_NUI_MESSAGE", [] (fx::ScriptContext& context)
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
					if (resourceUI->HasFrame())
					{
						// get the message as JSON and validate it by parsing/recreating (so we won't end up injecting malicious JS into the browser root)
						const char* messageJson = context.GetArgument<const char*>(0);

						rapidjson::Document document;
						document.Parse(messageJson);

						if (!document.HasParseError())
						{
							rapidjson::StringBuffer sb;
							rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

							if (document.Accept(writer))
							{
								// somewhat insane sanity check at the last minute
								if (resource->GetName().find('"') == std::string::npos)
								{
									// send to NUI as a formatted root script (std::string constructor is used to prevent having to count bytes)
									std::stringstream stream;
									stream << "citFrames[\"" << resource->GetName() << "\"].contentWindow.postMessage(" << std::string(sb.GetString(), sb.GetSize()) << ", '*');";

									// execute in NUI
									nui::ExecuteRootScript(stream.str());

									// and return 'true' to indicate to the script that we succeeded
									context.SetResult(true);
									return;
								}
							}
							else
							{
								trace("SEND_NUI_MESSAGE: writing to JSON writer failed\n");
							}
						}
						else
						{
							trace("SEND_NUI_MESSAGE: invalid JSON passed in resource %s (rapidjson error code %d)\n", resource->GetName().c_str(), document.GetParseError());
						}
					}
					else
					{
						trace("SEND_NUI_MESSAGE: resource %s has no UI frame\n", resource->GetName().c_str());
					}
				}
				else
				{
					trace("SEND_NUI_MESSAGE: resource %s has no UI\n", resource->GetName().c_str());
				}
			}
			else
			{
				trace("SEND_NUI_MESSAGE: no current resource\n");
			}
		}
		else
		{
			trace("SEND_NUI_MESSAGE called from outside a scripting runtime\n");
		}

		context.SetResult(false);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_NUI_FOCUS", [] (fx::ScriptContext& context)
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
					if (resourceUI->HasFrame())
					{
						if (resource->GetName().find('"') == std::string::npos)
						{
							bool hasFocus = context.GetArgument<bool>(0);
							const char* functionName = (hasFocus) ? "focusFrame" : "blurFrame";

							nui::ExecuteRootScript(va("%s(\"%s\");", functionName, resource->GetName().c_str()));
							nui::GiveFocus(hasFocus);
						}
					}
				}
			}
		}
	});
});