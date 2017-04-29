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
#include "memdbgon.h"

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

			CefV8Context::GetCurrentContext()->GetBrowser()->SendProcessMessage(PID_BROWSER, msg);
		}

		return CefV8Value::CreateUndefined();
	});

	NUIClient::OnClientCreated.Connect([] (NUIClient* client)
	{
		client->AddProcessMessageHandler("invokeNative", [] (CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> message)
		{
			auto args = message->GetArgumentList();
			auto nativeType = args->GetString(0);

			nui::OnInvokeNative(nativeType.c_str(), ToWide(args->GetString(1).ToString()).c_str());

			if (nativeType == "quit")
			{
				// TODO: CEF shutdown and native stuff related to it (set a shutdown flag)
				ExitProcess(0);
			}

			return true;
		});
	});
}, 1);