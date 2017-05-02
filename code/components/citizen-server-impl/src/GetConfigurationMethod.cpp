		#include "StdInc.h"
#include <ClientHttpHandler.h>

#include <ServerInstanceBase.h>

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->GetComponent<fx::ClientMethodRegistry>()->AddHandler("getConfiguration", [=](std::map<std::string, std::string>& postMap, const fwRefContainer<net::HttpRequest>& request)
		{
			return json::object({
				{ "fileServer", "http://localhost:8000/files" },
				{ "resources", json::array({
					json::object({
						{ "name", "spawnmanager" },
						{ "files", {
							{ "resource.rpf", "885154A635EA296DC168DC046089BEBA5140DC4E" }
						}},
						{ "streamFiles", json::object() }
					}),

					json::object({
						{ "name", "fivem" },
						{ "files",{
							{ "resource.rpf", "713E007005A2651FDEA45AC2AFB1E1DEF44675BD" }
						} },
						{ "streamFiles",json::object() }
					}),

					json::object({
						{ "name", "spawnmanager" },
						{ "files",{
							{ "resource.rpf", "885154A635EA296DC168DC046089BEBA5140DC4E" }
						} },
						{ "streamFiles",json::object() }
					}),

					json::object({
						{ "name", "fivem-map-skater" },
						{ "files",{
							{ "resource.rpf", "7DDD435700B2B3DC7B90E42E922BEE6B132885B8" }
						} },
						{ "streamFiles",json::object() }
					}),
				})}
			});
		});
	});
});