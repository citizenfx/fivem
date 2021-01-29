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
#include <HostSharedData.h>

static InitFunction initFunction([]()
{
	auto nuiApp = Instance<NUIApp>::Get();
	auto init = false;

	static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");

	auto wrapWithRgdInputMutex = [](int argsSize, auto fn)
	{
		return [=](const CefV8ValueList& arguments, CefString& exception)
		{
			if (arguments.size() != argsSize)
			{
				trace(__FUNCTION__ ": V8 Handler args size mismatch\n");
				return CefV8Value::CreateBool(false);
			}

			// FUCKING INPUT MUTEX OK
			return fn(arguments, exception);

			/*trace(__FUNCTION__ ": rgd->inputMutex == %p\n", rgd->inputMutex);

			auto waitResult = WaitForSingleObject(rgd->inputMutex, INFINITE);

			if (waitResult == WAIT_OBJECT_0)
			{
				try {
					auto ret = fn(arguments, exception);
					trace(__FUNCTION__ ": V8 Handler is ok, releasing mutex\n");
					ReleaseMutex(rgd->inputMutex);
					return ret;
				}
				catch (...) {
					trace(__FUNCTION__ ": V8 Handler fucked up, releasing mutex\n");
					ReleaseMutex(rgd->inputMutex);
					return CefV8Value::CreateBool(false);
				}
			}
			else if (waitResult == WAIT_TIMEOUT)
			{
				trace(__FUNCTION__ ": INPUT MUTEX TIEMOUT???\n");
				return CefV8Value::CreateBool(false);
			}
			else if (waitResult == WAIT_ABANDONED)
			{
				trace(__FUNCTION__ ": INPUT MUTEX ABANDONED\n");
				return CefV8Value::CreateBool(false);
			}
			else if (waitResult == WAIT_FAILED)
			{
				trace(__FUNCTION__ ": INPUT MUTEX WAIT FAILED, LAST ERROR: %x\n", GetLastError());
				return CefV8Value::CreateBool(false);
			}
			else
			{
				trace(__FUNCTION__ ": INPUT MUTEX FUCKED: %x\n", waitResult);
				return CefV8Value::CreateBool(false);
			}*/
		};
	};

	nuiApp->AddV8Handler("sendMousePos", wrapWithRgdInputMutex(2, [&](const CefV8ValueList& arguments, CefString& exception)
	{
		auto dx = arguments[0]->GetIntValue();
		auto dy = arguments[1]->GetIntValue();

		rgd->mouseX += dx;
		rgd->mouseY += dy;

		return CefV8Value::CreateBool(true);
	}));

	nuiApp->AddV8Handler("setKeyState", wrapWithRgdInputMutex(2, [&](const CefV8ValueList& arguments, CefString& exception)
	{
		auto key = arguments[0]->GetIntValue();
		auto state = arguments[1]->GetBoolValue() ? 0x80 : 0;

		rgd->keyboardState[key] = state;

		return CefV8Value::CreateBool(true);
	}));

	nuiApp->AddV8Handler("setMouseButtonState", wrapWithRgdInputMutex(2, [&](const CefV8ValueList& arguments, CefString& exception)
	{
		auto buttonIndex = arguments[0]->GetIntValue();
		auto state = arguments[1]->GetBoolValue();

		if (state)
		{
			rgd->mouseButtons |= (1 << buttonIndex);
		}
		else
		{
			rgd->mouseButtons &= ~(1 << buttonIndex);
		}

		return CefV8Value::CreateBool(true);
	}));

	nuiApp->AddV8Handler("sendMouseWheel", wrapWithRgdInputMutex(1, [&](const CefV8ValueList& arguments, CefString& exception)
	{
		auto wheel = arguments[0]->GetIntValue();

		rgd->mouseWheel = wheel;

		return CefV8Value::CreateBool(true);
	}));

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
			if (arguments[0]->IsString())
			{
				auto charString = arguments[0]->GetStringValue();
				
				rgd->inputChar = charString.c_str()[0];
			}
		}

		return CefV8Value::CreateUndefined();
	});
}, 1);
