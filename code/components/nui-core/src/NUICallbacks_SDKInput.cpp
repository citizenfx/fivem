/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIApp.h"
#include "NUIClient.h"

#include <variant>

#include <ReverseGameData.h>
#include <WorldEditorControls.h>
#include <HostSharedData.h>

#include <tbb/concurrent_queue.h>

enum class INPUT_OP
{
	MOUSE_POS,
	MOUSE_BUTTONS,
	MOUSE_WHEEL,
	KEY_STATE,
};

struct InputMousePos
{
	int x;
	int y;
};
struct InputMouseButtonState
{
	int index;
	bool state;
};
struct InputKeyState
{
	uint8_t key;
	uint8_t state;
};

typedef std::tuple<INPUT_OP, std::variant<int, InputMousePos, InputMouseButtonState, InputKeyState>> QueueOp;

static tbb::concurrent_queue<QueueOp> g_inputOpsQueue;

static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");
static bool g_rgdInputInited = false;
static HANDLE g_inputMutex = INVALID_HANDLE_VALUE;

static std::thread g_runner;
static HANDLE g_runnerWorkEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
static bool g_runnerShouldStop = false;

static void ExecOp(const QueueOp& opt, bool additive = false)
{
	auto& [op, payload] = opt;

	switch (op)
	{
	/*case INPUT_OP::MOUSE_POS:
	{
		auto& mousePos = std::get<InputMousePos>(payload);

		if (additive)
		{
			rgd->mouseDeltaX += mousePos.x;
			rgd->mouseDeltaY += mousePos.y;
		}
		else
		{
			rgd->mouseDeltaX = mousePos.x;
			rgd->mouseDeltaY = mousePos.y;
		}
		break;
	}
	case INPUT_OP::MOUSE_WHEEL:
	{
		if (additive)
		{
			rgd->mouseWheel += std::get<int>(payload);
		}
		else
		{
			rgd->mouseWheel = std::get<int>(payload);
		}
		break;
	}
	case INPUT_OP::MOUSE_BUTTONS:
	{
		auto& mouseButtonState = std::get<InputMouseButtonState>(payload);

		if (mouseButtonState.state)
		{
			rgd->mouseButtons |= (1 << mouseButtonState.index);
		}
		else
		{
			rgd->mouseButtons &= ~(1 << mouseButtonState.index);
		}
		break;
	}*/
	case INPUT_OP::KEY_STATE:
	{
		auto& keyState = std::get<InputKeyState>(payload);

		rgd->keyboardState[keyState.key] = keyState.state;
		break;
	}
	}
}

void QueueRunner()
{
	while (!g_runnerShouldStop)
	{
		// False alarm!
		if (WaitForSingleObject(g_runnerWorkEvent, INFINITE) != WAIT_OBJECT_0)
		{
			continue;
		}

		if (g_runnerShouldStop) {
			return;
		}

		while (!g_inputOpsQueue.empty())
		{
			if (WaitForSingleObject(g_inputMutex, 5) == WAIT_OBJECT_0)
			{
				QueueOp op;

				while (g_inputOpsQueue.try_pop(op))
				{
					ExecOp(op, true);
				}

				ReleaseMutex(g_inputMutex);
			}
		}
	}
}

static void ExecOrQueueOp(const QueueOp& op)
{
	if (WaitForSingleObject(g_inputMutex, 0) == WAIT_OBJECT_0)
	{
		ExecOp(op);
		ReleaseMutex(g_inputMutex);

		return;
	}

	g_inputOpsQueue.emplace(op);
	SetEvent(g_runnerWorkEvent);
}

static InitFunction initFunction([]()
{
	auto nuiApp = Instance<NUIApp>::Get();

	nuiApp->AddV8Handler("initRGDInput", [](const CefV8ValueList& arguments, CefString& exception)
	{
		if (g_rgdInputInited)
		{
			return CefV8Value::CreateBool(true);
		}

		auto srcProcHnd = OpenProcess(PROCESS_DUP_HANDLE, 0, rgd->inputMutexPID);

		auto success = DuplicateHandle(
			srcProcHnd,
			rgd->inputMutex,
			GetCurrentProcess(),
			&g_inputMutex,
			DUPLICATE_SAME_ACCESS,
			0,
			DUPLICATE_SAME_ACCESS
		);

		if (!success)
		{
			exception.FromString("Failed to duplicate input mutex handle");
			trace("Failed to duplicate input mutex handle\n");
			return CefV8Value::CreateBool(false);
		}

		g_rgdInputInited = true;
		g_runner = std::thread(QueueRunner);
		g_runner.detach();

		return CefV8Value::CreateBool(true);
	});

	nuiApp->AddV8Handler("setRawMouseCapture", [](const CefV8ValueList& arguments, CefString& exception)
	{
		if (arguments.size() != 1)
		{
			exception.FromString(fmt::sprintf("Expected 1 argument, got %d", arguments.size()));
			return CefV8Value::CreateBool(false);
		}

		rgd->useRawMouseCapture = arguments[0]->GetBoolValue();

		return CefV8Value::CreateBool(true);
	});

	//nuiApp->AddV8Handler("sendMousePos", [](const CefV8ValueList& arguments, CefString& exception)
	//{
	//	return CefV8Value::CreateBool(true);

	//	if (arguments.size() != 2)
	//	{
	//		exception.FromString(fmt::sprintf("Expected 2 arguments, got %d", arguments.size()));
	//		return CefV8Value::CreateBool(false);
	//	}

	//	int x = arguments[0]->GetIntValue();
	//	int y = arguments[1]->GetIntValue();

	//	rgd->mouseDeltaX = x;
	//	rgd->mouseDeltaY = y;

	//	//ExecOrQueueOp({ INPUT_OP::MOUSE_POS, InputMousePos{ x, y } });

	//	return CefV8Value::CreateBool(true);
	//});

	nuiApp->AddV8Handler("setMouseButtonState", [](const CefV8ValueList& arguments, CefString& exception)
	{
		if (arguments.size() != 2)
		{
			exception.FromString(fmt::sprintf("Expected 2 arguments, got %d", arguments.size()));
			return CefV8Value::CreateBool(false);
		}

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

		//ExecOrQueueOp({ INPUT_OP::MOUSE_BUTTONS, InputMouseButtonState{ buttonIndex, state } });

		return CefV8Value::CreateBool(true);
	});

	nuiApp->AddV8Handler("sendMouseWheel", [](const CefV8ValueList& arguments, CefString& exception)
	{
		if (arguments.size() != 1)
		{
			exception.FromString(fmt::sprintf("Expected 1 argument, got %d", arguments.size()));
			return CefV8Value::CreateBool(false);
		}

		auto wheel = arguments[0]->GetIntValue();

		rgd->mouseWheel = wheel;

		//ExecOrQueueOp({ INPUT_OP::MOUSE_WHEEL, wheel });

		return CefV8Value::CreateBool(true);
	});

	nuiApp->AddV8Handler("setKeyState", [](const CefV8ValueList& arguments, CefString& exception)
	{
		if (arguments.size() != 2)
		{
			exception.FromString(fmt::sprintf("Expected 2 arguments, got %d", arguments.size()));
			return CefV8Value::CreateBool(false);
		}

		uint8_t key = arguments[0]->GetIntValue();
		uint8_t state = arguments[1]->GetBoolValue() ? 0x80 : 0;

		ExecOrQueueOp({ INPUT_OP::KEY_STATE, InputKeyState{ key, state } });

		return CefV8Value::CreateBool(true);
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

	nuiApp->AddContextReleaseHandler([](CefRefPtr<CefV8Context>)
	{
		if (g_rgdInputInited)
		{
			g_runnerShouldStop = true;
			SetEvent(g_runnerWorkEvent);

			if (g_runner.joinable())
			{
				g_runner.join();
			}
		}
	});
});
