/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Utils.h"

#include <pthread.h>

fwPlatformString GetAbsoluteCitPath()
{
	static fwPlatformString citizenPath;

	if (!citizenPath.size())
	{
		// TODO: Linux-specific!
		pchar_t modulePath[512];

		ssize_t off = readlink("/proc/self/exe", modulePath, sizeof(modulePath) - 1);
		assert(off >= 0);

		modulePath[off] = '\0';

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
	ssize_t off = readlink("/proc/self/exe", modulePath, sizeof(modulePath) - 1);
	assert(off >= 0);

	modulePath[off] = '\0';

	pchar_t* filenamePart = strrchr(modulePath, '/');

	filenamePart++;

	return !_strnicmp(filenamePart, "tests_", 6);
}

void SetThreadName(int dwThreadID, const char* threadName)
{
	// this is limited to 15 characters on Linux
	std::string shortenedName = std::string(threadName).substr(0, 15);
	
	pthread_setname_np(pthread_self(), shortenedName.c_str());
}
