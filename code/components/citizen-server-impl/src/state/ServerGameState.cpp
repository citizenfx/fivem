#include <StdInc.h>
#include <state/ServerGameState.h>

#include <state/SyncTrees_Five.h>

#include <optional>

#include <NetBuffer.h>

#include <lz4.h>

namespace fx
{
inline uint32_t MakeEntityHandle(uint8_t playerId, uint16_t objectId)
{
	return (playerId << 16) | objectId;
}

std::shared_ptr<sync::SyncEntityState> ServerGameState::GetEntity(uint8_t playerId, uint16_t objectId)
{
	return GetEntity(MakeEntityHandle(playerId, objectId));
}

std::shared_ptr<sync::SyncEntityState> ServerGameState::GetEntity(uint32_t handle)
{
	auto it = m_entities.find(handle);

	if (it != m_entities.end())
	{
		return it->second;
	}

	return {};
}

void ServerGameState::ProcessCloneCreate(const std::shared_ptr<fx::Client>& client, net::Buffer& inPacket, net::Buffer& ackPacket)
{
	uint16_t objectId;
	ProcessClonePacket(client, inPacket, 1, &objectId);

	ackPacket.Write<uint8_t>(1);
	ackPacket.Write<uint16_t>(objectId);
}

void ServerGameState::ProcessCloneSync(const std::shared_ptr<fx::Client>& client, net::Buffer& inPacket, net::Buffer& ackPacket)
{
	ProcessClonePacket(client, inPacket, 2, nullptr);
}

void ServerGameState::ProcessCloneRemove(const std::shared_ptr<fx::Client>& client, net::Buffer& inPacket, net::Buffer& ackPacket)
{
	auto playerId = inPacket.Read<uint8_t>();
	auto objectId = inPacket.Read<uint16_t>();

	// TODO: verify ownership
	m_entities.erase(MakeEntityHandle(playerId, objectId));

	ackPacket.Write<uint8_t>(3);
	ackPacket.Write<uint16_t>(objectId);
}

void ServerGameState::ProcessClonePacket(const std::shared_ptr<fx::Client>& client, net::Buffer& inPacket, int parsingType, uint16_t* outObjectId)
{
	auto playerId = inPacket.Read<uint8_t>();
	auto objectId = inPacket.Read<uint16_t>();
	auto objectType = (sync::NetObjEntityType)inPacket.Read<uint8_t>();
	auto length = inPacket.Read<uint16_t>();

	std::vector<uint8_t> bitBytes(length);
	inPacket.Read(&bitBytes[0], bitBytes.size());

	auto entity = GetEntity(playerId, objectId);

	if (!entity || entity->client.expired() || entity->client.lock()->GetNetId() != client->GetNetId())
	{
		entity = std::make_shared<sync::SyncEntityState>();
		entity->client = client;
		entity->type = objectType;

		m_entities[MakeEntityHandle(playerId, objectId)] = entity;
	}

	auto state = sync::SyncParseState{ { bitBytes }, parsingType, entity };

	switch (objectType)
	{
		case sync::NetObjEntityType::Automobile:
			sync::CAutomobileSyncTree::Parse(state);
			break;
		case sync::NetObjEntityType::Bike:
			sync::CBikeSyncTree::Parse(state);
			break;
		case sync::NetObjEntityType::Boat:
			sync::CBoatSyncTree::Parse(state);
			break;
		case sync::NetObjEntityType::Door:
			sync::CDoorSyncTree::Parse(state);
			break;
		case sync::NetObjEntityType::Heli:
			sync::CHeliSyncTree::Parse(state);
			break;
		case sync::NetObjEntityType::Object:
			sync::CObjectSyncTree::Parse(state);
			break;
		case sync::NetObjEntityType::Ped:
			sync::CPedSyncTree::Parse(state);
			break;
		case sync::NetObjEntityType::Pickup:
			sync::CPickupSyncTree::Parse(state);
			break;
		case sync::NetObjEntityType::PickupPlacement:
			sync::CPickupPlacementSyncTree::Parse(state);
			break;
		case sync::NetObjEntityType::Plane:
			sync::CPlaneSyncTree::Parse(state);
			break;
		case sync::NetObjEntityType::Submarine:
			sync::CSubmarineSyncTree::Parse(state);
			break;
		case sync::NetObjEntityType::Player:
			sync::CPlayerSyncTree::Parse(state);

			if (parsingType == 1)
			{
				client->SetData("playerEntity", MakeEntityHandle(playerId, objectId));
				client->SetData("playerId", playerId);
			}
			break;
		case sync::NetObjEntityType::Trailer:
			sync::CAutomobileSyncTree::Parse(state);
			break;
		case sync::NetObjEntityType::Train:
			//sync::CTrainSyncTree::Parse(state);
			break;
	}

	if (outObjectId)
	{
		*outObjectId = objectId;
	}
}

static std::optional<net::Buffer> UncompressClonePacket(const std::vector<uint8_t>& packetData)
{
	net::Buffer readBuffer(packetData);

	if (readBuffer.Read<uint32_t>() != HashString("netClones"))
	{
		return {};
	}

	uint8_t bufferData[16384] = { 0 };
	int bufferLength = LZ4_decompress_safe(reinterpret_cast<const char*>(&readBuffer.GetData()[4]), reinterpret_cast<char*>(bufferData), readBuffer.GetRemainingBytes(), sizeof(bufferData));

	if (bufferLength <= 0)
	{
		return {};
	}

	return { {bufferData, size_t(bufferLength)} };
}

void ServerGameState::ParseGameStatePacket(const std::shared_ptr<fx::Client>& client, const std::vector<uint8_t>& packetData)
{
	auto packet = UncompressClonePacket(packetData);

	if (!packet)
	{
		return;
	}

	auto& buffer = *packet;

	net::Buffer ackPacket;
	ackPacket.Write(HashRageString("msgCloneAcks"));

	uint32_t numCreates = 0, numSyncs = 0, numRemoves = 0;
	
	while (!buffer.IsAtEnd())
	{
		auto dataType = buffer.Read<uint8_t>();

		switch (dataType)
		{
		case 1: // clone create
			ProcessCloneCreate(client, buffer, ackPacket);
			++numCreates;
			break;
		case 2: // clone sync
			ProcessCloneSync(client, buffer, ackPacket);
			++numSyncs;
			break;
		case 3: // clone remove
			ProcessCloneRemove(client, buffer, ackPacket);
			++numRemoves;
			break;
		default:
			return;
		}
	}

	trace("clone creates/syncs/removes: %d/%d/%d\n", numCreates, numSyncs, numRemoves);

	// any ACKs to send?
	if (ackPacket.GetCurOffset() > 4 && client)
	{
		client->SendPacket(0, ackPacket, ENET_PACKET_FLAG_RELIABLE);
	}
}
}

#include <ResourceManager.h>
#include <ServerInstanceBase.h>
#include <ServerInstanceBaseRef.h>
#include <ScriptEngine.h>

static InitFunction initFunction([]()
{
	auto makeEntityFunction = [](auto fn, uintptr_t defaultValue = 0)
	{
		return [=](fx::ScriptContext& context)
		{
			// get the current resource manager
			auto resourceManager = fx::ResourceManager::GetCurrent();

			// get the owning server instance
			auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

			// get the server's game state
			auto gameState = instance->GetComponent<fx::ServerGameState>();

			// parse the client ID
			auto id = context.GetArgument<uint32_t>(0);

			if (!id)
			{
				context.SetResult(defaultValue);
				return;
			}

			auto entity = gameState->GetEntity(id);

			if (!entity)
			{
				trace("Tried to access invalid entity.\n");

				context.SetResult(defaultValue);
				return;
			}

			context.SetResult(fn(context, entity));
		};
	};

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_COORDS", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<sync::SyncEntityState>& entity)
	{
		struct scrVector
		{
			float x;
			int pad;
			float y;
			int pad2;
			float z;
			int pad3;
		};

		scrVector resultVec = { 0 };
		resultVec.x = entity->GetData("posX", 0.0f);
		resultVec.y = entity->GetData("posY", 0.0f);
		resultVec.z = entity->GetData("posZ", 0.0f);

		return resultVec;
	}));

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->SetComponent(new fx::ServerGameState);
	});
});
