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

#include <CoreConsole.h>

#include <random>

#define GLM_ENABLE_EXPERIMENTAL

// TODO: clang style defines/checking
#if defined(_M_IX86) || defined(_M_AMD64)
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_SSE2
#define GLM_FORCE_SSE3
#endif

#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp>

tbb::concurrent_unordered_map<uint32_t, fx::EntityCreationState> g_entityCreationList;
static std::minstd_rand g_creationToken;
static std::linear_congruential_engine<uint32_t, 12820163, 0, (1 << 24) - 1> g_objectToken;

inline uint32_t MakeEntityHandle(uint16_t objectId)
{
	return objectId;
}

namespace fx
{
	glm::vec3 GetPlayerFocusPos(const fx::sync::SyncEntityPtr& entity);
}

static tbb::concurrent_unordered_map<uint32_t, std::list<std::tuple<uint64_t, net::Buffer>>> g_replayList;

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* ref)
	{
		auto rpcConfiguration = RpcConfiguration::Load("citizen:/scripting/rpc_natives.json");

		if (!rpcConfiguration)
		{
			console::PrintWarning("server", "Could not load rpc_natives.json. Is the server running from the correct directory, and is citizen_dir set?\n");
			return;
		}

		auto clientRegistry = ref->GetComponent<fx::ClientRegistry>();
		auto gameState = ref->GetComponent<fx::ServerGameState>();
		auto gameServer = ref->GetComponent<fx::GameServer>();

		clientRegistry->OnClientCreated.Connect([](const fx::ClientSharedPtr& client)
		{
			fx::Client* unsafeClient = client.get();
			unsafeClient->OnCreatePed.Connect([unsafeClient]()
			{
				for (auto& entry : g_replayList)
				{
					if (!entry.second.empty())
					{
						for (auto& [ native, buffer ] : entry.second)
						{
							unsafeClient->SendPacket(0, buffer, NetPacketType_ReliableReplayed);
						}
					}
				}
			});
		});

		gameState->OnEntityCreate.Connect([](fx::sync::SyncEntityPtr entity)
		{
			auto creationToken = entity->creationToken;
			auto objectId = entity->handle;

			auto it = g_entityCreationList.find(creationToken);

			if (it != g_entityCreationList.end())
			{
				auto guid = it->second.scriptGuid;

				if (guid && guid->type == fx::ScriptGuid::Type::TempEntity)
				{
					guid->type = fx::ScriptGuid::Type::Entity;
					guid->entity.handle = MakeEntityHandle(objectId);
				}

				g_entityCreationList[creationToken] = {};
			}
		});

		struct scrVector
		{
			float x;
		private:
			int32_t pad;
		public:
			float y;

		private:
			int32_t pad2;
		public:
			float z;

		private:
			int32_t pad3;
		};

		static std::map<std::tuple<uint32_t, uint64_t>, std::optional<std::variant<uint32_t, scrVector>>> nativeResults;

		for (auto& native : rpcConfiguration->GetNatives())
		{
			// RPC NATIVE
			fx::ScriptEngine::RegisterNativeHandler(native->GetName(), [=](fx::ScriptContext& ctx)
			{
				bool replay = false;
				bool delThis = false;
				uint32_t delId = 0;
				int clientIdx = -1;
				uint32_t contextId = 0;
				fx::sync::SyncEntityPtr entity;

				if (native->GetRpcType() == RpcConfiguration::RpcType::EntityContext)
				{
					// ascertain the client that is the context
					int ctxIdx = native->GetContextIndex();

					switch (native->GetContextType())
					{
					case RpcConfiguration::ArgumentType::Player:
					{
						clientIdx = ctx.GetArgument<int>(ctxIdx);
						contextId = clientIdx;
						break;
					}
					case RpcConfiguration::ArgumentType::ObjRef:
					{
						contextId = ctx.GetArgument<int>(ctxIdx);
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

						contextId = cxtEntity;

						fx::ScriptGuid* scriptGuid = g_scriptHandlePool->AtHandle(cxtEntity - 0x20000);

						if (scriptGuid)
						{
							if (scriptGuid->type == fx::ScriptGuid::Type::Entity)
							{
								// look up the entity owner
								entity = gameState->GetEntity(cxtEntity);

								if (entity)
								{
									auto client = entity->GetClient();

									if (client)
									{
										clientIdx = client->GetNetId();
									}
									else
									{
										clientIdx = -2;
									}
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
				else if (native->GetRpcType() == RpcConfiguration::RpcType::ObjectCreate)
				{
					clientIdx = -1;
					replay = true;
				}
				else if (native->GetRpcType() == RpcConfiguration::RpcType::EntityCreate)
				{
					// 3 floats next to one another = pos
					int flts = 0;
					int startIdx = 0;
					int idx = 0;
					bool found = false;

					for (auto& argument : native->GetArguments())
					{
						if (argument.GetType() == RpcConfiguration::ArgumentType::Float)
						{
							flts++;

							if (flts == 1)
							{
								startIdx = idx;
							}
							else if (flts == 3)
							{
								found = true;
								break;
							}
						}
						else
						{
							flts = 0;
						}

						idx++;
					}

					// if no pos-style argument, route to first client we find
					if (!found)
					{
						clientRegistry->ForAllClients([&](const fx::ClientSharedPtr& client)
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
					else
					{
						// route to a nearby candidate
						std::vector<std::tuple<float, fx::ClientSharedPtr>> candidates;

						glm::vec3 pos{ ctx.GetArgument<float>(startIdx), ctx.GetArgument<float>(startIdx + 1), ctx.GetArgument<float>(startIdx + 2) };

						clientRegistry->ForAllClients([&candidates, gameState, pos](const fx::ClientSharedPtr& tgtClient)
						{
							if (tgtClient->GetSlotId() == 0xFFFFFFFF)
							{
								return;
							}

							float distance = std::numeric_limits<float>::max();

							try
							{
								fx::sync::SyncEntityPtr playerEntity;

								{
									auto [lock, data] = gameState->ExternalGetClientData(tgtClient);
									playerEntity = data->playerEntity.lock();
								}

								if (playerEntity)
								{
									auto tgt = fx::GetPlayerFocusPos(playerEntity);

									distance = glm::distance2(tgt, pos);
								}
							}
							catch (std::bad_any_cast&)
							{

							}

							candidates.emplace_back(distance, tgtClient);
						});

						std::sort(candidates.begin(), candidates.end());

						if (candidates.size() == 0)
						{
							// TODO: add replaying when a player exists + gets in proximity
							ctx.SetResult(0);
							return;
						}

						clientIdx = std::get<1>(candidates[0])->GetNetId();
					}
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

				uint32_t creationToken = 0;

				if (native->GetRpcType() == RpcConfiguration::RpcType::EntityCreate)
				{
					creationToken = g_creationToken();

					buffer.Write<uint32_t>(creationToken);
				}
				else if (native->GetRpcType() == RpcConfiguration::RpcType::ObjectCreate)
				{
					creationToken = g_objectToken();

					buffer.Write<uint32_t>(creationToken);
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
					case RpcConfiguration::ArgumentType::ObjRef:
					case RpcConfiguration::ArgumentType::ObjDel:
					{
						auto obj = ctx.GetArgument<uint32_t>(i) - 0x1000000;
						buffer.Write<uint32_t>(obj);

						if (argument.GetType() == RpcConfiguration::ArgumentType::ObjDel)
						{
							delThis = true;
						}
						else
						{
							replay = true;
						}

						delId = obj;

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

				auto sendToClient = [&](const fx::ClientSharedPtr& cl)
				{
					cl->SendPacket(0, buffer, NetPacketType_ReliableReplayed);
				};

				if (clientIdx == -2)
				{
					if (entity && !entity->GetClient())
					{
						std::unique_lock<std::shared_mutex> _(entity->guidMutex);
						entity->onCreationRPC.push_back([buffer](const fx::ClientSharedPtr& client)
						{
							client->SendPacket(0, buffer, NetPacketType_ReliableReplayed);
						});
					}
				}
				else if (clientIdx == -1)
				{
					clientRegistry->ForAllClients(sendToClient);
				}
				else
				{
					sendToClient(clientRegistry->GetClientByNetID(clientIdx));
				}

				if (native->GetRpcType() == RpcConfiguration::RpcType::EntityCreate)
				{
					fx::EntityCreationState state;
					state.creationToken = creationToken;

					auto guid = new fx::ScriptGuid;
					guid->type = fx::ScriptGuid::Type::TempEntity;
					guid->tempEntity.creationToken = creationToken;
					guid->reference = nullptr;

					state.clientIdx = clientIdx;
					state.scriptGuid = guid;

					auto scrHdl = g_scriptHandlePool->GetIndex(guid) + 0x20000;
					g_entityCreationList.insert({ creationToken, state });

					ctx.SetResult(scrHdl);
				}
				else if (native->GetRpcType() == RpcConfiguration::RpcType::ObjectCreate)
				{
					auto scrHdl = creationToken + 0x1000000;
					ctx.SetResult(scrHdl);

					if (replay)
					{
						auto nativeHash = native->GetGameHash();

						gscomms_execute_callback_on_sync_thread([creationToken, nativeHash, buffer]()
						{
							g_replayList[creationToken].push_back({ nativeHash, buffer });
						});
					}
				}
				else
				{
					const auto& getter = native->GetGetter();

					if (getter)
					{
						std::variant<uint32_t, scrVector> refBit;

						if (getter->GetReturnType() == RpcConfiguration::ArgumentType::Vector3)
						{
							refBit = ctx.GetArgument<scrVector>(getter->GetReturnArgStart());
						}
						else
						{
							refBit = ctx.GetArgument<uint32_t>(getter->GetReturnArgStart());
						}

						nativeResults[{
							contextId,
							native->GetGameHash()
						}] = std::move(refBit);
					}

					if (replay)
					{
						auto nativeHash = native->GetGameHash();

						gscomms_execute_callback_on_sync_thread([delId, nativeHash, buffer]()
						{
							auto& rl = g_replayList[delId];

							for (auto it = rl.begin(); it != rl.end(); )
							{
								if (std::get<uint64_t>(*it) == nativeHash)
								{
									it = rl.erase(it);
								}
								else
								{
									it++;
								}
							}

							rl.push_back({ nativeHash, buffer });
						});
					}
				}

				if (delThis)
				{
					g_replayList[delId] = {};
				}
			});

			// GETTER
			const auto& getter = native->GetGetter();

			if (getter)
			{
				auto origHandler = fx::ScriptEngine::GetNativeHandler(HashString(getter->GetName().c_str()));

				fx::ScriptEngine::RegisterNativeHandler(getter->GetName(), [clientRegistry, gameState, native, origHandler](fx::ScriptContext& context)
				{
					auto contextRef = context.GetArgument<int>(0);

					switch (native->GetContextType())
					{
					case RpcConfiguration::ArgumentType::Entity:
					{
						auto cxtEntity = contextRef;

						if (cxtEntity < 0x20000)
						{
							auto client = clientRegistry->GetClientByNetID(cxtEntity);

							if (client)
							{
								cxtEntity = std::any_cast<uint32_t>(client->GetData("playerEntity"));
							}
						}

						contextRef = cxtEntity;

						fx::ScriptGuid* scriptGuid = g_scriptHandlePool->AtHandle(cxtEntity - 0x20000);

						if (scriptGuid)
						{
							if (scriptGuid->type == fx::ScriptGuid::Type::Entity)
							{
								// defer to original handler
								if (origHandler)
								{
									return (*origHandler)(context);
								}
							}
						}

						break;
					}
					}

					const auto& getter = native->GetGetter();
					const auto& resultRef = nativeResults[{
						contextRef,
						native->GetGameHash()
					}];

					if (getter->GetReturnType() == RpcConfiguration::ArgumentType::Vector3)
					{
						if (resultRef)
						{
							context.SetResult(std::get<scrVector>(*resultRef));
						}
						else
						{
							scrVector v;
							context.SetResult(v);
						}
					}
					else
					{
						if (resultRef)
						{
							context.SetResult(std::get<uint32_t>(*resultRef));
						}
						else
						{
							context.SetResult(0);
						}
					}
				});
			}
		}
	}, 99999999);
});
