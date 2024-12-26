#include "StdInc.h"

#include <Local.h>
#include <Hooking.h>
#include <ScriptEngine.h>

constexpr int KEYS_COUNT = 256;

using keysData = unsigned char[2][KEYS_COUNT];

int* ioKeyboardActive = nullptr;
keysData* ioKeyboardKeys = nullptr;

inline int ioKeyboard_KeyDown(int key)
{
	return (*ioKeyboardKeys)[*ioKeyboardActive][key];
}

inline int ioKeyboard_KeyUp(int key)
{
	return !ioKeyboard_KeyDown(key);
}

inline int ioKeyboard_KeyChanged(int key)
{
	return (*ioKeyboardKeys)[0][key] ^ (*ioKeyboardKeys)[1][key];
}

inline int ioKeyboard_KeyPressed(int key)
{
	return (*ioKeyboardKeys)[*ioKeyboardActive][key] & ioKeyboard_KeyChanged(key);
}

inline int ioKeyboard_KeyReleased(int key)
{
	return (*ioKeyboardKeys)[*ioKeyboardActive ^ 1][key] & ioKeyboard_KeyChanged(key);
}

static HookFunction initFunction([]()
{
	ioKeyboardActive = hook::get_address<int*>(hook::get_pattern("8B 2D ? ? ? ? 48 8B 03"), 2, 6);
	ioKeyboardKeys = hook::get_address<keysData*>(hook::get_pattern("48 8D 2D ? ? ? ? 49 C1 E6"), 3, 7);

	fx::ScriptEngine::RegisterNativeHandler("IS_RAW_KEY_PRESSED", [](fx::ScriptContext& context)
	{
		auto rawKeyIndex = context.GetArgument<uint32_t>(0);

		if (rawKeyIndex >= 0 && rawKeyIndex < KEYS_COUNT)
		{
			context.SetResult<bool>(ioKeyboard_KeyPressed(rawKeyIndex) != 0);
		}
		else
		{
			context.SetResult<bool>(false);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_RAW_KEY_RELEASED", [](fx::ScriptContext& context)
	{
		auto rawKeyIndex = context.GetArgument<uint32_t>(0);

		if (rawKeyIndex >= 0 && rawKeyIndex < KEYS_COUNT)
		{
			context.SetResult<bool>(ioKeyboard_KeyReleased(rawKeyIndex) != 0);
		}
		else
		{
			context.SetResult<bool>(false);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_RAW_KEY_DOWN", [](fx::ScriptContext& context)
	{
		auto rawKeyIndex = context.GetArgument<uint32_t>(0);

		if (rawKeyIndex >= 0 && rawKeyIndex < KEYS_COUNT)
		{
			context.SetResult<bool>(ioKeyboard_KeyDown(rawKeyIndex) != 0);
		}
		else
		{
			context.SetResult<bool>(false);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_RAW_KEY_UP", [](fx::ScriptContext& context)
	{
		auto rawKeyIndex = context.GetArgument<uint32_t>(0);

		if (rawKeyIndex >= 0 && rawKeyIndex < KEYS_COUNT)
		{
			context.SetResult<bool>(ioKeyboard_KeyUp(rawKeyIndex) != 0);
		}
		else
		{
			context.SetResult<bool>(false);
		}
	});
});
