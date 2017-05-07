#include "StdInc.h"

#include <ResourceManager.h>
#include <ResourceEventComponent.h>

#include <ServerInstanceBase.h>

#include <GameServer.h>
#include <ServerEventComponent.h>

#include <RelativeDevice.h>

#include <VFSManager.h>

#include <network/uri.hpp>

class LocalResourceMounter : public fx::ResourceMounter
{
public:
	LocalResourceMounter(fx::ResourceManager* manager)
		: m_manager(manager)
	{
		
	}

	virtual bool HandlesScheme(const std::string& scheme) override
	{
		return (scheme == "file");
	}

	virtual concurrency::task<fwRefContainer<fx::Resource>> LoadResource(const std::string& uri) override
	{
		std::error_code ec;
		auto uriParsed = network::make_uri(uri, ec);

		fwRefContainer<fx::Resource> resource;

		if (!ec)
		{
			auto pathRef = uriParsed.path();
			auto fragRef = uriParsed.fragment();

			if (pathRef && fragRef)
			{
				std::vector<char> path;
				std::string pr = pathRef->substr(1).to_string();
				network::uri::decode(pr.begin(), pr.end(), std::back_inserter(path));

				resource = m_manager->CreateResource(fragRef->to_string());
				resource->LoadFrom(std::string(path.begin(), path.begin() + path.size()));
				resource->Start();
			}
		}

		return concurrency::task_from_result<fwRefContainer<fx::Resource>>(resource);
	}

private:
	fx::ResourceManager* m_manager;
};

static void HandleServerEvent(fx::ServerInstanceBase* instance, const std::shared_ptr<fx::Client>& client, net::Buffer& buffer)
{
	uint16_t eventNameLength = buffer.Read<uint16_t>();

	std::vector<char> eventNameBuffer(eventNameLength - 1);
	buffer.Read(eventNameBuffer.data(), eventNameBuffer.size());
	buffer.Read<uint8_t>();

	uint32_t dataLength = buffer.GetRemainingBytes();

	std::vector<uint8_t> data(dataLength);
	buffer.Read(data.data(), data.size());

	fwRefContainer<fx::ResourceManager> resourceManager = instance->GetComponent<fx::ResourceManager>();
	fwRefContainer<fx::ResourceEventManagerComponent> eventManager = resourceManager->GetComponent<fx::ResourceEventManagerComponent>();

	eventManager->QueueEvent(
		std::string(eventNameBuffer.begin(), eventNameBuffer.end()),
		std::string(data.begin(), data.end()),
		fmt::sprintf("net:%d", client->GetNetId())
	);
}

namespace fx
{
	class ServerInstanceBaseRef : public fwRefCountable
	{
	public:
		ServerInstanceBaseRef(ServerInstanceBase* instance)
			: m_ref(instance)
		{
			
		}

		inline ServerInstanceBase* Get()
		{
			return m_ref;
		}

	private:
		ServerInstanceBase* m_ref;
	};
}

DECLARE_INSTANCE_TYPE(fx::ServerInstanceBaseRef);

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->SetComponent(fx::CreateResourceManager());
		instance->SetComponent(new fx::ServerEventComponent());

		vfs::Mount(new vfs::RelativeDevice("C:/fivem/data/citizen/"), "citizen:/");

		fwRefContainer<fx::ResourceManager> resman = instance->GetComponent<fx::ResourceManager>();
		resman->SetComponent(new fx::ServerInstanceBaseRef(instance));

		resman->AddMounter(new LocalResourceMounter(resman.GetRef()));
		resman->AddResource("file:///C:/cfx-server-data/resources/%5Bsystem%5D/spawnmanager#spawnmanager");
		resman->AddResource("file:///C:/cfx-server-data/resources/%5Bsystem%5D/chat#chat");

		auto gameServer = instance->GetComponent<fx::GameServer>();
		gameServer->GetComponent<fx::HandlerMapComponent>()->Add(HashRageString("msgServerEvent"), std::bind(&HandleServerEvent, instance, std::placeholders::_1, std::placeholders::_2));
		gameServer->OnTick.Connect([=]()
		{
			resman->Tick();
		});
	}, 50);
});

#include <ScriptEngine.h>
#include <optional>

void fx::ServerEventComponent::TriggerClientEvent(const std::string_view& eventName, const void* data, size_t dataLen, const std::optional<std::string_view>& targetSrc)
{
	// build the target event
	net::Buffer outBuffer;
	outBuffer.Write(0x7337FD7A);

	// source netId
	outBuffer.Write<uint16_t>(-1);

	// event name
	outBuffer.Write<uint16_t>(eventName.size() + 1);
	outBuffer.Write(eventName.data(), eventName.size());
	outBuffer.Write<uint8_t>(0);

	// payload
	outBuffer.Write(data, dataLen);

	// get the game server and client registry
	auto gameServer = m_instance->GetComponent<fx::GameServer>();
	auto clientRegistry = m_instance->GetComponent<fx::ClientRegistry>();

	// do we have a specific client to send to?
	if (targetSrc)
	{
		int targetNetId = atoi(targetSrc->substr(4).data());
		auto client = clientRegistry->GetClientByNetID(targetNetId);

		if (client)
		{
			// TODO(fxserver): >MTU size?
			client->SendPacket(0, outBuffer, ENET_PACKET_FLAG_RELIABLE);
		}
	}
	else
	{
		clientRegistry->ForAllClients([&](const std::shared_ptr<fx::Client>& client)
		{
			client->SendPacket(0, outBuffer, ENET_PACKET_FLAG_RELIABLE);
		});
	}
}

static InitFunction initFunction2([]()
{
	fx::ScriptEngine::RegisterNativeHandler("TRIGGER_CLIENT_EVENT_INTERNAL", [](fx::ScriptContext& context)
	{
		std::string_view eventName = context.GetArgument<const char*>(0);
		std::optional<std::string_view> targetSrc;

		if (context.GetArgument<int>(1) != -1)
		{
			targetSrc = context.GetArgument<const char*>(1);
		}

		const void* data = context.GetArgument<const void*>(2);
		uint32_t dataLen = context.GetArgument<uint32_t>(3);

		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		instance->GetComponent<fx::ServerEventComponent>()->TriggerClientEvent(eventName, data, dataLen, targetSrc);
	});
});