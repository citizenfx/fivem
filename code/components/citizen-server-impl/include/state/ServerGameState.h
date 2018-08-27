#pragma once

#include <Client.h>

#include <ServerInstanceBase.h>

#include <state/Pool.h>

#include <state/RlMessageBuffer.h>

#include <variant>

#include <bitset>

namespace fx
{
struct ScriptGuid;
}

namespace fx::sync
{
struct SyncEntityState;

struct SyncParseState
{
	rl::MessageBuffer buffer;
	int syncType;

	std::shared_ptr<SyncEntityState> entity;

	uint64_t frameIndex;
};

struct SyncUnparseState
{
	rl::MessageBuffer buffer;
	int syncType;

	std::shared_ptr<Client> client;
};

struct NodeBase;

using SyncTreeVisitor = std::function<bool(NodeBase&)>;

struct NodeBase
{
public:
	std::bitset<256> ackedPlayers;

	uint64_t frameIndex;

	virtual bool Parse(SyncParseState& state) = 0;

	virtual bool Unparse(SyncUnparseState& state) = 0;

	virtual bool Visit(const SyncTreeVisitor& visitor) = 0;
};

struct SyncTreeBase
{
public:
	virtual void Parse(SyncParseState& state) = 0;

	virtual bool Unparse(SyncUnparseState& state) = 0;

	virtual void Visit(const SyncTreeVisitor& visitor) = 0;
};

enum class NetObjEntityType
{
	Automobile = 0,
	Bike = 1,
	Boat = 2,
	Door = 3,
	Heli = 4,
	Object = 5,
	Ped = 6,
	Pickup = 7,
	PickupPlacement = 8,
	Plane = 9,
	Submarine = 10,
	Player = 11,
	Trailer = 12,
	Train = 13
};

struct SyncEntityState
{
	using TData = std::variant<int, float, bool, std::string>;

	std::unordered_map<std::string, TData> data;
	std::weak_ptr<fx::Client> client;
	NetObjEntityType type;
	std::bitset<256> ackedCreation;
	uint32_t timestamp;
	uint64_t frameIndex;

	std::unique_ptr<SyncTreeBase> syncTree;

	template<typename T>
	inline T GetData(std::string_view key, T defaultVal)
	{
		auto it = data.find(std::string(key));

		try
		{
			return (it != data.end()) ? std::get<T>(it->second) : defaultVal;
		}
		catch (std::bad_variant_access&)
		{
			return defaultVal;
		}
	}

	inline void CalculatePosition()
	{
		auto sectorX = GetData<int>("sectorX", 512);
		auto sectorY = GetData<int>("sectorY", 512);
		auto sectorZ = GetData<int>("sectorZ", 24);

		auto sectorPosX = GetData<float>("sectorPosX", 0.0f);
		auto sectorPosY = GetData<float>("sectorPosY", 0.0f);
		auto sectorPosZ = GetData<float>("sectorPosZ", 44.0f);

		data["posX"] = ((sectorX - 512.0f) * 54.0f) + sectorPosX;
		data["posY"] = ((sectorY - 512.0f) * 54.0f) + sectorPosY;
		data["posZ"] = ((sectorZ * 69.0f) + sectorPosZ) - 1700.0f;
	}

	ScriptGuid* guid;
	uint32_t handle;

	virtual ~SyncEntityState();
};
}

namespace fx
{
struct ScriptGuid
{
	enum class Type
	{
		Entity,
		TempEntity
	};

	Type type;
	union
	{
		struct
		{
			uint32_t handle;
		} entity;

		struct
		{
			uint32_t creationToken;
		} tempEntity;
	};
};

class ServerGameState : public fwRefCountable, public fx::IAttached<fx::ServerInstanceBase>
{
public:
	ServerGameState();


	void Tick(fx::ServerInstanceBase* instance);

	void UpdateWorldGrid(fx::ServerInstanceBase* instance);

	void ParseGameStatePacket(const std::shared_ptr<fx::Client>& client, const std::vector<uint8_t>& packetData);

	virtual void AttachToObject(fx::ServerInstanceBase* instance) override;

	void HandleClientDrop(const std::shared_ptr<fx::Client>& client);

	void SendObjectIds(const std::shared_ptr<fx::Client>& client, int numIds);

private:
	void ProcessCloneCreate(const std::shared_ptr<fx::Client>& client, rl::MessageBuffer& inPacket, net::Buffer& ackPacket);

	void ProcessCloneRemove(const std::shared_ptr<fx::Client>& client, rl::MessageBuffer& inPacket, net::Buffer& ackPacket);

	void ProcessCloneSync(const std::shared_ptr<fx::Client>& client, rl::MessageBuffer& inPacket, net::Buffer& ackPacket);

	void ProcessCloneTakeover(const std::shared_ptr<fx::Client>& client, rl::MessageBuffer& inPacket);

	void ProcessClonePacket(const std::shared_ptr<fx::Client>& client, rl::MessageBuffer& inPacket, int parsingType, uint16_t* outObjectId);

private:
	std::shared_ptr<sync::SyncEntityState> GetEntity(uint8_t playerId, uint16_t objectId);

public:
	std::shared_ptr<sync::SyncEntityState> GetEntity(uint32_t handle);

private:
	fx::ServerInstanceBase* m_instance;

	std::bitset<8192> m_objectIdsSent;
	std::bitset<8192> m_objectIdsUsed;

	uint64_t m_frameIndex;

	struct WorldGridEntry
	{
		uint8_t sectorX;
		uint8_t sectorY;
		uint8_t slotID;

		WorldGridEntry()
		{
			sectorX = 0;
			sectorY = 0;
			slotID = -1;
		}
	};

	struct WorldGridState
	{
		WorldGridEntry entries[12];
	};

	WorldGridState m_worldGrid[256];

	void SendWorldGrid(void* entry = nullptr, const std::shared_ptr<fx::Client>& client = {});

//private:
public:
	std::unordered_map<uint32_t, std::shared_ptr<sync::SyncEntityState>> m_entities;
};

std::unique_ptr<sync::SyncTreeBase> MakeSyncTree(sync::NetObjEntityType objectType);
}

DECLARE_INSTANCE_TYPE(fx::ServerGameState);

extern CPool<fx::ScriptGuid>* g_scriptHandlePool;
