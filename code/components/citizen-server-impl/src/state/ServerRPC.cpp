#include "StdInc.h"
#include <ScriptEngine.h>

#include <ServerInstanceBase.h>

#include <NetBuffer.h>

#include <ClientRegistry.h>
#include <GameServer.h>
#include <state/ServerGameState.h>

#include <Resource.h>
#include <ResourceManager.h>
#include <fxScripting.h>

#include <RpcConfiguration.h>

struct EntityCreationState
{
	// TODO: allow resending in case the target client disappears
	uint16_t creationToken;
	uint32_t clientIdx;
	fx::ScriptGuid* scriptGuid;
};

static tbb::concurrent_unordered_map<uint16_t, EntityCreationState> g_entityCreationList;
static std::atomic<uint16_t> g_creationToken;

inline uint32_t MakeEntityHandle(uint8_t playerId, uint16_t objectId)
{
	return ((playerId + 1) << 16) | objectId;
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* ref)
	{
		auto rpcConfiguration = RpcConfiguration::Load("citizen:/scripting/rpc_natives.json");

		auto clientRegistry = ref->GetComponent<fx::ClientRegistry>();
		auto gameState = ref->GetComponent<fx::ServerGameState>();
		auto gameServer = ref->GetComponent<fx::GameServer>();

		gameServer->GetComponent<fx::HandlerMapComponent>()->Add(HashRageString("msgEntityCreate"), [=](const std::shared_ptr<fx::Client>& client, net::Buffer& buffer)
		{
			auto creationToken = buffer.Read<uint16_t>();
			auto objectId = buffer.Read<uint16_t>();

			auto it = g_entityCreationList.find(creationToken);

			if (it != g_entityCreationList.end())
			{
				auto guid = it->second.scriptGuid;
				guid->type = fx::ScriptGuid::Type::Entity;
				guid->entity.handle = MakeEntityHandle(0, objectId);

				// broadcast entity creation
				net::Buffer outBuffer;
				outBuffer.Write<uint32_t>(HashRageString("msgRpcEntityCreation"));
				outBuffer.Write<uint16_t>(creationToken);
				outBuffer.Write<uint16_t>(objectId);

				clientRegistry->ForAllClients([&](const std::shared_ptr<fx::Client>& cl)
				{
					cl->SendPacket(0, outBuffer, ENET_PACKET_FLAG_RELIABLE);
				});

				g_entityCreationList[creationToken] = {};
			}
		});

		for (auto& native : rpcConfiguration->GetNatives())
		{
			fx::ScriptEngine::RegisterNativeHandler(native->GetName(), [=](fx::ScriptContext& ctx)
			{
				int clientIdx = -1;

				if (native->GetRpcType() == RpcConfiguration::RpcType::EntityContext)
				{
					// ascertain the client that is the context
					int ctxIdx = native->GetContextIndex();

					switch (native->GetContextType())
					{
					case RpcConfiguration::ArgumentType::Player:
					{
						clientIdx = ctx.GetArgument<int>(ctxIdx);
						break;
					}
					case RpcConfiguration::ArgumentType::Entity:
					{
						int cxtEntity = ctx.GetArgument<int>(ctxIdx);

						if (cxtEntity < 0x20000)
						{
							auto client = clientRegistry->GetClientByNetID(cxtEntity);

							if (client)
							{
								cxtEntity = std::any_cast<uint32_t>(client->GetData("playerEntity"));
							}
						}

						fx::ScriptGuid* scriptGuid = g_scriptHandlePool->AtHandle(cxtEntity - 0x20000);

						if (scriptGuid)
						{
							if (scriptGuid->type == fx::ScriptGuid::Type::Entity)
							{
								// look up the entity owner
								auto entity = gameState->GetEntity(cxtEntity);

								if (entity)
								{
									clientIdx = entity->client.lock()->GetNetId();
								}
							}
							else if (scriptGuid->type == fx::ScriptGuid::Type::TempEntity)
							{
								auto it = g_entityCreationList.find(scriptGuid->tempEntity.creationToken);

								if (it != g_entityCreationList.end())
								{
									clientIdx = it->second.clientIdx;
								}
							}
						}

						break;
					}
					}
				}
				else if (native->GetRpcType() == RpcConfiguration::RpcType::EntityCreate)
				{
					// #TODO1S: intercept native coordinates and get a client near there
					clientRegistry->ForAllClients([&](const std::shared_ptr<fx::Client>& client)
					{
						if (clientIdx != -1)
						{
							return;
						}

						if (client->GetData("playerEntity").has_value())
						{
							clientIdx = client->GetNetId();
						}
					});
				}

				uint32_t resourceHash = -1;

				fx::OMPtr<IScriptRuntime> runtime;

				if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
				{
					fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

					if (resource)
					{
						resourceHash = HashString(resource->GetName().c_str());
					}
				}

				// build an RPC packet
				net::Buffer buffer;
				buffer.Write<uint32_t>(HashRageString("msgRpcNative"));
				buffer.Write<uint64_t>(native->GetGameHash());
				buffer.Write<uint32_t>(resourceHash);

				uint16_t creationToken;

				if (native->GetRpcType() == RpcConfiguration::RpcType::EntityCreate)
				{
					creationToken = ++g_creationToken;

					buffer.Write<uint16_t>(creationToken);
				}

				int i = 0;
				
				for (auto& argument : native->GetArguments())
				{
					switch (argument.GetType())
					{
					case RpcConfiguration::ArgumentType::Entity:
					{
						int cxtEntity = ctx.GetArgument<int>(i);

						int entityHandle;

						if (cxtEntity < 0x20000)
						{
							auto client = clientRegistry->GetClientByNetID(cxtEntity);

							if (client)
							{
								cxtEntity = std::any_cast<uint32_t>(client->GetData("playerEntity"));
							}
						}
						
						fx::ScriptGuid* scriptGuid = g_scriptHandlePool->AtHandle(cxtEntity - 0x20000);

						if (scriptGuid)
						{
							if (scriptGuid->type == fx::ScriptGuid::Type::Entity)
							{
								auto entity = gameState->GetEntity(cxtEntity);

								if (entity)
								{
									entityHandle = entity->handle;

									buffer.Write<int>(entityHandle);
								}
								else
								{
									return;
								}
							}
							else if (scriptGuid->type == fx::ScriptGuid::Type::TempEntity)
							{
								auto it = g_entityCreationList.find(scriptGuid->tempEntity.creationToken);

								if (it != g_entityCreationList.end())
								{
									buffer.Write<uint32_t>(scriptGuid->tempEntity.creationToken | 0x80000000);
								}
								else
								{
									return;
								}
							}
						}
						else
						{
							return;
						}

						break;
					}
					case RpcConfiguration::ArgumentType::Player:
					{
						int player = ctx.GetArgument<int>(i);
						auto client = clientRegistry->GetClientByNetID(player);

						if (!client)
						{
							return;
						}

						buffer.Write<uint8_t>(client->GetSlotId());

						break;
					}
					case RpcConfiguration::ArgumentType::Int:
					case RpcConfiguration::ArgumentType::Hash:
						buffer.Write<int>(ctx.GetArgument<int>(i));
						break;
					case RpcConfiguration::ArgumentType::Float:
						buffer.Write<float>(ctx.GetArgument<float>(i));
						break;
					case RpcConfiguration::ArgumentType::Bool:
						buffer.Write<uint8_t>(ctx.GetArgument<bool>(i));
						break;
					case RpcConfiguration::ArgumentType::String:
					{
						const char* str = ctx.GetArgument<const char*>(i);
						buffer.Write<uint16_t>(strlen(str));
						buffer.Write(str, strlen(str));
						break;
					}
					}

					++i;
				}

				auto sendToClient = [&](const std::shared_ptr<fx::Client>& cl)
				{
					cl->SendPacket(0, buffer, ENET_PACKET_FLAG_RELIABLE);
				};

				if (clientIdx == -1)
				{
					clientRegistry->ForAllClients(sendToClient);
				}
				else
				{
					sendToClient(clientRegistry->GetClientByNetID(clientIdx));
				}

				if (native->GetRpcType() == RpcConfiguration::RpcType::EntityCreate)
				{
					EntityCreationState state;
					state.creationToken = creationToken;

					auto guid = g_scriptHandlePool->New();
					guid->type = fx::ScriptGuid::Type::TempEntity;
					guid->tempEntity.creationToken = creationToken;

					state.clientIdx = clientIdx;
					state.scriptGuid = guid;

					auto scrHdl = g_scriptHandlePool->GetIndex(guid) + 0x20000;
					g_entityCreationList.insert({ creationToken, state });

					ctx.SetResult(scrHdl);
				}
			});
		}
	}, 99999999);
});
