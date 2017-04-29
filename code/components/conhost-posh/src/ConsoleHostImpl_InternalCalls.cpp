/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ConsoleHost.h"
#include "ConsoleHostImpl.h"
#include "Screen.h"
#include "FontRenderer.h"

#include <mono/jit/jit.h>

void ConHostI_GetKeyCall(uint32_t* keyCode, uint32_t* characterCode, uint32_t* outModifiers)
{
	uint32_t vKey;
	wchar_t character;
	ConsoleModifiers modifiers;

	ConHost_WaitForKey(vKey, character, modifiers);

	*keyCode = vKey;
	*characterCode = character;
	*outModifiers = modifiers;
}

void* ConHostI_SetBufferCall(int width, int height)
{
	ConHost_NewBuffer(width, height);

	return ConHost_GetBuffer();
}

void ConHostI_GetDesiredBufferSizeCall(int* width, int* height)
{
	CRect stringRect;
	TheFonts->GetStringMetrics(L"a", 16.0f, 1.0f, "Lucida Console", stringRect);

	*width = (GetScreenResolutionX() - 16) / stringRect.Width();
	//*height = 25;
	*height = 2500;
}

static int g_cursorX;
static int g_cursorY;

void ConHostI_SetCursorPosCall(int x, int y)
{
	g_cursorX = x;
	g_cursorY = y;
}

void ConHost_GetCursorPos(int& x, int& y)
{
	x = g_cursorX;
	y = g_cursorY;
}

void ConHostI_InvokeHostFunction(MonoString* functionNameStr, MonoString* argumentStr)
{
	char* functionName = mono_string_to_utf8(functionNameStr);
	char* argument = mono_string_to_utf8(argumentStr);

	ConHost::OnInvokeNative(functionName, argument);
}

void ConHost_AddInternalCalls()
{
	mono_add_internal_call("CitizenFX.UI.ConsoleImpl::GetDesiredBufferSize", ConHostI_GetDesiredBufferSizeCall);
	mono_add_internal_call("CitizenFX.UI.ConsoleImpl::GetKeyInternal", ConHostI_GetKeyCall);
	mono_add_internal_call("CitizenFX.UI.ConsoleImpl::SetBuffer", ConHostI_SetBufferCall);
	mono_add_internal_call("CitizenFX.UI.ConsoleImpl::SetCursorPos", ConHostI_SetCursorPosCall);
	mono_add_internal_call("CitizenFX.UI.ConsoleImpl::InvokeHostFunction", ConHostI_InvokeHostFunction);
}

fwEvent<const char*, const char*> ConHost::OnInvokeNative;