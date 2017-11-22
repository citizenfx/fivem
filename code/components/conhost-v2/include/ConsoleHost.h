/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
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
	
	CONHOST_EXPORT void Print(int channel, const std::string& message);
}