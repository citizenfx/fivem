#pragma once

#include <Client.h>

#include <ServerInstanceBase.h>
#include <ServerTime.h>

#include <state/Pool.h>

#include <state/RlMessageBuffer.h>

#include <variant>
#include <shared_mutex>

#include <array>
#include <optional>
#include <EASTL/bitset.h>
#include <EASTL/fixed_map.h>
#include <EASTL/fixed_hash_map.h>
#include <EASTL/fixed_hash_set.h>
#include <EASTL/fixed_vector.h>
#include <shared_mutex>

#include <tbb/concurrent_unordered_map.h>
#include <thread_pool.hpp>

#define GLM_ENABLE_EXPERIMENTAL

// TODO: clang style defines/checking
#if defined(_M_IX86) || defined(_M_AMD64)
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_SSE2
#define GLM_FORCE_SSE3
#endif

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

#include <citizen_util/detached_queue.h>
#include <citizen_util/object_pool.h>
#include <citizen_util/shared_reference.h>

#include <StateBagComponent.h>

template<typename T>
inline constexpr T roundToWord(T val)
{
	constexpr auto multiple = sizeof(size_t);

	return ((val + multiple - 1) / multiple) * multiple;
}

namespace fx
{
struct ScriptGuid;
}

extern CPool<fx::ScriptGuid>* g_scriptHandlePool;
extern int m_ackTimeoutThreshold;

namespace fx::sync
{
struct SyncParseState;
struct SyncUnparseState;

struct NodeBase;

using SyncTreeVisitor = std::function<bool(NodeBase&)>;

struct NodeBase
{
public:
	eastl::bitset<roundToWord(MAX_CLIENTS)> ackedPlayers;

	uint64_t frameIndex;

	uint32_t timestamp;

	virtual bool Parse(SyncParseState& state) = 0;

	virtual bool Unparse(SyncUnparseState& state) = 0;

	virtual bool Visit(const SyncTreeVisitor& visitor) = 0;

	virtual bool IsAdditional() = 0;
};

struct CPlayerCameraNodeData
{
	int camMode;
	float freeCamPosX;
	float freeCamPosY;
	float freeCamPosZ;

	float cameraX;
	float cameraZ;

	float camOffX;
	float camOffY;
	float camOffZ;
};

struct CPlayerWantedAndLOSNodeData
{
	int wantedLevel;
	int isWanted;
	int isEvading;

	int timeInPursuit;
	int timeInPrevPursuit;

	inline CPlayerWantedAndLOSNodeData()
		: timeInPursuit(-1), timeInPrevPursuit(-1)
	{
	}
};

struct CPedGameStateNodeData
{
	int curVehicle;
	int curVehicleSeat;

	int lastVehicle;
	int lastVehicleSeat;

	int lastVehiclePedWasIn;

	int curWeapon;

	inline CPedGameStateNodeData()
		: lastVehicle(-1), lastVehicleSeat(-1), lastVehiclePedWasIn(-1)
	{
	}
};

struct CVehicleAppearanceNodeData
{
	int primaryColour;
	int secondaryColour;
	int pearlColour;
	int wheelColour;
	int interiorColour;
	int dashboardColour;

	bool isPrimaryColourRGB;
	bool isSecondaryColourRGB;

	int primaryRedColour;
	int primaryGreenColour;
	int primaryBlueColour;

	int secondaryRedColour;
	int secondaryGreenColour;
	int secondaryBlueColour;

	int dirtLevel;
	int extras;
	int liveryIndex;
	int roofLiveryIndex;

	int wheelChoice;
	int wheelType;

	bool hasCustomTires;

	int windowTintIndex;

	int tyreSmokeRedColour;
	int tyreSmokeGreenColour;
	int tyreSmokeBlueColour;

	char plate[9];

	int numberPlateTextIndex;

	inline CVehicleAppearanceNodeData()
	{
		memset(plate, 0, sizeof(plate));
	}
};

struct CVehicleHealthNodeData
{
	int engineHealth;
	int petrolTankHealth;
	bool tyresFine;
	int tyreStatus[1 << 4];
	int bodyHealth;
};

struct CVehicleGameStateNodeData
{
	uint16_t occupants[32];
	eastl::bitset<32> playerOccupants;
	int radioStation;
	bool isEngineOn;
	bool isEngineStarting;
	bool handbrake;
	int defaultHeadlights;
	int headlightsColour;
	bool sirenOn;
	int lockStatus;
	int doorsOpen;
	int doorPositions[1 << 7];
	bool noLongerNeeded;
	bool lightsOn;
	bool highbeamsOn;
	bool hasBeenOwnedByPlayer;
	bool hasLock;
	int lockedPlayers;

	inline CVehicleGameStateNodeData()
	{
		memset(occupants, 0, sizeof(occupants));
	}
};

#include <state/kumquat.h>

struct CEntityOrientationNodeData
{
	compressed_quaternion<11> quat;
};

struct CPhysicalVelocityNodeData
{
	float velX;
	float velY;
	float velZ;
};

struct CVehicleAngVelocityNodeData
{
	float angVelX;
	float angVelY;
	float angVelZ;
};

struct CPedHealthNodeData
{
	int maxHealth;
	int health;
	int armour;
	uint32_t causeOfDeath;
};

struct CPedOrientationNodeData
{
	float currentHeading;
	float desiredHeading;
};

struct CDynamicEntityGameStateNodeData
{
	std::map<int, int> decors;
};

struct CPlayerGameStateNodeData
{
	int playerTeam;
	float airDragMultiplier;

	int maxHealth;
	int maxArmour;

	bool neverTarget;
	int spectatorId;

	bool randomPedsFlee;
	bool everybodyBackOff;

	float voiceProximityOverrideX;
	float voiceProximityOverrideY;
	float voiceProximityOverrideZ;

	bool isFriendlyFireAllowed;

	float weaponDefenseModifier;
	float weaponDefenseModifier2;

	float weaponDamageModifier;
	float meleeWeaponDamageModifier;

	bool isSuperJumpEnabled;
};

enum ePopType
{
	POPTYPE_UNKNOWN = 0,
	POPTYPE_RANDOM_PERMANENT,
	POPTYPE_RANDOM_PARKED,
	POPTYPE_RANDOM_PATROL,
	POPTYPE_RANDOM_SCENARIO,
	POPTYPE_RANDOM_AMBIENT,
	POPTYPE_PERMANENT,
	POPTYPE_MISSION,
	POPTYPE_REPLAY,
	POPTYPE_CACHE,
	POPTYPE_TOOL
};

struct SyncTreeBase
{
public:
	virtual ~SyncTreeBase() = default;

	virtual void Parse(SyncParseState& state) = 0;

	virtual bool Unparse(SyncUnparseState& state) = 0;

	virtual void Visit(const SyncTreeVisitor& visitor) = 0;

	// accessors
public:
	virtual void GetPosition(float* posOut) = 0;

	virtual CPlayerCameraNodeData* GetPlayerCamera() = 0;

	virtual CPlayerWantedAndLOSNodeData* GetPlayerWantedAndLOS() = 0;

	virtual CPedGameStateNodeData* GetPedGameState() = 0;

	virtual uint64_t GetPedGameStateFrameIndex() = 0;

	virtual CVehicleGameStateNodeData* GetVehicleGameState() = 0;

	virtual CVehicleAppearanceNodeData* GetVehicleAppearance() = 0;

	virtual CPlayerGameStateNodeData* GetPlayerGameState() = 0;

	virtual CPedHealthNodeData* GetPedHealth() = 0;

	virtual CVehicleHealthNodeData* GetVehicleHealth() = 0;

	virtual CPedOrientationNodeData* GetPedOrientation() = 0;

	virtual CEntityOrientationNodeData* GetEntityOrientation() = 0;

	virtual CVehicleAngVelocityNodeData* GetAngVelocity() = 0;

	virtual CPhysicalVelocityNodeData* GetVelocity() = 0;

	virtual void CalculatePosition() = 0;

	virtual bool GetPopulationType(ePopType* popType) = 0;

	virtual bool GetModelHash(uint32_t* modelHash) = 0;

	virtual bool GetScriptHash(uint32_t* scriptHash) = 0;
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

	std::shared_mutex clientMutex;
	NetObjEntityType type;
	uint32_t timestamp;
	uint64_t frameIndex;
	uint64_t lastFrameIndex;
	uint16_t uniqifier;
	uint32_t creationToken;
	float overrideCullingRadius = 0.0f;

	eastl::bitset<roundToWord(MAX_CLIENTS)> relevantTo;

	std::chrono::milliseconds lastReceivedAt;

	std::shared_ptr<SyncTreeBase> syncTree;

	std::shared_mutex guidMutex;
	ScriptGuid* guid;
	uint32_t handle;

	bool deleting;
	bool hasSynced = false;
	bool passedFilter = false;

	std::list<std::function<void(const fx::ClientSharedPtr& ptr)>> onCreationRPC;

	std::shared_ptr<fx::StateBag> stateBag;

	SyncEntityState();

	SyncEntityState(const SyncEntityState&) = delete;

	virtual ~SyncEntityState();

	inline float GetDistanceCullingRadius()
	{
		// #TODO1S: figure out a good value for this
		return overrideCullingRadius != 0.0f ? overrideCullingRadius : (424.0f * 424.0f);
	}

	inline bool IsOwnedByScript()
	{
		uint32_t scriptHash = 0;

		if (syncTree && syncTree->GetScriptHash(&scriptHash) && scriptHash != 0)
		{
			if (creationToken != 0)
			{
				return true;
			}
		}

		return false;
	}

	// MAKE SURE YOU HAVE A LOCK BEFORE CALLING THIS
	fx::ClientWeakPtr& GetClientUnsafe()
	{
		return client;
	}

	//THIS ONE TOO
	fx::ClientWeakPtr& GetLastUpdaterUnsafe()
	{
		return lastUpdater;
	}

	fx::ClientSharedPtr GetClient()
	{
		std::shared_lock _lock(clientMutex);
		return client.lock();
	}

	fx::ClientSharedPtr GetLastUpdater()
	{
		std::shared_lock _lock(clientMutex);
		return lastUpdater.lock();
	}

	// make absolutely sure we don't accidentally mess up and get this info without a lock
private:
	fx::ClientWeakPtr client;
	fx::ClientWeakPtr lastUpdater;
};
}

namespace fx::sync
{
inline object_pool<fx::sync::SyncEntityState, 2 * 1024 * 1024> syncEntityPool;
using SyncEntityPtr = shared_reference<fx::sync::SyncEntityState, &syncEntityPool>;
using SyncEntityWeakPtr = weak_reference<SyncEntityPtr>;

struct SyncParseState
{
	rl::MessageBuffer buffer;
	int syncType;
	int objType;
	uint32_t timestamp;

	SyncEntityPtr entity;

	uint64_t frameIndex;
};

struct SyncUnparseState
{
	rl::MessageBuffer& buffer;
	int syncType;
	int objType;
	uint32_t timestamp;
	uint64_t lastFrameIndex;

	uint32_t targetSlotId;

	SyncUnparseState(rl::MessageBuffer& buffer)
		: buffer(buffer), lastFrameIndex(0)
	{
	}
};

struct SyncCommandState
{
	rl::MessageBuffer cloneBuffer;
	std::function<void(bool finalFlush)> flushBuffer;
	std::function<void(size_t)> maybeFlushBuffer;
	uint64_t frameIndex;
	fx::ClientSharedPtr client;
	bool hadTime;

	SyncCommandState(size_t size)
		: cloneBuffer(size)
	{
	}

	inline void Reset()
	{
		cloneBuffer.SetCurrentBit(0);
		flushBuffer = {};
		maybeFlushBuffer = {};
		frameIndex = 0;
		client = {};
		hadTime = false;
	}
};

struct SyncCommand
{
	using SyncCommandKey = typename detached_scsc_queue<SyncCommand>::key;
	using SyncCommandCallback = tp::FixedFunction<void(SyncCommandState&), 128>;

	SyncCommandCallback callback;
	SyncCommandKey commandKey = {};

	SyncCommand(SyncCommandCallback&& callback)
		: callback(std::move(callback))
	{
	}
};
struct SyncCommandList
{
	uint64_t frameIndex;
	detached_scsc_queue<SyncCommand> commands;

	void Execute(const fx::ClientSharedPtr& client);

	using SyncCommandPool = object_pool<SyncCommand, 4 * 1024 * 1024, 4, 2>;
	inline static SyncCommandPool syncPool;

	template<typename Fn>
	void EnqueueCommand(Fn&& fn)
	{
		auto* ptr = syncPool.construct(std::forward<Fn>(fn));
		commands.push(&ptr->commandKey);
	}

	SyncCommandList(SyncCommandList&& other) = delete;
	SyncCommandList(const SyncCommandList& other) = delete;

	SyncCommandList& operator=(SyncCommandList&& other) = delete;
	SyncCommandList& operator=(const SyncCommandList& other) = delete;

	SyncCommandList(uint64_t frameIndex)
		: frameIndex(frameIndex)
	{
	}
};
}

namespace fx
{
struct ScriptGuid
{
	enum class Type
	{
		Undefined = 0,
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

	void* reference;

	inline void* operator new(size_t s)
	{
		return g_scriptHandlePool->New();
	}

	inline void operator delete(void* ptr)
	{
		g_scriptHandlePool->Delete((fx::ScriptGuid*)ptr);
	}
};

struct EntityCreationState
{
	// TODO: allow resending in case the target client disappears
	uint32_t creationToken;
	uint32_t clientIdx;
	fx::ScriptGuid* scriptGuid;

	EntityCreationState()
		: creationToken(0), clientIdx(0), scriptGuid(0)
	{
	}
};

struct AckPacketWrapper
{
	rl::MessageBuffer& ackPacket;
	std::function<void()> flush;

	inline explicit AckPacketWrapper(rl::MessageBuffer& ackPacket)
		: ackPacket(ackPacket)
	{
	}

	template<typename TData>
	inline void Write(int length, TData data)
	{
		ackPacket.Write(length, data);
	}
};

static constexpr const int MaxObjectId = (1 << 16) - 1;

struct ClientEntityState
{

	uint16_t uniqifier;
	bool isPlayer;
	bool overrideFrameIndex;
	uint32_t netId;

	uint64_t frameIndex;

	std::chrono::milliseconds syncDelay;
	std::chrono::milliseconds lastSend;
};

using EntityStateObject = eastl::fixed_map<uint16_t, ClientEntityState, 400>;
inline object_pool<EntityStateObject, 2 * 1024 * 1024> gameStateClientPool;

struct GameStateClientData : public sync::ClientSyncDataBase
{
	rl::MessageBuffer ackBuffer{ 16384 };
	std::set<int> objectIds;

	std::mutex selfMutex;

	// gets its own mutex due to frequent accessing
	std::shared_mutex playerEntityMutex;
	sync::SyncEntityWeakPtr playerEntity;

	std::optional<int> playerId;

	bool syncing;

	glm::mat4x4 viewMatrix{};

	struct EntityStateDeleter
	{
		void operator()(EntityStateObject* obj)
		{
			gameStateClientPool.destruct(obj);
		}
	};

	static auto MakeEntityState()
	{
		return std::unique_ptr<EntityStateObject, EntityStateDeleter>(gameStateClientPool.construct(), {});
	}

	eastl::fixed_map<uint64_t, std::unique_ptr<EntityStateObject, EntityStateDeleter>, 200> entityStates;
	eastl::bitset<roundToWord(MaxObjectId)> createdEntities;

	uint64_t lastAckIndex;

	fx::ClientWeakPtr client;

	std::map<int, int> playersToSlots;
	std::map<int, int> slotsToPlayers;

	std::vector<std::tuple<uint16_t, std::chrono::milliseconds, bool>> relevantEntities;

	uint32_t syncTs = 0;
	uint32_t ackTs = 0;

	std::shared_ptr<fx::StateBag> playerBag;

	GameStateClientData()
		: syncing(false), lastAckIndex(0)
	{
	}

	void FlushAcks();

	void MaybeFlushAcks();
};

enum class EntityLockdownMode
{
	Inactive,
	Relaxed,
	Strict
};

class ServerGameState : public fwRefCountable, public fx::IAttached<fx::ServerInstanceBase>, public StateBagGameInterface
{
private:
	using ThreadPool = tp::ThreadPool;

public:
	ServerGameState();

	void Tick(fx::ServerInstanceBase* instance);

	void UpdateWorldGrid(fx::ServerInstanceBase* instance);

	void UpdateEntities();

	void ParseGameStatePacket(const fx::ClientSharedPtr& client, const std::vector<uint8_t>& packetData);

	virtual void AttachToObject(fx::ServerInstanceBase* instance) override;

	void HandleClientDrop(const fx::ClientSharedPtr& client);

	void SendObjectIds(const fx::ClientSharedPtr& client, int numIds);

	void ReassignEntity(uint32_t entityHandle, const fx::ClientSharedPtr& targetClient);

	void DeleteEntity(const fx::sync::SyncEntityPtr& entity);

	fx::sync::SyncEntityPtr CreateEntityFromTree(sync::NetObjEntityType type, const std::shared_ptr<sync::SyncTreeBase>& tree);

	inline EntityLockdownMode GetEntityLockdownMode()
	{
		return m_entityLockdownMode;
	}

	inline void SetEntityLockdownMode(EntityLockdownMode mode)
	{
		m_entityLockdownMode = mode;
	}

	uint32_t MakeScriptHandle(const fx::sync::SyncEntityPtr& ptr);

	std::tuple<std::unique_lock<std::mutex>, std::shared_ptr<GameStateClientData>> ExternalGetClientData(const fx::ClientSharedPtr& client);

private:
	void ProcessCloneCreate(const fx::ClientSharedPtr& client, rl::MessageBuffer& inPacket, AckPacketWrapper& ackPacket);

	void ProcessCloneRemove(const fx::ClientSharedPtr& client, rl::MessageBuffer& inPacket, AckPacketWrapper& ackPacket);

	void ProcessCloneSync(const fx::ClientSharedPtr& client, rl::MessageBuffer& inPacket, AckPacketWrapper& ackPacket);

	void ProcessCloneTakeover(const fx::ClientSharedPtr& client, rl::MessageBuffer& inPacket);

	bool ProcessClonePacket(const fx::ClientSharedPtr& client, rl::MessageBuffer& inPacket, int parsingType, uint16_t* outObjectId, uint16_t* outUniqifier);

	void OnCloneRemove(const fx::sync::SyncEntityPtr& entity, const std::function<void()>& doRemove);

	void RemoveClone(const fx::ClientSharedPtr& client, uint16_t objectId, uint16_t uniqifier = 0);

	void ParseClonePacket(const fx::ClientSharedPtr& client, net::Buffer& buffer);

	void ParseAckPacket(const fx::ClientSharedPtr& client, net::Buffer& buffer);

	bool ValidateEntity(const fx::sync::SyncEntityPtr& entity);

public:
	fx::sync::SyncEntityPtr GetEntity(uint8_t playerId, uint16_t objectId);
	fx::sync::SyncEntityPtr GetEntity(uint32_t handle);

	fwEvent<fx::sync::SyncEntityPtr> OnEntityCreate;

private:
	fx::ServerInstanceBase* m_instance;

	std::unique_ptr<ThreadPool> m_tg;

	// as bitset is not thread-safe
	std::mutex m_objectIdsMutex;

	eastl::bitset<roundToWord(MaxObjectId)> m_objectIdsSent;
	eastl::bitset<roundToWord(MaxObjectId)> m_objectIdsUsed;
	eastl::bitset<roundToWord(MaxObjectId)> m_objectIdsStolen;

	uint64_t m_frameIndex;

	struct WorldGridEntry
	{
		uint8_t sectorX;
		uint8_t sectorY;
		uint16_t netID;

		WorldGridEntry()
		{
			sectorX = 0;
			sectorY = 0;
			netID = -1;
		}
	};

	struct WorldGridState
	{
		WorldGridEntry entries[32];
	};

	WorldGridState m_worldGrid[MAX_CLIENTS];

	struct WorldGridOwnerIndexes
	{
		uint16_t netIDs[256][256];

		inline WorldGridOwnerIndexes()
		{
			for (int x = 0; x < std::size(netIDs); x++)
			{
				for (int y = 0; y < std::size(netIDs[0]); y++)
				{
					netIDs[x][y] = -1;
				}
			}
		}
	};

	WorldGridOwnerIndexes m_worldGridAccel;

	void SendWorldGrid(void* entry = nullptr, const fx::ClientSharedPtr& client = {});

public:
	bool MoveEntityToCandidate(const fx::sync::SyncEntityPtr& entity, const fx::ClientSharedPtr& client);

	void SendPacket(int peer, std::string_view data);

	inline const fwRefContainer<fx::StateBagComponent>& GetStateBags()
	{
		return m_sbac;
	}

private:
	fwRefContainer<fx::StateBagComponent> m_sbac;

	std::shared_ptr<fx::StateBag> m_globalBag;

	//private:
public:
	std::shared_mutex m_entitiesByIdMutex;
	std::vector<sync::SyncEntityWeakPtr> m_entitiesById;

	std::shared_mutex m_entityListMutex;
	std::list<fx::sync::SyncEntityPtr> m_entityList;

	EntityLockdownMode m_entityLockdownMode;
};

std::shared_ptr<sync::SyncTreeBase> MakeSyncTree(sync::NetObjEntityType objectType);
}

DECLARE_INSTANCE_TYPE(fx::ServerGameState);
