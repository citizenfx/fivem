/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include "Server.h"

#ifndef _WIN32
#define _GNU_SOURCE
#include <pthread.h>
#endif

#include "cpuinfo_x86.h"

bool InitializeExceptionHandler(int argc, char* argv[]);

int main(int argc, char* argv[])
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
	if (!x86info.features.popcnt)
	{
		fmt::printf("The Cfx.re Platform Server requires support for x86-64-v2 instructions (such as POPCNT).\n");
		fmt::printf("Your current CPU (\"%s\") does not appear to support this. Supported CPUs include most CPUs from around 2010 or newer.\n", x86info.brand_string);
		fmt::printf("\n");
		fmt::printf("Exiting.\n");
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
