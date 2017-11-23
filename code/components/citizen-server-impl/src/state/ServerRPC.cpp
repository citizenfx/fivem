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

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* ref)
	{
		auto rpcConfiguration = RpcConfiguration::Load("citizen:/scripting/rpc_natives.json");

		auto clientRegistry = ref->GetComponent<fx::ClientRegistry>();
		auto gameState = ref->GetComponent<fx::ServerGameState>();
		auto gameServer = ref->GetComponent<fx::GameServer>();

		for (auto& native : rpcConfiguration->GetNatives())
		{
			fx::ScriptEngine::RegisterNativeHandler(native->GetName(), [=](fx::ScriptContext& ctx)
			{
				// ascertain the client that is the context
				int ctxIdx = native->GetContextIndex();

				int clientIdx = -1;

				switch (native->GetContextType())
				{
				case RpcConfiguration::ArgumentType::Player:
				{
					clientIdx = ctx.GetArgument<int>(0);
					break;
				}
				case RpcConfiguration::ArgumentType::Entity:
				{
					int cxtEntity = ctx.GetArgument<int>(0);

					int playerIdx = cxtEntity >> 16;

					// it's a player, not an entity
					if (playerIdx == 0)
					{
						clientIdx = cxtEntity;
					}
					else
					{
						// look up the entity owner
						auto entity = gameState->GetEntity(cxtEntity);

						if (entity)
						{
							clientIdx = entity->client.lock()->GetNetId();
						}
					}

					break;
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

				int i = 0;
				
				for (auto& argument : native->GetArguments())
				{
					switch (argument.GetType())
					{
					case RpcConfiguration::ArgumentType::Entity:
					{
						int cxtEntity = ctx.GetArgument<int>(i);

						int playerIdx = cxtEntity >> 16;

						// it's a player, not an entity
						if (playerIdx == 0)
						{
							cxtEntity = std::any_cast<uint32_t>(clientRegistry->GetClientByNetID(cxtEntity)->GetData("playerEntity"));
						}

						buffer.Write<int>(cxtEntity);

						break;
					}
					case RpcConfiguration::ArgumentType::Player:
					case RpcConfiguration::ArgumentType::Int:
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
			});
		}
	}, 100);
});
