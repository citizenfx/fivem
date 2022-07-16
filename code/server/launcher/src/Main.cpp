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
#include <stdio.h>
#endif

bool InitializeExceptionHandler(int argc, char* argv[]);

int main(int argc, char* argv[])
{
#ifndef _WIN32
	printf("- ERROR -\n");
	printf("The Cfx.re platform server is currently not working on Linux.\n");
	printf("Use platform version 5715 or below (or the current recommended) instead. This issue will be fixed in the near future.\n\n");
	printf("See https://forum.cfx.re/t/4881227/6 for more information and context.\n");
	return 2;

	pthread_attr_t attrs;
	if (pthread_getattr_default_np(&attrs) == 0)
	{
		pthread_attr_setstacksize(&attrs, 4 * 1024 * 1024);
		pthread_setattr_default_np(&attrs);
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
