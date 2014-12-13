/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

enum ConsoleModifiers
{
	ConsoleModifierNone = 0,
	ConsoleModifierAlt = 1,
	ConsoleModifierShift = 2,
	ConsoleModifierControl = 4
};

// THANKS MICROSOFT
DEFINE_ENUM_FLAG_OPERATORS(ConsoleModifiers);

void ConHost_Run();

void ConHost_KeyEnter(uint32_t vKey, wchar_t character, ConsoleModifiers modifiers);

// private
void ConHost_AddInternalCalls();

void ConHost_WaitForKey(uint32_t& vKey, wchar_t& character, ConsoleModifiers& modifiers);

void ConHost_GetCursorPos(int& x, int& y);

void ConHost_NewBuffer(int width, int height);

void* ConHost_GetBuffer();