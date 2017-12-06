#pragma once

#include <Client.h>

#include <state/Pool.h>

#include <variant>

namespace fx
{
struct ScriptGuid;
}

namespace fx::sync
{
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

class ServerGameState : public fwRefCountable
{
public:
	void ParseGameStatePacket(const std::shared_ptr<fx::Client>& client, const std::vector<uint8_t>& packetData);

private:
	void ProcessCloneCreate(const std::shared_ptr<fx::Client>& client, net::Buffer& inPacket, net::Buffer& ackPacket);

	void ProcessCloneRemove(const std::shared_ptr<fx::Client>& client, net::Buffer& inPacket, net::Buffer& ackPacket);

	void ProcessCloneSync(const std::shared_ptr<fx::Client>& client, net::Buffer& inPacket, net::Buffer& ackPacket);

	void ProcessClonePacket(const std::shared_ptr<fx::Client>& client, net::Buffer& inPacket, int parsingType, uint16_t* outObjectId);

private:
	std::shared_ptr<sync::SyncEntityState> GetEntity(uint8_t playerId, uint16_t objectId);

public:
	std::shared_ptr<sync::SyncEntityState> GetEntity(uint32_t handle);

private:
	std::unordered_map<uint32_t, std::shared_ptr<sync::SyncEntityState>> m_entities;
};
}

DECLARE_INSTANCE_TYPE(fx::ServerGameState);

extern CPool<fx::ScriptGuid>* g_scriptHandlePool;
