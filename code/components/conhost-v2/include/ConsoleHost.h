/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifdef COMPILING_CONHOST_POSH
#define CONHOST_EXPORT DLL_EXPORT
#else
#define CONHOST_EXPORT DLL_IMPORT
#endif

namespace ConHost
{
	extern CONHOST_EXPORT fwEvent<const char*, const char*> OnInvokeNative;

	extern CONHOST_EXPORT fwEvent<> OnDrawGui;

	extern CONHOST_EXPORT fwEvent<bool*> OnShouldDrawGui;

	CONHOST_EXPORT bool IsConsoleOpen();
	
	CONHOST_EXPORT void Print(const std::string& channel, const std::string& message);

	CONHOST_EXPORT void SetCursorMode(bool mode);
}
