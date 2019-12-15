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

bool InitializeExceptionHandler();

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

	if (InitializeExceptionHandler())
	{
		return 0;
	}

	fx::Server server;
	server.Start(argc, argv);
}
