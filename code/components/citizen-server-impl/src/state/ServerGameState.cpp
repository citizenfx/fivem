#include <StdInc.h>
#include <state/ServerGameState.h>

#include <state/SyncTrees_Five.h>

#include <optional>

#include <NetBuffer.h>

#include <lz4.h>

#include <state/Pool.h>

CPool<fx::ScriptGuid>* g_scriptHandlePool;

namespace fx
{
inline uint32_t MakeEntityHandle(uint8_t playerId, uint16_t objectId)
{
	return ((playerId + 1) << 16) | objectId;
}

static uint32_t MakeScriptHandle(const std::shared_ptr<sync::SyncEntityState>& ptr)
{
	if (!ptr->guid)
	{
		// find an existing handle (transformed TempEntity?)
		for (int i = 0; i < g_scriptHandlePool->m_Size; i++)
		{
			auto hdl = g_scriptHandlePool->GetAt(i);

			if (hdl && hdl->type == ScriptGuid::Type::Entity && hdl->entity.handle == ptr->handle)
			{
				ptr->guid = hdl;
			}
		}

		if (!ptr->guid)
		{
			auto guid = g_scriptHandlePool->New();
			guid->type = ScriptGuid::Type::Entity;
			guid->entity.handle = ptr->handle;

			ptr->guid = guid;
		}
	}

	return g_scriptHandlePool->GetIndex(ptr->guid) + 0x20000;
}

std::shared_ptr<sync::SyncEntityState> ServerGameState::GetEntity(uint8_t playerId, uint16_t objectId)
{
	auto handle = MakeEntityHandle(playerId, objectId);

	auto it = m_entities.find(handle);

	if (it != m_entities.end())
	{
		return it->second;
	}

	return {};
}

std::shared_ptr<sync::SyncEntityState> ServerGameState::GetEntity(uint32_t guid)
{
	// subtract the minimum index GUID
	guid -= 0x20000;

	// get the pool entry
	auto guidData = g_scriptHandlePool->AtHandle(guid);

	if (guidData)
	{
		if (guidData->type == ScriptGuid::Type::Entity)
		{
			auto it = m_entities.find(guidData->entity.handle);

			if (it != m_entities.end())
			{
				return it->second;
			}
		}
	}

	return {};
}

namespace sync
{
	SyncEntityState::~SyncEntityState()
	{
		if (guid)
		{
			g_scriptHandlePool->Delete(guid);
		}
	}
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
		entity->guid = nullptr;
		entity->handle = MakeEntityHandle(playerId, objectId);

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
				client->SetData("playerEntity", MakeScriptHandle(entity));
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
	g_scriptHandlePool = new CPool<fx::ScriptGuid>(1500, "fx::ScriptGuid");

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

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_COORDS", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
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

	fx::ScriptEngine::RegisterNativeHandler("GET_HASH_KEY", [](fx::ScriptContext& context)
	{
		context.SetResult(HashString(context.GetArgument<const char*>(0)));
	});

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->SetComponent(new fx::ServerGameState);
	});
});
