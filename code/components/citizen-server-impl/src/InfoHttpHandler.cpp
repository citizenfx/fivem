#include "StdInc.h"

#include <ClientRegistry.h>

#include <ServerInstanceBase.h>
#include <HttpServerManager.h>

#include <json.hpp>

using json = nlohmann::json;

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->GetComponent<fx::HttpServerManager>()->AddEndpoint("/info.json", [=](const fwRefContainer<net::HttpRequest>& request, const fwRefContainer<net::HttpResponse>& response)
		{
			json data{
				{"enhancedHostSupport", true},
				{"server", "FXServer"},
				{"version", 1},
				{"resources", json::array({"hardcap"})},
				{"icon", "iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAIAAAAlC+aJAAAAAXNSR0IArs4c6QAAAARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAHDSURBVGhD7dPRjuswCEXR+f+f7kTxJkrrkACFuFW9nq4wEM5U9+/x5X4vwN8zquP4LuDqI3TkYe/VZseH2WfDTBRbVpQUVQEaJp0YFlQVwQCUbKloNWNMUFW8G6Chqtj3tH+faG0bqrqcAC9oEocVDR2Cqq4kwAtmOjzv8CConrKewkpB1YyxI3REP3FTgIbhI/1rG7kUCUApii2naDWIBNjwFsIKBU0Gpla26ujzY15B06mcAA3dfswraFJkBmiYMWPsCt0ddwBKVx+myYAB0Vea1tyLB9jw8Iy3K3QLqoLqilInIUDDs6B6hW5B1SMtQAAbBVWnTwlAyW9YANYJqn4zQAi7BNWQMQEWrFtRCrEO86m8AFk+7iCvGWC0GWDFf3AdfQUSVnOjDTN53t3IXU4MZ/j5AAuOWlF6xtsODxmSAyyo3mUGWHH7Dg/1qgI0PFdK+wYnK2gqkLyaexU0par623Byh+c8lT+uguckhQE2HL7DQ4Y7Aiw4XFDNMH+BU9zb4TlDYQCO7fCcpCQAlx6hI09wI+d4MJktspeLzBir4d7OUTbMVCr8BegudtNn6swAo80Ao80Ao80Ao315gMfjH5DJergQ/xTsAAAAAElFTkSuQmCC"}
			};

			response->End(data.dump());
		});

		instance->GetComponent<fx::HttpServerManager>()->AddEndpoint("/players.json", [=](const fwRefContainer<net::HttpRequest>& request, const fwRefContainer<net::HttpResponse>& response)
		{
			auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

			json data = json::array();

			clientRegistry->ForAllClients([&](const std::shared_ptr<fx::Client>& client)
			{
				if (client->GetNetId() >= 0xFFFF)
				{
					return;
				}

				data.push_back({
					{ "endpoint", client->GetAddress().ToString() },
					{ "id", client->GetNetId() },
					{ "identifiers", client->GetIdentifiers() },
					{ "name", client->GetName() },
					{ "ping", client->GetPeer()->roundTripTime }
				});
			});

			response->End(data.dump());
		});
	});
});
