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

	client->ConnectRemote("layer1://localhost:3036").then([=] (Result<ConnectRemoteDetail> result)
	{
		if (result.HasSucceeded())
		{
			IUser1* user = static_cast<IUser1*>(client->GetUserService(IUser1::InterfaceID).GetDetail());

			TokenBag tokenBag;


			//user->AuthenticateWithLicenseKey("").then([=] (Result<AuthenticateDetail> result)
			user->AuthenticateWithTokenBag(tokenBag).then([=] (Result<AuthenticateDetail> result)
			{
				if (result.HasSucceeded())
				{
					printf("our npid may be %llx\n", user->GetNPID());
				}
				else
				{
					__debugbreak();
				}
			});
		}
	});

	while (true)
	{
		Sleep(50);
	}

	return 0;
}