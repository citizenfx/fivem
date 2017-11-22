/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"

#include "Server.h"

int main(int argc, char* argv[])
{
	fx::Server server;
	server.Start(argc, argv);
}