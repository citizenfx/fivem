/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ComponentLoader.h"

#include <ServerInstance.h>

class ServerComponentInstance : public RunnableComponent
{
private:
	fwRefContainer<fx::ServerInstance> m_server;

public:
	virtual bool Initialize(const std::string& args);

	virtual bool DoGameLoad(HANDLE module);

	virtual bool Shutdown();

	virtual void Run();
};

bool ServerComponentInstance::Initialize(const std::string& args)
{
	InitFunctionBase::RunAll();

	m_server = new fx::ServerInstance();
	m_server->SetArguments(args);

	return true;
}

void ServerComponentInstance::Run()
{
	m_server->Run();
}

bool ServerComponentInstance::DoGameLoad(HANDLE module)
{
	FatalError("ServerComponentInstance should not be loaded into a game instance.");

	return false;
}

bool ServerComponentInstance::Shutdown()
{
	return true;
}

extern "C" __declspec(dllexport) Component* CreateComponent()
{
	return new ServerComponentInstance();
}