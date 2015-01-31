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
			tokenBag.AddToken(TokenType::ROS, "QhDsq/pnTuVSTlCeMoM7TT6YZwDhLkyZrwfuLX2bUUCifTb0uJTrEE9JPVzr7dR7yYdk9Pc1UFGmT/STkEWXfafiYBLcSLxZcEUAT7FdpEuDBf4m113xTyrfWjgIw0XK0vrCDraI/xqkulodr6yJQ4wppsWy05fTSZ/z8r25uv1h2G7rLOXBElQ22xUY1CmODytl&&40366");
			tokenBag.AddToken(TokenType::ROS, "hhBGK3wGkCbjCjySn7vJIrcyXwDfBdJfzHhkJ/KnXNm2ZnSKcWJwX+Wlwf362e4ON6vZFIpLHhLrRr53i2HATTqjCMElHR2sqY6WFk05I9b+GBe0ynmWqcB/gD3IIPm9le+7T8/ZaDYgNIQXuIV3+R4JkKy+hRHgB3AR0dLYURi9Sfb4YLJQ3MBKzA==&&51216702");

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