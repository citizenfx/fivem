/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include "Server.h"

#ifndef _WIN32
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <pthread.h>
#endif

#include "cpuinfo_x86.h"
#include "Error.h"

bool InitializeExceptionHandler(int argc, char* argv[]);

int utf8main(int argc, char* argv[])
{
#ifndef _WIN32
	pthread_attr_t attrs;
	if (pthread_getattr_default_np(&attrs) == 0)
	{
		pthread_attr_setstacksize(&attrs, 4 * 1024 * 1024);
		pthread_setattr_default_np(&attrs);
	}
#endif

#ifdef CPU_FEATURES_ARCH_X86
	auto x86info = cpu_features::GetX86Info();

	bool ssse3 = x86info.features.ssse3;

	// manually check for SSSE3 on Windows (PF_SSSE3_INSTRUCTIONS_AVAILABLE used by cpu_features does not exist below Vibranium
	// and the code path in cpu_features for CPUs w/o AVX support - such as Westmere - depends on such)
#ifdef _WIN32
	if (!ssse3)
	{
		int cpuid[4] = { 0 };
		__cpuid(cpuid, 1); // leaf 1, field 2 contains 'Feature Information'

		ssse3 = (cpuid[2] >> 9) & 1; // bit 9 is SSSE3
	}
#endif

	if (!x86info.features.popcnt || !ssse3)
	{
		std::string errorMessage = fmt::sprintf(
		"The Cfx.re Platform Server requires support for x86-64-v2 instructions (such as POPCNT).\n"
		"Your current CPU (\"%s\") does not appear to support this. Supported CPUs include most CPUs from around 2010 or newer.\n"
		"\n"
		"The server will exit now.",
		x86info.brand_string);

		if (strcmp(x86info.brand_string, "Common KVM processor") == 0)
		{
			errorMessage =
				"The Cfx.re Platform Server requires support for x86-64-v2 instructions (such as POPCNT).\n"
				"You seem to be running on QEMU/KVM using the 'kvm64' CPU type, which does not properly indicate support for these instructions.\n"
				"Please use a different CPU type (such as 'host'). See https://aka.cfx.re/fxs-kvm64 for more information.";
		}

#ifdef _WIN32
		FatalError("%s", errorMessage);
#else
		fmt::printf("%s\n", errorMessage);
#endif

		return 1;
	}
#endif

	if (InitializeExceptionHandler(argc, argv))
	{
		return 0;
	}

	fx::Server server;
	server.Start(argc, argv);

	return 1;
}

#ifdef _WIN32
// Supplementary wmain procedure for Windows builds, this is needed to receive non-ANSI characters in argv[]
// Standard main(int argc, char* argv[]) treats input as ANSI instead of UTF8, turning non-ANSI characters into '?'
int wmain(int argc, wchar_t* argv[])
{
	static std::vector<std::string> argvStrs;
	static std::vector<char*> argvRefs;

	argvStrs.resize(argc);
	argvRefs.resize(argc);

	for (int i = 0; i < argc; i++)
	{
		argvStrs[i] = ToNarrow(argv[i]);
		argvRefs[i] = &argvStrs[i][0];
	}

	// Set current locale to UTF8, this is needed for CRT IO functions (such as fopen) to work
	// correctly with Unicode paths that use non-ANSI characters, otherwise they turn into '?'
	setlocale(LC_CTYPE, ".utf8");

	return utf8main(argc, argvRefs.data());
}
#else
int main(int argc, char* argv[])
{
	return utf8main(argc, argv);
}
#endif
