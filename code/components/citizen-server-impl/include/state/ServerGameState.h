#pragma once

#include "StateGuard.h"

#include <state/ServerGameStatePublic.h>

static constexpr const size_t kGamePlayerCap =
#ifdef STATE_FIVE
128
#elif defined(STATE_RDR3)
32
#endif
;

#include <Client.h>
#include <GameServer.h>
#include <CrossBuildRuntime.h>

#include <ServerInstanceBase.h>
#include <ServerTime.h>

#include <state/Pool.h>

#include <state/RlMessageBuffer.h>

#include <array>
#include <optional>
#include <shared_mutex>
#include <unordered_set>
#include <variant>

#include <EASTL/bitset.h>
#include <EASTL/deque.h>
#include <EASTL/fixed_map.h>
#include <EASTL/fixed_hash_map.h>
#include <EASTL/fixed_hash_set.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/vector_map.h>

#include <tbb/concurrent_unordered_map.h>
#include <thread_pool.hpp>

#include <OneSyncVars.h>

#include <citizen_util/detached_queue.h>
#include <citizen_util/object_pool.h>
#include <citizen_util/shared_reference.h>

#include <StateBagComponent.h>

#include <net/NetObjEntityType.h>

#ifdef STATE_FIVE
// For GTA5, if the feature flag for new build system is set, we use the latest stable build executable even if lower version is enforced.
// The different sv_enforceGameBuild behaviors is achieved on the client side by loading different DLC sets with IsDlcIncludedInBuild.

inline bool Is2060()
{
	static bool value = ([]()
	{
		return (!fx::GetReplaceExecutable() && xbr::GetDefaultGTA5Build() >= 2060) || fx::GetEnforcedGameBuildNumber() >= 2060;
	})();

	return value;
}

inline bool Is2189()
{
	static bool value = ([]()
	{
		return (!fx::GetReplaceExecutable() && xbr::GetDefaultGTA5Build() >= 2189) || fx::GetEnforcedGameBuildNumber() >= 2189;
	})();

	return value;
}

inline bool Is2372()
{
	static bool value = ([]()
	{
		return (!fx::GetReplaceExecutable() && xbr::GetDefaultGTA5Build() >= 2372) || fx::GetEnforcedGameBuildNumber() >= 2372;
	})();

	return value;
}

inline bool Is2545()
{
	static bool value = ([]()
	{
		return (!fx::GetReplaceExecutable() && xbr::GetDefaultGTA5Build() >= 2545) || fx::GetEnforcedGameBuildNumber() >= 2545;
	})();

	return value;
}

inline bool Is2612()
{
	static bool value = ([]()
	{
		return (!fx::GetReplaceExecutable() && xbr::GetDefaultGTA5Build() >= 2612) || fx::GetEnforcedGameBuildNumber() >= 2612;
	})();

	return value;
}

inline bool Is2699()
{
	static bool value = ([]()
	{
		return (!fx::GetReplaceExecutable() && xbr::GetDefaultGTA5Build() >= 2699) || fx::GetEnforcedGameBuildNumber() >= 2699;
	})();

	return value;
}

inline bool Is2802()
{
	static bool value = ([]()
	{
		return (!fx::GetReplaceExecutable() && xbr::GetDefaultGTA5Build() >= 2802) || fx::GetEnforcedGameBuildNumber() >= 2802;
	})();

	return value;
}

inline bool Is2944()
{
	static bool value = ([]()
	{
		return (!fx::GetReplaceExecutable() && xbr::GetDefaultGTA5Build() >= 2944) || fx::GetEnforcedGameBuildNumber() >= 2944;
	})();

	return value;
}

inline bool Is3095()
{
	static bool value = ([]()
	{
		return (!fx::GetReplaceExecutable() && xbr::GetDefaultGTA5Build() >= 3095) || fx::GetEnforcedGameBuildNumber() >= 3095;
	})();

	return value;
}

inline bool Is3258()
{
	static bool value = ([]()
	{
		return (!fx::GetReplaceExecutable() && xbr::GetDefaultGTA5Build() >= 3258) || fx::GetEnforcedGameBuildNumber() >= 3258;
	})();

	return value;
}

inline bool Is3323()
{
	static bool value = ([]()
	{
		return (!fx::GetReplaceExecutable() && xbr::GetDefaultGTA5Build() >= 3323) || fx::GetEnforcedGameBuildNumber() >= 3323;
	})();

	return value;
}

inline bool Is3407()
{
	static bool value = ([]()
	{
		return (!fx::GetReplaceExecutable() && xbr::GetDefaultGTA5Build() >= 3407) || fx::GetEnforcedGameBuildNumber() >= 3407;
	})();

	return value;
}

inline bool IsSummerUpdate25()
{
	static bool value = ([]()
	{
		return (!fx::GetReplaceExecutable() && xbr::GetDefaultGTA5Build() >= xbr::Build::Summer_2025) || fx::GetEnforcedGameBuildNumber() >= xbr::Build::Summer_2025;
	})();

	return value;
}
#elif defined(STATE_RDR3)
inline bool Is1491()
{
	static bool value = ([]()
	{
		return fx::GetEnforcedGameBuildNumber() >= 1491;
	})();

	return value;
}
#endif

template<typename T>
inline constexpr T roundToWord(T val)
{
	constexpr auto multiple = sizeof(size_t);

	return ((val + multiple - 1) / multiple) * multiple;
}

namespace fx
{
struct ScriptGuid;
class ServerGameState;
}

extern CPool<fx::ScriptGuid>* g_scriptHandlePool;
extern int m_ackTimeoutThreshold;

namespace fx::sync
{
struct SyncParseStateDynamic;
struct SyncUnparseState;

struct NodeBase;

using SyncTreeVisitor = std::function<bool(NodeBase&)>;

struct NodeBase
{
public:
	// required so ackedPlayers offsetof isn't 0
	uint64_t unusedSentinel = 0;

	eastl::bitset<roundToWord(MAX_CLIENTS)> ackedPlayers;

	uint64_t frameIndex;

	uint32_t timestamp;
};

struct CDoorMovementDataNodeData
{
	bool isManualDoor;
	float openRatio;

	bool opening;
	bool fullyOpen;
	bool closed;
};

struct CDoorScriptInfoDataNodeData
{
	uint32_t scriptHash;
	uint32_t doorSystemHash;
};

struct CDoorScriptGameStateDataNodeData
{
	uint32_t doorSystemState;
	bool holdOpen;
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
	int fakeWantedLevel;
	int wantedLevel;
	int isWanted;
	int isEvading;

	int timeInPursuit;
	int timeInPrevPursuit;

	float wantedPositionX;
	float wantedPositionY;
	float wantedPositionZ;

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

	bool isHandcuffed;
	bool actionModeEnabled;
	bool isFlashlightOn;

	inline CPedGameStateNodeData()
		: lastVehicle(-1), lastVehicleSeat(-1), lastVehiclePedWasIn(-1)
	{
	}
};

/// <summary>
/// Shared CPhysicalAttachNodeData/CPedAttachNodeData data
/// </summary>
struct CBaseAttachNodeData
{
	bool attached;
	uint16_t attachedTo;

	bool hasOffset;
	float x, y, z;

	bool hasOrientation;
	float qx, qy, qz, qw;

	uint16_t attachBone;
	uint32_t attachmentFlags;
};

struct CObjectGameStateNodeData
{
	bool hasTask;
	uint16_t taskType;
	uint16_t taskDataSize;

	//uint8_t taskData[256];

	bool isBroken;
	uint32_t brokenFlags;

	bool hasExploded;
	bool hasAddedPhysics;
	bool isVisible;
	bool unk_0x165;
	bool unk_0x166;
} ;

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

	int envEffScale;
	bool hasExtras;

    int dirtLevel;
    int extras;
	bool hasCustomLivery;
	bool hasCustomLiveryIndex;
    int liveryIndex;
    int roofLiveryIndex;

	bool hasCustomRoofLivery;

	bool hasMod;
	int kitMods[32];

	bool hasToggleMods;
	int toggleMods;

    int wheelChoice;
    int wheelType;

	bool hasDifferentRearWheel;
	int rearWheelChoice;

	int kitIndex;
    bool hasCustomTires;
	bool hasWheelVariation1;

	bool hasWindowTint;
    int windowTintIndex;

	bool hasTyreSmokeColours;

    int tyreSmokeRedColour;
    int tyreSmokeGreenColour;
    int tyreSmokeBlueColour;

	bool hasPlate;
    char plate[9];

    int numberPlateTextIndex;

    int hornTypeHash;


    // Badge related fields
    bool hasEmblems;
	bool isEmblem;
	int emblemType;
    int emblemId;
	bool isSizeModified;
    int emblemSize;
    int txdName;
    int textureName;

	bool hasNeonLights;
	bool hasVehicleBadge;

    bool hasBadge[4];
    int badgeBoneIndex[4];
    int badgeAlpha[4];
    float badgeOffsetX[4];
    float badgeOffsetY[4];
    float badgeOffsetZ[4];
    float badgeDirX[4];
    float badgeDirY[4];
    float badgeDirZ[4];
    float badgeSideX[4];
    float badgeSideY[4];
    float badgeSideZ[4];
    float badgeSize[4];
	
    int neonRedColour;
    int neonGreenColour;
    int neonBlueColour;
    bool neonLeftOn;
    bool neonRightOn;
    bool neonFrontOn;
    bool neonBackOn;
    bool isNeonSuppressed;

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
	int health;
	int totalRepairs;
};

struct CVehicleGameStateNodeData
{
	uint16_t occupants[32];
	eastl::bitset<32> playerOccupants;
	uint16_t lastOccupant[32];
	int radioStation;
	bool isEngineOn;
	bool isEngineStarting;
	bool handbrake;
	int defaultHeadlights;
	int headlightsColour;
	bool sirenOn;
	int lockStatus;
	int doorsOpen;
	int doorPositions[7];
	bool isStationary;
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

struct CDummyObjectCreationNodeData
{
	float dummyPosX, dummyPosY, dummyPosZ;

	bool playerWantsControl;
	bool hasFragGroup;
	bool isBroken;
	bool unk11;
	bool hasExploded;
	bool _explodingEntityExploded; // Estimated name from CExplosionEvent.
	bool keepRegistered;

	uint16_t fragGroupIndex;

	// Other unused fields...

	/// <summary>
	/// Estimated name from CExplosionEvent. When false the dummy object has an
	/// ownership token, position, and orientation.
	/// </summary>
	bool _hasRelatedDummy;
};

struct CObjectOrientationNodeData
{
	bool highRes;
	compressed_quaternion<11> quat;
	float rotX, rotY, rotZ;
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
	int sourceOfDamage;
};

struct CPedOrientationNodeData
{
	float currentHeading;
	float desiredHeading;
};

struct CPedTaskTreeDataNodeData
{
	uint32_t scriptCommand;
	uint32_t scriptTaskStage;

	uint32_t specifics;

	struct
	{
		uint32_t type;
		bool active;
		uint32_t priority;
		uint32_t treeDepth;
		uint32_t sequenceId;
	} tasks[8];
};

struct CPlaneGameStateDataNodeData
{
	uint32_t landingGearState;

	uint16_t lockOnEntity;
	uint32_t lockOnState;

	uint32_t visibleDistance;
};

struct CPlaneControlDataNodeData
{
	float nozzlePosition;
};

struct CDynamicEntityGameStateNodeData
{
	std::map<int, int> decors;
};

struct CTrainGameStateDataNodeData
{
	int engineCarriage;
	int linkedToBackwardId;
	int linkedToForwardId;

	float distanceFromEngine;

	int trainConfigIndex;
	int carriageIndex;

	int trackId;
	float cruiseSpeed;

	int trainState;

	bool isEngine;
	bool isCaboose;

	bool isMissionTrain;

	bool direction;

	bool hasPassengerCarriages;

	bool renderDerailed;

	// 2372 {
	bool allowRemovalByPopulation;
	bool highPrecisionBlending;
	bool stopAtStations;
	// }

	bool forceDoorsOpen;
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

	bool isInvincible;
	bool isFriendlyFireAllowed;

	float weaponDefenseModifier;
	float weaponDefenseModifier2;

	float weaponDamageModifier;
	float meleeWeaponDamageModifier;

	bool isSuperJumpEnabled;
};

struct CHeliHealthNodeData
{
	int mainRotorHealth;
	int rearRotorHealth;

	bool boomBroken;
	bool canBoomBreak;
	bool hasCustomHealth;

	int bodyHealth;
	int gasTankHealth;
	int engineHealth;

	float mainRotorDamage;
	float rearRotorDamage;
	float tailRotorDamage;

	bool disableExplosionFromBodyDamage;
};

struct CHeliControlDataNodeData
{
	float yawControl;
	float pitchControl;
	float rollControl;
	float throttleControl;

	bool engineOff;

	bool hasLandingGear;
	uint32_t landingGearState;

	bool isThrusterModel;
	float thrusterSideRCSThrottle;
	float thrusterThrottle;

	bool hasVehicleTask;
	bool lockedToXY;
};

struct CVehicleSteeringNodeData
{
	float steeringAngle;
};

struct CEntityScriptGameStateNodeData
{
	bool usesCollision;
	bool isFixed;
};

struct CVehicleDamageStatusNodeData
{
	bool damagedByBullets;
	bool anyWindowBroken;
	bool windowsState[8];
};

struct CBoatGameStateNodeData
{
	bool lockedToXY;
	float sinkEndTime;
	int wreckedAction;
	bool isWrecked;
};

struct CPedMovementGroupNodeData
{
	bool isStealthy;
	bool isStrafing;
	bool isRagdolling;
};

struct CPedAINodeData
{
	int relationShip;
	int decisionMaker;
};

struct CPedVehicleNodeData
{
	bool inVehicle;
	int curVehicle;
	int lastVehiclePedWasIn;

	bool onHorse;
	int curHorse;
	int lastHorsePedWasOn;

	int curSeat;
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

	virtual void ParseSync(SyncParseStateDynamic& state) = 0;

	virtual void ParseCreate(SyncParseStateDynamic& state) = 0;

	virtual bool Unparse(SyncUnparseState& state) = 0;

	virtual void Visit(const SyncTreeVisitor& visitor) = 0;

	// accessors
public:
	virtual void GetPosition(float* posOut) = 0;

	virtual CDoorMovementDataNodeData* GetDoorMovement() = 0;

	virtual CDoorScriptInfoDataNodeData* GetDoorScriptInfo() = 0;

	virtual CDoorScriptGameStateDataNodeData* GetDoorScriptGameState() = 0;

	virtual CPlayerCameraNodeData* GetPlayerCamera() = 0;

	virtual CPlayerWantedAndLOSNodeData* GetPlayerWantedAndLOS() = 0;

	virtual CPedGameStateNodeData* GetPedGameState() = 0;

	virtual uint64_t GetPedGameStateFrameIndex() = 0;

	virtual CVehicleGameStateNodeData* GetVehicleGameState() = 0;

	virtual CVehicleAppearanceNodeData* GetVehicleAppearance() = 0;

	virtual CPedTaskTreeDataNodeData* GetPedTaskTree() = 0;

	virtual CPlaneGameStateDataNodeData* GetPlaneGameState() = 0;

	virtual CPlaneControlDataNodeData* GetPlaneControl() = 0;

	virtual CTrainGameStateDataNodeData* GetTrainState() = 0;

	virtual CPlayerGameStateNodeData* GetPlayerGameState() = 0;

	virtual CPedHealthNodeData* GetPedHealth() = 0;

	virtual CVehicleHealthNodeData* GetVehicleHealth() = 0;

	virtual CPedOrientationNodeData* GetPedOrientation() = 0;

	virtual CEntityOrientationNodeData* GetEntityOrientation() = 0;

	virtual CObjectOrientationNodeData* GetObjectOrientation() = 0;

	virtual CVehicleAngVelocityNodeData* GetAngVelocity() = 0;

	virtual CPhysicalVelocityNodeData* GetVelocity() = 0;

	virtual CBaseAttachNodeData* GetAttachment() = 0;

	virtual CObjectGameStateNodeData* GetObjectGameState() = 0;

	virtual CDummyObjectCreationNodeData* GetDummyObjectState() = 0;

	virtual CHeliHealthNodeData* GetHeliHealth() = 0;

	virtual CHeliControlDataNodeData* GetHeliControl() = 0;

	virtual CVehicleSteeringNodeData* GetVehicleSteeringData() = 0;

	virtual CEntityScriptGameStateNodeData* GetEntityScriptGameState() = 0;

	virtual CVehicleDamageStatusNodeData* GetVehicleDamageStatus() = 0;

	virtual CBoatGameStateNodeData* GetBoatGameState() = 0;

	virtual CPedMovementGroupNodeData* GetPedMovementGroup() = 0;

	virtual CPedAINodeData* GetPedAI() = 0;

	virtual void CalculatePosition() = 0;

	virtual bool GetPopulationType(ePopType* popType) = 0;

	virtual bool GetModelHash(uint32_t* modelHash) = 0;

	virtual bool GetScriptHash(uint32_t* scriptHash) = 0;

	virtual bool IsEntityVisible(bool* visible) = 0;

	virtual CPedVehicleNodeData* GetPedVehicleData() = 0;
};

enum EntityOrphanMode : uint8_t
{
	DeleteWhenNotRelevant = 0,
	DeleteOnOwnerDisconnect = 1,
	KeepEntity = 2,
};
struct SyncEntityState
{
	using TData = std::variant<int, float, bool, std::string>;

	std::shared_mutex clientMutex;
	NetObjEntityType type;
	uint32_t timestamp;
	uint32_t lastOutOfBandTimestamp;
	uint64_t frameIndex;
	uint64_t lastFrameIndex;
	uint16_t uniqifier;
	uint32_t creationToken;
	uint32_t routingBucket = 0;
	float overrideCullingRadius = 0.0f;
	bool ignoreRequestControlFilter = false;

	std::shared_mutex guidMutex;
	eastl::bitset<roundToWord(MAX_CLIENTS)> relevantTo;
	eastl::bitset<roundToWord(MAX_CLIENTS)> deletedFor;
	eastl::bitset<roundToWord(MAX_CLIENTS)> outOfScopeFor;

	std::mutex frameMutex;

	// #IFARQ this means lastFramesAcked
	std::array<uint64_t, MAX_CLIENTS> lastFramesSent;

	// #IFARQ this means lastFramesSent
	std::array<uint64_t, MAX_CLIENTS> lastFramesPreSent;

	std::chrono::milliseconds createdAt{ 0 };
	std::chrono::milliseconds lastReceivedAt;
	std::chrono::milliseconds lastMigratedAt;

	std::shared_ptr<SyncTreeBase> syncTree;

	ScriptGuid* guid = nullptr;
	uint32_t handle;

	bool deleting = false;
	bool finalizing = false;
	bool hasSynced = false;
	bool passedFilter = false;
	bool wantsReassign = false;
	bool firstOwnerDropped = false;
	EntityOrphanMode orphanMode = EntityOrphanMode::DeleteWhenNotRelevant;
#ifdef STATE_FIVE
	bool allowRemoteSyncedScenes = false;
#endif

	std::list<std::function<void(const fx::ClientSharedPtr& ptr)>> onCreationRPC;

private:
	std::shared_mutex stateBagPtrMutex;
	std::shared_ptr<fx::StateBag> stateBag;

public:
	SyncEntityState();

	SyncEntityState(const SyncEntityState&) = delete;

	virtual ~SyncEntityState();

	inline bool HasStateBag()
	{
		std::shared_lock _(stateBagPtrMutex);
		return (stateBag) ? true : false;
	}

	inline auto GetStateBag()
	{
		std::shared_lock _(stateBagPtrMutex);
		return stateBag;
	}

	inline void SetStateBag(std::shared_ptr<fx::StateBag>&& newStateBag)
	{
		std::unique_lock _(stateBagPtrMutex);
		stateBag = std::move(newStateBag);
	}

	inline float GetDistanceCullingRadius(float playerCullingRadius)
	{
		//Use priority ordering
		if (overrideCullingRadius != 0.0f) 
		{
			return overrideCullingRadius;
		}
		else if (playerCullingRadius != 0.0f) 
		{
			return playerCullingRadius;
		}
		else
		{
			// #TODO1S: figure out a good value for this
			return (424.0f * 424.0f);
		}
	}

	inline uint32_t GetScriptHash()
	{
		uint32_t scriptHash = 0;
		if (syncTree)
		{
			syncTree->GetScriptHash(&scriptHash);
		}
		return scriptHash;
	}

	/// <summary>
	/// Checks of the entity is set to be kept by the server via orphan mode or by being owned by a server script.
	/// </summary>
	inline bool ShouldServerKeepEntity()
	{
		return IsOwnedByServerScript() || orphanMode == EntityOrphanMode::KeepEntity;
	}

	inline bool IsOwnedByScript()
	{
		return GetScriptHash() != 0;
	}

	inline bool IsOwnedByClientScript()
	{
		return GetScriptHash() != 0 && creationToken == 0;
	}

	inline bool IsOwnedByServerScript()
	{
		return GetScriptHash() != 0 && creationToken != 0;
	}

	// MAKE SURE YOU HAVE A LOCK BEFORE CALLING THIS
	fx::ClientWeakPtr& GetClientUnsafe()
	{
		return client;
	}

	fx::ClientWeakPtr& GetFirstOwnerUnsafe()
	{
		return firstOwner;
	}

	fx::ClientWeakPtr& GetLastOwnerUnsafe()
	{
		return lastOwner;
	}


	fx::ClientSharedPtr GetClient()
	{
		std::shared_lock _lock(clientMutex);
		return client.lock();
	}

	fx::ClientSharedPtr GetFirstOwner()
	{
		std::shared_lock _lock(clientMutex);
		return firstOwner.lock();
	}

	fx::ClientSharedPtr GetLastOwner()
	{
		std::shared_lock _lock(clientMutex);
		return lastOwner.lock();
	}

	// make absolutely sure we don't accidentally mess up and get this info without a lock
private:
	fx::ClientWeakPtr client;
	fx::ClientWeakPtr firstOwner;
	fx::ClientWeakPtr lastOwner;
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
	uint32_t timestamp;

	SyncEntityPtr entity;

	uint64_t frameIndex;

	inline SyncParseState(rl::MessageBuffer&& buffer, uint32_t timestamp, const SyncEntityPtr& entity, uint64_t frameIndex)
		: buffer(std::move(buffer)), timestamp(timestamp), entity(entity), frameIndex(frameIndex)
	{
	}

private:
	// use SyncParseStateDynamic instead
	inline SyncParseState(rl::MessageBuffer&& buffer, int syncType, int objType, uint32_t timestamp, const SyncEntityPtr& entity, uint64_t frameIndex)
	{
		
	}
};

struct SyncParseStateDynamic : SyncParseState
{
	int syncType;
	int objType;

	inline SyncParseStateDynamic(rl::MessageBuffer&& buffer, int syncType, int objType, uint32_t timestamp, const SyncEntityPtr& entity, uint64_t frameIndex)
		: SyncParseState(std::move(buffer), timestamp, entity, frameIndex), syncType(syncType), objType(objType)
	{
	
	}
};

struct SyncUnparseState
{
	rl::MessageBuffer& buffer;
	int syncType;
	int objType;
	uint32_t timestamp;
	uint64_t lastFrameIndex;
	uint32_t targetSlotId;
	bool isFirstUpdate;

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

struct ClientEntityData
{
	sync::SyncEntityWeakPtr entityWeak;
	uint64_t lastSent;
	bool isCreated;

	inline ClientEntityData()
		: lastSent(0), entityWeak(), isCreated(false)
	{
	
	}

	ClientEntityData(const sync::SyncEntityPtr& entity, uint64_t lastSent, bool isCreated);

	sync::SyncEntityPtr GetEntity(fx::ServerGameState* sgs) const;
};

struct EntityDeletionData
{
	bool outOfScope; // is this a deletion due to being out-of-scope?
	bool forceSteal; // should we force a steal from the client?
};

struct ClientEntityState
{
#ifdef _WIN32
	eastl::vector_map<uint16_t, ClientEntityData, std::less<uint16_t>, EASTLAllocatorType, eastl::deque<eastl::pair<uint16_t, ClientEntityData>, EASTLAllocatorType>> syncedEntities;
#else
	// on Linux/Clang/libstdc++/dunno the above vector_map leads to very rare corruption under high load
	eastl::fixed_map<uint16_t, ClientEntityData, 192> syncedEntities;
#endif

	// and 24 deletions per frame
	eastl::fixed_vector<std::tuple<uint32_t, EntityDeletionData>, 24> deletions;
};

struct SyncedEntityData
{
	std::chrono::milliseconds nextSync;
	std::chrono::milliseconds syncDelta;
	sync::SyncEntityPtr entity;
	bool forceUpdate;
	bool hasCreated;
	bool hasRoutedStateBag = false;
	bool hasNAckedCreate = false;
};

constexpr auto maxSavedClientFrames = 650; // enough for ~8-9 seconds, after 5 we'll start using worst-case frames
constexpr auto maxSavedClientFramesWorstCase = (60000 / 15); // enough for ~60 seconds

struct GameStateClientData
{
	rl::MessageBuffer ackBuffer{ 16384 };
	std::unordered_set<int> objectIds;

	std::mutex selfMutex;

	// gets its own mutex due to frequent accessing
	std::shared_mutex playerEntityMutex;
	sync::SyncEntityWeakPtr playerEntity;

	std::optional<int> playerId;

	bool syncing;

	glm::mat4x4 viewMatrix{};

	eastl::fixed_hash_map<uint32_t, uint64_t, 100> pendingCreates;
	eastl::fixed_map<uint64_t, ClientEntityState, maxSavedClientFrames, true> frameStates;
	uint64_t firstSavedFrameState = 0;

	fx::ClientWeakPtr client;

	eastl::fixed_hash_map<int, int, kGamePlayerCap> playersToSlots;
	eastl::bitset<kGamePlayerCap> playersInScope;
	
	// use fixed_map to make insertion into the vector_map cheap (as sorted)
	eastl::fixed_map<uint32_t, SyncedEntityData, 256> syncedEntities;
	eastl::fixed_hash_map<uint32_t, std::tuple<sync::SyncEntityPtr, EntityDeletionData>, 16> entitiesToDestroy;

	uint32_t syncTs = 0;
	uint32_t ackTs = 0;
	uint64_t fidx = 0;

	eastl::fixed_hash_map<uint16_t /* (x << 8) | y */, std::chrono::milliseconds, 10> worldGridCooldown;

	std::shared_ptr<fx::StateBag> playerBag;

	uint32_t routingBucket = 0;

	float playerCullingRadius = 0.0f;
	
	inline float GetPlayerCullingRadius()
	{
		return playerCullingRadius;
	}

	GameStateClientData()
		: syncing(false)
	{
	}

	void FlushAcks();

	void MaybeFlushAcks();
};

class ServerGameState : public ServerGameStatePublic, public fx::IAttached<fx::ServerInstanceBase>, public StateBagGameInterface
{
private:
	using ThreadPool = tp::ThreadPool;

public:
	ServerGameState();

	void Tick(fx::ServerInstanceBase* instance);

	void UpdateWorldGrid(fx::ServerInstanceBase* instance);

	void UpdateEntities();

	void ParseGameStatePacket(const fx::ClientSharedPtr& client, const net::packet::ClientRoute& packetData);

	virtual void AttachToObject(fx::ServerInstanceBase* instance) override;

	void HandleClientDrop(const fx::ClientSharedPtr& client, uint16_t netId, uint32_t slotId);

	void HandleArrayUpdate(const fx::ClientSharedPtr& client, net::packet::ClientArrayUpdate& buffer) override;

	void HandleGameStateNAck(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::packet::ClientGameStateNAck& buffer) override;

	void HandleGameStateAck(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::packet::ClientGameStateAck& buffer) override;
	
	void GetFreeObjectIds(const fx::ClientSharedPtr& client, uint8_t numIds, std::vector<uint16_t>& freeIds);

#ifdef STATE_FIVE
	void IterateTrainLink(const sync::SyncEntityPtr& train, std::function<bool(sync::SyncEntityPtr&)> fn, bool callOnInitialEntity = true);
#endif

	void ReassignEntity(uint32_t entityHandle, const fx::ClientSharedPtr& targetClient, std::unique_lock<std::shared_mutex>&& lock = {});

	bool SetEntityStateBag(uint8_t playerId, uint16_t objectId, std::function<std::shared_ptr<StateBag>()> createStateBag) override;

	uint32_t GetClientRoutingBucket(const fx::ClientSharedPtr& client) override;

	std::function<bool()> GetGameEventHandlerWithEvent(const fx::ClientSharedPtr& client, const std::vector<uint16_t>& targetPlayers, net::packet::ClientNetGameEventV2& netGameEvent) override;

	bool IsClientRelevantEntity(const fx::ClientSharedPtr& client, uint32_t objectId) override;

private:
	void ReassignEntityInner(uint32_t entityHandle, const fx::ClientSharedPtr& targetClient, std::unique_lock<std::shared_mutex>&& lock = {});

public:

	template<bool IgnoreTrainChecks = false>
	void DeleteEntity(const fx::sync::SyncEntityPtr& entity)
	{
		if (entity->type == sync::NetObjEntityType::Player || !entity->syncTree)
		{
			return;
		}

		// can only be used on FiveM, RDR doesn't have its sync nodes filled out
#ifdef STATE_FIVE
		// this will be ignored by DELETE_TRAIN so calling on any part of the train will delete the entire thing
		if constexpr (!IgnoreTrainChecks)
		{

			if (entity->type == sync::NetObjEntityType::Train;
				auto trainState = entity->syncTree->GetTrainState())
			{
				// don't allow the deletion of carriages until we can modify sync node data and overwrite the linked forward/linked backwards state
				if (trainState->engineCarriage && trainState->engineCarriage != entity->handle)
				{
					return;
				}
			}
		}
#endif

		gscomms_execute_callback_on_sync_thread([=]()
		{
#ifdef STATE_FIVE
			if (entity->type == sync::NetObjEntityType::Train)
			{
				// recursively delete every part of the train
				IterateTrainLink(entity, [=](fx::sync::SyncEntityPtr& train)
				{
					RemoveClone({}, train->handle);

					return true;
				});
				return;
			}
#endif
			RemoveClone({}, entity->handle);
		});
	}

	void ClearClientFromWorldGrid(const fx::ClientSharedPtr& targetClient);

	fx::sync::SyncEntityPtr CreateEntityFromTree(sync::NetObjEntityType type, const std::shared_ptr<sync::SyncTreeBase>& tree);

	inline EntityLockdownMode GetEntityLockdownMode()
	{
		return m_entityLockdownMode;
	}

	inline void SetEntityLockdownMode(EntityLockdownMode mode)
	{
		m_entityLockdownMode = mode;
	}

	bool GetStateBagStrictMode() const override
	{
		return m_stateBagStrictMode;
	}

	EntityLockdownMode GetEntityLockdownMode(const fx::ClientSharedPtr& client);
	void SetEntityLockdownMode(int bucket, EntityLockdownMode mode);

	void SetPopulationDisabled(int bucket, bool disabled);

	inline SyncStyle GetSyncStyle()
	{
		return m_syncStyle;
	}

	uint32_t MakeScriptHandle(const fx::sync::SyncEntityPtr& ptr);

	std::tuple<std::unique_lock<std::mutex>, std::shared_ptr<GameStateClientData>> ExternalGetClientData(const fx::ClientSharedPtr& client);

	void ForAllEntities(const std::function<void(sync::Entity*)>& cb);

	inline auto GetServerInstance() const
	{
		return m_instance;
	}

private:
	void ProcessCloneCreate(const fx::ClientSharedPtr& client, rl::MessageBufferView& inPacket, AckPacketWrapper& ackPacket);

	void ProcessCloneRemove(const fx::ClientSharedPtr& client, rl::MessageBufferView& inPacket, AckPacketWrapper& ackPacket);

	void ProcessCloneSync(const fx::ClientSharedPtr& client, rl::MessageBufferView& inPacket, AckPacketWrapper& ackPacket);

	void ProcessCloneTakeover(const fx::ClientSharedPtr& client, rl::MessageBufferView& inPacket);

	bool ProcessClonePacket(const fx::ClientSharedPtr& client, rl::MessageBufferView& inPacket, int parsingType, uint16_t* outObjectId, uint16_t* outUniqifier);

	void OnCloneRemove(const fx::sync::SyncEntityPtr& entity, const std::function<void()>& doRemove);

	void RemoveClone(const fx::ClientSharedPtr& client, uint16_t objectId, uint16_t uniqifier = 0);

	void FinalizeClone(const fx::ClientSharedPtr& client, const fx::sync::SyncEntityPtr& entity, uint16_t objectId, uint16_t uniqifier = 0, std::string_view finalizeReason = "");

	void ParseClonePacket(const fx::ClientSharedPtr& client, net::ByteReader& buffer);

	void ParseAckPacket(const fx::ClientSharedPtr& client, net::ByteReader& buffer);

	bool ValidateEntity(EntityLockdownMode entityLockdownMode, const fx::sync::SyncEntityPtr& entity);

public:
	std::unordered_set<uint32_t> blockedEvents;
	std::shared_mutex blockedEventsMutex;
	bool IsNetGameEventBlocked(uint32_t eventNameHash);
	std::function<bool()> GetGameEventHandler(const fx::ClientSharedPtr& client, const std::vector<uint16_t>& targetPlayers, net::Buffer&& buffer);

private:
	std::function<bool()> GetRequestControlEventHandler(const fx::ClientSharedPtr& client, net::Buffer&& buffer);
	std::function<bool()> GetRequestControlEventHandlerWithEvent(const fx::ClientSharedPtr& client, net::packet::ClientNetGameEventV2& netGameEvent);

public:
	fx::sync::SyncEntityPtr GetEntity(uint8_t playerId, uint16_t objectId);
	fx::sync::SyncEntityPtr GetEntity(uint32_t handle);

	fwEvent<fx::sync::SyncEntityPtr> OnEntityCreate;

private:
	fx::ServerInstanceBase* m_instance;

#ifdef USE_ASYNC_SCL_POSTING
	std::unique_ptr<ThreadPool> m_tg;
#endif

	// as bitset is not thread-safe
	std::shared_mutex m_objectIdsMutex;
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

	struct WorldGrid
	{
		WorldGridState state[MAX_CLIENTS];
		WorldGridOwnerIndexes accel;
	};

	std::map<int, std::unique_ptr<WorldGrid>> m_worldGrids;
	std::shared_mutex m_worldGridsMutex;
	
	void SendWorldGrid(void* entry = nullptr, const fx::ClientSharedPtr& client = {});

public:
	struct ArrayHandlerBase
	{
		virtual ~ArrayHandlerBase() = default;

		virtual bool ReadUpdate(const fx::ClientSharedPtr& client, net::packet::ClientArrayUpdate& buffer) = 0;

		virtual void WriteUpdates(const fx::ClientSharedPtr& client) = 0;

		virtual void PlayerHasLeft(const fx::ClientSharedPtr& client) = 0;

		virtual uint32_t GetCount() = 0;

		virtual uint32_t GetElementSize() = 0;
	};

private:
	struct ArrayHandlerData
	{
		std::array<std::shared_ptr<ArrayHandlerBase>, 20> handlers;

		ArrayHandlerData();
	};

	std::map<int, std::unique_ptr<ArrayHandlerData>> m_arrayHandlers;
	std::shared_mutex m_arrayHandlersMutex;

	struct RoutingBucketMetaData
	{
		std::optional<EntityLockdownMode> lockdownMode;
		bool noPopulation = false;
	};

	std::map<int, RoutingBucketMetaData> m_routingData;
	std::shared_mutex m_routingDataMutex;

	void SendArrayData(const fx::ClientSharedPtr& client);

public:
	bool MoveEntityToCandidate(const fx::sync::SyncEntityPtr& entity, const fx::ClientSharedPtr& client);

	void SendPacket(int peer, net::packet::StateBagPacket& packet) override;

	void SendPacket(int peer, net::packet::StateBagV2Packet& packet) override;

	bool IsAsynchronous() override;

	void QueueTask(std::function<void()>&& task) override;

	inline const fwRefContainer<fx::StateBagComponent>& GetStateBags()
	{
		return m_sbac;
	}

private:
	fwRefContainer<fx::StateBagComponent> m_sbac;

	std::shared_ptr<fx::StateBag> m_globalBag;

	std::shared_ptr<ConVar<EntityLockdownMode>> m_lockdownModeVar;
	std::shared_ptr<ConVar<bool>> m_stateBagStrictModeVar;

	//private:
public:
	std::shared_mutex m_entitiesByIdMutex;
	std::vector<sync::SyncEntityWeakPtr> m_entitiesById;

	std::shared_mutex m_entityListMutex;
	std::set<fx::sync::SyncEntityPtr> m_entityList;

	EntityLockdownMode m_entityLockdownMode;
	SyncStyle m_syncStyle = SyncStyle::NAK;

	bool m_stateBagStrictMode {false};
};

// for use in sync trees
inline ServerGameState* g_serverGameState = nullptr;

std::shared_ptr<sync::SyncTreeBase> MakeSyncTree(sync::NetObjEntityType objectType);
}

DECLARE_INSTANCE_TYPE(fx::ServerGameState);
