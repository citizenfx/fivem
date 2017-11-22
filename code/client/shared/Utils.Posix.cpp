/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"
#include "Utils.h"

fwPlatformString GetAbsoluteCitPath()
{
	static fwPlatformString citizenPath;

	if (!citizenPath.size())
	{
		// TODO: Linux-specific!
		pchar_t modulePath[512];
		readlink("/proc/self/exe", modulePath, sizeof(modulePath));

		pchar_t* dirPtr = strrchr(modulePath, '/');

		dirPtr[1] = '\0';

		citizenPath = modulePath;
	}

	return citizenPath;
}

fwPlatformString GetAbsoluteGamePath()
{
	return GetAbsoluteCitPath();
}

bool IsRunningTests()
{
	// TODO: Linux-specific!
	pchar_t modulePath[512];
	readlink("/proc/self/exe", modulePath, sizeof(modulePath));

	pchar_t* filenamePart = strrchr(modulePath, '/');

	filenamePart++;

	return !_strnicmp(filenamePart, "tests_", 6);
}

void SetThreadName(int dwThreadID, char* threadName)
{
	// TODO: implement
}