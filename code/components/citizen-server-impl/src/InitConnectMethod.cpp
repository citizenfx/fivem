#include "StdInc.h"
#include <ClientHttpHandler.h>

#include <ServerInstanceBase.h>

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->GetComponent<fx::ClientMethodRegistry>()->AddHandler("initConnect", [=](std::map<std::string, std::string>& postMap, const fwRefContainer<net::HttpRequest>& request)
		{
			auto name = postMap["name"];
			auto guid = postMap["guid"];

			auto protocol = postMap["protocol"];

			if (name.empty() || guid.empty() || protocol.empty())
			{
				return json::object({ {"error", "fields missing"} });
			}

			json json = json::object();
			json["protocol"] = 4;
			json["sH"] = true;
			json["token"] = "lol";
			json["netlibVersion"] = 2;

			return json;
		});
	}, 50);
});