/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
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