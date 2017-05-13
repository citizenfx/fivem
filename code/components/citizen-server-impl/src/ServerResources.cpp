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

	virtual pplx::task<fwRefContainer<fx::Resource>> LoadResource(const std::string& uri) override
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
#ifdef _WIN32
				std::string pr = pathRef->substr(1).to_string();
#else
				std::string pr = pathRef->to_string();
#endif
				network::uri::decode(pr.begin(), pr.end(), std::back_inserter(path));

				resource = m_manager->CreateResource(fragRef->to_string());
				resource->LoadFrom(std::string(path.begin(), path.begin() + path.size()));
			}
		}

		return pplx::task_from_result<fwRefContainer<fx::Resource>>(resource);
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

static void ScanResources(fx::ServerInstanceBase* instance)
{
	auto resMan = instance->GetComponent<fx::ResourceManager>();

	std::string resourceRoot(instance->GetRootPath() + "/resources/");

	std::queue<std::string> pathsToIterate;
	pathsToIterate.push(resourceRoot);

	std::vector<pplx::task<fwRefContainer<fx::Resource>>> tasks;

	while (!pathsToIterate.empty())
	{
		std::string thisPath = pathsToIterate.front();
		pathsToIterate.pop();

		auto vfsDevice = vfs::GetDevice(thisPath);

		vfs::FindData findData;
		auto handle = vfsDevice->FindFirst(thisPath, &findData);

		if (handle != INVALID_DEVICE_HANDLE)
		{
			do
			{
				if (findData.name[0] == '.')
				{
					continue;
				}

				// TODO(fxserver): non-win32
				if (findData.attributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					std::string resPath(thisPath + "/" + findData.name);

					// is this a category?
					if (findData.name[0] == '[' && findData.name[findData.name.size() - 1] == ']')
					{
						pathsToIterate.push(resPath);
					}
					// it's a resource
					else
					{
						tasks.push_back(resMan->AddResource(network::uri_builder{}
							.scheme("file")
							.host("")
							.path(resPath)
							.fragment(findData.name)
							.uri().string()));
					}
				}
			} while (vfsDevice->FindNext(handle, &findData));

			vfsDevice->FindClose(handle);
		}
	}

	pplx::when_all(tasks.begin(), tasks.end()).wait();
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->SetComponent(fx::CreateResourceManager());
		instance->SetComponent(new fx::ServerEventComponent());

		fwRefContainer<fx::ResourceManager> resman = instance->GetComponent<fx::ResourceManager>();
		resman->SetComponent(new fx::ServerInstanceBaseRef(instance));

		resman->AddMounter(new LocalResourceMounter(resman.GetRef()));

		instance->OnReadConfiguration.Connect([=](const boost::property_tree::ptree& pt)
		{
			vfs::Mount(new vfs::RelativeDevice(pt.get<std::string>("server.citizen_dir")), "citizen:/");
			vfs::Mount(new vfs::RelativeDevice(instance->GetRootPath() + "/cache/"), "cache:/");

			ScanResources(instance);

			// start all auto-start resources
			for (auto& child : pt.get_child("server"))
			{
				if (child.first != "resource")
				{
					continue;
				}

				auto resourceName = child.second.get_optional<std::string>("<xmlattr>.name").get_value_or(
					child.second.get_value_optional<std::string>().get_value_or(
						""
					)
				);

				if (resourceName.empty())
				{
					continue;
				}

				auto resource = resman->GetResource(resourceName);

				if (!resource.GetRef())
				{
					trace("^3Couldn't find auto-started resource %s.^7\n", resourceName);
					continue;
				}

				if (!resource->Start())
				{
					trace("^3Couldn't start resource %s.^7\n", resourceName);
					continue;
				}
			}
		});

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

	fx::ScriptEngine::RegisterNativeHandler("START_RESOURCE", [](fx::ScriptContext& context)
	{
		auto resourceManager = fx::ResourceManager::GetCurrent();
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(context.GetArgument<const char*>(0));

		bool success = false;

		if (resource.GetRef())
		{
			success = resource->Start();
		}

		context.SetResult(success);
	});

	fx::ScriptEngine::RegisterNativeHandler("STOP_RESOURCE", [](fx::ScriptContext& context)
	{
		auto resourceManager = fx::ResourceManager::GetCurrent();
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(context.GetArgument<const char*>(0));

		bool success = false;

		if (resource.GetRef())
		{
			success = resource->Stop();
		}

		context.SetResult(success);
	});
});
