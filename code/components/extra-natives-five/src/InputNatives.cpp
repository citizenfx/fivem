#include "StdInc.h"

#include <Local.h>
#include <Hooking.h>
#include <ScriptEngine.h>

#include "nutsnbolts.h"

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

// List of raw keys to count as disabled until the next frame
static std::set<int> disabledKeys{};

template<bool HandleDisabled = true>
static bool IsRawKeyInvalidOrDisabled(int key)
{
	if constexpr (HandleDisabled)
	{
		if (disabledKeys.find(key) != disabledKeys.end())
		{
			// the  key should be disabled
			return true;
		}
	}

	if (key >= 0 && key < KEYS_COUNT)
	{
		// the keys valid, we shouldn't disable it
		return false;
	}

	// :( out of bounds
	return true;
}

template<bool HandleDisabled = true, auto fn>
static void IsRawKeyWrapper(fx::ScriptContext& context)
{
	auto rawKeyIndex = context.GetArgument<uint32_t>(0);

	if (!IsRawKeyInvalidOrDisabled<HandleDisabled>(rawKeyIndex))
	{
		context.SetResult<bool>(fn(rawKeyIndex) != 0);
	}
	else
	{
		context.SetResult<bool>(false);
	}
}

static HookFunction initFunction([]()
{
#ifdef IS_RDR3
	uint8_t* location = hook::get_pattern<uint8_t>("48 63 05 ? ? ? ? 4C 8D 35 ? ? ? ? 48 83 F0 ? B9");
	ioKeyboardActive = hook::get_address<int*>(location, 3, 7);
	ioKeyboardKeys = hook::get_address<keysData*>(location + 7, 3, 7);
#else
	ioKeyboardActive = hook::get_address<int*>(hook::get_pattern("8B 2D ? ? ? ? 48 8B 03"), 2, 6);
	ioKeyboardKeys = hook::get_address<keysData*>(hook::get_pattern("48 8D 2D ? ? ? ? 49 C1 E6"), 3, 7);
#endif

	// reset the disabled keys every frame
	OnGameFrame.Connect([]
	{
		disabledKeys.clear();
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_RAW_KEY_PRESSED", IsRawKeyWrapper<true, &ioKeyboard_KeyPressed>);
	fx::ScriptEngine::RegisterNativeHandler("IS_RAW_KEY_RELEASED", IsRawKeyWrapper<true, &ioKeyboard_KeyReleased>);
	fx::ScriptEngine::RegisterNativeHandler("IS_RAW_KEY_DOWN", IsRawKeyWrapper<true, &ioKeyboard_KeyDown>);
	fx::ScriptEngine::RegisterNativeHandler("IS_RAW_KEY_UP", IsRawKeyWrapper<true, &ioKeyboard_KeyUp>);

	fx::ScriptEngine::RegisterNativeHandler("IS_DISABLED_RAW_KEY_PRESSED", IsRawKeyWrapper<false, &ioKeyboard_KeyPressed>);
	fx::ScriptEngine::RegisterNativeHandler("IS_DISABLED_RAW_KEY_RELEASED", IsRawKeyWrapper<false, &ioKeyboard_KeyReleased>);
	fx::ScriptEngine::RegisterNativeHandler("IS_DISABLED_RAW_KEY_DOWN", IsRawKeyWrapper<false, &ioKeyboard_KeyDown>);
	fx::ScriptEngine::RegisterNativeHandler("IS_DISABLED_RAW_KEY_UP", IsRawKeyWrapper<false, &ioKeyboard_KeyUp>);

	fx::ScriptEngine::RegisterNativeHandler("DISABLE_RAW_KEY_THIS_FRAME", [](fx::ScriptContext& context)
	{
		auto rawKeyIndex = context.GetArgument<uint32_t>(0);

		// We only want the bounds check here, we don't care if its already disabled
		if (!IsRawKeyInvalidOrDisabled<false>(rawKeyIndex))
		{
			disabledKeys.insert(rawKeyIndex);
		}
	});
});
