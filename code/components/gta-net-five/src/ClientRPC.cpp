#include <StdInc.h>
#include <ScriptEngine.h>
#include <RpcConfiguration.h>

#include <fxScripting.h>

#include <NetLibrary.h>
#include <NetBuffer.h>

#include <Resource.h>
#include <ResourceManager.h>

#include <NetworkPlayerMgr.h>

static inline int GetServerId(const ScPlayerData* platformData)
{
	return (platformData->addr.ipLan & 0xFFFF) ^ 0xFEED;
}

extern NetLibrary* g_netLibrary;

#include <nutsnbolts.h>

#include <concurrent_queue.h>

static std::shared_ptr<RpcConfiguration> g_rpcConfiguration;

class RpcNextTickQueue : public fwRefCountable, public fx::IAttached<fx::Resource>
{
public:
	virtual void AttachToObject(fx::Resource* resource) override
	{
		resource->OnTick.Connect([=]()
		{
			std::function<void()> fn;

			while (m_queue.try_pop(fn))
			{
				fn();
			}
		});
	}

	void Enqueue(const std::function<void()>& fn)
	{
		m_queue.push(fn);
	}

private:
	concurrency::concurrent_queue<std::function<void()>> m_queue;
};

class DummyScriptEnvironment : public fx::OMClass<DummyScriptEnvironment, IScriptRuntime>
{
public:
	NS_DECL_ISCRIPTRUNTIME;

private:
	void* m_parentObject;
};

result_t DummyScriptEnvironment::Create(IScriptHost *scriptHost)
{
	return FX_S_OK;
}

result_t DummyScriptEnvironment::Destroy()
{
	return FX_S_OK;
}

int DummyScriptEnvironment::GetInstanceId()
{
	return 1;
}

void* DummyScriptEnvironment::GetParentObject()
{
	return m_parentObject;
}

void DummyScriptEnvironment::SetParentObject(void* object)
{
	m_parentObject = object;
}

DummyScriptEnvironment g_se;

DECLARE_INSTANCE_TYPE(RpcNextTickQueue);

int ObjectToEntity(int objectId);

static InitFunction initFunction([]()
{
	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		resource->SetComponent(new RpcNextTickQueue());
	});

	OnGameFrame.Connect([]()
	{
		if (g_netLibrary == nullptr)
		{
			return;
		}

		if (g_netLibrary)
		{
			static bool g_netlibHookInited;

			if (!g_netlibHookInited)
			{
				g_se.AddRef();

				g_rpcConfiguration = RpcConfiguration::Load("citizen:/scripting/rpc_natives.json");

				g_netLibrary->AddReliableHandler("msgRpcNative", [](const char* data, size_t len)
				{
					net::Buffer buf(reinterpret_cast<const uint8_t*>(data), len);

					auto nativeHash = buf.Read<uint64_t>();
					auto resourceHash = buf.Read<uint32_t>();

					// TODO: FIXME: optimize this, it's O(n)
					fx::Resource* resource = nullptr;
					fx::ResourceManager* manager = Instance<fx::ResourceManager>::Get();

					manager->ForAllResources([&](fwRefContainer<fx::Resource> thisResource)
					{
						if (HashString(thisResource->GetName().c_str()) == resourceHash)
						{
							resource = thisResource.GetRef();
						}
					});

					std::shared_ptr<RpcConfiguration::Native> native;

					for (auto& n : g_rpcConfiguration->GetNatives())
					{
						if (n->GetGameHash() == nativeHash)
						{
							native = n;
							break;
						}
					}

					if (!native || !resource)
					{
						return;
					}

					auto executionCtx = std::make_shared<fx::ScriptContext>();

					int i = 0;

					for (auto& argument : native->GetArguments())
					{
						switch (argument.GetType())
						{
						case RpcConfiguration::ArgumentType::Player:
						{
							int id = buf.Read<int>();

							for (int i = 0; i < 32; i++)
							{
								CNetGamePlayer* player = CNetworkPlayerMgr::GetPlayer(i);

								if (player)
								{
									auto platformData = player->GetPlatformPlayerData();

									if (GetServerId(platformData) == id)
									{
										executionCtx->Push(i);
										break;
									}
								}
							}

							break;
						}
						case RpcConfiguration::ArgumentType::Entity:
						{
							int entity = buf.Read<int>();

							executionCtx->Push(ObjectToEntity(entity));

							break;
						}
						case RpcConfiguration::ArgumentType::Int:
							executionCtx->Push(buf.Read<int>());
							break;
						case RpcConfiguration::ArgumentType::Float:
							executionCtx->Push(buf.Read<float>());
							break;
						case RpcConfiguration::ArgumentType::Bool:
							executionCtx->Push(buf.Read<uint8_t>());
							break;
						case RpcConfiguration::ArgumentType::String:
						{
							// TODO: actually store a string
							executionCtx->Push((const char*)"");
							break;
						}
						}

						++i;
					}

					if (resource)
					{
						auto ntq = resource->GetComponent<RpcNextTickQueue>();

						ntq->Enqueue([=]()
						{
							g_se.SetParentObject(resource);

							fx::PushEnvironment pushed(&g_se);
							auto n = fx::ScriptEngine::GetNativeHandler(nativeHash);

							if (n)
							{
								(*n)(*executionCtx);
							}
						});
					}
				});

				g_netlibHookInited = true;
			}
		}
	});
});
