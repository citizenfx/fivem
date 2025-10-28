/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIApp.h"
#include "NUIClient.h"
#include "CefOverlay.h"
#include <CoreConsole.h>
#include <json.hpp>
#include "memdbgon.h"

#include <shellapi.h>

void NuiConsole_SetConvars();

static InitFunction initFunction([] ()
{
	auto nuiApp = Instance<NUIApp>::Get();

	nuiApp->AddV8Handler("invokeNative", [] (const CefV8ValueList& arguments, CefString& exception)
	{
		if (arguments.size() == 2)
		{
			auto msg = CefProcessMessage::Create("invokeNative");
			auto argList = msg->GetArgumentList();

			argList->SetSize(2);
			argList->SetString(0, arguments[0]->GetStringValue());

			CefString str = arguments[1]->GetStringValue();
			argList->SetString(1, str);

			CefV8Context::GetCurrentContext()->GetFrame()->SendProcessMessage(PID_BROWSER, msg);
		}

		return CefV8Value::CreateUndefined();
	});

	NUIClient::OnClientCreated.Connect([] (NUIClient* client)
	{
		trace(__FUNCTION__ ": Hello!");
		client->AddProcessMessageHandler("invokeNative", [] (CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> message)
		{
			auto args = message->GetArgumentList();
			auto nativeType = args->GetString(0);

			trace(__FUNCTION__ ": Calling onInvokeNative!\n");
			nui::OnInvokeNative(ToWide(nativeType.ToString()).c_str(), ToWide(args->GetString(1).ToString()).c_str());
			trace(__FUNCTION__ ": Called onInvokeNative!\n");

			if (nativeType == "quit")
			{
				// TODO: CEF shutdown and native stuff related to it (set a shutdown flag)
				ExitProcess(0);
			}
			else if (nativeType == "openUrl")
			{
				std::string arg = args->GetString(1).ToString();

				if (arg.find("http://") == 0 || arg.find("https://") == 0)
				{
					ShellExecute(nullptr, L"open", ToWide(arg).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
				}
			}
			else if (nativeType == "setConvar" || nativeType == "setArchivedConvar")
			{
				if (nui::HasMainUI())
				{
					auto json = nlohmann::json::parse(args->GetString(1).ToString());

					try
					{
						auto cmd = (nativeType == "setArchivedConvar") ? "seta" : "set";
						auto name = json.value("name", "");
						auto value = json.value("value", "");

						se::ScopedPrincipal ps{ se::Principal{"system.console"} };
						console::GetDefaultContext()->ExecuteSingleCommandDirect(ProgramArguments{ cmd, name, value });
					}
					catch (std::exception&)
					{

					}
				}
			}
			else if (nativeType == "getConvars")
			{
				if (nui::HasMainUI())
				{
					NuiConsole_SetConvars();
				}
			}

			return true;
		});
	});
}, 1);
