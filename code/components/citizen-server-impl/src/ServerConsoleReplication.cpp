#include <StdInc.h>

#include <CoreConsole.h>

#include <ClientRegistry.h>
#include <GameServer.h>

#include <ServerInstanceBase.h>

#include <msgpack.hpp>

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		auto console = instance->GetComponent<console::Context>();
		auto gameServer = instance->GetComponent<fx::GameServer>().GetRef();
		auto clientRegistry = instance->GetComponent<fx::ClientRegistry>().GetRef();

		auto sendVariableList = [](fx::Client* client, const std::map<std::string, std::string>& list)
		{
			msgpack::sbuffer buf;
			msgpack::packer<msgpack::sbuffer> packer(buf);

			packer.pack(list);

			// build the target event
			net::Buffer outBuffer;
			outBuffer.Write(HashRageString("msgConVars"));

			// payload
			outBuffer.Write(buf.data(), buf.size());

			client->SendPacket(0, outBuffer, NetPacketType_Reliable);
		};

		clientRegistry->OnConnectedClient.Connect([console, sendVariableList](fx::Client* client)
		{
			std::map<std::string, std::string> variableList;
			
			console->GetVariableManager()->ForAllVariables([&variableList](const std::string& name, int flags, const std::shared_ptr<internal::ConsoleVariableEntryBase>& var)
			{
				variableList.emplace(name, var->GetValue());
			}, ConVar_Replicated);

			sendVariableList(client, variableList);
		});

		gameServer->OnTick.Connect([=]()
		{
			std::map<std::string, std::string> variableList;

			console->GetVariableManager()->ForAllVariables([console, &variableList](const std::string& name, int flags, const std::shared_ptr<internal::ConsoleVariableEntryBase>& var)
			{
				if ((flags & (ConVar_Replicated | ConVar_Modified)) == (ConVar_Replicated | ConVar_Modified))
				{
					variableList.emplace(name, var->GetValue());

					console->GetVariableManager()->RemoveEntryFlags(name, ConVar_Modified);
				}
			}, ConVar_Replicated | ConVar_Modified);

			if (!variableList.empty())
			{
				clientRegistry->ForAllClients([&variableList, sendVariableList](const fx::ClientSharedPtr& client)
				{
					sendVariableList(client.get(), variableList);
				});
			}
		});
	}, 999999);
});
