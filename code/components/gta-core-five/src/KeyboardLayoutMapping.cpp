#include <StdInc.h>
#include <Hooking.h>
#include <MinHook.h>

#include <CrossBuildRuntime.h>
#include <USKeyboardMapping.h>

enum KeyboardKeys
{
	KEY_NULL = 0,
	KEY_BACK = 8,
	KEY_TAB = 9,
	KEY_RETURN = 13,
	KEY_PAUSE = 19,
	KEY_CAPITAL = 20,
	KEY_ESCAPE = 27,
	KEY_SPACE = 32,
	KEY_PAGEUP = 33,
	KEY_PRIOR = 33,
	KEY_PAGEDOWN = 34,
	KEY_NEXT = 34,
	KEY_END = 35,
	KEY_HOME = 36,
	KEY_LEFT = 37,
	KEY_UP = 38,
	KEY_RIGHT = 39,
	KEY_DOWN = 40,
	KEY_SNAPSHOT = 44,
	KEY_SYSRQ = 44,
	KEY_INSERT = 45,
	KEY_DELETE = 46,
	KEY_0 = 48,
	KEY_1 = 49,
	KEY_2 = 50,
	KEY_3 = 51,
	KEY_4 = 52,
	KEY_5 = 53,
	KEY_6 = 54,
	KEY_7 = 55,
	KEY_8 = 56,
	KEY_9 = 57,
	KEY_A = 65,
	KEY_B = 66,
	KEY_C = 67,
	KEY_D = 68,
	KEY_E = 69,
	KEY_F = 70,
	KEY_G = 71,
	KEY_H = 72,
	KEY_I = 73,
	KEY_J = 74,
	KEY_K = 75,
	KEY_L = 76,
	KEY_M = 77,
	KEY_N = 78,
	KEY_O = 79,
	KEY_P = 80,
	KEY_Q = 81,
	KEY_R = 82,
	KEY_S = 83,
	KEY_T = 84,
	KEY_U = 85,
	KEY_V = 86,
	KEY_W = 87,
	KEY_X = 88,
	KEY_Y = 89,
	KEY_Z = 90,
	KEY_LWIN = 91,
	KEY_RWIN = 92,
	KEY_APPS = 93,
	KEY_NUMPAD0 = 96,
	KEY_NUMPAD1 = 97,
	KEY_NUMPAD2 = 98,
	KEY_NUMPAD3 = 99,
	KEY_NUMPAD4 = 100,
	KEY_NUMPAD5 = 101,
	KEY_NUMPAD6 = 102,
	KEY_NUMPAD7 = 103,
	KEY_NUMPAD8 = 104,
	KEY_NUMPAD9 = 105,
	KEY_MULTIPLY = 106,
	KEY_ADD = 107,
	KEY_SUBTRACT = 109,
	KEY_DECIMAL = 110,
	KEY_DIVIDE = 111,
	KEY_F1 = 112,
	KEY_F2 = 113,
	KEY_F3 = 114,
	KEY_F4 = 115,
	KEY_F5 = 116,
	KEY_F6 = 117,
	KEY_F7 = 118,
	KEY_F8 = 119,
	KEY_F9 = 120,
	KEY_F10 = 121,
	KEY_F11 = 122,
	KEY_F12 = 123,
	KEY_F13 = 124,
	KEY_F14 = 125,
	KEY_F15 = 126,
	KEY_F16 = 127,
	KEY_F17 = 128,
	KEY_F18 = 129,
	KEY_F19 = 130,
	KEY_F20 = 131,
	KEY_F21 = 132,
	KEY_F22 = 133,
	KEY_F23 = 134,
	KEY_F24 = 135,
	KEY_NUMLOCK = 144,
	KEY_SCROLL = 145,
	KEY_NUMPADEQUALS = 146,
	KEY_LSHIFT = 160,
	KEY_RSHIFT = 161,
	KEY_LCONTROL = 162,
	KEY_RCONTROL = 163,
	KEY_LMENU = 164,
	KEY_RMENU = 165,
	KEY_SEMICOLON = 186,
	KEY_OEM_1 = 186,
	KEY_PLUS = 187,
	KEY_EQUALS = 187,
	KEY_COMMA = 188,
	KEY_MINUS = 189,
	KEY_PERIOD = 190,
	KEY_SLASH = 191,
	KEY_OEM_2 = 191,
	KEY_GRAVE = 192,
	KEY_OEM_3 = 192,
	KEY_LBRACKET = 219,
	KEY_OEM_4 = 219,
	KEY_BACKSLASH = 220,
	KEY_OEM_5 = 220,
	KEY_RBRACKET = 221,
	KEY_OEM_6 = 221,
	KEY_APOSTROPHE = 222,
	KEY_OEM_7 = 222,
	KEY_OEM_102 = 226,
	KEY_RAGE_EXTRA1 = 240,
	KEY_RAGE_EXTRA2 = 241,
	KEY_RAGE_EXTRA3 = 242,
	KEY_RAGE_EXTRA4 = 244,
	KEY_NUMPADENTER = 253,
	KEY_CHATPAD_GREEN_SHIFT = 254,
	KEY_CHATPAD_ORANGE_SHIFT = 255,
};

static std::vector<KeyboardKeys> mappedKeys{
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_SEMICOLON,
	KEY_OEM_1,
	KEY_PLUS,
	KEY_EQUALS,
	KEY_COMMA,
	KEY_MINUS,
	KEY_PERIOD,
	KEY_SLASH,
	KEY_OEM_2,
	KEY_GRAVE,
	KEY_OEM_3,
	KEY_LBRACKET,
	KEY_OEM_4,
	KEY_BACKSLASH,
	KEY_OEM_5,
	KEY_RBRACKET,
	KEY_OEM_6,
	KEY_APOSTROPHE,
	KEY_OEM_7,
	KEY_OEM_102
};

struct KeyLayoutMap
{
	int icon;
	char text[10];
	char pad[2];
};

struct ControlClass1604
{
	// 1604
	uint8_t pad[2544];

	KeyLayoutMap keys[255];
	uint32_t numKeys;
};

struct ControlClass1868
{
	// 1868 (+8)
	uint8_t pad[2544 + 8];

	KeyLayoutMap keys[255];
	uint32_t numKeys;
};

struct ControlClass2060
{
	// 2060 (+16)
	uint8_t pad[2544 + 16];

	KeyLayoutMap keys[255];
	uint32_t numKeys;
};

struct ControlClass2699
{
	// 2699 (+24)
	uint8_t pad[2544 + 24];

	KeyLayoutMap keys[255];
	uint32_t numKeys;
};


static void(*g_origLoadLayout)(void*);

static void* g_controlData;

template<typename TClass>
static void UpdateControlData(TClass* a1)
{
	for (auto& key : mappedKeys)
	{
		// first, map the US VK to a scan code
		auto scanCode = MapVirtualKeyInternal(key, MAPVK_VK_TO_VSC);

		// then, map the scan code to a localized vkey
		auto vk = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK);

		// map the virtual key to a character
		wchar_t ch = MapVirtualKey(vk, MAPVK_VK_TO_CHAR);

		// make a temporary string
		wchar_t str[] = { ch, L'\0' };

		// and make it UTF-8
		std::string text = ToNarrow(str);

		// finally, copy to the output
		strcpy_s(a1->keys[key].text, text.c_str());
	}
}

template<typename TClass>
static void LoadKeyboardLayoutWrap(TClass* a1)
{
	g_origLoadLayout(a1);

	g_controlData = a1;

	UpdateControlData(a1);
}

static void(*g_origOnInputLanguageChange)();

static void OnInputLanguageChange()
{
	g_origOnInputLanguageChange();

	if (g_controlData)
	{
		if (xbr::IsGameBuildOrGreater<2699>())
		{
			UpdateControlData((ControlClass2699*)g_controlData);
		}
		else if (xbr::IsGameBuildOrGreater<2060>())
		{
			UpdateControlData((ControlClass2060*)g_controlData);
		}
		else
		{
			UpdateControlData((ControlClass1604*)g_controlData);
		}
	}
}

static HookFunction hookFunction([]()
{
	if (Is372())
	{
		return;
	}

	{
		// Disable loading of en-US layout in _initKeyboard, we tap into kbdus directly
		auto enKeyboardInitBlock = hook::get_pattern<uint8_t>("40 8A C6 48 39 35 ? ? ? ? 75 08 84 C0 0F 84 ? ? ? ? 33 D2 33 C9", 3);
		hook::nop(enKeyboardInitBlock, 12);
		hook::put<uint8_t>(enKeyboardInitBlock + 12, 0xE9);

		// Ditch built-in MapGameKey impl for our reimplementation so graceful handling of non-QWERTY layouts is kept (game's depends on layout skipped above)
		auto mapGameKeyFunc = hook::get_pattern("48 89 5C 24 08 57 48 83 EC 20 41 8A D8 8B FA 83 F9 13");
		hook::jump(mapGameKeyFunc, MapGameKey);
	}

	{
		auto location = hook::get_pattern("E8 ? ? ? ? 48 8D 45 88 48 3B D8 74 1E");
		hook::set_call(&g_origLoadLayout, location);
		hook::call(location, (xbr::IsGameBuildOrGreater<2699>() ? (void*)&LoadKeyboardLayoutWrap<ControlClass2699> : xbr::IsGameBuildOrGreater<2060>() ? (void*)&LoadKeyboardLayoutWrap<ControlClass2060> : (void*)&LoadKeyboardLayoutWrap<ControlClass1604>));
	}

	{
		MH_Initialize();
		MH_CreateHook(hook::get_pattern("48 83 EC 28 80 3D ? ? ? ? 00 75 0A 48 83 3D ? ? ? ? 00"), OnInputLanguageChange, (void**)&g_origOnInputLanguageChange);
		MH_EnableHook(MH_ALL_HOOKS);
	}
});
