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
#include <ReverseGameData.h>
#include <WorldEditorControls.h>
#include <HostSharedData.h>

static float clamp01(float num)
{
	if (num < 0.0f)
	{
		return 0.0f;
	}

	if (num > 1.0f)
	{
		return 1.0f;
	}

	return num;
}

static InitFunction initFunction([]()
{
	auto nuiApp = Instance<NUIApp>::Get();

	static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");

	nuiApp->AddV8Handler("resizeGame", [](const CefV8ValueList& arguments, CefString& exception)
	{
		if (arguments.size() == 2)
		{
			auto msg = CefProcessMessage::Create("resizeGame");
			auto argList = msg->GetArgumentList();

			auto widthArg = arguments[0];
			auto heightArg = arguments[1];

			int width = widthArg->IsInt() ? widthArg->GetIntValue() : widthArg->GetDoubleValue();
			int height = heightArg->IsInt() ? heightArg->GetIntValue() : heightArg->GetDoubleValue();

			if (width == 0 || height == 0)
			{
				return CefV8Value::CreateBool(false);
			}

			argList->SetSize(2);
			argList->SetInt(0, width);
			argList->SetInt(1, height);

			CefV8Context::GetCurrentContext()->GetFrame()->SendProcessMessage(PID_BROWSER, msg);
		}

		return CefV8Value::CreateUndefined();
	});

	nuiApp->AddV8Handler("setWorldEditorControls", [](const CefV8ValueList& arguments, CefString& exception)
	{
		if (arguments.size() == 3)
		{
			static HostSharedData<WorldEditorControls> wec("CfxWorldEditorControls");

			wec->gizmoSelect = arguments[0]->GetBoolValue();
			wec->gizmoMode = (WorldEditorMode)arguments[1]->GetIntValue();
			wec->gizmoLocal = arguments[2]->GetBoolValue();

			return CefV8Value::CreateBool(true);
		}

		exception.FromString(fmt::sprintf("Expected 5 arguments, got %d", arguments.size()));

		return CefV8Value::CreateUndefined();
	});

	nuiApp->AddV8Handler("setWorldEditorMouse", [](const CefV8ValueList & arguments, CefString & exception)
	{
		if (arguments.size() == 2)
		{
			static HostSharedData<WorldEditorControls> wec("CfxWorldEditorControls");

			wec->mouseX = clamp01(arguments[0]->GetDoubleValue());
			wec->mouseY = clamp01(arguments[1]->GetDoubleValue());

			return CefV8Value::CreateBool(true);
		}

		exception.FromString(fmt::sprintf("Expected 2 arguments, got %d", arguments.size()));

		return CefV8Value::CreateUndefined();
	});

	nuiApp->AddV8Handler("openDevTools", [](const CefV8ValueList& arguments, CefString& exception)
	{
		auto msg = CefProcessMessage::Create("openDevTools");

		CefV8Context::GetCurrentContext()->GetFrame()->SendProcessMessage(PID_BROWSER, msg);

		return CefV8Value::CreateUndefined();
	});

	nuiApp->AddV8Handler("setFPSLimit", [](const CefV8ValueList& arguments, CefString& exception)
	{
		if (arguments.size() > 0)
		{
			rgd->fpsLimit = arguments[0]->IsInt() ? arguments[0]->GetIntValue() : 0;
		}

		return CefV8Value::CreateUndefined();
	});

	nuiApp->AddV8Handler("setInputChar", [](const CefV8ValueList& arguments, CefString& exception)
	{
		if (arguments.size() == 1)
		{
			// "x", "X", "y", "Y"
			if (arguments[0]->IsString())
			{
				auto charString = arguments[0]->GetStringValue();
				rgd->inputChar = charString.c_str()[0];
			} 
			// big keys in JS are processed as "BackSpace", "Shift", "Alt", ... The charcode is sent instead in this case.
			else
			{
				auto charCode = arguments[0]->GetIntValue();
				rgd->inputChar = (wchar_t)charCode;
			}
		}

		return CefV8Value::CreateUndefined();
	});

	nuiApp->AddV8Handler("fxdkSendApiMessage", [](const CefV8ValueList& arguments, CefString& exception)
	{
		if (arguments.size() == 1)
		{
			auto msg = arguments[0];
			if (!msg->IsString())
			{
				return CefV8Value::CreateBool(false);
			}

			auto cefMsg = CefProcessMessage::Create("fxdkSendApiMessage");
			auto cefMsgArgs = cefMsg->GetArgumentList();

			cefMsgArgs->SetSize(1);
			cefMsgArgs->SetString(0, msg->GetStringValue());

			CefV8Context::GetCurrentContext()->GetFrame()->SendProcessMessage(PID_BROWSER, cefMsg);

			return CefV8Value::CreateBool(true);
		}

		return CefV8Value::CreateBool(false);
	});

	nuiApp->AddV8Handler("sendGameClientEvent", [](const CefV8ValueList& arguments, CefString& exception)
	{
		if (arguments.size() == 2)
		{
			auto eventName = arguments[0];
			if (!eventName->IsString())
			{
				exception.FromString("Event name must be a string");
				return CefV8Value::CreateBool(false);
			}

			auto eventPayload = arguments[1];
			if (!eventPayload->IsString())
			{
				exception.FromString("Payload must be a string");
				return CefV8Value::CreateBool(false);
			}

			auto cefMsg = CefProcessMessage::Create("sendGameClientEvent");
			auto cefMsgArgs = cefMsg->GetArgumentList();

			cefMsgArgs->SetSize(2);
			cefMsgArgs->SetString(0, eventName->GetStringValue());
			cefMsgArgs->SetString(1, eventPayload->GetStringValue());

			CefV8Context::GetCurrentContext()->GetFrame()->SendProcessMessage(PID_BROWSER, cefMsg);

			return CefV8Value::CreateBool(true);
		}

		return CefV8Value::CreateBool(false);
	});

	static bool selectFolderDialogOpen = false;
	static std::pair<CefRefPtr<CefV8Context>, CefRefPtr<CefV8Value>> selectFolderDialogCallback;

	nuiApp->AddV8Handler("fxdkOpenSelectFolderDialog", [](const CefV8ValueList& arguments, CefString& exception)
	{
		if (selectFolderDialogOpen)
		{
			return CefV8Value::CreateBool(false);
		}

		selectFolderDialogOpen = true;

		if (arguments.size() < 3)
		{
			return CefV8Value::CreateBool(false);
		}

		if (!arguments[0]->IsString())
		{
			exception.FromWString(L"First argument (Select Dialog start path) must be a string"); // = ;
			return CefV8Value::CreateBool(false);
		}
		if (!arguments[1]->IsString())
		{
			exception.FromWString(L"Second argument (Select Dialog Title) must be a string");
			return CefV8Value::CreateBool(false);
		}
		if (!arguments[2]->IsFunction())
		{
			exception.FromWString(L"Third argument (Select Dialog callback) must be a function");
			return CefV8Value::CreateBool(false);
		}

		selectFolderDialogCallback.first = CefV8Context::GetCurrentContext();
		selectFolderDialogCallback.second = arguments[2];

		auto cefMsg = CefProcessMessage::Create("fxdkOpenSelectFolderDialog");
		auto cefMsgArgs = cefMsg->GetArgumentList();

		cefMsgArgs->SetSize(3);

		cefMsgArgs->SetString(0, arguments[0]->GetStringValue());
		cefMsgArgs->SetString(1, arguments[1]->GetStringValue());
		cefMsgArgs->SetBool(2, true);

		CefV8Context::GetCurrentContext()->GetFrame()->SendProcessMessage(PID_BROWSER, cefMsg);

		return CefV8Value::CreateBool(true);
	});

	nuiApp->AddV8Handler("fxdkOpenSelectFileDialog", [](const CefV8ValueList& arguments, CefString& exception)
	{
		if (selectFolderDialogOpen)
		{
			return CefV8Value::CreateBool(false);
		}

		selectFolderDialogOpen = true;

		if (arguments.size() < 3)
		{
			return CefV8Value::CreateBool(false);
		}

		if (!arguments[0]->IsString())
		{
			exception.FromWString(L"First argument (Select Dialog start path) must be a string"); // = ;
			return CefV8Value::CreateBool(false);
		}
		if (!arguments[1]->IsString())
		{
			exception.FromWString(L"Second argument (Select Dialog Title) must be a string");
			return CefV8Value::CreateBool(false);
		}
		if (!arguments[2]->IsFunction())
		{
			exception.FromWString(L"Third argument (Select Dialog callback) must be a function");
			return CefV8Value::CreateBool(false);
		}

		selectFolderDialogCallback.first = CefV8Context::GetCurrentContext();
		selectFolderDialogCallback.second = arguments[2];

		auto cefMsg = CefProcessMessage::Create("fxdkOpenSelectFolderDialog");
		auto cefMsgArgs = cefMsg->GetArgumentList();

		cefMsgArgs->SetSize(3);

		cefMsgArgs->SetString(0, arguments[0]->GetStringValue());
		cefMsgArgs->SetString(1, arguments[1]->GetStringValue());
		cefMsgArgs->SetBool(2, false);

		CefV8Context::GetCurrentContext()->GetFrame()->SendProcessMessage(PID_BROWSER, cefMsg);

		return CefV8Value::CreateBool(true);
	});

	nuiApp->AddProcessMessageHandler("fxdkOpenSelectFolderDialogResult", [](CefRefPtr<CefBrowser> browser, CefRefPtr<CefProcessMessage> message)
	{
		selectFolderDialogOpen = false;

		CefRefPtr<CefV8Context> context = selectFolderDialogCallback.first;
		CefRefPtr<CefV8Value> callback = selectFolderDialogCallback.second;

		std::string path = message->GetArgumentList()->GetString(0);

		context->Enter();

		CefV8ValueList args;
		args.push_back(CefV8Value::CreateString(path));

		callback->ExecuteFunction(NULL, args);

		context->Exit();

		selectFolderDialogCallback.first = nullptr;
		selectFolderDialogCallback.second = nullptr;

		return true;
	});
}, 1);
