#include "StdInc.h"
#include <terminal.h>

using namespace terminal;

class CORE_EXPORT Component : public fwRefCountable
{
public:
	virtual bool Initialize() = 0;

	virtual bool Shutdown() = 0;

	virtual bool DoGameLoad(HANDLE hModule);
};

extern "C" Component* CreateComponent();

int main()
{
	Component* component = CreateComponent();
	component->Initialize();

	fwRefContainer<IClient> client = terminal::Create<IClient>();

	client->ConnectRemote("localhost", 3036).then([] (Result<ConnectRemoteDetail> result)
	{
		//__debugbreak();
	});

	while (true)
	{
		Sleep(50);
	}

	return 0;
}