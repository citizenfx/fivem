/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ComponentLoader.h"

#include "MumbleClient.h"

class ComponentInstance : public Component
{
public:
	virtual bool Initialize();

	virtual bool DoGameLoad(void* module);

	virtual bool Shutdown();
};

bool ComponentInstance::Initialize()
{
	InitFunctionBase::RunAll();

	/*auto mumbleClient = CreateMumbleClient();

	mumbleClient->SetInputDevice("{F406F463-D04A-4709-BFA0-BA597A28CEA9}");

	mumbleClient->Initialize();

	while (true)
	{
		mumbleClient->ConnectAsync(net::PeerAddress::FromString("127.0.0.1:30120").get(), fmt::sprintf("honestly-idk %d", GetTickCount() % 100)).wait();

		bool toggle = false;

		while (true)
		{
			Sleep(2500);

			mumbleClient->SetInputDevice(!toggle ? "" : "{F406F463-D04A-4709-BFA0-BA597A28CEA9}");

			toggle = !toggle;
		}

		mumbleClient->DisconnectAsync().wait();

		Sleep(500);
	}*/

	return true;
}

bool ComponentInstance::DoGameLoad(void* module)
{
	HookFunction::RunAll();

	return true;
}

bool ComponentInstance::Shutdown()
{
	return true;
}

extern "C" __declspec(dllexport) Component* CreateComponent()
{
	return new ComponentInstance();
}
