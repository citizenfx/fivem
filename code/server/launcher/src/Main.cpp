/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include "Server.h"

int main(int argc, char* argv[])
{
	fx::Server server;
	server.Start(argc, argv);
}