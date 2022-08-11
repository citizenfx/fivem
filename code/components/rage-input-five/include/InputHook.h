#pragma once

#include <SharedInput.h>
#include <CrossBuildRuntime.h>

#ifndef COMPILING_RAGE_INPUT_FIVE
#define INPUT_DECL __declspec(dllimport)
#else
#define INPUT_DECL __declspec(dllexport)
#endif

template<int Bytes>
struct Padding
{
	uint8_t m_padding[Bytes];
};

struct EmptyStruct
{
};

//[[no_unique_address]] (C++20) doesn't work on MSVC! Can't use this yet
template<bool Enable, int Bytes>
using IfPadding = std::conditional_t<Enable, Padding<Bytes>, EmptyStruct>;

template<bool Enable, int Bytes, int BytesIfFalse>
using IfElsePadding = std::conditional_t<Enable, Padding<Bytes>, Padding<BytesIfFalse>>;


#define DECLARE_ACCESSOR(x)                                                   \
	inline auto& x()                                                          \
	{                                                                         \
		if (xbr::IsGameBuildOrGreater<2699>())								  \
		{																	  \
			return impl2699.x;												  \
		}																	  \
		if (xbr::IsGameBuildOrGreater<2372>())                                \
		{                                                                     \
			return impl2372.x;                                                \
		}                                                                     \
		return (xbr::IsGameBuildOrGreater<2060>() ? impl2189.x : impl1604.x); \
	}                                                                         \
	inline const auto& x() const                                              \
	{                                                                         \
		if (xbr::IsGameBuildOrGreater<2699>())								  \
		{                                                                     \
			return impl2699.x;												  \
		}																	  \
		if (xbr::IsGameBuildOrGreater<2372>())                                \
		{                                                                     \
			return impl2372.x;                                                \
		}                                                                     \
		return (xbr::IsGameBuildOrGreater<2060>() ? impl2189.x : impl1604.x); \
	}

// static global members in ioMouse - possible that padding may change
namespace rage
{
static constexpr uint32_t METHOD_RAW_INPUT = 0;
static constexpr uint32_t METHOD_DIRECTINPUT = 1;
static constexpr uint32_t METHOD_WINDOWS = 2;

struct ioMouseStruct1604
{
	int32_t m_dZ; // mousewheel
	char pad_0028[16];
	uint32_t m_inputMethod;
	char pad_4[4];
	int32_t m_X; // Based on your monitor's resolution. Zero when alt-tabbed and Zero in launcher
	int32_t m_Y; // ^^^^^^^^^^^
	int32_t m_dX; // in pixels
	int32_t m_lastDX;
	int32_t m_dY;
	int32_t m_lastDY;
	char pad_0050[76];
	bool _unk;
	bool shouldZeroMouseValues;
	bool _unk2;
	bool _align;
	uint32_t _unk4;
	int32_t cursorAbsX; // Based on your monitor's resolution. center of screen when alt-tabbed (ex: 1280/720 on a 1440p screen)
	int32_t cursorAbsY; // ^^^^^^^^^^^^^^
	float cursorDiffX; // cursor pixel diff in float, but is always rounded to a whole number
	float cursorDiffY; // ^^^^^^
	int32_t scrollDiffTicks; // unsure about this
	int32_t m_InternalButtonsState;
	char pad_00BC[4];
	int32_t m_LastButtons;
	int32_t m_Buttons;
	int32_t N00000048; // some sort of frame ticker, only goes up while alt-tabbed
	float mouseDiffDirectionX; // -1.000[Left] -> 1.000[Right]
	float mouseDiffDirectionY; // -1.000[Up] -> 1.000[Down]
};
struct ioMouseStruct2189
{
	int32_t m_dZ; // mousewheel
	char pad_8[8];
	uint32_t m_inputMethod;
	char pad_4[4];
	int32_t m_X; // Based on your monitor's resolution. Zero when alt-tabbed and Zero in launcher
	int32_t m_Y; // ^^^^^^^^^^^
	char pad_4_2[4];
	int32_t m_dX; // in pixels
	int32_t m_lastDX;
	int32_t m_dY;
	int32_t m_lastDY;
	char pad_0050[72];
	bool _unk;
	bool shouldZeroMouseValues;
	bool _unk2;
	bool _align;
	uint32_t _unk4;
	int32_t cursorAbsX; // Based on your monitor's resolution. center of screen when alt-tabbed (ex: 1280/720 on a 1440p screen)
	int32_t cursorAbsY; // ^^^^^^^^^^^^^^
	float cursorDiffX; // cursor pixel diff in float, but is always rounded to a whole number
	float cursorDiffY; // ^^^^^^
	int32_t scrollDiffTicks; // unsure about this
	int32_t m_InternalButtonsState;
	char pad_00BC[4];
	int32_t m_LastButtons;
	int32_t m_Buttons;
	int32_t N00000048; // some sort of frame ticker, only goes up while alt-tabbed
	float mouseDiffDirectionX; // -1.000[Left] -> 1.000[Right]
	float mouseDiffDirectionY; // -1.000[Up] -> 1.000[Down]
};
struct ioMouseStruct2372
{
	int32_t m_dZ; // mousewheel
	char pad_8[8];
	int32_t m_X; // Based on your monitor's resolution. Zero when alt-tabbed and Zero in launcher
	int32_t m_Y; // ^^^^^^^^^^^
	int32_t m_dX; // in pixels
	int32_t m_lastDX;
	int32_t m_dY;
	int32_t m_lastDY;
	char pad_0024[84]; //0x0024
	int32_t cursorAbsX; // Based on your monitor's resolution. center of screen when alt-tabbed (ex: 1280/720 on a 1440p screen)
	int32_t cursorAbsY; // ^^^^^^^^^^^^^^
	float cursorDiffX; // cursor pixel diff in float, but is always rounded to a whole number
	float cursorDiffY; // ^^^^^^
	int32_t scrollDiffTicks; // unsure about this
	int32_t m_InternalButtonsState;
	int32_t m_LastButtons;
	int32_t m_Buttons;
	int32_t N00000048; // some sort of frame ticker, only goes up while alt-tabbed
	float mouseDiffDirectionX; // -1.000[Left] -> 1.000[Right]
	float mouseDiffDirectionY; // -1.000[Up] -> 1.000[Down]
};
struct ioMouseStruct2699
{
	int32_t m_dZ; // mousewheel
	char pad_[20];
	int32_t m_X; // Based on your monitor's resolution. Zero when alt-tabbed and Zero in launcher
	int32_t m_Y; // ^^^^^^^^^^^
	int32_t m_dX; // in pixels
	int32_t m_lastDX;
	int32_t m_dY;
	int32_t m_lastDY;
	char pad_0024[84];
	int32_t cursorAbsX; // Based on your monitor's resolution. center of screen when alt-tabbed (ex: 1280/720 on a 1440p screen)
	int32_t cursorAbsY; // ^^^^^^^^^^^^^^
	float cursorDiffX; // cursor pixel diff in float, but is always rounded to a whole number
	float cursorDiffY; // ^^^^^^
	int32_t scrollDiffTicks; // unsure about this
	int32_t m_InternalButtonsState;
	int32_t m_LastButtons;
	int32_t m_Buttons;
	int32_t N00000048; // some sort of frame ticker, only goes up while alt-tabbed
	float mouseDiffDirectionX; // -1.000[Left] -> 1.000[Right]
	float mouseDiffDirectionY; // -1.000[Up] -> 1.000[Down]
};

struct ioMouse
{
	union
	{
		ioMouseStruct1604 impl1604;
		ioMouseStruct2189 impl2189;
		ioMouseStruct2372 impl2372;
		ioMouseStruct2699 impl2699;
	};
	DECLARE_ACCESSOR(m_Buttons);
	DECLARE_ACCESSOR(m_lastDX);
	DECLARE_ACCESSOR(m_lastDY);
	DECLARE_ACCESSOR(m_dX);
	DECLARE_ACCESSOR(m_dY);
	DECLARE_ACCESSOR(m_dZ);
	DECLARE_ACCESSOR(m_X);
	DECLARE_ACCESSOR(m_Y);
	DECLARE_ACCESSOR(cursorAbsX);
	DECLARE_ACCESSOR(cursorAbsY);

	DECLARE_ACCESSOR(mouseDiffDirectionX);
	DECLARE_ACCESSOR(mouseDiffDirectionY);
};
}

#define INPUT_HOOK_HOST_CURSOR_SUPPORT

namespace InputHook
{
struct ControlBypass
{
	bool isMouse;
	int ctrlIdx; // (RAGE) VK if keyboard, mouse button bit if not
};

INPUT_DECL void SetControlBypasses(int subsystem, std::initializer_list<ControlBypass> bypasses);

extern INPUT_DECL fwEvent<std::vector<InputTarget*>&> QueryInputTarget;

extern INPUT_DECL fwEvent<HWND, UINT, WPARAM, LPARAM, bool&, LRESULT&> DeprecatedOnWndProc;

extern INPUT_DECL fwEvent<int&> QueryMayLockCursor;

INPUT_DECL void SetGameMouseFocus(bool focus);

INPUT_DECL void EnableSetCursorPos(bool enabled);

INPUT_DECL bool IsMouseButtonDown(int buttonFlag);

INPUT_DECL bool IsKeyDown(int vk_keycode);

INPUT_DECL void SetHostCursorEnabled(bool enabled);
}
