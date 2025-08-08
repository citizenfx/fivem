#include <StdInc.h>
#include <GameServer.h>

#include <state/ServerGameState.h>

#include <optional>
#include <charconv>

#include <NetBuffer.h>

#include <lz4.h>
#include <lz4hc.h>

#include <tbb/concurrent_queue.h>
#include <tbb/parallel_for_each.h>
#include <thread_pool.hpp>

#include <EASTL/fixed_set.h>
#include <EASTL/fixed_vector.h>

#include <state/Pool.h>

#include <IteratorView.h>

#include <ResourceEventComponent.h>
#include <ResourceManager.h>

#include <ServerEventComponent.h>

#include <boost/range/adaptors.hpp>
#include <boost/math/constants/constants.hpp>

#include <KeyedRateLimiter.h>

#include <OneSyncVars.h>
#include <DebugAlias.h>

#include <StateBagComponent.h>

#include <citizen_util/object_pool.h>
#include <citizen_util/shared_reference.h>

#include "ByteReader.h"
#include "ByteWriter.h"
#include "NetGameEventPacket.h"

#ifdef STATE_FIVE
static constexpr int kNetObjectTypeBitLength = 4;
#elif defined(STATE_RDR3)
static constexpr int kNetObjectTypeBitLength = 5;
#endif

static constexpr int kSyncPacketMaxLength = 2400;
static constexpr int kPacketWarnLength = 1300;

namespace rl
{
	bool MessageBufferLengthHack::GetState()
	{
		return fx::IsLengthHack();
	}
}

CPool<fx::ScriptGuid>* g_scriptHandlePool;
std::shared_mutex g_scriptHandlePoolMutex;

enum class RequestControlFilterMode : int
{
	// Default is currently equivalent to FilterPlayer
	Default = -1,
	// NoFilter will not filter any control requests
	NoFilter = 0,
	// FilterPlayerSettled will filter control requests targeting player-controlled settled entities
	FilterPlayerSettled,
	// FilterPlayer will filter control requests targeting any player-controlled entities
	FilterPlayer,
	// FilterPlayerPlusNonPlayerSettled will filter control requests targeting player-controlled entities, or settled entities
	FilterPlayerPlusNonPlayerSettled,
	// FilterAll will filter all control requests, i.e. allow none
	FilterAll,
};

std::shared_ptr<ConVar<bool>> g_oneSyncEnabledVar;
std::shared_ptr<ConVar<bool>> g_oneSyncCulling;
std::shared_ptr<ConVar<bool>> g_oneSyncVehicleCulling;
std::shared_ptr<ConVar<bool>> g_oneSyncForceMigration;
std::shared_ptr<ConVar<bool>> g_oneSyncRadiusFrequency;
std::shared_ptr<ConVar<std::string>> g_oneSyncLogVar;
std::shared_ptr<ConVar<bool>> g_oneSyncWorkaround763185;
std::shared_ptr<ConVar<bool>> g_oneSyncBigMode;
std::shared_ptr<ConVar<bool>> g_oneSyncLengthHack;
std::shared_ptr<ConVar<bool>> g_experimentalOneSyncPopulation;
std::shared_ptr<ConVar<bool>> g_experimentalNetGameEventHandler;
std::shared_ptr<ConVar<fx::OneSyncState>> g_oneSyncVar;
std::shared_ptr<ConVar<bool>> g_oneSyncPopulation;
std::shared_ptr<ConVar<bool>> g_oneSyncARQ;

static std::shared_ptr<ConVar<bool>> g_networkedSoundsEnabledVar;
static bool g_networkedSoundsEnabled;

static std::shared_ptr<ConVar<bool>> g_networkedPhoneExplosionsEnabledVar;
static bool g_networkedPhoneExplosionsEnabled;

static std::shared_ptr<ConVar<bool>> g_networkedScriptEntityStatesEnabledVar;
static bool g_networkedScriptEntityStatesEnabled;

static std::shared_ptr<ConVar<bool>> g_protectServerEntitiesDeletionVar;
static bool g_protectServerEntitiesDeletion;

static std::shared_ptr<ConVar<int>> g_requestControlVar;
static std::shared_ptr<ConVar<int>> g_requestControlSettleVar;

static RequestControlFilterMode g_requestControlFilterState;
static int g_requestControlSettleDelay;

static uint32_t MakeHandleUniqifierPair(uint16_t objectId, uint16_t uniqifier)
{
	return ((uint32_t)objectId << 16) | (uint32_t)uniqifier;
}

static std::tuple<uint16_t, uint16_t> DeconstructHandleUniqifierPair(uint32_t pair)
{
	return {
		(uint16_t)(pair >> 16),
		(uint16_t)pair
	};
}

namespace fx
{
ClientEntityData::ClientEntityData(const sync::SyncEntityPtr& entity, uint64_t lastSent, bool isCreated)
	: entityWeak(entity), lastSent(lastSent), isCreated(isCreated)
{
	
}

sync::SyncEntityPtr ClientEntityData::GetEntity(fx::ServerGameState* sgs) const
{
	return entityWeak.lock();
}
}

extern tbb::concurrent_unordered_map<uint32_t, fx::EntityCreationState> g_entityCreationList;

static tbb::concurrent_queue<std::string> g_logQueue;

static std::condition_variable g_consoleCondVar;
static std::mutex g_consoleMutex;

static std::once_flag g_logOnceFlag;

static void Logv(const char* format, fmt::printf_args argumentList)
{
	if (!g_oneSyncLogVar->GetValue().empty())
	{
		std::call_once(g_logOnceFlag, []()
		{
			std::thread([]()
			{
				while (true)
				{
					{
						std::unique_lock<std::mutex> lock(g_consoleMutex);
						g_consoleCondVar.wait(lock);
					}

					static std::string lastLogFile;
					static FILE* file;

					if (lastLogFile != g_oneSyncLogVar->GetValue())
					{
						if (file)
						{
							fclose(file);
							file = nullptr;
						}

						if (!g_oneSyncLogVar->GetValue().empty())
						{
							file = _pfopen(MakeRelativeCitPath(g_oneSyncLogVar->GetValue()).c_str(), _P("w"));
						}

						lastLogFile = g_oneSyncLogVar->GetValue();
					}

					std::string str;

					while (g_logQueue.try_pop(str))
					{
						if (file)
						{
							fprintf(file, "%s", str.c_str());
						}
					}
				}
			}).detach();
		});

		if (strchr(format, '\n'))
		{
			g_logQueue.push(fmt::sprintf("[% 10d] ", msec().count()));
		}

		g_logQueue.push(fmt::vsprintf(format, argumentList));

		g_consoleCondVar.notify_all();
	}
}

template<typename... TArgs>
inline void Log(const char* msg, const TArgs&... args)
{
	Logv(msg, fmt::make_printf_args(args...));
}

#define GS_LOG(x, ...) \
	do \
	{ \
		if (!g_oneSyncLogVar->GetValue().empty()) \
		{ \
			Log(x, __VA_ARGS__); \
		} \
	} while (false)

namespace fx
{
static const glm::mat4x4 g_projectionMatrix = glm::perspective(90.0f, 4.f / 3.f, 0.1f, 1000.f);

struct ViewClips
{
	glm::vec4 nearClip;
	glm::vec4 farClip;
	glm::vec4 topClip;
	glm::vec4 bottomClip;
	glm::vec4 leftClip;
	glm::vec4 rightClip;

	ViewClips(const glm::mat4x4& matrix)
	{
		auto tpmatrix = glm::transpose(matrix);

		leftClip	= tpmatrix * glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f };
		rightClip	= tpmatrix * glm::vec4{ -1.0f, 0.0f, 0.0f, 1.0f };
		bottomClip	= tpmatrix * glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f };
		topClip		= tpmatrix * glm::vec4{ 0.0f, -1.0f, 0.0f, 1.0f };
		nearClip	= tpmatrix * glm::vec4{ 0.0f, 0.0f, 1.0f, 1.0f };
		farClip		= tpmatrix * glm::vec4{ 0.0f, 0.0f, -1.0f, 1.0f };
	}
};

static const ViewClips g_projectionClips{ g_projectionMatrix };

static bool IsInFrustum(const glm::vec3& pos, float radius, const glm::mat4x4& viewMatrix)
{
	auto viewCoords = viewMatrix * glm::vec4{ pos, 1.0f };

	auto testPlane = [&viewCoords, &radius](const glm::vec4& plane)
	{
		glm::vec3 mcoords = viewCoords * plane;

		return (mcoords.x + mcoords.y + mcoords.z + plane.w + radius) >= 0.0f;
	};

	return (testPlane(g_projectionClips.nearClip)
		&& testPlane(g_projectionClips.bottomClip)
		&& testPlane(g_projectionClips.topClip)
		&& testPlane(g_projectionClips.leftClip)
		&& testPlane(g_projectionClips.rightClip)
		&& testPlane(g_projectionClips.farClip));
}

sync::SyncEntityState::SyncEntityState()
	: deleting(false), lastOutOfBandTimestamp(0)
{

}

static auto CreateSyncData(ServerGameState* state, const fx::ClientSharedPtr& client)
{
	auto lock = client->AcquireSyncDataCreationLock();

	if (auto existingData = client->GetSyncData())
	{
		return existingData;
	}

	fx::ClientWeakPtr weakClient(client);

	auto data = std::make_shared<GameStateClientData>();
	data->client = weakClient;

	std::weak_ptr<GameStateClientData> weakData(data);

	auto setupBag = [weakClient, weakData, state]()
	{
		auto client = weakClient.lock();
		auto data = weakData.lock();

		if (client && data)
		{
			if (client->HasConnected())
			{
				data->playerBag = state->GetStateBags()->RegisterStateBag(fmt::sprintf("player:%d", client->GetNetId()));

				if (fx::IsBigMode())
				{
					data->playerBag->AddRoutingTarget(client->GetSlotId());
				}

				data->playerBag->SetOwningPeer(client->GetSlotId());
			}
		}
	};

	if (client->HasConnected())
	{
		setupBag();
	}
	else
	{
		client->OnAssignNetId.Connect([setupBag](const uint32_t previousNetId)
		{
			setupBag();
		},
		INT32_MAX);
	}

	client->SetSyncData(data);
	client->OnDrop.Connect([weakClient, state]()
	{
		auto client = weakClient.lock();

		if (client)
		{
			auto slotId = client->GetSlotId();
			auto netId = client->GetNetId();

			gscomms_execute_callback_on_sync_thread([state, client, slotId, netId]()
			{
				state->HandleClientDrop(client, netId, slotId);
			});
		}
	});

	return data;
}

inline std::shared_ptr<GameStateClientData> GetClientDataUnlocked(ServerGameState* state, const fx::ClientSharedPtr& client)
{
	std::shared_ptr<GameStateClientData> data = client->GetSyncData();
	if (data)
	{
		return data;
	}

	return CreateSyncData(state, client);
}

inline std::tuple<std::unique_lock<std::mutex>, std::shared_ptr<GameStateClientData>> GetClientData(ServerGameState* state, const fx::ClientSharedPtr& client)
{
	auto val = GetClientDataUnlocked(state, client);

	if (!val)
	{
		return {};
	}

	std::unique_lock<std::mutex> lock(val->selfMutex);
	return { std::move(lock), val };
}

inline uint32_t MakeEntityHandle(uint16_t objectId)
{
	return objectId;
}

std::tuple<std::unique_lock<std::mutex>, std::shared_ptr<GameStateClientData>> ServerGameState::ExternalGetClientData(const fx::ClientSharedPtr& client)
{
	return GetClientData(this, client);
}


static const char* PopTypeToString(fx::sync::ePopType type)
{
	using namespace fx;

	switch (type)
	{
		case fx::sync::POPTYPE_UNKNOWN:
			return "POPTYPE_UNKNOWN";
		case fx::sync::POPTYPE_RANDOM_PERMANENT:
			return "POPTYPE_RANDOM_PERMANENT";
		case fx::sync::POPTYPE_RANDOM_PARKED:
			return "POPTYPE_RANDOM_PARKED";
		case fx::sync::POPTYPE_RANDOM_PATROL:
			return "POPTYPE_RANDOM_PATROL";
		case fx::sync::POPTYPE_RANDOM_SCENARIO:
			return "POPTYPE_RANDOM_SCENARIO";
		case fx::sync::POPTYPE_RANDOM_AMBIENT:
			return "POPTYPE_RANDOM_AMBIENT";
		case fx::sync::POPTYPE_PERMANENT:
			return "POPTYPE_PERMANENT";
		case fx::sync::POPTYPE_MISSION:
			return "POPTYPE_MISSION";
		case fx::sync::POPTYPE_REPLAY:
			return "POPTYPE_REPLAY";
		case fx::sync::POPTYPE_CACHE:
			return "POPTYPE_CACHE";
		case fx::sync::POPTYPE_TOOL:
			return "POPTYPE_TOOL";
		default:
			return "";
	}
}

static const char* TypeToString(fx::sync::NetObjEntityType type)
{
	using namespace fx;

	switch (type)
	{
		case sync::NetObjEntityType::Automobile:
			return "Automobile";
		case sync::NetObjEntityType::Bike:
			return "Bike";
		case sync::NetObjEntityType::Boat:
			return "Boat";
		case sync::NetObjEntityType::Door:
			return "Door";
		case sync::NetObjEntityType::Heli:
			return "Heli";
		case sync::NetObjEntityType::Object:
			return "Object";
		case sync::NetObjEntityType::Ped:
			return "Ped";
		case sync::NetObjEntityType::Pickup:
			return "Pickup";
		case sync::NetObjEntityType::PickupPlacement:
			return "PickupPlacement";
		case sync::NetObjEntityType::Plane:
			return "Plane";
		case sync::NetObjEntityType::Submarine:
			return "Submarine";
		case sync::NetObjEntityType::Player:
			return "Player";
		case sync::NetObjEntityType::Trailer:
			return "Trailer";
		case sync::NetObjEntityType::Train:
			return "Train";
	}

	return "UNKNOWN";
}

struct EntityImpl : sync::Entity
{
	EntityImpl(const sync::SyncEntityPtr& ent)
		: ent(ent)
	{

	}

	bool IsPlayer() override
	{
		return ent->type == sync::NetObjEntityType::Player;
	}

	fx::ClientSharedPtr GetOwner() override
	{
		return ent->GetClient();
	}

	glm::vec3 GetPosition() override
	{
		if (ent->syncTree)
		{
			float positionFloat[3];
			ent->syncTree->GetPosition(positionFloat);

			return { positionFloat[0], positionFloat[1], positionFloat[2] };
		}
		
		return {};
	}

private:
	sync::SyncEntityPtr ent;

	// Inherited via Entity
	virtual uint32_t GetId() override
	{
		return ent->handle;
	}
	virtual std::string GetPopType() override
	{
		auto popType = fx::sync::POPTYPE_UNKNOWN;

		if (ent->syncTree)
		{
			ent->syncTree->GetPopulationType(&popType);
		}

		return PopTypeToString(popType);
	}
	virtual uint32_t GetModel() override
	{
		uint32_t model = 0;

		if (ent->syncTree)
		{
			ent->syncTree->GetModelHash(&model);
		}

		return model;
	}
	virtual std::string GetType() override
	{
		return TypeToString(ent->type);
	}
};

void ServerGameState::ForAllEntities(const std::function<void(sync::Entity*)>& cb)
{
	std::shared_lock _(m_entityListMutex);

	for (auto& entity : m_entityList)
	{
		EntityImpl ent(entity);
		cb(&ent);
	}
}

uint32_t ServerGameState::MakeScriptHandle(const fx::sync::SyncEntityPtr& ptr)
{
	// only one unique lock here since we can't upgrade from reader to writer
	std::unique_lock<std::shared_mutex> guidLock(ptr->guidMutex);

	if (!ptr->guid)
	{
		{
			std::shared_lock<std::shared_mutex> _(g_scriptHandlePoolMutex);

			// find an existing handle (transformed TempEntity?)
			for (int i = 0; i < g_scriptHandlePool->m_Size; i++)
			{
				auto hdl = g_scriptHandlePool->GetAt(i);

				if (hdl && !hdl->reference && hdl->type == ScriptGuid::Type::Entity && hdl->entity.handle == ptr->handle)
				{
					hdl->reference = ptr.get();
					ptr->guid = hdl;
					break;
				}
			}
		}

		if (!ptr->guid)
		{
			std::unique_lock<std::shared_mutex> _(g_scriptHandlePoolMutex);

			auto guid = new ScriptGuid();
			guid->type = ScriptGuid::Type::Entity;
			guid->entity.handle = ptr->handle;
			guid->reference = ptr.get();

			ptr->guid = guid;
		}
	}

#ifdef VALIDATE_SCRIPT_GUIDS
	if (ptr->guid->entity.handle != ptr->handle)
	{
		__debugbreak();
	}
#endif

	{
		std::shared_lock<std::shared_mutex> _(g_scriptHandlePoolMutex);
		auto guid = g_scriptHandlePool->GetIndex(ptr->guid) + 0x20000;

#ifdef VALIDATE_SCRIPT_GUIDS
		if (!g_scriptHandlePool->AtHandle(guid - 0x20000) || guid & 0x80)
		{
			__debugbreak();
		}

		bool bad = false;
		fx::ClientSharedPtr badClient;

		m_instance->GetComponent<fx::ClientRegistry>()->ForAllClients([&](const fx::ClientSharedPtr& client)
		{
			try
			{
				if (fx::AnyCast<uint32_t>(client->GetData("playerEntity")) == guid)
				{
					bad = true;
					badClient = client;
				}
			}
			catch (std::bad_any_cast&)
			{
				
			}
		});

		if (bad && ptr->type != sync::NetObjEntityType::Player)
		{
			__debugbreak();
		}
#endif

		return guid;
	}
}

using FocusResult = eastl::fixed_vector<glm::vec3, 5>;

FocusResult GetPlayerFocusPos(const fx::sync::SyncEntityPtr& entity)
{
	auto syncTree = entity->syncTree;

	if (!syncTree)
	{
		return {};
	}

	float playerPos[3];
	syncTree->GetPosition(playerPos);

	auto camData = syncTree->GetPlayerCamera();

	if (!camData)
	{
		return { { playerPos[0], playerPos[1], playerPos[2] } };
	}

	switch (camData->camMode)
	{
	case 0:
	default:
		return { { playerPos[0], playerPos[1], playerPos[2] } };
	case 1:
		return {
			{ playerPos[0], playerPos[1], playerPos[2] },
			{ camData->freeCamPosX, camData->freeCamPosY, camData->freeCamPosZ }
		};
	case 2:
		return {
			{ playerPos[0], playerPos[1], playerPos[2] },
			{ playerPos[0] + camData->camOffX, playerPos[1] + camData->camOffY, playerPos[2] + camData->camOffZ }
		};
	}
}

ServerGameState::ServerGameState()
	: m_frameIndex(1), m_entitiesById(MaxObjectId), m_entityLockdownMode(EntityLockdownMode::Inactive)
{
#ifdef USE_ASYNC_SCL_POSTING
	m_tg = std::make_unique<ThreadPool>();
#endif

	fx::g_serverGameState = this;
}

fx::sync::SyncEntityPtr ServerGameState::GetEntity(uint8_t playerId, uint16_t objectId)
{
	if (objectId >= m_entitiesById.size() || objectId < 0)
	{
		return {};
	}

	uint16_t objIdAlias = objectId;
	debug::Alias(&objIdAlias);

	std::shared_lock _lock(m_entitiesByIdMutex);
	return m_entitiesById[objectId].lock();
}

fx::sync::SyncEntityPtr ServerGameState::GetEntity(uint32_t guid)
{
	// subtract the minimum index GUID
	guid -= 0x20000;

	// get the pool entry
	std::shared_lock<std::shared_mutex> _(g_scriptHandlePoolMutex);
	auto guidData = g_scriptHandlePool->AtHandle(guid);

	if (guidData)
	{
		if (guidData->type == ScriptGuid::Type::Entity)
		{
			std::shared_lock _lock(m_entitiesByIdMutex);
			return m_entitiesById[guidData->entity.handle].lock();
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
			std::unique_lock<std::shared_mutex> _(g_scriptHandlePoolMutex);
			delete guid;

			guid = nullptr;
		}
	}
}

struct FrameIndex
{
	union
	{
		struct
		{
			uint64_t lastFragment : 1;
			uint64_t currentFragment : 7;
			uint64_t frameIndex : 56;
		};

		uint64_t full;
	};

	FrameIndex()
		: full(0)
	{
	}

	FrameIndex(uint64_t idx)
		: full(idx)
	{
	}
};

static void FlushBuffer(rl::MessageBuffer& buffer, uint32_t msgType, uint64_t frameIndex, const fx::ClientSharedPtr& client, uint32_t* fragmentIndex = nullptr, bool finalFlush = false)
{
	// condition is commented out to indicate this *was* there before without having to dig through commit history
	// not sending any blank frames to clients leads to them not ACKing any frames, which will lead to an infinite buildup of frame states on server
	//if (buffer.GetDataLength() > 0)
	{
		// end
		buffer.Write(3, 7);

		// compress and send
		std::vector<char> outData(LZ4_compressBound(buffer.GetDataLength()) + 4 + 8);
		int len = LZ4_compress_default(reinterpret_cast<const char*>(buffer.GetBuffer().data()), outData.data() + 4 + 8, buffer.GetDataLength(), outData.size() - 4 - 8);

		FrameIndex newFrame{};
		newFrame.frameIndex = frameIndex;
		if (fragmentIndex != nullptr)
		{
			newFrame.lastFragment = finalFlush;
			newFrame.currentFragment = ++*fragmentIndex;
		}

		*(uint32_t*)(outData.data()) = msgType;
		*(uint64_t*)(outData.data() + 4) = newFrame.full;

		int bufferLen = len + 4 + 8;
		net::Buffer netBuffer(reinterpret_cast<uint8_t*>(outData.data()), bufferLen);
		netBuffer.Seek(bufferLen); // since the buffer constructor doesn't actually set the offset

	    if (bufferLen >= kPacketWarnLength)
		{
			console::DPrintf("net", "Sending a large packet (%d compressed bytes) to client %d, report if there will be any sync issues\n", bufferLen, client->GetNetId());
		}

		GS_LOG("flushBuffer: sending %d bytes to %d\n", bufferLen, client->GetNetId());

		client->SendPacket(1, netBuffer, NetPacketType_Unreliable);

		size_t oldCurrentBit = buffer.GetCurrentBit();

		debug::Alias(&oldCurrentBit);
		debug::Alias(&len);

		buffer.SetCurrentBit(0);
	}
}

static void MaybeFlushBuffer(rl::MessageBuffer& buffer, uint32_t msgType, uint64_t frameIndex, const fx::ClientSharedPtr& client, size_t size = 0, uint32_t* fragmentIndex = nullptr)
{
	if (LZ4_compressBound(buffer.GetDataLength() + (size / 8)) > (1100 - 12 - 12))
	{
		FlushBuffer(buffer, msgType, frameIndex, client, fragmentIndex);
	}
}

void GameStateClientData::FlushAcks()
{
	auto clientRef = client.lock();

	if (clientRef)
	{
		FlushBuffer(ackBuffer, HashRageString("msgPackedAcks"), 0, clientRef);
	}
}

void GameStateClientData::MaybeFlushAcks()
{
	auto clientRef = client.lock();

	if (clientRef)
	{
		MaybeFlushBuffer(ackBuffer, HashRageString("msgPackedAcks"), 0, clientRef);
	}
}

void sync::SyncCommandList::Execute(const fx::ClientSharedPtr& client)
{
	static thread_local SyncCommandState scs(16384);
	auto& scsSelf = scs;

	scs.frameIndex = frameIndex;
	scs.client = client;

	uint32_t fragmentIndex = 0;

	scs.flushBuffer = [this, &scsSelf, &client, &fragmentIndex](bool finalFlush)
	{
		FlushBuffer(scsSelf.cloneBuffer, HashRageString("msgPackedClones"), frameIndex, client, &fragmentIndex, true);
	};

	scs.maybeFlushBuffer = [this, &scsSelf, &client, &fragmentIndex](size_t plannedBits)
	{
		MaybeFlushBuffer(scsSelf.cloneBuffer, HashRageString("msgPackedClones"), frameIndex, client, plannedBits, &fragmentIndex);
	};

	auto c_key = commands.flush();
	while (c_key != nullptr)
	{
		auto to_run = c_key->get(&sync::SyncCommand::commandKey);
		c_key = c_key->next;

		to_run->callback(scs);
		syncPool.destruct(to_run);
	}

	scs.flushBuffer(true);

	scs.Reset();
}

#ifdef STATE_FIVE
static auto GetTrain(fx::ServerGameState* sgs, uint32_t objectId) -> fx::sync::SyncEntityPtr
{
	if (objectId != 0)
	{
		auto entity = sgs->GetEntity(0, objectId);

		if (entity && entity->type == sync::NetObjEntityType::Train && entity->syncTree)
		{
			return entity;
		}
	}

	return {};
};

static auto GetNextTrain(fx::ServerGameState* sgs, const fx::sync::SyncEntityPtr& entity) -> fx::sync::SyncEntityPtr
{
	if (auto trainState = entity->syncTree->GetTrainState())
	{
		return GetTrain(sgs, trainState->linkedToBackwardId);
	}

	return {};
};
#endif

void ServerGameState::Tick(fx::ServerInstanceBase* instance)
{
	// #TOOD1SBIG: limit tick rate divisor somewhat more sanely (currently it's 'only' 12.5ms as tick rate was upped from 30fps to 50fps)
	static uint32_t ticks = 0;
	ticks++;

	int tickMul = 1;
	auto gs = m_instance->GetComponent<fx::GameServer>();

	if (!gs->UseAccurateSends())
	{
		tickMul = 2;
	}

	if ((ticks % (3 * tickMul)) != 1)
	{
		return;
	}

	if (g_oneSyncARQ->GetValue())
	{
		m_syncStyle = SyncStyle::ARQ;
	}
	else
	{
		m_syncStyle = SyncStyle::NAK;
	}

	// approximate amount of ticks per second, 120 is svSync from GameServer.cpp
	int effectiveTicksPerSecond = (120 / (3 * tickMul));

	UpdateWorldGrid(instance);

	UpdateEntities();

	fwRefContainer<ServerEventComponent> sec = m_instance->GetComponent<ServerEventComponent>();
	fwRefContainer<fx::ResourceManager> resMan = m_instance->GetComponent<fx::ResourceManager>();
	fwRefContainer<fx::ResourceEventManagerComponent> evMan = resMan->GetComponent<fx::ResourceEventManagerComponent>();

	// cache entities so we don't have to iterate the concurrent_map for each client
	static std::tuple<
		fx::sync::SyncEntityPtr,
		glm::vec3,
		sync::CVehicleGameStateNodeData*,
		fx::ClientWeakPtr
	> relevantEntities[MaxObjectId + 1];

	int maxValidEntity = 0;

	{
		std::unique_lock _(m_entityListMutex);
		for (auto entityIt = m_entityList.begin(), entityEnd = m_entityList.end(); entityIt != entityEnd;)
		{
			auto entity = *entityIt;
			auto entityItOld = entityIt;
			++entityIt;

			entity->frameIndex = m_frameIndex;

			if (!entity->syncTree)
			{
				continue;
			}

			// if entity relevant to nobody
			{
				std::lock_guard _(entity->guidMutex);
				
				if (entity->relevantTo.none())
				{
					// entity was requested as delete, nobody knows of it anymore: finalize
					if (entity->deleting)
					{
						FinalizeClone({}, entity, entity->handle, 0, "Requested deletion");
						continue;
					}
					// it's a client-owned entity, let's check for a few things
					else if (entity->IsOwnedByClientScript() && entity->orphanMode != sync::KeepEntity)
					{
						// is the original owner offline?
						if (entity->firstOwnerDropped)
						{
							// we can delete
							FinalizeClone({}, entity, entity->handle, 0, "First owner dropped");
							continue;
						}
					}
					// it's a script-less entity, we can collect it.
					else if (!entity->IsOwnedByScript() && (entity->type != sync::NetObjEntityType::Player || !entity->GetClient()) && entity->orphanMode != sync::KeepEntity)
					{
						FinalizeClone({}, entity, entity->handle, 0, "Regular entity GC");
						continue;
					}
				}
			}

			float position[3];
			entity->syncTree->GetPosition(position);

			glm::vec3 entityPosition(position[0], position[1], position[2]);

			GS_LOG("found relevant entity %d for %d clients with position (%f, %f, %f)\n", entity->handle, entity->relevantTo.count(), entityPosition.x, entityPosition.y, entityPosition.z);

			sync::CVehicleGameStateNodeData* vehicleData = nullptr;

			if (entity->type == sync::NetObjEntityType::Automobile ||
				entity->type == sync::NetObjEntityType::Bike ||
				entity->type == sync::NetObjEntityType::Boat ||
				entity->type == sync::NetObjEntityType::Heli ||
				entity->type == sync::NetObjEntityType::Plane ||
				entity->type == sync::NetObjEntityType::Submarine ||
				entity->type == sync::NetObjEntityType::Trailer ||
#ifdef STATE_RDR3
				entity->type == sync::NetObjEntityType::DraftVeh ||
#endif
				entity->type == sync::NetObjEntityType::Train)
			{
				vehicleData = entity->syncTree->GetVehicleGameState();
			}
			
			relevantEntities[maxValidEntity] = { entity, entityPosition, vehicleData, entity->GetClient() };
			maxValidEntity++;
		}
	}

	auto creg = instance->GetComponent<fx::ClientRegistry>();

	int initSlot = ((fx::IsBigMode()) ? MAX_CLIENTS : 129) - 1;

	static int lastUpdateSlot = initSlot;
	int iterations = 0;
	int slot = lastUpdateSlot;
	
	while (iterations < ((fx::IsBigMode() ? 8 : 16) * tickMul))
	{
		iterations++;

		fx::ClientSharedPtr client;

		while (!client)
		{
			client = creg->GetClientBySlotID(slot);
			slot--;

			if (slot < 0)
			{
				slot = initSlot;
			}

			if (slot == lastUpdateSlot)
			{
				break;
			}
		}

		if (!client)
		{
			break;
		}

		auto clientDataUnlocked = GetClientDataUnlocked(this, client);

		sync::SyncEntityPtr playerEntity;
		{
			std::shared_lock _lock(clientDataUnlocked->playerEntityMutex);
			playerEntity = clientDataUnlocked->playerEntity.lock();
		}

		FocusResult playerPosns;

		if (playerEntity)
		{
			playerPosns = GetPlayerFocusPos(playerEntity);
		}

		auto slotId = client->GetSlotId();
		auto netId = client->GetNetId();

		auto& currentSyncedEntities = clientDataUnlocked->syncedEntities;
		decltype(clientDataUnlocked->syncedEntities) newSyncedEntities{};

		for (int entityIndex = 0; entityIndex < maxValidEntity; entityIndex++)
		{
			const auto& [entity, entityPos, vehicleData, entityClientWeak] = relevantEntities[entityIndex];
			auto entityClient = entityClientWeak.lock();
			auto ownsEntity = entityClient && entityClient->GetNetId() == client->GetNetId();

			{
				std::shared_lock _(entity->guidMutex);
				if (entity->deletedFor.test(slotId))
				{
					continue;
				}
			}
			
			bool isRelevant = (g_oneSyncCulling->GetValue()) ? false : true;

			if (ownsEntity && entity->type == fx::sync::NetObjEntityType::Player)
			{
				isRelevant = true;
			}

			auto isRelevantViaPos = [&, this](const fx::sync::SyncEntityPtr& entity, const glm::vec3& entityPos)
			{
				if (playerEntity)
				{
					for (auto& playerPos : playerPosns)
					{
						float diffX = entityPos.x - playerPos.x;
						float diffY = entityPos.y - playerPos.y;

						float distSquared = (diffX * diffX) + (diffY * diffY);
						if (distSquared < entity->GetDistanceCullingRadius(clientDataUnlocked->GetPlayerCullingRadius()))
						{
							return true;
						}
					}

					// are we owning the world grid in which this entity exists?
					int sectorX = std::max(entityPos.x + 8192.0f, 0.0f) / 150;
					int sectorY = std::max(entityPos.y + 8192.0f, 0.0f) / 150;

					auto selfBucket = clientDataUnlocked->routingBucket;

					std::shared_lock _(m_worldGridsMutex);
					const auto& grid = m_worldGrids[selfBucket];

					if (grid && sectorX >= 0 && sectorY >= 0 && sectorX < 256 && sectorY < 256)
					{
						if (grid->accel.netIDs[sectorX][sectorY] == netId)
						{
							return true;
						}
					}
				}

				return false;
			};

			if (!isRelevant)
			{
				isRelevant = isRelevantViaPos(entity, entityPos);
			}

			// #TODO1S: improve logic for what should and shouldn't exist based on game code
			bool isPlayerOrVehicle = false;

			if (!isRelevant)
			{
				if (entity->type == sync::NetObjEntityType::Player)
				{
					isPlayerOrVehicle = true;

					if (!fx::IsBigMode())
					{
						isRelevant = true;
					}
					else
					{
						// #TODO1SBIG: check if the player is not acked when bigmode is made unreliable
					}
				}
				else if (entity->type == sync::NetObjEntityType::Automobile ||
					entity->type == sync::NetObjEntityType::Bike ||
					entity->type == sync::NetObjEntityType::Boat ||
					entity->type == sync::NetObjEntityType::Heli ||
					entity->type == sync::NetObjEntityType::Plane ||
					entity->type == sync::NetObjEntityType::Submarine ||
					entity->type == sync::NetObjEntityType::Trailer ||
#ifdef STATE_RDR3
					entity->type == sync::NetObjEntityType::DraftVeh ||
#endif
					entity->type == sync::NetObjEntityType::Train)
				{
					if (vehicleData)
					{
						if (vehicleData->playerOccupants.any())
						{
							if (!g_oneSyncVehicleCulling->GetValue() && !fx::IsBigMode())
							{
								isRelevant = true;
							}

							isPlayerOrVehicle = true;
						}
					}
				}
			}

#ifdef STATE_FIVE
			// train chain linking: become relevant if any part of the chain is relevant
			if (!isRelevant && entity->type == sync::NetObjEntityType::Train)
			{
				auto trainState = entity->syncTree->GetTrainState();

				if (auto engine = GetTrain(this, trainState->engineCarriage))
				{
					IterateTrainLink(entity, [&isRelevant, isRelevantViaPos](sync::SyncEntityPtr& train) {
						float position[3];
						train->syncTree->GetPosition(position);

						glm::vec3 entityPosition(position[0], position[1], position[2]);
						isRelevant = isRelevantViaPos(train, entityPosition);

						// if we're not still relevant then we should keep going
						return !isRelevant;
					});
				}
			}
#endif

			// -------------------------------------------
			// -- DON'T CHANGE isRelevant TO TRUE BELOW --
			// -------------------------------------------
			//
			// this is a final pass, NO game logic
			// (only ownership overrides beyond this point)

			// don't route entities that haven't passed filter to others
			if (!entity->passedFilter && !ownsEntity)
			{
				isRelevant = false;
			}

			// don't route entities that aren't part of the routing bucket
			if (clientDataUnlocked->routingBucket != entity->routingBucket)
			{
				if (!(entity == playerEntity))
				{
					isRelevant = false;
				}
			}

			// if we own this entity, we need to assign as relevant.
			// -> even if not client-script, as if it isn't, we'll end up stuck without migrating it
			if (ownsEntity/* && entity->IsOwnedByClientScript()*/)
			{
				// we're fine
				if (isRelevant)
				{
					entity->wantsReassign = false;
				}

				// we want to reassign to someone else ASAP
				else
				{
					entity->wantsReassign = true;

					// but don't force it to exist for ourselves if it's not script-owned
					// (also, if not hasSynced, we can't tell if a script owns it or not, so we may delete early)
					if ((!entity->hasSynced || entity->IsOwnedByClientScript()) && entity->routingBucket == clientDataUnlocked->routingBucket)
					{
						isRelevant = true;
					}
				}
			}

			// if this is an owned entity and we're deleting it, delete the heck out of it
			// (and if not, why are we even trying?)
			if (entity->deleting)
			{
				// we should be relevant to delete if we own the entity...
				bool shouldDelete = ownsEntity;

				// ... or if it's currently created for us (as otherwise relevantTo won't clear as we never get a delete)
				if (!shouldDelete)
				{
					if (auto syncIt = currentSyncedEntities.find(MakeHandleUniqifierPair(entity->handle, entity->uniqifier)); syncIt != currentSyncedEntities.end())
					{
						auto& entityData = syncIt->second;

						if (entityData.hasCreated)
						{
							shouldDelete = true;
						}
					}
				}

				if (shouldDelete)
				{
					isRelevant = true;
				}
				else
				{
					isRelevant = false;

					// we aren't going to delete this entity as it is, reset relevantTo to be sure we won't block finalization
					std::lock_guard<std::shared_mutex> _(entity->guidMutex);
					entity->relevantTo.reset(slotId);
				}
			}

			// only update sync delay if should-be-created
			// isRelevant should **not** be updated after this
			auto syncDelay = 50ms;
			if (isRelevant && g_oneSyncRadiusFrequency->GetValue() && !isPlayerOrVehicle)
			{
				const auto& position = entityPos;

				if (entity->syncTree)
				{
					// get an average radius from a list of type radii (until we store modelinfo somewhere)
					float objRadius = 5.0f;

					switch (entity->type)
					{
					case sync::NetObjEntityType::Ped:
					case sync::NetObjEntityType::Player:
#ifdef STATE_RDR3
					case sync::NetObjEntityType::Animal:
					case sync::NetObjEntityType::Horse:
#endif
						objRadius = 2.5f;
						break;
					case sync::NetObjEntityType::Heli:
					case sync::NetObjEntityType::Boat:
					case sync::NetObjEntityType::Plane:
						objRadius = 15.0f;
						break;
					}

					if (!IsInFrustum(position, objRadius, clientDataUnlocked->viewMatrix))
					{
						syncDelay = 150ms;
					}

					if (playerEntity)
					{
						float dist = std::numeric_limits<float>::max();

						for (const auto& playerPos : playerPosns)
						{
							auto thisDist = glm::distance2(position, playerPos);
							dist = std::min(thisDist, dist);
						}

						if (dist > 500.0f * 500.0f)
						{
							syncDelay = 500ms;
						}
						else if (dist > 250.0f * 250.0f)
						{
							syncDelay = 250ms;
						}
						else if (dist < 35.0f * 35.0f)
						{
							syncDelay /= 4;
						}
					}
				}
			}
			else if (isPlayerOrVehicle)
			{
				if (playerEntity)
				{
					float dist = std::numeric_limits<float>::max();

					for (const auto& playerPos : playerPosns)
					{
						auto thisDist = glm::distance2(entityPos, playerPos);
						dist = std::min(thisDist, dist);
					}

					if (dist < 35.0f * 35.0f)
					{
						syncDelay /= 4;
					}
				}
			}

			auto entIdentifier = MakeHandleUniqifierPair(entity->handle, entity->uniqifier);

			// already syncing
			if (auto syncIt = currentSyncedEntities.find(entIdentifier); syncIt != currentSyncedEntities.end())
			{
				auto& entityData = syncIt->second;
				if (isRelevant)
				{
					const auto deltaTime = syncDelay - entityData.syncDelta;
					newSyncedEntities[entIdentifier] = { entityData.nextSync + deltaTime, syncDelay, entity, entityData.forceUpdate, entityData.hasCreated, false };
				}
				else if (entityData.hasCreated || entityData.hasNAckedCreate)
				{
					GS_LOG("destroying entity %d:%d for client %d due to scope exit\n", entity->handle, entity->uniqifier, client->GetNetId());
					clientDataUnlocked->entitiesToDestroy[entIdentifier] = { entity, { true, false } };
				}
			}
			
			// create entity here
			else if (isRelevant)
			{
				{
					std::lock_guard<std::shared_mutex> _(entity->guidMutex);
					entity->relevantTo.set(slotId);
					entity->outOfScopeFor.reset(slotId);
				}

				newSyncedEntities[entIdentifier] = { 0ms, syncDelay, entity, true, false, false };
			}
		}

		{
			std::unique_lock _(clientDataUnlocked->selfMutex);
			clientDataUnlocked->syncedEntities = std::move(newSyncedEntities);
		}
	}

	lastUpdateSlot = slot;

	auto curTime = msec();

	// gather client refs
	eastl::fixed_vector<fx::ClientSharedPtr, MAX_CLIENTS> clientRefs;

	// since we're doing essentially the same thing as what ForAllClients does, we use ForAllClientsLocked to prevent extra copies
	creg->ForAllClientsLocked([&clientRefs](const fx::ClientSharedPtr& clientRef)
	{
		clientRefs.push_back(clientRef);
	});

	tbb::parallel_for_each(clientRefs.begin(), clientRefs.end(), [this, curTime, effectiveTicksPerSecond, &sec, &evMan, &gs](const fx::ClientSharedPtr& clientRef)
	{
		// get our own pointer ownership
		auto client = clientRef;

		// no
		// #TODO: imagine if this mutates state alongside but after OnDrop clears it. WHAT COULD GO WRONG?
		// serialize OnDrop for gamestate onto the sync thread?
		if (!client->HasSlotId())
		{
			return;
		}

		auto slotId = client->GetSlotId();

		uint64_t time = curTime.count();

		NetPeerStackBuffer stackBuffer;
		gscomms_get_peer(client->GetPeer(), stackBuffer);
		auto enPeer = stackBuffer.GetBase();
		bool fakeSend = false;

		if (!enPeer || enPeer->GetPing() == -1)
		{
			// no peer, no connection, no service
			// we still need to process the player though so we can tell what to NAck!
			fakeSend = true;
		}

		SendArrayData(client);

		static fx::object_pool<sync::SyncCommandList, 512 * 1024> syncPool;
		auto scl = shared_reference<sync::SyncCommandList, &syncPool>::Construct(m_frameIndex);

		auto clientDataUnlocked = GetClientDataUnlocked(this, client);

		sync::SyncEntityPtr playerEntity;
		{
			std::shared_lock _lock(clientDataUnlocked->playerEntityMutex);
			playerEntity = clientDataUnlocked->playerEntity.lock();
		}

		FocusResult playerPosns;

		if (playerEntity)
		{
			playerPosns = GetPlayerFocusPos(playerEntity);
		}

	
		// process entities leaving our scope
		const auto& clientRegistry = m_instance->GetComponent<fx::ClientRegistry>();
		
		// if any relevant entities are getting deleted, add them to our removal list and remove them from the relevancy list.
		auto& syncedEntities = clientDataUnlocked->syncedEntities;
		auto& entitiesToDestroy = clientDataUnlocked->entitiesToDestroy;
		for (auto entityIt = syncedEntities.begin(), entityEnd = syncedEntities.end(); entityIt != entityEnd;)
		{
			auto [identPair, syncData] = *entityIt;
			auto oldIt = entityIt;

			++entityIt;

			auto& entity = syncData.entity;

			if (entity->deleting)
			{
				GS_LOG("deleting [obj:%d:%d] because it's deleting\n", entity->handle, entity->uniqifier);
				entitiesToDestroy[identPair] = { entity, { false, false } };
				syncedEntities.erase(oldIt);
			}
		}

		// new client entity state
		ClientEntityState ces;

		// process deletions
		for (auto& [_entityPair, _entity] : entitiesToDestroy)
		{
			auto [_objectId, _uniqifier] = DeconstructHandleUniqifierPair(_entityPair);
			auto objectId = _objectId;
			auto uniqifier = _uniqifier;
			auto [entity, _deletionData] = _entity;
			auto deletionData = _deletionData;

			if (entity)
			{
				{
					std::unique_lock _(entity->guidMutex);
					entity->relevantTo.reset(slotId);
				}

				// permanent deletion?
				if (!deletionData.outOfScope)
				{
					entity->deletedFor.set(slotId);
				}	
				// entity still exists, just going out of scope
				else
				{
					entity->outOfScopeFor.set(slotId);

					auto entityClient = entity->GetClient();

					// if the entity still exists, and this is the *owner* we're deleting it for
					if (entityClient && entityClient->GetNetId() == client->GetNetId())
					{
						// if this entity is owned by a server script, reassign to nobody and wait until someone else owns it
						if (entity->ShouldServerKeepEntity())
						{
							ReassignEntity(entity->handle, {});
						}

						// we should tell them their object ID is stolen
						// as ReassignEntity will mark the object ID as part of the steal pool
						deletionData.forceSteal = true;

						GS_LOG("marking object %d as stolen from client %d\n", objectId, client->GetNetId());

						// mark the object as stolen already, in case we're not stealing it later
						std::unique_lock lock(m_objectIdsMutex);
						m_objectIdsStolen.set(objectId);
					}
				}

				// marked as stolen? if so, yea
				{
					std::shared_lock lock(m_objectIdsMutex);
					deletionData.forceSteal = m_objectIdsStolen.test(objectId);
				}

				// delete player
				if (fx::IsBigMode())
				{
					if (entity->type == sync::NetObjEntityType::Player)
					{
						auto ownerRef = entity->GetClient();
						if (ownerRef)
						{
							const auto ownerNetId = ownerRef->GetNetId();

							if (ownerNetId != ~0U)
							{
								auto [clientDataLock, clientData] = GetClientData(this, client);

								auto plit = clientData->playersToSlots.find(ownerNetId);
								bool hasCreatedPlayer = (plit != clientData->playersToSlots.end());

								{
									if (hasCreatedPlayer)
									{
										int otherSlot = plit->second;

										sec->TriggerClientEvent("onPlayerDropped", fmt::sprintf("%d", client->GetNetId()), ownerNetId, ownerRef->GetName(), otherSlot);

										/*NETEV playerLeftScope SERVER
								/#*
								 * A server-side event that is triggered when a player leaves another player's scope.
								 *
								 * @param data - Data containing the players leaving each other's scope.
								 #/
								declare function playerLeftScope(data: {
									/#*
									 * The player that is leaving the scope.
									 #/
									player: string,

									/#*
									 * The player for which the scope is being left.
									 #/
									for: string
								}): void;
								*/
										evMan->QueueEvent2("playerLeftScope", {}, std::map<std::string, std::string>{ { "player", fmt::sprintf("%d", ownerNetId) }, { "for", fmt::sprintf("%d", client->GetNetId()) } });

										auto oldClientData = GetClientDataUnlocked(this, ownerRef);

										if (oldClientData->playerBag)
										{
											oldClientData->playerBag->RemoveRoutingTarget(client->GetSlotId());
										}

										clientData->playersInScope.reset(otherSlot);
										clientData->playersToSlots.erase(ownerNetId);
									}
								}
							}
						}
					}
				}
			}

			GS_LOG("Tick: deleting object %d@%d for %d\n", objectId, uniqifier, client->GetNetId());

			if (entity)
			{
				if (auto stateBag = entity->GetStateBag())
				{
					stateBag->RemoveRoutingTarget(slotId);
				}
			}

			// delet
			if (!fakeSend)
			{
				scl->EnqueueCommand([this, objectId, uniqifier, deletionData](sync::SyncCommandState& cmdState)
				{
					cmdState.maybeFlushBuffer(3 + 1 + 16 + 16);
					cmdState.cloneBuffer.Write(3, 3);

					cmdState.cloneBuffer.WriteBit(deletionData.forceSteal);
					cmdState.cloneBuffer.Write(13, int32_t(objectId));
					cmdState.cloneBuffer.Write(16, uniqifier);
				});
			}

			ces.deletions.push_back({ MakeHandleUniqifierPair(objectId, uniqifier), deletionData });
		}

		// #IFNAK
		if (m_syncStyle == SyncStyle::NAK)
		{
			entitiesToDestroy.clear();
		}

		for (auto syncIt = syncedEntities.begin(), syncItEnd = syncedEntities.end(); syncIt != syncItEnd;)
		{
			auto& [identPair, syncData] = *syncIt;
			auto [objectId, uniqifier] = DeconstructHandleUniqifierPair(identPair);
			auto& entity = syncData.entity;
			auto& forceUpdate = syncData.forceUpdate;

			// relevant entity owned by nobody, or wants a reassign? try to yoink it
			// (abuse clientMutex for wantsReassign safety)
			{
				std::unique_lock _(entity->clientMutex);
				auto cl = entity->GetClientUnsafe().lock();
				if (!cl || (entity->wantsReassign && cl->GetNetId() != client->GetNetId()))
				{
					entity->wantsReassign = false;
					ReassignEntity(entity->handle, client, std::move(_)); // transfer the lock inside
				}
			}

			if (!syncData.hasCreated)
			{
				bool canCreate = true;
				if (fx::IsBigMode())
				{
					if (entity->type == sync::NetObjEntityType::Player)
					{
						const auto entityClient = entity->GetClient();
						if (entityClient)
						{
							auto [clientDataLock, clientData] = GetClientData(this, client);

							auto plit = clientData->playersToSlots.find(entityClient->GetNetId());
							bool hasCreatedPlayer = (plit != clientData->playersToSlots.end());

							if (!hasCreatedPlayer)
							{
								constexpr const int kSlotIdStart = 
#ifdef STATE_RDR3
									30
#else
									127
#endif
;

								int slotId = kSlotIdStart;

								for (; slotId >= 0; slotId--)
								{
									if (slotId == 31)
									{
										slotId--;
									}

#ifdef STATE_RDR3
									if (slotId == 16)
									{
										slotId--;
									}
#endif

									if (!clientData->playersInScope.test(slotId))
									{
										break;
									}
								}

								if (slotId >= 0)
								{
									clientData->playersInScope[slotId] = entityClient->GetNetId();
									clientData->playersToSlots[entityClient->GetNetId()] = slotId;

									sec->TriggerClientEvent("onPlayerJoining", fmt::sprintf("%d", client->GetNetId()), entityClient->GetNetId(), entityClient->GetName(), slotId);

									auto ecData = GetClientDataUnlocked(this, entityClient);

									if (ecData->playerBag)
									{
										ecData->playerBag->AddRoutingTarget(client->GetSlotId());
									}

									/*NETEV playerEnteredScope SERVER
								/#*
								 * A server-side event that is triggered when a player enters another player's scope.
								 *
								 * @param data - Data containing the players entering each other's scope.
								 #/
								declare function playerEnteredScope(data: {
									/#*
									 * The player that is entering the scope.
									 #/
									player: string,

									/#*
									 * The player for which the scope is being entered.
									 #/
									for: string
								}): void;
								*/
									evMan->QueueEvent2("playerEnteredScope", {}, std::map<std::string, std::string>{ { "player", fmt::sprintf("%d", entityClient->GetNetId()) }, { "for", fmt::sprintf("%d", client->GetNetId()) } });
								}
								else
								{
									// oof
									canCreate = false;
								}
							}
						}
					}
				}

				if (!canCreate)
				{
					// darn
					auto oldSyncIt = syncIt;
					++syncIt;

					syncedEntities.erase(oldSyncIt);
					continue;
				}
				else
				{
					GS_LOG("creating entity %d for client %d\n", objectId, client->GetNetId());

					//yay
					forceUpdate = true;
					clientDataUnlocked->pendingCreates[identPair] = m_frameIndex;
				}
			}
			else if (clientDataUnlocked->pendingCreates.find(identPair) == clientDataUnlocked->pendingCreates.end())
			{
				auto entityClient = entity->GetClient();

				// we know the entity has been created, so we can try sending some entity RPC to 'em
				if (entityClient && client->GetNetId() == entityClient->GetNetId() && !entity->onCreationRPC.empty())
				{
					std::lock_guard<std::shared_mutex> _(entity->guidMutex);

					for (auto& entry : entity->onCreationRPC)
					{
						entry(client);
					}

					entity->onCreationRPC.clear();
				}
			}

			bool wasThisIt = false;

			if (entity->timestamp <= entity->lastOutOfBandTimestamp)
			{
				wasThisIt = !forceUpdate;

				forceUpdate = true;
				GS_LOG("Oh, is this it? %d <= %d\n", entity->timestamp, entity->lastOutOfBandTimestamp);
			}

			// don't tell players what to do with their own entities (unless we're forcing an update)
			if (entity->GetClient() == client && !forceUpdate)
			{
				++syncIt;
				continue;
			}

			uint64_t baseFrameIndex;
			uint64_t localLastFrameIndex = 0;

			if (forceUpdate && !wasThisIt)
			{
				baseFrameIndex = 0;

				std::lock_guard _(entity->frameMutex);
				entity->lastFramesSent[slotId] = 0;
				localLastFrameIndex = entity->lastFrameIndex;
			}
			else
			{
				std::lock_guard _(entity->frameMutex);
				baseFrameIndex = entity->lastFramesSent[slotId];
				localLastFrameIndex = entity->lastFrameIndex;
			}

			ces.syncedEntities[entity->handle] = { entity, baseFrameIndex, syncData.hasCreated };

			if (syncData.hasCreated && !syncData.hasRoutedStateBag)
			{
				if (auto stateBag = entity->GetStateBag())
				{
					syncData.hasRoutedStateBag = true;
					stateBag->AddRoutingTarget(slotId);
				}
			}

			// should we sync?
			if (forceUpdate || syncData.nextSync - curTime <= 0ms)
			{
				if (!forceUpdate)
				{
					syncData.nextSync = curTime + syncData.syncDelta;
				}

				bool wasForceUpdate = forceUpdate;
				forceUpdate = false;

				auto syncType = syncData.hasCreated ? 2 : 1;

				auto _ent = entity;

				auto runSync = [this, _ent, &syncType, curTime, &scl, baseFrameIndex, localLastFrameIndex, wasForceUpdate](auto&& preCb) 
				{
					scl->EnqueueCommand([this,
										entity = _ent,
										syncType,
										preCb = std::move(preCb),
										baseFrameIndex,
										localLastFrameIndex,
										curTime,
										wasForceUpdate](sync::SyncCommandState& cmdState) 
					{
						auto entityClient = entity->GetClient();
						if (!entityClient)
						{
							return;
						}

						auto slotId = cmdState.client->GetSlotId();

						if (slotId == -1)
						{
							return;
						}

						auto frameIndex = baseFrameIndex;
						bool isFirstFrameUpdate = false;
						preCb(frameIndex, isFirstFrameUpdate);

						// create a buffer once (per thread) to save allocations
						static thread_local rl::MessageBuffer mb(kSyncPacketMaxLength);

						mb.SetCurrentBit(0);

						sync::SyncUnparseState state(mb);
						state.syncType = syncType;
						state.targetSlotId = slotId;
						state.timestamp = 0;
						state.lastFrameIndex = frameIndex;
						state.isFirstUpdate = isFirstFrameUpdate;

						bool wroteData = entity->syncTree->Unparse(state);

						if (wroteData)
						{
							if (!cmdState.hadTime)
							{
								uint64_t time = curTime.count();

								cmdState.maybeFlushBuffer(3 + 32 + 32);
								cmdState.cloneBuffer.Write(3, 5);
								cmdState.cloneBuffer.Write(32, uint32_t(time & 0xFFFFFFFF));
								cmdState.cloneBuffer.Write(32, uint32_t((time >> 32) & 0xFFFFFFFF));

								cmdState.hadTime = true;
							}

							if (!isFirstFrameUpdate && syncType == 2)
							{
								// #IFNAK
								if (m_syncStyle == SyncStyle::NAK)
								{
									std::lock_guard _(entity->frameMutex);
									entity->lastFramesSent[slotId] = localLastFrameIndex;
								}
								// #IFARQ
								else
								{
									std::lock_guard _(entity->frameMutex);
									entity->lastFramesPreSent[slotId] = localLastFrameIndex;
								}
							}

							auto len = (state.buffer.GetCurrentBit() / 8) + 1;

							bool mayWrite = true;

							if (syncType == 2 && !isFirstFrameUpdate && wasForceUpdate && entity->GetClient() == cmdState.client)
							{
								mayWrite = false;
								len = 0;
							}

							auto startBit = cmdState.cloneBuffer.GetCurrentBit();
							cmdState.maybeFlushBuffer(3 + /* 13 */ 16 + 16 + 4 + 32 + 16 + 64 + 32 + 12 + (len * 8));
							cmdState.cloneBuffer.Write(3, syncType);
							cmdState.cloneBuffer.Write(13, entity->handle);
							cmdState.cloneBuffer.Write(16, entityClient->GetNetId());

							if (syncType == 1)
							{
								cmdState.cloneBuffer.Write(kNetObjectTypeBitLength, (uint8_t)entity->type);
								cmdState.cloneBuffer.Write(32, entity->creationToken);
							}

							if (isFirstFrameUpdate)
							{
								cmdState.cloneBuffer.Write(16, (uint16_t)(~entity->uniqifier));
							}
							else
							{
								cmdState.cloneBuffer.Write(16, (uint16_t)entity->uniqifier);
							}

							cmdState.cloneBuffer.Write(32, (uint32_t)(frameIndex >> 32));
							cmdState.cloneBuffer.Write(32, (uint32_t)frameIndex);

							cmdState.cloneBuffer.Write<uint32_t>(32, (syncType == 1) ?
								curTime.count() :
								(isFirstFrameUpdate) ? (curTime.count() + 1) : entity->timestamp);

							if (mayWrite)
							{
								cmdState.cloneBuffer.Write(12, len);
								cmdState.cloneBuffer.WriteBits(state.buffer.GetBuffer().data(), len * 8);
							}
							else
							{
								cmdState.cloneBuffer.Write(12, 0);
							}
						}
					});
				};

				if (!fakeSend)
				{
					runSync([](uint64_t&, bool&) {});

					if (syncType == 1)
					{
						// #IFNAK
						if (m_syncStyle == SyncStyle::NAK)
						{
							syncData.hasCreated = true;
						}

						// first-frame update
						syncType = 2;
						runSync([](uint64_t& lfi, bool& isLfi)
						{
							lfi = 0;
							isLfi = true;
						});
					}
				}
			}

			++syncIt;
		}

		// erase old frames
		{
			std::unique_lock _(clientDataUnlocked->selfMutex);
			auto& firstFrameState = clientDataUnlocked->firstSavedFrameState;
			if (firstFrameState != 0)
			{
				size_t thisMaxBacklog = maxSavedClientFrames;

				if (client->GetLastSeen() > 5s)
				{
					thisMaxBacklog = maxSavedClientFramesWorstCase;
				}

				// gradually remove if needed
				thisMaxBacklog = std::max(thisMaxBacklog, clientDataUnlocked->frameStates.size() - 2);

				// *should* only be looped once but meh
				while (m_frameIndex - firstFrameState >= thisMaxBacklog - 1)
				{
					clientDataUnlocked->frameStates.erase(firstFrameState);
					++firstFrameState;
				}
			}
			else
			{
				firstFrameState = m_frameIndex;
			}

			// emplace new frame
			clientDataUnlocked->frameStates.emplace(m_frameIndex, std::move(ces));
		}

#ifdef USE_ASYNC_SCL_POSTING
		// #TODO: syncing flag check and queue afterwards!
		if (!m_tg->tryPost([this, scl = std::move(scl), client]() mutable
			{
				scl->Execute(client);

				{
					auto [clientDataLock, clientData] = GetClientData(this, client);
					clientData->syncing = false;
				}

				scl = {};
				client = {};
			}))
		{
#ifndef _MSC_VER
			GS_LOG("Thread pool full?\n", 0);
#else
			GS_LOG("Thread pool full?\n");
#endif
		}
#else
		scl->Execute(client);
#endif
	
		GS_LOG("Tick completed for cl %d.\n", client->GetNetId());
	});

	for (int entityIndex = 0; entityIndex < maxValidEntity; entityIndex++)
	{
		relevantEntities[entityIndex] = {};
	}

	++m_frameIndex;
}

void ServerGameState::OnCloneRemove(const fx::sync::SyncEntityPtr& entity, const std::function<void()>& doRemove)
{
	// trigger a clone removal event
	gscomms_execute_callback_on_main_thread([this, entity, doRemove]()
	{
		auto evComponent = m_instance->GetComponent<fx::ResourceManager>()->GetComponent<fx::ResourceEventManagerComponent>();

		/*NETEV entityRemoved SERVER
		/#*
		 * Triggered when an entity is removed on the server.
		 *
		 * @param entity - The handle of the entity that got removed.
		 #/
		declare function entityRemoved(entity: number): void;
		*/
		evComponent->TriggerEvent2("entityRemoved", { }, MakeScriptHandle(entity));

		gscomms_execute_callback_on_sync_thread(doRemove);
	});

	// remove vehicle occupants
	if (entity->type == sync::NetObjEntityType::Ped ||
		entity->type == sync::NetObjEntityType::Player)
	{
		auto pedHandle = entity->handle;
		auto vehicleData = entity->syncTree->GetPedGameState();

		if (vehicleData)
		{
			auto curVehicle = (vehicleData->curVehicle != -1) ? GetEntity(0, vehicleData->curVehicle) : fx::sync::SyncEntityPtr{};
			auto curVehicleData = (curVehicle && curVehicle->syncTree) ? curVehicle->syncTree->GetVehicleGameState() : nullptr;

			if (curVehicleData && curVehicleData->occupants[vehicleData->curVehicleSeat] == pedHandle)
			{
				curVehicleData->occupants[vehicleData->curVehicleSeat] = 0;
				curVehicleData->playerOccupants.reset(vehicleData->curVehicleSeat);
			}
		}
	}
}

void ServerGameState::UpdateEntities()
{
	std::shared_lock<std::shared_mutex> lock(m_entityListMutex);

	auto time = msec();

	for (auto& entity : m_entityList)
	{
		if (!entity->syncTree)
		{
			continue;
		}

		// update client camera
		if (entity->type == sync::NetObjEntityType::Player)
		{
			fx::ClientSharedPtr client = entity->GetClient();

			if (client)
			{
				float playerPos[3];
				entity->syncTree->GetPosition(playerPos);

				auto camData = entity->syncTree->GetPlayerCamera();

				if (camData)
				{
					glm::vec3 camTranslate;

					switch (camData->camMode)
					{
					case 0:
					default:
						camTranslate = { playerPos[0], playerPos[1], playerPos[2] };
						break;
					case 1:
						camTranslate = { camData->freeCamPosX, camData->freeCamPosY, camData->freeCamPosZ };
						break;
					case 2:
						camTranslate = { playerPos[0] + camData->camOffX, playerPos[1] + camData->camOffY, playerPos[2] + camData->camOffZ };
						break;
					}

					glm::vec3 camRotation{ camData->cameraX, 0.0f, camData->cameraZ };
					auto camQuat = glm::quat{ camRotation };
					auto rot = glm::toMat4(camQuat);

					auto[dataLock, data] = GetClientData(this, client);
					data->viewMatrix = glm::inverse(glm::translate(glm::identity<glm::mat4>(), camTranslate) * rot);
				}
			}
		}
		else
		{
			// workaround: force-migrate a stuck entity
			if ((time - entity->lastReceivedAt) > 10s)
			{
				if (g_oneSyncForceMigration->GetValue() || fx::IsBigMode())
				{
					fx::ClientSharedPtr client = entity->GetClient();

					// #TODO: maybe non-client too?
					if (client)
					{
						MoveEntityToCandidate(entity, {});
					}

					// store the current time so we'll only try again in 10 seconds, not *the next frame*
					entity->lastReceivedAt = time;
				}
			}
		}

		// update vehicle seats, if it's a ped
		if (entity->type == sync::NetObjEntityType::Ped ||
			entity->type == sync::NetObjEntityType::Player)
		{
			auto pedHandle = entity->handle;
			auto vehicleData = entity->syncTree->GetPedGameState();

			if (vehicleData)
			{
				if (vehicleData->lastVehicle != vehicleData->curVehicle || vehicleData->lastVehicleSeat != vehicleData->curVehicleSeat)
				{
					auto lastVehicle = (vehicleData->lastVehicle != -1) ? GetEntity(0, vehicleData->lastVehicle) : fx::sync::SyncEntityPtr{};
					auto curVehicle = (vehicleData->curVehicle != -1) ? GetEntity(0, vehicleData->curVehicle) : fx::sync::SyncEntityPtr{};

					auto lastVehicleData = (lastVehicle && lastVehicle->syncTree) ? lastVehicle->syncTree->GetVehicleGameState() : nullptr;
					auto curVehicleData = (curVehicle && curVehicle->syncTree) ? curVehicle->syncTree->GetVehicleGameState() : nullptr;

					if (lastVehicleData && lastVehicleData->occupants[vehicleData->lastVehicleSeat] == pedHandle)
					{
						lastVehicleData->occupants[vehicleData->lastVehicleSeat] = 0;
						lastVehicleData->playerOccupants.reset(vehicleData->lastVehicleSeat);
					}

					if (curVehicleData && curVehicleData->occupants[vehicleData->curVehicleSeat] == 0)
					{
						curVehicleData->occupants[vehicleData->curVehicleSeat] = pedHandle;
						curVehicleData->lastOccupant[vehicleData->curVehicleSeat] = pedHandle;

						if (entity->type == sync::NetObjEntityType::Player)
						{
							curVehicleData->playerOccupants.set(vehicleData->curVehicleSeat);
						}
					}

					vehicleData->lastVehicle = vehicleData->curVehicle;
					vehicleData->lastVehicleSeat = vehicleData->curVehicleSeat;
				}
			}
		}
	}
}

void ServerGameState::SendWorldGrid(void* entry /* = nullptr */, const fx::ClientSharedPtr& client /* = */ )
{
	auto sendWorldGrid = [this, entry](const fx::ClientSharedPtr& client)
	{
		auto data = GetClientDataUnlocked(this, client);

		decltype(m_worldGrids)::iterator gridRef;

		{
			std::shared_lock s(m_worldGridsMutex);
			gridRef = m_worldGrids.find(data->routingBucket);
		}

		if (gridRef != m_worldGrids.end())
		{
			auto& grid = gridRef->second;

			if (grid)
			{
				net::Buffer msg;
				msg.Write<uint32_t>(HashRageString("msgWorldGrid3"));

				uint32_t base = 0;
				uint32_t length = sizeof(grid->state[0]);

				if (entry)
				{
					base = ((WorldGridEntry*)entry - &grid->state[0].entries[0]) * sizeof(WorldGridEntry);
					length = sizeof(WorldGridEntry);
				}

				if (client->GetSlotId() == -1)
				{
					return;
				}

				// really bad way to snap the world grid data to a client's own base
				uint32_t baseRef = sizeof(grid->state[0]) * client->GetSlotId();
				uint32_t lengthRef = sizeof(grid->state[0]);

				if (base < baseRef)
				{
					base = baseRef;
				}
				else if (base > baseRef + lengthRef)
				{
					return;
				}

				// everyone's state starts at their own
				length = std::min(lengthRef, length);

				msg.Write<uint32_t>(base - baseRef);
				msg.Write<uint32_t>(length);

				msg.Write(reinterpret_cast<char*>(grid->state) + base, length);

				client->SendPacket(1, msg, NetPacketType_Reliable);
			}
		}
	};

	if (client)
	{
		sendWorldGrid(client);		
	}
	else
	{
		m_instance->GetComponent<fx::ClientRegistry>()->ForAllClients([&sendWorldGrid](const fx::ClientSharedPtr& client)
		{
			sendWorldGrid(client);
		});
	}
}

void ServerGameState::UpdateWorldGrid(fx::ServerInstanceBase* instance)
{
	auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

	// clean any non-existent clients from world grid accel periodically
	static std::chrono::milliseconds nextWorldCheck;
	auto now = msec();

	if (now >= nextWorldCheck)
	{
		{
			std::shared_lock _(m_worldGridsMutex);

			for (auto& [id, grid] : m_worldGrids)
			{
				if (!grid)
				{
					continue;
				}

				for (size_t x = 0; x < std::size(grid->accel.netIDs); x++)
				{
					for (size_t y = 0; y < std::size(grid->accel.netIDs[x]); y++)
					{
						if (grid->accel.netIDs[x][y] != 0xFFFF && !clientRegistry->GetClientByNetID(grid->accel.netIDs[x][y]))
						{
							grid->accel.netIDs[x][y] = 0xFFFF;
						}
					}
				}
			}

			nextWorldCheck = now + 10s;
		}

		eastl::fixed_set<int, 128> liveBuckets;

		auto creg = m_instance->GetComponent<fx::ClientRegistry>();
		creg->ForAllClients([this, &liveBuckets](const fx::ClientSharedPtr& client)
		{
			auto data = GetClientDataUnlocked(this, client);
			liveBuckets.insert(data->routingBucket);
		});

		{
			std::unique_lock _(m_worldGridsMutex);
			for (auto it = m_worldGrids.begin(); it != m_worldGrids.end();)
			{
				if (liveBuckets.find(it->first) == liveBuckets.end())
				{
					it = m_worldGrids.erase(it);
				}
				else
				{
					it++;
				}
			}
		}

		{
			std::unique_lock _(m_arrayHandlersMutex);
			for (auto it = m_arrayHandlers.begin(); it != m_arrayHandlers.end();)
			{
				if (liveBuckets.find(it->first) == liveBuckets.end())
				{
					it = m_arrayHandlers.erase(it);
				}
				else
				{
					it++;
				}
			}
		}
	}

	// update world grid
	clientRegistry->ForAllClients([&](const fx::ClientSharedPtr& client)
	{
		auto slotID = client->GetSlotId();

		if (slotID == -1)
		{
			return;
		}

		auto data = GetClientDataUnlocked(this, client);

		sync::SyncEntityPtr playerEntity;
		{
			std::shared_lock _lock(data->playerEntityMutex);
			playerEntity = data->playerEntity.lock();
		}

		if (!playerEntity)
		{
			return;
		}

		auto posns = GetPlayerFocusPos(playerEntity);

		if (posns.empty())
		{
			return;
		}

		// only set world grid for the most focused position
		const auto& pos = posns[0];

		int minSectorX = std::max((pos.x - 299.0f) + 8192.0f, 0.0f) / 150;
		int maxSectorX = std::max((pos.x + 299.0f) + 8192.0f, 0.0f) / 150;
		int minSectorY = std::max((pos.y - 299.0f) + 8192.0f, 0.0f) / 150;
		int maxSectorY = std::max((pos.y + 299.0f) + 8192.0f, 0.0f) / 150;

		decltype(m_worldGrids)::iterator gridRef;

		{
			std::shared_lock s(m_worldGridsMutex);
			gridRef = m_worldGrids.find(data->routingBucket);

			if (gridRef == m_worldGrids.end())
			{
				s.unlock();
				std::unique_lock l(m_worldGridsMutex);
				gridRef = m_worldGrids.emplace(data->routingBucket, std::make_unique<WorldGrid>()).first;
			}
		}

		auto& grid = gridRef->second;

		if (!grid)
		{
			return;
		}

		if (minSectorX < 0 || minSectorX > std::size(grid->accel.netIDs) || minSectorY < 0 || minSectorY > std::size(grid->accel.netIDs[0]))
		{
			return;
		}

		auto netID = client->GetNetId();

		WorldGridState* gridState = &grid->state[slotID];

		// remove any cooldowns we're not at
		for (auto it = data->worldGridCooldown.begin(); it != data->worldGridCooldown.end();)
		{
			auto x = it->first >> 8;
			auto y = it->first & 0xFF;

			if (x < minSectorX || x > maxSectorX ||
				y < minSectorY || y > maxSectorY)
			{
				it = data->worldGridCooldown.erase(it);
			}
			else
			{
				++it;
			}
		}

		// disown any grid entries that aren't near us anymore
		for (auto& entry : gridState->entries)
		{
			if (entry.netID != 0xFFFF)
			{
				if (entry.sectorX < minSectorX || entry.sectorX > maxSectorX ||
					entry.sectorY < minSectorY || entry.sectorY > maxSectorY)
				{
					if (grid->accel.netIDs[entry.sectorX][entry.sectorY] == netID)
					{
						grid->accel.netIDs[entry.sectorX][entry.sectorY] = -1;
					}

					data->worldGridCooldown.erase((uint16_t(entry.sectorX) << 8) | entry.sectorY);

					entry.sectorX = 0;
					entry.sectorY = 0;
					entry.netID = -1;

					SendWorldGrid(&entry, client);
				}
			}
		}

		auto now = msec();

		// if we're settled enough in place, create our world grid chunks
		for (int x = minSectorX; x <= maxSectorX; x++)
		{
			for (int y = minSectorY; y <= maxSectorY; y++)
			{
				// find if this x/y is owned by someone already
				bool found = (grid->accel.netIDs[x][y] != 0xFFFF);

				// if not, find out if we even have any chance at having an entity in there (cooldown of ~5 ticks, might need a more accurate query somewhen)
				auto secAddr = (uint16_t(x) << 8) | y;

				if (!found)
				{
					auto entry = data->worldGridCooldown.find(secAddr);

					if (entry == data->worldGridCooldown.end())
					{
						entry = data->worldGridCooldown.emplace(secAddr, now + 100ms).first;
					}

					if (now < entry->second)
					{
						found = true;
					}
				}
				else
				{
					data->worldGridCooldown.erase(secAddr);
				}

				// is it free?
				if (!found)
				{
					// time to have some fun!

					// find a free entry slot
					for (auto& entry : gridState->entries)
					{
						if (entry.netID == 0xFFFF)
						{
							// and take it
							entry.sectorX = x;
							entry.sectorY = y;
							entry.netID = netID;

							grid->accel.netIDs[x][y] = netID;

							SendWorldGrid(&entry, client);

							break;
						}
					}
				}
			}
		}
	});
}

void ServerGameState::SetEntityLockdownMode(int bucket, EntityLockdownMode mode)
{
	std::unique_lock _(m_routingDataMutex);
	m_routingData[bucket].lockdownMode = mode;
}

void ServerGameState::SetPopulationDisabled(int bucket, bool disabled)
{
	std::unique_lock _(m_routingDataMutex);
	m_routingData[bucket].noPopulation = disabled;
}

// make sure you have a lock to the client mutex before calling this function!
void ServerGameState::ReassignEntityInner(uint32_t entityHandle, const fx::ClientSharedPtr& targetClient, std::unique_lock<std::shared_mutex>&& lockIn)
{
	auto entity = GetEntity(0, entityHandle);

	if (!entity)
	{
		return;
	}

	// a client can have only one player, handing a player to someone else would be really awkward
	if (entity->type == sync::NetObjEntityType::Player)
	{
		return;
	}

	// perform a final std::move on the lock
	std::unique_lock lock = std::move(lockIn);

	// if 'safe', we'll lock the clientMutex here (for use when the mutex isn't already locked)
	if (!lock)
	{
		lock = std::unique_lock<std::shared_mutex>{
			entity->clientMutex
		};
	}

	auto oldClientRef = entity->GetClientUnsafe().lock();
	{
		entity->lastMigratedAt = msec();

		entity->GetLastOwnerUnsafe() = oldClientRef;
		entity->GetClientUnsafe() = targetClient;

		if (auto stateBag = entity->GetStateBag())
		{
			if (targetClient)
			{
				stateBag->SetOwningPeer(targetClient->GetSlotId());
			}
			else
			{
				stateBag->SetOwningPeer(-1);
			}
		}

		GS_LOG("%s: obj id %d, old client %d, new client %d\n", __func__, entityHandle, (!oldClientRef) ? -1 : oldClientRef->GetNetId(), (targetClient) ? targetClient->GetNetId() : -1);

		if (oldClientRef)
		{
			auto [lock, sourceData] = GetClientData(this, oldClientRef);
			sourceData->objectIds.erase(entityHandle);
		}
	}

	{
		if (targetClient)
		{
			auto [lock, targetData] = GetClientData(this, targetClient);
			targetData->objectIds.insert(entityHandle);
		}
	}

	// force a resend to people who need one

	const auto uniqPair = MakeHandleUniqifierPair(entity->handle, entity->uniqifier);
	const auto& clientRegistry = m_instance->GetComponent<fx::ClientRegistry>();
	clientRegistry->ForAllClients([&, uniqPair](const fx::ClientSharedPtr& crClient)
	{
		if (!crClient->HasSlotId())
		{
			return;
		}

		const auto slotId = crClient->GetSlotId();
		{
			std::lock_guard _(entity->guidMutex);
			if (!entity->relevantTo.test(slotId))
			{
				return;
			}
		}

		auto [lock, data] = GetClientData(this, crClient);
		if (auto entIt = data->syncedEntities.find(uniqPair); entIt != data->syncedEntities.end())
		{
			entIt->second.forceUpdate = true;
		}
	});

	// when deleted, we want to make this object ID return to the global pool, not to the player who last owned it
	// therefore, mark it as stolen
	{
		std::unique_lock lock(m_objectIdsMutex);
		m_objectIdsStolen.set(entityHandle);
	}
}

#ifdef STATE_FIVE
/// <summary>
/// Takes the initialTrain and if its the engine train, iterates down the train link from the train, if it's not then it will try to get the engine
/// </summary>
/// <param name="initialTrain">The initial to start the iteration from, this will get the engine entity internally, if the engine doesn't exist it will early return as there's no valid part in the link to start from.</param>
/// <param name="fn">The function to call, if the function returns `true` it will keep iterating, if it returns `false` it will stop</param>
/// <param name="callOnInitialEntity">Whether the function should do the `fn` call on the initialTrain</param>
void ServerGameState::IterateTrainLink(const sync::SyncEntityPtr& initialTrain, std::function<bool(sync::SyncEntityPtr&)> fn, bool callOnInitialEntity)
{
	static constexpr uint16_t kMaxDuplicateEntityIds = 3;

	static thread_local std::unordered_set<uint32_t> processedTrains{};
	processedTrains.clear();

	// for most stuff we want to call on the intial entity
	if (callOnInitialEntity)
	{
		if (!fn(const_cast<sync::SyncEntityPtr&>(initialTrain)))
		{
			return;
		}
	}

	if (const auto trainState = initialTrain->syncTree->GetTrainState())
	{
		uint16_t duplicateEntityIds = 0;
		auto recurseTrain = [this, &duplicateEntityIds, &fn](const fx::sync::SyncEntityPtr& train)
		{
			for (auto link = GetNextTrain(this, train); link; link = GetNextTrain(this, link))
			{
				// train links back to another train that has already been processed, this shouldn't happen.
				if (!processedTrains.insert(link->handle).second)
				{
					// our linked trains are looped at least 3 times together
					if (++duplicateEntityIds > kMaxDuplicateEntityIds)
					{
						return;
					}

					continue;
				}

				if (!fn(link))
				{
					return;
				}
			}
		};

		if (trainState->isEngine)
		{
			recurseTrain(initialTrain);
			return;
		}

		if (trainState->engineCarriage && static_cast<uint32_t>(trainState->engineCarriage) != initialTrain->handle)
		{
			if (auto engine = GetTrain(this, trainState->engineCarriage))
			{
				if (!fn(engine))
				{
					return;
				}

				processedTrains.insert(engine->handle);
				recurseTrain(engine);
			}
		}
	}
}
#endif

void ServerGameState::ReassignEntity(uint32_t entityHandle, const fx::ClientSharedPtr& targetClient, std::unique_lock<std::shared_mutex>&& lock)
{
	ReassignEntityInner(entityHandle, targetClient, std::move(lock));

	// if this is a train, we want to migrate the entire train chain
	// this matches the logic in CNetObjTrain::_?TestProximityMigration
#ifdef STATE_FIVE
	if (auto train = GetTrain(this, entityHandle))
	{
		// game code works as follows:
		// -> if train isEngine, enumerate the entire list backwards and migrate that one along
		// -> if not isEngine, migrate the engine


		// This call expects link-handle != entityHandle
		// This will prevent
		// 1. double-locking clientMutex
		// 2. reassigning the same entity twice
		IterateTrainLink(train, [=](const fx::sync::SyncEntityPtr& link) {

			// we directly use ReassignEntityInner here to ensure no infinite recursion
			ReassignEntityInner(link->handle, targetClient);

			return true;
		}, false);
	}
#endif
}

bool ServerGameState::SetEntityStateBag(uint8_t playerId, uint16_t objectId, std::function<std::shared_ptr<StateBag>()> createStateBag) 
{
	if (auto entity = GetEntity(0, objectId))
	{
		if (entity->GetStateBag())
		{
			trace("Creating a new state bag while there's already a state bag on this entity, please report this.\n");
		}

		entity->SetStateBag(createStateBag());
		return true;
	}

	return false;
}

uint32_t ServerGameState::GetClientRoutingBucket(const fx::ClientSharedPtr& client)
{
	auto data = GetClientDataUnlocked(this, client);
	return data->routingBucket;
}

bool ServerGameState::MoveEntityToCandidate(const fx::sync::SyncEntityPtr& entity, const fx::ClientSharedPtr& client)
{
	// pickup placements at 0,0,0 are transient and shouldn't migrate
	if (entity->type == sync::NetObjEntityType::PickupPlacement)
	{
		float position[3] = { 0 };

		if (entity->syncTree)
		{
			entity->syncTree->GetPosition(position);
		}

		if (position[0] == 0.0f && position[1] == 0.0f)
		{
			return false;
		}
	}

	const auto& clientRegistry = m_instance->GetComponent<fx::ClientRegistry>();
	bool hasClient = true;

	{
		auto entityClient = entity->GetClient();

		if (!entityClient || !client)
		{
			hasClient = false;
		}
		else if (entityClient == client)
		{
			hasClient = false;
		}
	}

	if (!hasClient)
	{
		float position[3] = { 0 };

		if (entity->syncTree)
		{
			entity->syncTree->GetPosition(position);
		}

		glm::vec3 pos{
			position[0],
			position[1],
			position[2]
		};

		eastl::fixed_set<std::tuple<float, fx::ClientSharedPtr>, MAX_CLIENTS> candidates;

		uint32_t eh = entity->handle;
		auto candidateSet = entity->relevantTo;
		constexpr auto maxCandidates = 5;

		if (entity->type != sync::NetObjEntityType::Player)
		{
			for (auto bit = candidateSet.find_last(); bit != decltype(candidateSet)::kSize; bit = candidateSet.find_prev(bit))
			{
				auto tgtClient = clientRegistry->GetClientBySlotID(bit);

				if (!tgtClient || tgtClient == client)
				{
					continue;
				}

				float distance = std::numeric_limits<float>::max();

				auto data = GetClientDataUnlocked(this, tgtClient);
				sync::SyncEntityPtr playerEntity;
				{
					std::shared_lock _lock(data->playerEntityMutex);
					playerEntity = data->playerEntity.lock();
				}

				if (playerEntity)
				{
					auto tgts = GetPlayerFocusPos(playerEntity);

					if (pos.x != 0.0f && !tgts.empty())
					{
						distance = glm::distance2(tgts[0], pos);
					}
				}

				candidates.emplace(distance, tgtClient);

				if (candidates.size() >= maxCandidates)
				{
					break;
				}
			}
		}

		if (candidates.empty()) // no candidate?
		{
			if (entity->ShouldServerKeepEntity())
			{
				GS_LOG("no candidates for entity %d, assigning as unowned\n", entity->handle);
				ReassignEntity(entity->handle, {});
			}
			else
			{
				return false;
			}
		}
		else
		{
			auto& candidate = *candidates.begin();

			GS_LOG("reassigning entity %d from %s to %s\n", entity->handle, client ? client->GetName() : "", std::get<1>(candidate)->GetName());

			ReassignEntity(entity->handle, std::get<1>(candidate));
		}
	}

	return true;
}

void ServerGameState::HandleClientDrop(const fx::ClientSharedPtr& client, uint16_t netId, uint32_t slotId)
{
	if (!IsOneSync())
	{
		return;
	}

	const bool hasSlotId = slotId != 0xFFFFFFFF;

	auto clientRegistry = m_instance->GetComponent<fx::ClientRegistry>();

	GS_LOG("client drop - reassigning\n", 0);

	if (fx::IsBigMode())
	{
		clientRegistry->ForAllClients([this, &client, netId](const fx::ClientSharedPtr& tgtClient)
		{
			auto [lock, clientData] = GetClientData(this, tgtClient);

			auto si = clientData->playersToSlots.find(netId);

			if (si != clientData->playersToSlots.end())
			{
				fwRefContainer<ServerEventComponent> events = m_instance->GetComponent<ServerEventComponent>();
				events->TriggerClientEvent("onPlayerDropped", fmt::sprintf("%d", tgtClient->GetNetId()), netId, client->GetName(), si->second);

				clientData->playersInScope.reset(si->second);
				clientData->playersToSlots.erase(si);
			}
		});
	}

	// clear the player's world grid ownership
	ClearClientFromWorldGrid(client);

	std::set<uint32_t> toErase;

	{
		std::shared_lock<std::shared_mutex> lock(m_entityListMutex);

		for (auto& entity : m_entityList)
		{
			if (!entity->syncTree)
			{
				continue;
			}

			bool markedForDeletion = false;

			auto firstOwner = entity->GetFirstOwner();
			if (firstOwner && firstOwner->GetNetId() == client->GetNetId())
			{
				entity->firstOwnerDropped = true;

				if (entity->orphanMode == sync::DeleteOnOwnerDisconnect)
				{
					toErase.insert(entity->handle);
					markedForDeletion = true;
				}
			}

			if (hasSlotId)
			{
				{
					std::lock_guard<std::shared_mutex> _(entity->guidMutex);
					entity->relevantTo.reset(slotId);
					entity->deletedFor.reset(slotId);
					entity->outOfScopeFor.reset(slotId);
				}

				{
					std::lock_guard _(entity->frameMutex);
					entity->lastFramesSent[slotId] = 0;
				}
			}

			// we don't want to re-assign the entity at all, this entity should be deleted on the next tick
			if (markedForDeletion)
			{
				continue;
			}

			if (!MoveEntityToCandidate(entity, client))
			{
				if (entity->IsOwnedByClientScript() && !entity->firstOwnerDropped)
				{
					ReassignEntity(entity->handle, firstOwner);
				}
				else
				{
					toErase.insert(entity->handle);
				}
			}
		}
	}

	// here temporarily, needs to be unified with ProcessCloneRemove
	for (auto& set : toErase)
	{
		RemoveClone(client, set & 0xFFFF);
	}

	{
		// remove object IDs from sent map
		auto [ lock, data ] = GetClientData(this, client);

		std::unique_lock objectIdsLock(m_objectIdsMutex);

		for (auto& objectId : data->objectIds)
		{
			m_objectIdsSent.reset(objectId);
		}
	}

	// remove ACKs for this client
	if (hasSlotId)
	{
		std::shared_lock<std::shared_mutex> lock(m_entityListMutex);

		for (auto& entity : m_entityList)
		{
			if (entity->syncTree)
			{
				entity->syncTree->Visit([slotId](sync::NodeBase& node)
				{
					node.ackedPlayers.reset(slotId);

					return true;
				});
			}
		}
	}
}

void ServerGameState::ClearClientFromWorldGrid(const fx::ClientSharedPtr& targetClient)
{
	{
		std::shared_lock _(m_arrayHandlersMutex);

		for (auto& [ id, handlerList ] : m_arrayHandlers)
		{
			for (auto& handler : handlerList->handlers)
			{
				if (handler)
				{
					handler->PlayerHasLeft(targetClient);
				}
			}
		}
	}

	auto clientDataUnlocked = GetClientDataUnlocked(this, targetClient);
	auto slotId = targetClient->GetSlotId();
	auto netId = targetClient->GetNetId();

	
	decltype(m_worldGrids)::iterator gridRef;

	{
		std::shared_lock s(m_worldGridsMutex);
		gridRef = m_worldGrids.find(clientDataUnlocked->routingBucket);
	}

	if (gridRef != m_worldGrids.end())
	{
		auto& grid = gridRef->second;

		if (grid)
		{
			if (slotId != -1)
			{
				WorldGridState* gridState = &grid->state[slotId];

				for (auto& entry : gridState->entries)
				{
					entry.netID = -1;
					entry.sectorX = 0;
					entry.sectorY = 0;
				}
			}

			{
				for (size_t x = 0; x < std::size(grid->accel.netIDs); x++)
				{
					for (size_t y = 0; y < std::size(grid->accel.netIDs[x]); y++)
					{
						if (grid->accel.netIDs[x][y] == netId)
						{
							grid->accel.netIDs[x][y] = 0xFFFF;
						}
					}
				}
			}
		}
	}

	SendWorldGrid(nullptr, targetClient);
}

void ServerGameState::ProcessCloneCreate(const fx::ClientSharedPtr& client, rl::MessageBufferView& inPacket, AckPacketWrapper& ackPacket)
{
	uint16_t objectId = 0;
	uint16_t uniqifier = 0;
	if (ProcessClonePacket(client, inPacket, 1, &objectId, &uniqifier))
	{
		std::unique_lock objectIdsLock(m_objectIdsMutex);
		m_objectIdsUsed.set(objectId);
	}

	ackPacket.Write(3, 1);
	ackPacket.Write(13, objectId);
	ackPacket.Write(16, uniqifier);
	ackPacket.flush();

	GS_LOG("%s: cl %d, id %d\n", __func__, client->GetNetId(), objectId);
}

void ServerGameState::ProcessCloneSync(const fx::ClientSharedPtr& client, rl::MessageBufferView& inPacket, AckPacketWrapper& ackPacket)
{
	uint16_t objectId = 0;
	uint16_t uniqifier = 0;
	ProcessClonePacket(client, inPacket, 2, &objectId, &uniqifier);

	ackPacket.Write(3, 2);
	ackPacket.Write(13, objectId);
	ackPacket.Write(16, uniqifier);
	ackPacket.flush();

	GS_LOG("%s: cl %d, id %d\n", __func__, client->GetNetId(), objectId);
}

void ServerGameState::ProcessCloneTakeover(const fx::ClientSharedPtr& client, rl::MessageBufferView& inPacket)
{
	auto clientId = inPacket.Read<uint16_t>(16);
	auto playerId = 0;
	auto objectId = inPacket.Read<uint16_t>(13);

	auto entity = GetEntity(0, objectId);

	if (entity)
	{
		auto tgtCl = (clientId != 0) ? m_instance->GetComponent<fx::ClientRegistry>()->GetClientByNetID(clientId) : client;

		if (!tgtCl)
		{
			return;
		}

		// don't do duplicate migrations
		{
			auto entityClient = entity->GetClient();

			if (entityClient && entityClient->GetNetId() == tgtCl->GetNetId())
			{
				return;
			}

			if (entityClient && entityClient->GetNetId() != client->GetNetId())
			{
				GS_LOG("%s: trying to send object %d from %s to %s, but the sender is %s. Rejecting.\n", __func__, objectId, (!entityClient) ? "null?" : entityClient->GetName(), tgtCl->GetName(), client->GetName());
				return;
			}

			GS_LOG("%s: migrating object %d from %s to %s\n", __func__, objectId, (!entityClient) ? "null?" : entityClient->GetName(), tgtCl->GetName());
		}

		if (!entity || !entity->syncTree)
		{
			return;
		}

		ReassignEntity(entity->handle, tgtCl);
	}
}

void ServerGameState::ProcessCloneRemove(const fx::ClientSharedPtr& client, rl::MessageBufferView& inPacket, AckPacketWrapper& ackPacket)
{
	auto playerId = 0;
	auto objectId = inPacket.Read<uint16_t>(13);
	auto uniqifier = inPacket.Read<uint16_t>(16);

	auto entity = GetEntity(0, objectId);

	// ack remove no matter if we accept it
	ackPacket.Write(3, 3);
	ackPacket.Write(13, objectId);
	ackPacket.Write(16, uniqifier);
	ackPacket.flush();

	if (entity)
	{
		auto entityClient = entity->GetClient();

		if (!entityClient || client->GetNetId() != entityClient->GetNetId())
		{
			GS_LOG("%s: wrong owner (%d)\n", __func__, objectId);

			return;
		}

		if (entity->uniqifier != uniqifier)
		{
			GS_LOG("%s: wrong uniqifier (%d - %d->%d)\n", __func__, objectId, uniqifier, entity->uniqifier);

			return;
		}

		if (entity->IsOwnedByServerScript() && g_protectServerEntitiesDeletion)
		{
			GS_LOG("%s: entity is owned by server script %d\n", __func__, objectId);

			return;
		}

		GS_LOG("%s: queueing remove (%d - %d)\n", __func__, objectId, uniqifier);
		RemoveClone(client, objectId, uniqifier);
	}
}

void ServerGameState::RemoveClone(const fx::ClientSharedPtr& client, uint16_t objectId, uint16_t uniqifier)
{
	GS_LOG("%s: removing object %d %d\n", __func__, (client) ? client->GetNetId() : 0, objectId);

	sync::SyncEntityPtr entityRef;

	{
		std::shared_lock entitiesByIdLock(m_entitiesByIdMutex);
		entityRef = m_entitiesById[objectId].lock();
		
		// start deleting
		if (entityRef)
		{
			entityRef->deleting = true;
		}
	}
}

void ServerGameState::FinalizeClone(const fx::ClientSharedPtr& client, const fx::sync::SyncEntityPtr& entity, uint16_t objectId, uint16_t uniqifier, std::string_view finalizeReason)
{
	sync::SyncEntityPtr entityRef;

	{
		std::shared_lock entitiesByIdLock(m_entitiesByIdMutex);
		entityRef = m_entitiesById[objectId].lock();
	}

	if (entityRef && entityRef == entity)
	{
		if (!entityRef->finalizing)
		{
			entityRef->finalizing = true;

			GS_LOG("%s: finalizing object %d (for reason %s)\n", __func__, objectId, finalizeReason);

			OnCloneRemove(entityRef, [this, objectId, entityRef]()
			{
				{
					std::unique_lock entitiesByIdLock(m_entitiesByIdMutex);
					m_entitiesById[objectId].reset();
				}

				bool stolen = false;

				{
					std::unique_lock objectIdsLock(m_objectIdsMutex);
					m_objectIdsUsed.reset(objectId);

					if (m_objectIdsStolen.test(objectId))
					{
						stolen = true;

						m_objectIdsSent.reset(objectId);
						m_objectIdsStolen.reset(objectId);
					}
				}

				if (stolen)
				{
					fx::ClientSharedPtr clientRef = entityRef->GetClient();
					if (clientRef)
					{
						auto [lock, clientData] = GetClientData(this, clientRef);
						clientData->objectIds.erase(objectId);
					}
				}

				{
					std::unique_lock elm(m_entityListMutex);
					m_entityList.erase(entityRef);
				}
			});
		}
	}
}

auto ServerGameState::CreateEntityFromTree(sync::NetObjEntityType type, const std::shared_ptr<sync::SyncTreeBase>& tree) -> fx::sync::SyncEntityPtr
{
	int id = fx::IsLengthHack() ? (MaxObjectId - 1) : 8191;

	{
		bool valid = false;
		std::unique_lock objectIdsLock(m_objectIdsMutex);

		for (; id >= 1; id--)
		{
			if (!m_objectIdsSent.test(id) && !m_objectIdsUsed.test(id))
			{
				valid = true;
				break;
			}
		}

		if (!valid)
		{
			return {};
		}

		m_objectIdsSent.set(id);
		m_objectIdsUsed.set(id);
		m_objectIdsStolen.set(id);
	}

	fx::sync::SyncEntityPtr entity = fx::sync::SyncEntityPtr::Construct();
	entity->type = type;
	entity->frameIndex = m_frameIndex;
	entity->lastFrameIndex = 0;
	entity->handle = MakeEntityHandle(id);
	entity->uniqifier = rand();
	entity->creationToken = msec().count();
	entity->createdAt = msec();
	entity->passedFilter = true;

	entity->syncTree = tree;

	entity->lastReceivedAt = msec();

	entity->timestamp = msec().count();

	{
		std::unique_lock<std::shared_mutex> entityListLock(m_entityListMutex);
		m_entityList.insert(entity);
	}

	{
		std::unique_lock entitiesByIdLock(m_entitiesByIdMutex);
		m_entitiesById[id] = entity;
	}

	const auto evComponent = m_instance->GetComponent<fx::ResourceManager>()->GetComponent<fx::ResourceEventManagerComponent>();

	/*NETEV serverEntityCreated SERVER
	/#*
	 * A server-side event that is triggered when an entity has been created by a server-side script.
	 *
	 * Unlike "entityCreated" the newly created entity may not yet have an assigned network owner.
	 *
	 * @param entity - The created entity handle.
	 #/
	declare function serverEntityCreated(handle: number): void;
	*/
	evComponent->QueueEvent2("serverEntityCreated", { }, MakeScriptHandle(entity));

	return entity;
}

bool ServerGameState::ProcessClonePacket(const fx::ClientSharedPtr& client, rl::MessageBufferView& inPacket, int parsingType, uint16_t* outObjectId, uint16_t* outUniqifier)
{
	auto playerId = 0;
	auto uniqifier = inPacket.Read<uint16_t>(16);
	auto objectId = inPacket.Read<uint16_t>(13);
	//auto objectType = (sync::NetObjEntityType)inPacket.Read<uint8_t>();
	//auto timestamp = inPacket.Read<int32_t>();

	if (outUniqifier)
	{
		*outUniqifier = uniqifier;
	}

	if (outObjectId)
	{
		*outObjectId = objectId;
	}

	auto objectType = sync::NetObjEntityType::Train;
	uint32_t creationToken = 0;

	if (parsingType == 1)
	{
		creationToken = inPacket.Read<uint32_t>(32);

		objectType = (sync::NetObjEntityType)inPacket.Read<uint8_t>(kNetObjectTypeBitLength);
	}

	auto length = inPacket.Read<uint16_t>(12);
	GS_LOG("process sync packet for entity %d of length %d\n", objectId, length);

	uint32_t timestamp = 0;

	// move this back down under
	{
		auto [lock, data] = GetClientData(this, client);
		data->playerId = playerId;
		timestamp = data->syncTs;
	}

	std::vector<uint8_t> bitBytes(length);

	if (length > 0)
	{
		inPacket.ReadBits(&bitBytes[0], bitBytes.size() * 8);
	}

	// that's not an object ID, that's a snail!
	if (objectId == 0xFFFF)
	{
		return false;
	}

	auto entity = GetEntity(playerId, objectId);

	bool createdHere = false;

	bool validEntity = false;

	if (entity)
	{
		validEntity = true;
	}

	if (parsingType == 1)
	{
		if (!validEntity)
		{
			entity = fx::sync::SyncEntityPtr::Construct();
			entity->GetFirstOwnerUnsafe() = client;
			entity->GetClientUnsafe() = client;
			entity->type = objectType;
			entity->frameIndex = m_frameIndex;
			entity->lastFrameIndex = 0;
			entity->handle = MakeEntityHandle(objectId);
			entity->uniqifier = uniqifier;
			entity->creationToken = creationToken;
			entity->syncTree = MakeSyncTree(objectType);
			entity->createdAt = msec();

			// no sync tree -> invalid object type -> nah
			if (!entity->syncTree)
			{
				return false;
			}

			auto data = GetClientDataUnlocked(this, client);
			entity->routingBucket = data->routingBucket;

			createdHere = true;
		}
		else // duplicate create? that's not supposed to happen
		{
			auto lcl = entity->GetClient();

			// if (objectType != entity->type)
			{
				GS_LOG("%s: client %d %s tried to create entity %d (type %d), but this is already owned by %d %s (type %d). bad!\n",
					__func__,
					client->GetNetId(),
					client->GetName(),
					objectId,
					(int)objectType,
					(lcl) ? lcl->GetNetId() : -1,
					(lcl) ? lcl->GetName() : "(null)",
					(int)entity->type);
			}

			return false;
		}
	}
	else if (!validEntity)
	{
		GS_LOG("%s: wrong entity (%d)!\n", __func__, objectId);

		return false;
	}

	if (entity->uniqifier != uniqifier)
	{
		GS_LOG("%s: wrong uniqifier (%d) [%d -> %d]!\n", __func__, objectId, entity->uniqifier, uniqifier);

		return false;
	}

	auto slotId = client->GetSlotId();

	if (slotId < 0 || slotId > MAX_CLIENTS)
	{
		return false;
	}

	// so we won't set relevantTo right after and deadlock finalization
	if (entity->deletedFor.test(slotId) || entity->outOfScopeFor.test(slotId))
	{
		return false;
	}

	{
		std::lock_guard<std::shared_mutex> _(entity->guidMutex);
		entity->relevantTo.set(slotId);
		entity->outOfScopeFor.reset(slotId);
	}

	fx::ClientSharedPtr entityClient = entity->GetClient();

	if (!entityClient)
	{
		GS_LOG("%s: no client (%d)!\n", __func__, objectId);

		return false;
	}

	auto now = msec();

	if (entityClient->GetNetId() != client->GetNetId())
	{
		// did the entity recently migrate?
		auto lastOwner = entity->GetLastOwner();

		if (!lastOwner || lastOwner != client || (now - entity->lastMigratedAt) > 1s)
		{
			GS_LOG("%s: wrong owner (%d)!\n", __func__, objectId);

			return false;
		}

		entity->lastOutOfBandTimestamp = timestamp;
	}

	entity->lastReceivedAt = now;

	if (length > 0)
	{
		entity->lastFrameIndex = m_frameIndex;
		entity->timestamp = timestamp;

		GS_LOG("sync for entity %d and length %d\n", entity->handle, length);
		auto state = sync::SyncParseStateDynamic{ { bitBytes }, parsingType, 0, timestamp, entity, m_frameIndex };

		auto syncTree = entity->syncTree;
		if (syncTree)
		{
			if (parsingType == 2)
			{
				syncTree->ParseSync(state);

				entity->hasSynced = true;
			}
			else if (parsingType == 1)
			{
				syncTree->ParseCreate(state);

				syncTree->Visit([](sync::NodeBase& node)
				{
					node.ackedPlayers.reset();

					return true;
				});
			}
		}
	}

	switch (entity->type)
	{
		case sync::NetObjEntityType::Player:
		{
			auto data = GetClientDataUnlocked(this, client);
			
			std::unique_lock _lock(data->playerEntityMutex);
			sync::SyncEntityPtr playerEntity = data->playerEntity.lock();

			if (!playerEntity)
			{
				SendWorldGrid(nullptr, client);
				client->OnCreatePed();
			}

			data->playerEntity = entity;
			client->SetData("playerEntity", MakeScriptHandle(entity));

			break;
		}
	}

	// trigger a clone creation event
	if (createdHere)
	{
		if (entity->type == sync::NetObjEntityType::Player)
		{
			entity->passedFilter = true;
		}

		// if this is not a real creation token, reset it
		if (entity->creationToken != 0)
		{
			if (g_entityCreationList.find(entity->creationToken) == g_entityCreationList.end())
			{
				entity->creationToken = 0;
			}
		}

		auto startDelete = [this, entity, client, objectId, uniqifier]()
		{
			RemoveClone({}, entity->handle);

			// if the entity isn't added yet, the owner should be told of its deletion too
			{
				auto [lock, data] = GetClientData(this, client);
				data->entitiesToDestroy[MakeHandleUniqifierPair(objectId, uniqifier)] = { entity, { false, false } };
			}
		};


		// if we're in entity lockdown, validate the entity first
		if (entity->type != sync::NetObjEntityType::Player)
		{
			auto clientData = GetClientDataUnlocked(this, client);
			std::shared_lock _(m_routingDataMutex);

			auto entityLockdownMode = m_entityLockdownMode;
			if (auto rbmdIt = m_routingData.find(clientData->routingBucket); rbmdIt != m_routingData.end())
			{
				auto& rbmd = rbmdIt->second;

				if (rbmd.lockdownMode)
				{
					entityLockdownMode = *rbmd.lockdownMode;
				}
			}

			if (entityLockdownMode != EntityLockdownMode::Inactive && !ValidateEntity(entityLockdownMode, entity))
			{
				// yeet
				startDelete();

				return false;
			}
		}

		if (!OnEntityCreate(entity))
		{
			startDelete();

			return false;
		}

		// well, yeah, it's their entity, they have it
		{
			auto [lock, data] = GetClientData(this, client);
			auto identPair = MakeHandleUniqifierPair(objectId, uniqifier);

			data->syncedEntities[identPair] = { 0ms, 10ms, entity, false, true, false };
			data->pendingCreates.erase(identPair);
		}

		{
			std::unique_lock _(m_entitiesByIdMutex);
			m_entitiesById[objectId] = { entity };
		}

		{
			std::unique_lock _(m_entityListMutex);
			m_entityList.insert(entity);
		}

		// update all clients' lists so the system knows that this entity is valid and should not be deleted anymore
		// (otherwise embarrassing things happen like a new player's ped having the same object ID as a pending-removed entity, and the game trying to remove it)
		gscomms_execute_callback_on_main_thread([this, entity]()
		{
			auto evComponent = m_instance->GetComponent<fx::ResourceManager>()->GetComponent<fx::ResourceEventManagerComponent>();

			/*NETEV entityCreating SERVER
			/#*
			 * A server-side event that is triggered when an entity is being created.
			 * 
			 * This event **can** be canceled to instantly delete the entity.
			 *
			 * @param entity - The created entity handle.
			 #/
			declare function entityCreating(handle: number): void;
			*/
			if (!evComponent->TriggerEvent2("entityCreating", { }, MakeScriptHandle(entity)))
			{
				if (entity->type != sync::NetObjEntityType::Player)
				{
					gscomms_execute_callback_on_sync_thread([this, entity]()
					{
						RemoveClone({}, entity->handle);
					});
				}

				return;
			}

			entity->passedFilter = true;

			/*NETEV entityCreated SERVER
			/#*
			 * A server-side event that is triggered when an entity has been created.
			 *
			 * @param entity - The created entity handle.
			 #/
			declare function entityCreated(handle: number): void;
			*/
			evComponent->QueueEvent2("entityCreated", { }, MakeScriptHandle(entity));
		});
	}
	else
	{
		// erase from pending create list if we're syncing
		auto [lock, data] = GetClientData(this, client);
		data->pendingCreates.erase(MakeHandleUniqifierPair(objectId, uniqifier));
	}

	if (auto stateBag = entity->GetStateBag())
	{
		stateBag->AddRoutingTarget(slotId);
	}

	return true;
}

bool ServerGameState::ValidateEntity(EntityLockdownMode entityLockdownMode, const fx::sync::SyncEntityPtr& entity)
{
	// can't validate an entity without sync tree
	if (!entity->syncTree)
	{
		return false;
	}

	bool allowed = false;
	// allow auto-generated population in non-strict lockdown
	if (entityLockdownMode != EntityLockdownMode::Strict)
	{
		sync::ePopType popType;

		if (entity->syncTree->GetPopulationType(&popType))
		{
			if (popType == sync::POPTYPE_RANDOM_AMBIENT || popType == sync::POPTYPE_RANDOM_PARKED || popType == sync::POPTYPE_RANDOM_PATROL ||
				popType == sync::POPTYPE_RANDOM_PERMANENT || popType == sync::POPTYPE_RANDOM_SCENARIO)
			{
				allowed = true;
			}
		}
	}

	// Networked CDummyObjects are used to ensure map objects, e.g., gas pumps
	// and propane tanks, stay in an destroyed/exploded state. When lockdown
	// mode is enabled, infinite explosions may occur if the exploded objects
	// are not validated.
	//
	// This logic will require refactoring for CDoor instances
#if !defined(STATE_RDR3)
	if (!allowed)
	{
		sync::CDummyObjectCreationNodeData* dummy = entity->syncTree->GetDummyObjectState();
		if (dummy)
		{
			allowed = dummy->_hasRelatedDummy && ((dummy->hasFragGroup && dummy->hasExploded) || (dummy->_explodingEntityExploded));
			if (entityLockdownMode == EntityLockdownMode::Dummy)
			{
				return allowed;
			}
		}
		// "Dummy" mode allows everything except dummy objects.
		else if (entityLockdownMode == EntityLockdownMode::Dummy)
		{
			allowed = true;
		}
	}
#endif

	// Allow gamestate and task generated entities when in entity lockdown.
	//
	// @NOTE Missing CVehicleGadgetPickUpRopeWithMagnet.
#if !defined(STATE_RDR3)
	if (!allowed)
	{
		// CTaskParachuteObject changed from 335 to 336 in 1868. While CTaskScriptedAnimation
		// has remained 134. Reference 0x1410FC34C in CNetObject::SetObjectGameStateData.
		sync::CObjectGameStateNodeData* state = entity->syncTree->GetObjectGameState();
		sync::CBaseAttachNodeData* attachment = entity->syncTree->GetAttachment();
		if (attachment != nullptr && state != nullptr && state->hasTask)
		{
			auto base = g_serverGameState->GetEntity(0, attachment->attachedTo);
			allowed |= (base && ((attachment->attachmentFlags & 130) == 130)); // Parachute attachment flags.
		}
	}
#endif

	// check the entity creation token, only if the check above didn't pass
	if (!allowed)
	{
		auto it = g_entityCreationList.find(entity->creationToken);

		if (it != g_entityCreationList.end())
		{
			if (it->second.scriptGuid)
			{
				allowed = true;
			}
		}
	}

	return allowed;
}


static std::tuple<uint32_t, int> UncompressClonePacket(const net::Span<char>& bufferData, const net::packet::ClientRoute& packetData)
{
	net::ByteReader readBuffer(packetData.data.GetValue().data(), packetData.data.GetValue().size());
	uint32_t type;
	if (!readBuffer.Field(type))
	{
		return { type, 0 };
	}

	if (type != net::force_consteval<uint32_t, HashString("netClones")> && type != net::force_consteval<uint32_t, HashString("netAcks")>)
	{
		return { type, 0 };
	}

	const static uint8_t dictBuffer[65536] = 
	{
#include <state/dict_five_20210329.h>
	};
	int bufferLength = LZ4_decompress_safe_usingDict(reinterpret_cast<const char*>(readBuffer.GetData() + readBuffer.GetOffset()), bufferData.data(), readBuffer.GetRemaining(), bufferData.size(), reinterpret_cast<const char*>(dictBuffer), std::size(dictBuffer));

	return { type, bufferLength };
}

void ServerGameState::ParseGameStatePacket(const fx::ClientSharedPtr& client, const net::packet::ClientRoute& packetData)
{
	if (!IsOneSync())
	{
		return;
	}

	char bufferData[16384];
	net::Span bufferDataSpan (bufferData, sizeof(bufferData));
	auto [type, length] = UncompressClonePacket(bufferDataSpan, packetData);

	if (length <= 0)
	{
		return;
	}

	net::ByteReader reader (reinterpret_cast<uint8_t*>(bufferData), length);

	switch (type)
	{
		case HashString("netClones"):
			ParseClonePacket(client, reader);
		break;
		// #IFARQ
		case HashString("netAcks"):
			ParseAckPacket(client, reader);
		break;
	}
}

void ServerGameState::ParseAckPacket(const fx::ClientSharedPtr& client, net::ByteReader& buffer)
{
	net::Span span (const_cast<uint8_t*>(buffer.GetData() + buffer.GetOffset()), buffer.GetRemaining());
	rl::MessageBufferView msgBuf(span);

	bool end = false;

	while (!msgBuf.IsAtEnd() && !end)
	{
		auto dataType = msgBuf.Read<uint8_t>(3);

		switch (dataType)
		{
			case 1: // clone create
			{
				auto objectId = msgBuf.Read<uint16_t>(13);
				auto uniqifier = msgBuf.Read<uint16_t>(16);
				auto entity = GetEntity(0, objectId);

				if (entity && entity->uniqifier == uniqifier)
				{
					auto syncTree = entity->syncTree;

					if (syncTree)
					{
						entity->deletedFor.reset(client->GetSlotId());
						
						auto [lock, clientData] = GetClientData(this, client);
						if (auto secIt = clientData->syncedEntities.find(MakeHandleUniqifierPair(objectId, uniqifier)); secIt != clientData->syncedEntities.end())
						{
							secIt->second.hasCreated = true;
						}
					}
				}

				break;
			}
			case 3: // clone remove
			{
				auto objectId = msgBuf.Read<uint16_t>(13);
				auto uniqifier = msgBuf.Read<uint16_t>(16);

				auto [lock, clientData] = GetClientData(this, client);
				clientData->entitiesToDestroy.erase(MakeHandleUniqifierPair(objectId, uniqifier));

				GS_LOG("handle remove ack for [obj:%d:%d]\n", objectId, uniqifier);

				break;
			}
			case 7: // end
				end = true;
				break;
			default:
				end = true;
				break;
		}
	}
}


void ServerGameState::ParseClonePacket(const fx::ClientSharedPtr& client, net::ByteReader& buffer)
{
	net::Span span (const_cast<uint8_t*>(buffer.GetData() + buffer.GetOffset()), buffer.GetRemaining());
	rl::MessageBufferView msgBuf(span);

	rl::MessageBuffer ackPacket;

	{
		auto[lock, clientData] = GetClientData(this, client);

		ackPacket = std::move(clientData->ackBuffer);
	}

	auto prepare = [this, &client, &ackPacket]
	{
		uint64_t fidx = 0;

		{
			auto [lock, data] = GetClientData(this, client);
			fidx = data->fidx;
		}

		// #IFARQ: frame index didn't use to be 0
		if (fidx)
		{
			// we don't send any data here
			ackPacket.SetCurrentBit(0);
		}

		return fidx;
	};

	AckPacketWrapper ackPacketWrapper{ ackPacket };
	ackPacketWrapper.flush = [&ackPacket, &client, &prepare]()
	{
		auto fidx = prepare();
		MaybeFlushBuffer(ackPacket, HashRageString("msgPackedAcks"), fidx, client);
	};

	uint32_t numCreates = 0, numSyncs = 0, numRemoves = 0;

	bool end = false;
	
	while (!msgBuf.IsAtEnd() && !end)
	{
		auto dataType = msgBuf.Read<uint8_t>(3);

		switch (dataType)
		{
		case 1: // clone create
			ProcessCloneCreate(client, msgBuf, ackPacketWrapper);
			++numCreates;
			break;
		case 2: // clone sync
			ProcessCloneSync(client, msgBuf, ackPacketWrapper);
			++numSyncs;
			break;
		case 3: // clone remove
			ProcessCloneRemove(client, msgBuf, ackPacketWrapper);
			++numRemoves;
			break;
		case 4: // clone takeover
			ProcessCloneTakeover(client, msgBuf);
			break;
		case 5: // set timestamp
		{
			auto newTs = msgBuf.Read<uint32_t>(32);

			// this is the timestamp that the client will use for following acks
			ackPacket.Write(3, 5);
			ackPacket.Write(32, newTs);
			ackPacketWrapper.flush();

			auto [lock, data] = GetClientData(this, client);
			auto oldTs = data->ackTs;

			if (!oldTs || oldTs < newTs)
			{
				data->ackTs = newTs;
				data->syncTs = newTs;
			}

			break;
		}
		case 6: // set index
		{
			auto newIndex = msgBuf.Read<uint32_t>(32);
			auto [lock, data] = GetClientData(this, client);
			data->fidx = newIndex;

			break;
		}
		case 7: // end
			end = true;
			break;
		default:
			end = true;
			break;
		}
	}

	auto fidx = prepare();
	FlushBuffer(ackPacket, HashRageString("msgPackedAcks"), fidx, client, nullptr, true);

	{
		auto [lock, clientData] = GetClientData(this, client);

		clientData->ackBuffer = std::move(ackPacket);
	}
}

void ServerGameState::GetFreeObjectIds(const fx::ClientSharedPtr& client, uint8_t numIds, std::vector<uint16_t>& freeIds)
{
	auto [lock, data] = GetClientData(this, client);
	std::unique_lock objectIdsLock(m_objectIdsMutex);

	uint16_t id = 1;

	for (uint8_t i = 0; i < numIds; i++)
	{
		bool hadId = false;

		for (; id < m_objectIdsSent.size(); id++)
		{
			if (!m_objectIdsSent.test(id) && !m_objectIdsUsed.test(id))
			{
				hadId = true;

				data->objectIds.insert(id);
				freeIds.push_back(id);
				m_objectIdsSent.set(id);

				break;
			}
		}

		if (!hadId)
		{
			trace("couldn't assign an object id for player!\n");
			break;
		}
	}
}

template<int MaxElemSize, int Count>
struct ArrayHandler : public ServerGameState::ArrayHandlerBase
{
public:
	ArrayHandler(int index)
		: m_index(index)
	{
	}

	virtual uint32_t GetElementSize() override
	{
		return MaxElemSize;
	}

	virtual uint32_t GetCount() override
	{
		return Count;
	}

	virtual bool ReadUpdate(const fx::ClientSharedPtr& client, net::packet::ClientArrayUpdate& buffer) override
	{
		std::unique_lock lock(m_mutex);

		if (buffer.index >= Count)
		{
			return false;
		}

		if (buffer.data.GetValue().size() > MaxElemSize)
		{
			return false;
		}

		{
			auto curClient = m_owners[buffer.index].lock();

			if (curClient && !(curClient == client))
			{
				return false;
			}
		}

		if (!buffer.data.GetValue().empty())
		{
			m_owners[buffer.index] = client;
			memcpy(&m_array[buffer.index * MaxElemSize], buffer.data.GetValue().data(), buffer.data.GetValue().size());
		}
		else
		{
			m_owners[buffer.index] = {};
		}

		m_sizes[buffer.index] = buffer.data.GetValue().size();

		m_dirtyFlags[buffer.index].set();
		m_dirtyFlags[buffer.index].reset(client->GetSlotId());

		return true;
	}

	virtual void PlayerHasLeft(const fx::ClientSharedPtr& client) override
	{
		std::unique_lock lock(m_mutex);

		auto slotId = client->GetSlotId();

		if (slotId != -1)
		{
			for (auto& flagSet : m_dirtyFlags)
			{
				flagSet.reset(slotId);
			}
		}

		int i = 0;

		for (auto& owner : m_owners)
		{
			if (owner == client)
			{
				owner = {};
				m_sizes[i] = 0;
				m_dirtyFlags[i].set();
			}

			i++;
		}
	}

	virtual void WriteUpdates(const fx::ClientSharedPtr& client) override
	{
		std::shared_lock lock(m_mutex);

		for (int i = 0; i < Count; i++)
		{
			if (m_dirtyFlags[i].test(client->GetSlotId()))
			{
				auto owner = m_owners[i].lock();

				if (owner)
				{
					net::Buffer msg;
					msg.Write<uint32_t>(HashRageString("msgArrayUpdate"));
					msg.Write<uint8_t>(m_index);
					msg.Write<uint16_t>(owner->GetNetId());
					msg.Write<uint32_t>(i);
					msg.Write<uint32_t>(m_sizes[i]);

					if (m_sizes[i])
					{
						msg.Write(&m_array[i * MaxElemSize], m_sizes[i]);
					}

					client->SendPacket(0, msg, NetPacketType_Reliable);
				}

				m_dirtyFlags[i].reset(client->GetSlotId());
			}
		}
	}

private:
	std::array<uint32_t, Count> m_sizes;
	std::array<fx::ClientWeakPtr, Count> m_owners;
	std::array<uint8_t, MaxElemSize * Count> m_array;
	std::array<eastl::bitset<MAX_CLIENTS + 1>, Count> m_dirtyFlags;

	int m_index;
	std::shared_mutex m_mutex;
};

void ServerGameState::SendArrayData(const fx::ClientSharedPtr& client)
{
	auto data = GetClientDataUnlocked(this, client);

	decltype(m_arrayHandlers)::iterator arrayRef;

	{
		std::shared_lock s(m_arrayHandlersMutex);
		arrayRef = m_arrayHandlers.find(data->routingBucket);
	}

	if (arrayRef != m_arrayHandlers.end())
	{
		for (auto& handler : arrayRef->second->handlers)
		{
			if (handler)
			{
				handler->WriteUpdates(client);
			}
		}
	}
}

void ServerGameState::HandleArrayUpdate(const fx::ClientSharedPtr& client, net::packet::ClientArrayUpdate& buffer)
{
	auto data = GetClientDataUnlocked(this, client);

	decltype(m_arrayHandlers)::iterator gridRef;

	{
		std::shared_lock s(m_arrayHandlersMutex);
		gridRef = m_arrayHandlers.find(data->routingBucket);

		if (gridRef == m_arrayHandlers.end())
		{
			s.unlock();
			std::unique_lock l(m_arrayHandlersMutex);
			gridRef = m_arrayHandlers.emplace(data->routingBucket, std::make_unique<ArrayHandlerData>()).first;
		}
	}

	auto& ah = gridRef->second;

	if (buffer.handler >= ah->handlers.size())
	{
		return;
	}

	auto handler = ah->handlers[buffer.handler];

	if (!handler)
	{
		return;
	}

	handler->ReadUpdate(client, buffer);
}

void ServerGameState::HandleGameStateNAck(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::packet::ClientGameStateNAck& packet)
{
	if (GetSyncStyle() != fx::SyncStyle::NAK)
	{
		return;
	}

	auto slotId = client->GetSlotId();
	if (slotId == -1)
	{
		return;
	}

	const uint8_t flags = packet.GetFlags();
	const auto thisFrame = packet.GetFrameIndex();
				
	if (flags & 1)
	{
		const auto firstMissingFrame = packet.GetFirstMissingFrame();
		const auto lastMissingFrame = packet.GetLastMissingFrame();

		auto [lock, clientData] = GetClientData(this, client);
		auto& states = clientData->frameStates;

		eastl::fixed_map<uint16_t, uint64_t, 64> lastSentCorrections;

		for (uint64_t frame = lastMissingFrame; frame >= firstMissingFrame; --frame)
		{
			if (auto frameIt = states.find(frame); frameIt != states.end())
			{
				auto& [synced, deletions] = frameIt->second;
				for (auto& [objectId, entData] : synced)
				{
					if (auto ent = entData.GetEntity(this))
					{
						const auto entIdentifier = MakeHandleUniqifierPair(objectId, ent->uniqifier);

						if (!entData.isCreated)
						{
							if (auto syncedIt = clientData->syncedEntities.find(entIdentifier); syncedIt != clientData->syncedEntities.end())
							{
								syncedIt->second.hasCreated = false;
								syncedIt->second.hasNAckedCreate = true;
							}
						}
						else
						{
							std::lock_guard _(ent->frameMutex);
							ent->lastFramesSent[slotId] = std::min(entData.lastSent, ent->lastFramesSent[slotId]);
							lastSentCorrections[objectId] = ent->lastFramesSent[slotId];
						}
					}
				}

				for (auto [identPair, deletionData] : deletions)
				{
					clientData->entitiesToDestroy[identPair] = { fx::sync::SyncEntityPtr{}, deletionData };
				}
			}
			else
			{
				instance->GetComponent<fx::GameServer>()->DropClientWithReason(client, fx::serverOneSyncDropResourceName, fx::ClientDropReason::ONE_SYNC_TOO_MANY_MISSED_FRAMES, "Timed out after 60 seconds (1, %d)", lastMissingFrame - firstMissingFrame);
				return;
			}
		}

		// propagate these frames into newer states, as well
		for (auto frameIt = states.upper_bound(lastMissingFrame); frameIt != states.end(); frameIt++)
		{
			auto& [synced, deletions] = frameIt->second;

			for (const auto& [objectId, correction] : lastSentCorrections)
			{
				if (auto entIt = synced.find(objectId); entIt != synced.end())
				{
					entIt->second.lastSent = correction;
				}
			}
		}
	}

	auto [lock, clientData] = GetClientData(this, client);
	auto& states = clientData->frameStates;

	if (auto frameIt = states.find(thisFrame); frameIt != states.end())
	{
		// ignore list
		if (flags & 2)
		{
			for (auto& ignoreListEntry : packet.GetIgnoreList())
			{
				auto& [synced, deletions] = frameIt->second;
				if (auto entIter = synced.find(ignoreListEntry.entry); entIter != synced.end())
				{
					if (auto ent = entIter->second.GetEntity(this))
					{
						std::lock_guard _(ent->frameMutex);
						ent->lastFramesSent[slotId] = std::min(ignoreListEntry.lastFrame, ent->lastFramesSent[slotId]);
					}
				}
			}
		}

		// recreate list
		if (flags & 4)
		{
			for (uint16_t objectId : packet.GetRecreateList())
			{
				GS_LOG("attempt recreate of id %d for client %d\n", objectId, client->GetNetId());
				auto& [synced, deletions] = frameIt->second;
				if (auto entIter = synced.find(objectId); entIter != synced.end())
				{
					if (auto ent = entIter->second.GetEntity(this))
					{
						const auto entIdentifier = MakeHandleUniqifierPair(objectId, ent->uniqifier);
						if (auto syncedIt = clientData->syncedEntities.find(entIdentifier); syncedIt != clientData->syncedEntities.end())
						{
							GS_LOG("recreating id %d for client %d\n", objectId, client->GetNetId());
							syncedIt->second.hasCreated = false;
							syncedIt->second.hasNAckedCreate = true;
						}
					}
				}
			}
		}
	}
	else
	{
		instance->GetComponent<fx::GameServer>()->DropClientWithReason(client, fx::serverOneSyncDropResourceName, fx::ClientDropReason::ONE_SYNC_TOO_MANY_MISSED_FRAMES, "Timed out after 60 seconds (2)");
	}
}

void ServerGameState::HandleGameStateAck(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::packet::ClientGameStateAck& packet)
{
	auto sgs = instance->GetComponent<fx::ServerGameState>();

	if (sgs->GetSyncStyle() != fx::SyncStyle::ARQ)
	{
		return;
	}

	auto slotId = client->GetSlotId();

	if (slotId == -1)
	{
		return;
	}

	uint64_t frameIndex = packet.GetFrameIndex();

	eastl::fixed_map<uint16_t, uint64_t, 32> ignoreHandles;
	for (auto& ignoreListEntry : packet.GetIgnoreList())
	{
		ignoreHandles.emplace(ignoreListEntry.entry, ignoreListEntry.lastFrame);
	}

	auto [lock, clientData] = GetClientData(sgs.GetRef(), client);
			
	const auto& ref = clientData->frameStates[frameIndex];
	const auto& [synced, deletions] = ref;

	{
		for (const uint16_t objectId : packet.GetRecreateList())
		{
			if (auto entIter = synced.find(objectId); entIter != synced.end())
			{
				if (auto ent = entIter->second.GetEntity(sgs.GetRef()))
				{
					if (auto secIt = clientData->syncedEntities.find(MakeHandleUniqifierPair(objectId, ent->uniqifier)); secIt != clientData->syncedEntities.end())
					{
						secIt->second.hasCreated = false;
					}
				}
			}
		}

		for (auto& ignoreListEntry : packet.GetIgnoreList())
		{
			if (auto entIter = synced.find(ignoreListEntry.entry); entIter != synced.end())
			{
				if (auto ent = entIter->second.GetEntity(sgs.GetRef()))
				{
					std::lock_guard _(ent->frameMutex);
					ent->lastFramesSent[slotId] = std::min(ent->lastFramesSent[slotId], ignoreListEntry.lastFrame);
					ent->lastFramesPreSent[slotId] = std::min(ent->lastFramesPreSent[slotId], ignoreListEntry.lastFrame);
				}
			}
		}
	}

	{
		for (auto& [id, entityData] : synced)
		{
			fx::sync::SyncEntityPtr entityRef = entityData.GetEntity(sgs.GetRef());

			if (entityRef)
			{
				if (!entityRef->syncTree)
				{
					continue;
				}

				bool hasCreated = false;

				if (auto secIt = clientData->syncedEntities.find(MakeHandleUniqifierPair(id, entityRef->uniqifier)); secIt != clientData->syncedEntities.end())
				{
					hasCreated = secIt->second.hasCreated;
				}

				bool hasDeleted = entityRef->deletedFor.test(slotId);

				if (!hasCreated || hasDeleted)
				{
					continue;
				}

				if (ignoreHandles.find(entityRef->handle) != ignoreHandles.end())
				{
					continue;
				}

				std::lock_guard _(entityRef->frameMutex);
				entityRef->lastFramesSent[slotId] = std::min(frameIndex, entityRef->lastFramesPreSent[slotId]);
			}
		}

		clientData->frameStates.erase(frameIndex);
	}
}


void ServerGameState::SendPacket(int peer, net::packet::StateBagPacket& packet)
{
	auto creg = m_instance->GetComponent<fx::ClientRegistry>();

	if (peer < 0)
	{
		return;
	}

	auto client = creg->GetClientBySlotID(peer);

	if (client)
	{
		net::Buffer buffer (packet.data.data.GetValue().size() + 4);
		net::ByteWriter writer(buffer.GetBuffer(), buffer.GetLength());
		if (!packet.Process(writer))
		{
			trace("Serialization of the server state bag packet failed. Please report this error at https://github.com/citizenfx/fivem.\n");
			return;
		}

		buffer.Seek(writer.GetOffset());
		client->SendPacket(1, buffer, NetPacketType_Reliable);
	}
}

void ServerGameState::SendPacket(int peer, net::packet::StateBagV2Packet& packet)
{
	auto creg = m_instance->GetComponent<fx::ClientRegistry>();

	if (peer < 0)
	{
		return;
	}

	auto client = creg->GetClientBySlotID(peer);

	if (client)
	{
		net::Buffer buffer (packet.data.stateBagName.GetValue().size() + 2 + packet.data.key.GetValue().size() + 2 + packet.data.data.GetValue().size() + 4);
		net::ByteWriter writer(buffer.GetBuffer(), buffer.GetLength());
		if (!packet.Process(writer))
		{
			trace("Serialization of the server state bag v2 packet failed. Please report this error at https://github.com/citizenfx/fivem.\n");
			return;
		}

		buffer.Seek(writer.GetOffset());
		client->SendPacket(1, buffer, NetPacketType_Reliable);
	}
}

bool ServerGameState::IsAsynchronous()
{
	return true;
}

void ServerGameState::QueueTask(std::function<void()>&& task)
{
	gscomms_execute_callback_on_main_thread(task);
}

ServerGameState::ArrayHandlerData::ArrayHandlerData()
{
#ifdef STATE_FIVE
	handlers[4] = std::make_shared<ArrayHandler<128, 50>>(4);
	handlers[7] = std::make_shared<ArrayHandler<128, 50>>(7);
#elif STATE_RDR3
	handlers[16] = std::make_shared<ArrayHandler<128, 400>>(16);
#endif
}

auto ServerGameState::GetEntityLockdownMode(const fx::ClientSharedPtr& client) -> EntityLockdownMode
{
	auto clientData = GetClientDataUnlocked(this, client);
	std::shared_lock _(m_routingDataMutex);

	if (auto rbmdIt = m_routingData.find(clientData->routingBucket); rbmdIt != m_routingData.end())
	{
		auto& rbmd = rbmdIt->second;

		// since this API is for external use
		if (rbmd.noPopulation)
		{
			return EntityLockdownMode::Strict;
		}

		if (rbmd.lockdownMode)
		{
			return *rbmd.lockdownMode;
		}
	}

	return GetEntityLockdownMode();
}

void ServerGameState::AttachToObject(fx::ServerInstanceBase* instance)
{
	m_instance = instance;

	m_lockdownModeVar = instance->AddVariable<fx::EntityLockdownMode>("sv_entityLockdown", ConVar_None, m_entityLockdownMode, &m_entityLockdownMode);
	m_stateBagStrictModeVar = instance->AddVariable<bool>("sv_stateBagStrictMode", ConVar_None, m_stateBagStrictMode, &m_stateBagStrictMode);

	auto sbac = fx::StateBagComponent::Create(fx::StateBagRole::Server);
	sbac->SetGameInterface(this);


	instance->GetComponent<fx::ResourceManager>()->SetComponent(sbac);

	auto creg = instance->GetComponent<fx::ClientRegistry>();
	m_globalBag = sbac->RegisterStateBag("global", true);
	m_globalBag->SetOwningPeer(-1);
	m_sbac = sbac;

	creg->OnConnectedClient.Connect([this](fx::Client* client)
	{
		if (!fx::IsOneSync())
		{
			return;
		}

		assert(client->GetSlotId() != -1);

		m_sbac->RegisterTarget(client->GetSlotId());

		client->OnDrop.Connect([this, client]()
		{
			m_sbac->UnregisterTarget(client->GetSlotId());
		}, INT32_MIN);
	});

	static auto clearAreaCommand = instance->AddCommand("onesync_clearArea", [this](float x1, float y1, float x2, float y2)
	{
		gscomms_execute_callback_on_sync_thread([=]() 
		{
			std::shared_lock lock(m_entityListMutex);
			for (auto& entity : m_entityList)
			{
				if (entity->type != sync::NetObjEntityType::Player && entity->syncTree)
				{
					float pos[3];
					pos[0] = 9999.0f;
					pos[1] = 9999.0f;
					pos[2] = 9999.0f;

					entity->syncTree->GetPosition(pos);

					if (pos[0] >= x1 && pos[1] >= y1 && pos[0] < x2 && pos[1] < y2)
					{
						RemoveClone({}, entity->handle);
					}
				}
			}
		});
	});

	static auto showObjectIdsCommand = instance->AddCommand("onesync_showObjectIds", [this]()
	{
		console::Printf("net", "^2GLOBAL: %d/%d object IDs used/sent (%.2f percent)^7\n", m_objectIdsUsed.count(), m_objectIdsSent.count(), (m_objectIdsUsed.count() / (float)m_objectIdsSent.count()) * 100.0f);
		
		std::shared_lock entitesByIdLock(m_entitiesByIdMutex);
		m_instance->GetComponent<fx::ClientRegistry>()->ForAllClients([this](const fx::ClientSharedPtr& client)
		{
			auto [lock, data] = GetClientData(this, client);
			int used = 0;

			{
				for (auto object : data->objectIds)
				{
					if (m_entitiesById[object].lock())
					{
						used++;
					}
				}
			}

			console::Printf("net", "%s^7: %d/%d object IDs used/sent (%.2f percent)\n", client->GetName(), used, data->objectIds.size(), (used / (float)data->objectIds.size()) * 100.0f);
		});

		console::Printf("net", "---------------- END OBJECT ID DUMP ----------------\n");
	});

	static auto blockNetGameEvent = instance->AddCommand("block_net_game_event", [this](std::string& eventName)
	{
		if (eventName.empty())
		{
			trace("^3You must specify an event name to block.^7\n");
			return;
		}
		if (!g_experimentalNetGameEventHandler->GetValue())
		{
			trace("^3You must enable sv_experimentalNetGameEventHandler convar before using this command.^7\n");
			return;
		}

		std::transform(eventName.begin(), eventName.end(), eventName.begin(),
		[](unsigned char c)
		{
			return std::toupper(c);
		});

		std::unique_lock lock(this->blockedEventsMutex);
		this->blockedEvents.insert(HashRageString(eventName));
	});

	static auto unblockNetGameEvent = instance->AddCommand("unblock_net_game_event", [this](std::string& eventName)
	{
		if (eventName.empty())
		{
			trace("^3You must specify an event name to unblock.^7\n");
			return;
		}
		if (!g_experimentalNetGameEventHandler->GetValue())
		{
			trace("^3You must enable sv_experimentalNetGameEventHandler convar before using this command.^7\n");
			return;
		}

		std::transform(eventName.begin(), eventName.end(), eventName.begin(),
		[](unsigned char c)
		{
			return std::toupper(c);
		});

		std::unique_lock lock(this->blockedEventsMutex);
		this->blockedEvents.erase(HashRageString(eventName));
	});
}

bool ServerGameState::IsNetGameEventBlocked(uint32_t eventNameHash)
{
	std::shared_lock lock(this->blockedEventsMutex);
	return blockedEvents.find(eventNameHash) != blockedEvents.end();
}
}

#include <ResourceManager.h>
#include <ServerInstanceBase.h>
#include <ServerInstanceBaseRef.h>
#include <ScriptEngine.h>

#ifdef STATE_FIVE
struct CFireEvent
{
	void Parse(rl::MessageBufferView& buffer);

	inline std::string GetName()
	{
		return "fireEvent";
	}

	struct fire
	{
		int v1;
		bool isEntity;
		bool v2;
		uint16_t entityGlobalId;
		uint16_t v3;
		bool v4;
		float v5X;
		float v5Y;
		float v5Z;
		float posX;
		float posY;
		float posZ;
		uint16_t v7;
		bool v8;
		uint8_t maxChildren;
		float v10;
		float v11;
		bool v12;
		int weaponHash;
		int v13;
		uint16_t fireId;

		MSGPACK_DEFINE_MAP(v1,isEntity,v2,entityGlobalId,v3,v4,v5X,v5Y,v5Z,posX,posY,posZ,v7,v8,maxChildren,v10,v11,v12,weaponHash, v13,fireId);
	};

	std::vector<fire> fires;

	MSGPACK_DEFINE(fires);
};

void CFireEvent::Parse(rl::MessageBufferView& buffer)
{
	int count = buffer.Read<int>(3);
	if (count > 5)
		count = 5;

	for (int i = 0; i < count; i++)
	{
		fire f;
		f.v1 = buffer.Read<int>(4);
		f.isEntity = buffer.Read<uint8_t>(1);
		f.v2 = f.isEntity;
		if (f.isEntity)
		{
			f.entityGlobalId = buffer.Read<uint16_t>(13);
			f.v3 = f.entityGlobalId;
		}
		else
		{
			f.entityGlobalId = 0;
			f.v3 = 0;
		}
		if (buffer.Read<uint8_t>(1))
		{
			f.v5X = buffer.ReadSignedFloat(19, 27648.0f);
			f.v5Y = buffer.ReadSignedFloat(19, 27648.0f);
			f.v5Z = buffer.ReadFloat(19, 4416.0f) - 1700.0f;
		}
		else
		{
			f.v5X = 0.0f;
			f.v5Y = 0.0f;
			f.v5Z = 0.0f;
		}
		f.posX = buffer.ReadSignedFloat(19, 27648.0f);
		f.posY = buffer.ReadSignedFloat(19, 27648.0f);
		f.posZ = buffer.ReadFloat(19, 4416.0f) - 1700.0f;
		f.v7 = buffer.Read<uint16_t>(13);
		f.v8 = buffer.Read<uint8_t>(1);
		f.maxChildren = buffer.Read<uint8_t>(5);
		f.v10 = (buffer.Read<int>(16) / 65535.0f) * 90.0f;
		f.v11 = (buffer.Read<int>(16) / 65535.0f) * 25.0f;
		if (buffer.Read<uint8_t>(1)) {
			f.weaponHash = buffer.Read<int>(32);
			f.v13 = f.weaponHash;
		}
		else
		{
			f.weaponHash = 0;
			f.v13 = 0;
		}
		f.fireId = buffer.Read<uint16_t>(16);
		fires.push_back(f);
	}
}

struct CExplosionEvent
{
	void Parse(rl::MessageBufferView& buffer);

	inline std::string GetName()
	{
		return "explosionEvent";
	}

	uint16_t f186;
	uint16_t f208;
	int ownerNetId;
	uint16_t f214;
	int explosionType;
	float damageScale;

	float posX;
	float posY;
	float posZ;

	bool f242;
	uint16_t f104;
	float cameraShake;

	bool isAudible;
	bool f189;
	bool isInvisible;
	bool f126;
	bool f241;
	bool f243;

	uint16_t f210;

	float unkX;
	float unkY;
	float unkZ;

	bool f190;
	bool f191;

	uint32_t f164;
	
	float posX224;
	float posY224;
	float posZ224;

	bool f240;
	uint16_t f218;
	bool f216;

	MSGPACK_DEFINE_MAP(f186,f208,ownerNetId,f214,explosionType,damageScale,posX,posY,posZ,f242,f104,cameraShake,isAudible,f189,isInvisible,f126,f241,f243,f210,unkX,unkY,unkZ,f190,f191,f164,posX224,posY224,posZ224,f240,f218,f216);
};

void CExplosionEvent::Parse(rl::MessageBufferView& buffer)
{
	f186 = buffer.Read<uint16_t>(16);
	f208 = buffer.Read<uint16_t>(13);
	ownerNetId = buffer.Read<uint16_t>(13);
	f214 = buffer.Read<uint16_t>(13); // 1604+
	explosionType = buffer.ReadSigned<int>(8); // 1604+ bit size
	damageScale = buffer.Read<int>(8) / 255.0f;

	posX = buffer.ReadSignedFloat(22, 27648.0f);
	posY = buffer.ReadSignedFloat(22, 27648.0f);
	posZ = buffer.ReadFloat(22, 4416.0f) - 1700.0f;

	f242 = buffer.Read<uint8_t>(1);
	f104 = buffer.Read<uint16_t>(16);
	cameraShake = buffer.Read<int>(8) / 127.0f;

	isAudible = buffer.Read<uint8_t>(1);
	f189 = buffer.Read<uint8_t>(1);
	isInvisible = buffer.Read<uint8_t>(1);
	f126 = buffer.Read<uint8_t>(1);

	if (Is2944())
	{
		auto unk2944 = buffer.Read<uint8_t>(1);
	}

	f241 = buffer.Read<uint8_t>(1);
	f243 = buffer.Read<uint8_t>(1); // 1604+

	f210 = buffer.Read<uint16_t>(13);

	unkX = buffer.ReadSignedFloat(16, 1.1f);
	unkY = buffer.ReadSignedFloat(16, 1.1f);
	unkZ = buffer.ReadSignedFloat(16, 1.1f);

	f190 = buffer.Read<uint8_t>(1);
	f191 = buffer.Read<uint8_t>(1);

	f164 = buffer.Read<uint32_t>(32);
	
	if (f242)
	{
		posX224 = buffer.ReadSignedFloat(31, 27648.0f);
		posY224 = buffer.ReadSignedFloat(31, 27648.0f);
		posZ224 = buffer.ReadFloat(31, 4416.0f) - 1700.0f;
	}
	else
	{
		posX224 = 0;
		posY224 = 0;
		posZ224 = 0;
	}

	if (Is2060()) // >= 1868: f_168
	{
		auto f168 = buffer.Read<uint32_t>(32);
	}

	f240 = buffer.Read<uint8_t>(1);
	if (f240)
	{
		f218 = buffer.Read<uint16_t>(16);

		if (f191)
		{
			f216 = buffer.Read<uint8_t>(8);
		}
	}
}

/*NETEV weaponDamageEvent SERVER
/#*
 * Triggered when a client wants to apply damage to a remotely-owned entity. This event can be canceled.
 *
 * @param sender - The server-side player ID of the player that triggered the event.
 * @param data - The event data.
 #/
declare function weaponDamageEvent(sender: number, data: {
	/#*
	 * A value (between 0 and 3) containing an internal damage type.
	 * Specific values are currently unknown.
	 #/
	damageType: number,

	/#*
	 * The weapon hash for the inflicted damage.
	 #/
	weaponType: number,

	/#*
	 * If set, 'weaponDamage' is valid. If unset, the game infers the damage from weapon metadata.
	 #/
	overrideDefaultDamage: boolean,

	/#*
	 * Whether the damage should be inflicted as if it hit the weapon the entity is carrying.
	 * This likely applies to grenades being hit, which should explode, but also normal weapons, which should not harm the player much.
	 #/
	hitEntityWeapon: boolean,

	/#*
	 * Whether the damage should be inflicted as if it hit an ammo attachment component on the weapon.
	 * This applies to players/peds carrying weapons where another player shooting the ammo component makes the weapon explode.
	 #/
	hitWeaponAmmoAttachment: boolean,

	/#*
	 * Set when the damage is applied using a silenced weapon.
	 #/
	silenced: boolean,

	damageFlags: number,
	hasActionResult: boolean,

	actionResultName: number,
	actionResultId: number,
	f104: number,

	/#*
	 * The amount of damage inflicted, if `overrideDefaultDamage` is set. If not, this value is set to `0`.
	 #/
	weaponDamage: number,

	isNetTargetPos: boolean,

	localPosX: number,
	localPosY: number,
	localPosZ: number,

	f112: boolean,

	/#*
	 * The timestamp the damage was originally inflicted at. This should match the global network timer.
	 #/
	damageTime: number,

	/#*
	 * Whether the originating client thinks this should be instantly-lethal damage, such as a critical headshot.
	 #/
	willKill: boolean,

	f120: number,
	hasVehicleData: boolean,

	f112_1: number,

	parentGlobalId: number,

	/#*
	 * The network ID of the victim entity.
	 #/
	hitGlobalId: number,

	/#*
	 * An array containing network IDs of victim entities. If there is more than one, the first one will be set in `hitGlobalId`.
	 #/
	hitGlobalIds: number[],

	tyreIndex: number,
	suspensionIndex: number,
	hitComponent: number,

	f133: boolean,
	hasImpactDir: boolean,

	impactDirX: number,
	impactDirY: number,
	impactDirZ: number,
}): void;
*/
struct CWeaponDamageEvent
{
	void Parse(rl::MessageBufferView& buffer);

	void SetTargetPlayers(fx::ServerGameState* sgs, const std::vector<uint16_t>& targetPlayers);

	inline std::string GetName()
	{
		return "weaponDamageEvent";
	}

	uint8_t damageType;
	uint32_t weaponType; // weaponHash

	bool overrideDefaultDamage;
	bool hitEntityWeapon;
	bool hitWeaponAmmoAttachment;
	bool silenced;

	uint32_t damageFlags;
	bool hasActionResult;

	uint32_t actionResultName;
	uint16_t actionResultId;
	uint32_t f104;

	uint32_t weaponDamage;
	bool isNetTargetPos;

	float localPosX;
	float localPosY;
	float localPosZ;

	bool f112;

	uint32_t damageTime;
	bool willKill;
	uint32_t f120;
	bool hasVehicleData;

	uint16_t f112_1;

	uint16_t parentGlobalId; // Source entity?
	uint16_t hitGlobalId; // Target entity?

	std::vector<uint16_t> hitGlobalIds;

	uint8_t tyreIndex;
	uint8_t suspensionIndex;
	uint8_t hitComponent;

	bool f133;
	bool hasImpactDir;

	float impactDirX;
	float impactDirY;
	float impactDirZ;

	MSGPACK_DEFINE_MAP(damageType, weaponType, overrideDefaultDamage, hitEntityWeapon, hitWeaponAmmoAttachment, silenced, damageFlags, hasActionResult, actionResultName, actionResultId, f104, weaponDamage, isNetTargetPos, localPosX, localPosY, localPosZ, f112, damageTime, willKill, f120, hasVehicleData, f112_1, parentGlobalId, hitGlobalId, tyreIndex, suspensionIndex, hitComponent, f133, hasImpactDir, impactDirX, impactDirY, impactDirZ, hitGlobalIds);
};

void CWeaponDamageEvent::Parse(rl::MessageBufferView& buffer)
{
	if (Is2060() && !Is2372())
	{
		buffer.Read<uint16_t>(16);
	}

	damageType = buffer.Read<uint8_t>(2);
	weaponType = buffer.Read<uint32_t>(32);

	overrideDefaultDamage = buffer.Read<uint8_t>(1);
	hitEntityWeapon = buffer.Read<uint8_t>(1);
	hitWeaponAmmoAttachment = buffer.Read<uint8_t>(1);
	silenced = buffer.Read<uint8_t>(1);

	if (Is3258())
	{
		damageFlags = buffer.Read<uint32_t>(25);
	}
	else if (Is2060())
	{
		damageFlags = buffer.Read<uint32_t>(24);
	}
	else
	{
		damageFlags = buffer.Read<uint32_t>(21);
	}

	// (damageFlags >> 1) & 1
	hasActionResult = buffer.Read<uint8_t>(1);

	if (hasActionResult)
	{
		actionResultName = buffer.Read<uint32_t>(32);
		actionResultId = buffer.Read<uint16_t>(16);
		f104 = buffer.Read<uint32_t>(32);
	}

	if (overrideDefaultDamage)
	{
		weaponDamage = buffer.Read<uint32_t>(Is2699() ? 17 : 14);
	}
	else
	{
		weaponDamage = 0;
	}

	if (Is2060())
	{
		bool _f92 = buffer.Read<uint8_t>(1);

		if (_f92)
		{
			buffer.Read<uint8_t>(Is2802() ? 5 : 4);
		}
	}

	isNetTargetPos = buffer.Read<uint8_t>(1);

	if (isNetTargetPos)
	{
		localPosX = buffer.ReadSignedFloat(16, 55.f);  // divisor: 0x425C0000
		localPosY = buffer.ReadSignedFloat(16, 55.f);
		localPosZ = buffer.ReadSignedFloat(16, 55.f);
	}

	if (damageType == 3)
	{
		damageTime = buffer.Read<uint32_t>(32);
		willKill = buffer.Read<uint8_t>(1);

		if (hasActionResult)
		{
			hitGlobalId = buffer.Read<uint16_t>(13);
		}
		else
		{
			hitGlobalId = 0;
		}

		f112 = buffer.Read<uint8_t>(1);

		if (!f112)
		{
			f112_1 = buffer.Read<uint16_t>(11);
		}
		else
		{
			f112_1 = buffer.Read<uint16_t>(16);
		}
	}
	else
	{
		parentGlobalId = buffer.Read<uint16_t>(13);  // +118
		hitGlobalId = buffer.Read<uint16_t>(13);  // +120
	}

	if (damageType < 2)
	{
		localPosX = buffer.ReadSignedFloat(16, 55.f);  // divisor: 0x425C0000
		localPosY = buffer.ReadSignedFloat(16, 55.f);
		localPosZ = buffer.ReadSignedFloat(16, 55.f);

		if (damageType == 1)
		{
			hasVehicleData = buffer.Read<uint8_t>(1);

			if (hasVehicleData)
			{
				tyreIndex = buffer.Read<uint8_t>(4); // +122
				suspensionIndex = buffer.Read<uint8_t>(4); // +123
			}
		}
	}
	else
	{
		hitComponent = buffer.Read<uint8_t>(5); // +108
	}

	f133 = buffer.Read<uint8_t>(1);
	hasImpactDir = buffer.Read<uint8_t>(1);

	if (hasImpactDir)
	{
		impactDirX = buffer.ReadSignedFloat(16, 6.2831854820251f);  // divisor: 0x40C90FDB
		impactDirY = buffer.ReadSignedFloat(16, 6.2831854820251f);
		impactDirZ = buffer.ReadSignedFloat(16, 6.2831854820251f);
	}
}

void CWeaponDamageEvent::SetTargetPlayers(fx::ServerGameState* sgs, const std::vector<uint16_t>& targetPlayers)
{
	if (hitGlobalId == 0)
	{
		if (!targetPlayers.empty())
		{
			auto instance = sgs->GetServerInstance();
			auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

			for (uint16_t player : targetPlayers)
			{
				if (auto client = clientRegistry->GetClientByNetID(player))
				{
					auto clientDataUnlocked = GetClientDataUnlocked(sgs, client);

					fx::sync::SyncEntityPtr playerEntity;
					{
						std::shared_lock _lock(clientDataUnlocked->playerEntityMutex);
						playerEntity = clientDataUnlocked->playerEntity.lock();
					}

					if (playerEntity)
					{
						uint16_t objectId = playerEntity->handle & 0xFFFF;

						if (hitGlobalId == 0)
						{
							hitGlobalId = objectId;
						}

						hitGlobalIds.push_back(objectId);
					}
				}
			}
		}
	}
	else
	{
		hitGlobalIds.push_back(hitGlobalId);
	}
}

struct CWeaponDamageEventReply
{
	uint32_t health;
	uint32_t time;
	bool f131;

	inline std::string GetName()
	{
		return "weaponDamageReply";
	}

	void Parse(rl::MessageBufferView& buffer)
	{
		health = buffer.Read<uint32_t>(14);
		time = buffer.Read<uint32_t>(32);
		f131 = buffer.ReadBit();
	}

	MSGPACK_DEFINE_MAP(health, time, f131);
};

struct CVehicleComponentControlEvent
{
	void Parse(rl::MessageBufferView& buffer)
	{
		vehicleGlobalId = buffer.Read<uint16_t>(13);
		pedGlobalId = buffer.Read<uint16_t>(13);
		componentIndex = buffer.Read<uint16_t>(5);

		request = buffer.Read<uint8_t>(1);
		componentIsSeat = buffer.Read<uint8_t>(1);

		if (componentIsSeat && request)
		{
			pedInSeat = buffer.Read<uint16_t>(13);
		}
		else
		{
			pedInSeat = 0;
		}
	}

	inline std::string GetName()
	{
		return "vehicleComponentControlEvent";
	}

	int vehicleGlobalId;
	int pedGlobalId;
	int componentIndex;
	bool request;
	bool componentIsSeat;
	int pedInSeat;

	MSGPACK_DEFINE_MAP(vehicleGlobalId, pedGlobalId, componentIndex, request, componentIsSeat, pedInSeat);
};

struct CVehicleComponentControlReply
{
	bool isGranted;
	bool componentIsSeat;
	uint16_t pedGlobalId;

	inline std::string GetName()
	{
		return "vehicleComponentControlReply";
	}

	void Parse(rl::MessageBufferView& buffer)
	{
		isGranted = buffer.ReadBit();
		if (isGranted)
		{
			componentIsSeat = buffer.ReadBit();
			pedGlobalId = componentIsSeat ? buffer.Read<uint16_t>(13) : 0;
		}
		else
		{
			pedGlobalId = 0;
			componentIsSeat = false;
		}
	}

	MSGPACK_DEFINE_MAP(isGranted, componentIsSeat, pedGlobalId);
};

struct CClearPedTasksEvent
{
	void Parse(rl::MessageBufferView& buffer)
	{
		pedId = buffer.Read<uint16_t>(13);
		immediately = buffer.Read<uint8_t>(1);
	}

	inline std::string GetName()
	{
		return "clearPedTasksEvent";
	}

	int pedId;
	bool immediately;

	MSGPACK_DEFINE_MAP(pedId, immediately);
};

struct CRespawnPlayerPedEvent
{
	void Parse(rl::MessageBufferView& buffer)
	{
		posX = buffer.ReadSignedFloat(19, 27648.0f);
		posY = buffer.ReadSignedFloat(19, 27648.0f);
		posZ = buffer.ReadFloat(19, 4416.0f) - 1700.0f;

		f64 = buffer.Read<uint32_t>(32);
		f70 = buffer.Read<uint16_t>(13);
		f72 = buffer.Read<uint32_t>(32);
		f92 = buffer.Read<uint32_t>(32);

		f96 = buffer.Read<uint8_t>(1);
		f97 = buffer.Read<uint8_t>(1);
		f99 = buffer.Read<uint8_t>(1);
		f100 = buffer.Read<uint8_t>(1);

		if (f100)
		{
			f80 = buffer.Read<uint32_t>(32);
			f84 = buffer.Read<uint32_t>(32);
			f88 = buffer.Read<uint32_t>(32);
		}
		else
		{
			f80 = 0;
			f84 = 0;
			f88 = 0;
		}
	}

	inline std::string GetName()
	{
		return "respawnPlayerPedEvent";
	}

	int posX;
	int posY;
	int posZ;

	int f64;
	int f70;
	int f72;
	int f92;

	bool f96;
	bool f97;
	bool f99;
	bool f100;

	int f80;
	int f84;
	int f88;

	MSGPACK_DEFINE_MAP(posX, posY, posZ, f64, f70, f72, f92, f96, f97, f99, f100, f80, f84, f88);
};

struct CRespawnPlayerPedReply
{
	bool respawnFailedResult;

	inline std::string GetName()
	{
		return "respawnPlayerPedReply";
	}

	void Parse(rl::MessageBufferView& buffer)
	{
		respawnFailedResult = buffer.ReadBit();
	}

	MSGPACK_DEFINE_MAP(respawnFailedResult);
};


struct CGiveWeaponEvent
{
    void Parse(rl::MessageBufferView& buffer)
    {
        pedId = buffer.Read<uint16_t>(13);
        weaponType = buffer.Read<uint32_t>(32);
        ammo = buffer.ReadSigned<int>(16);
        unk1 = ammo < 0; // this used to represent the sign
        givenAsPickup = buffer.Read<uint8_t>(1);
    }

    inline std::string GetName()
    {
        return "giveWeaponEvent";
    }

    int pedId;
    int weaponType;
    bool unk1;
    int ammo;
    bool givenAsPickup;

    MSGPACK_DEFINE_MAP(pedId, weaponType, unk1, ammo, givenAsPickup);
};

struct CRemoveWeaponEvent
{
    void Parse(rl::MessageBufferView& buffer)
    {
        pedId = buffer.Read<uint16_t>(13);
        weaponType = buffer.Read<uint32_t>(32);
    }

    inline std::string GetName()
    {
        return "removeWeaponEvent";
    }

    int pedId;
    int weaponType;

    MSGPACK_DEFINE_MAP(pedId, weaponType);
};

/*NETEV removeAllWeaponsEvent SERVER
/#*
 * Triggered when a player removes all weapons from a ped owned by another player.
 *
 * @param sender - The ID of the player that triggered the event.
 * @param data - The event data.
 #/
declare function removeAllWeaponsEvent(sender: number, data: {
	pedId: number
}): void;
*/
struct CRemoveAllWeaponsEvent
{
	void Parse(rl::MessageBufferView& buffer)
	{
		pedId = buffer.Read<uint16_t>(13);
	}

	inline std::string GetName()
	{
		return "removeAllWeaponsEvent";
	}

	int pedId;

	MSGPACK_DEFINE_MAP(pedId);
};

/*NETEV startProjectileEvent SERVER
/#*
 * Triggered when a projectile is created.
 *
 * @param sender - The ID of the player that triggered the event.
 * @param data - The event data.
 #/
declare function startProjectileEvent(sender: number, data: {
    ownerId: number,
	projectileHash: number,
	weaponHash: number,
	initialPositionX: number,
	initialPositionY: number,
	initialPositionZ: number,
	targetEntity: number,
	firePositionX: number,
	firePositionY: number,
	firePositionZ: number,
	effectGroup: number,
	unk3: number,
	commandFireSingleBullet: boolean,
	unk4: number,
	unk5: number,
	unk6: number,
	unk7: number,
	unkX8: number,
	unkY8: number,
	unkZ8: number,
	unk9: number,
	unk10: number,
	unk11: number,
	throwTaskSequence: number,
	unk12: number,
	unk13: number,
	unk14: number,
	unk15: number,
	unk16: number
}): void;
*/
struct CStartProjectileEvent
{
    void Parse(rl::MessageBufferView& buffer)
    {
        ownerId = buffer.Read<uint16_t>(13);
        projectileHash = buffer.Read<uint32_t>(32);

        weaponHash = buffer.Read<uint32_t>(32);
        initialPositionX = buffer.ReadSignedFloat(32, 16000.0f);
        initialPositionY = buffer.ReadSignedFloat(32, 16000.0f);
        initialPositionZ = buffer.ReadSignedFloat(32, 16000.0f);

        targetEntity = buffer.Read<uint16_t>(13);
        firePositionX = buffer.ReadSignedFloat(16, 1.1f);
        firePositionY = buffer.ReadSignedFloat(16, 1.1f);
        firePositionZ = buffer.ReadSignedFloat(16, 1.1f);

        effectGroup = buffer.Read<uint16_t>(5);
        unk3 = buffer.Read<uint16_t>(8);

        commandFireSingleBullet = buffer.Read<uint8_t>(1);
        unk4 = buffer.Read<uint8_t>(1);
        unk5 = buffer.Read<uint8_t>(1);
        unk6 = buffer.Read<uint8_t>(1);

        if (unk6)
        {
            unk7 = buffer.Read<uint16_t>(7);
        }

        if (unk4)
        {
            unkX8 = buffer.ReadSignedFloat(16, 400.0f); // divisor 0x1418BC42C
            unkY8 = buffer.ReadSignedFloat(16, 400.0f);
            unkZ8 = buffer.ReadSignedFloat(16, 400.0f);
        }

        unk9 = buffer.Read<uint8_t>(1);
        unk10 = buffer.Read<uint8_t>(1);

        if (unk10)
        {
            // 0x1419E9B08 - 0x1418F0FDC
            unk11 = buffer.ReadSignedFloat(18, 8000.0f) * 0.000003814712f;
        }
        else
        {
            unk11 = -1;
        }

        if (unk9)
        {
            throwTaskSequence = buffer.Read<uint32_t>(32);
        }

        unk12 = buffer.Read<uint8_t>(1);
        unk13 = buffer.Read<uint16_t>(13);
        unk14 = buffer.Read<uint16_t>(13);
        unk15 = buffer.Read<uint8_t>(1);

        if (unk15)
        {
            // TODO
            buffer.Read<uint8_t>(9);
            buffer.Read<uint8_t>(9);
            buffer.Read<uint8_t>(9);
        }

        unk16 = buffer.Read<uint8_t>(16);
    }

    inline std::string GetName()
    {
        return "startProjectileEvent";
    }

    int ownerId;
    int projectileHash; // Ammo hash

    int weaponHash;
    float initialPositionX;
    float initialPositionY;
    float initialPositionZ;

    int targetEntity;
    float firePositionX; // Direction?
    float firePositionY;
    float firePositionZ;

    int effectGroup;
    int unk3;

    bool commandFireSingleBullet;
    bool unk4;
    bool unk5;
    bool unk6;

    int unk7;

    float unkX8;
    float unkY8;
    float unkZ8;

    bool unk9;
    bool unk10;

    int unk11;

    int throwTaskSequence;

    bool unk12;
    int unk13;
    int unk14;
    bool unk15;

    int unk16;

    MSGPACK_DEFINE_MAP(ownerId, projectileHash, weaponHash, initialPositionX, initialPositionY, initialPositionZ, targetEntity, firePositionX, firePositionY, firePositionZ, effectGroup, unk3, commandFireSingleBullet, unk4, unk5, unk6, unk7, unkX8, unkY8, unkZ8, unk9, unk10, unk11, throwTaskSequence, unk12, unk13, unk14, unk15, unk16);
};

/*NETEV ptFxEvent SERVER
/#*
 * Triggered when a particle fx (ptFx) is created.
 *
 * @param sender - The ID of the player that triggered the event.
 * @param data - The event data.
 #/
declare function ptFxEvent(sender: number, data: {
	effectHash: number,
	assetHash: number,
	posX: number,
	posY: number,
	posZ: number,
	offsetX: number,
	offsetY: number,
	offsetZ: number,
	rotX: number,
	rotY: number,
	rotZ: number,
	scale: number,
	axisBitset: number,
	isOnEntity: boolean,
	entityNetId: number,
	f109: boolean,
	f92: number,
	f110: boolean,
	f105: number,
	f106: number,
	f107: number,
	f111: boolean,
	f100: number
}): void;
*/
struct CNetworkPtFXEvent
{
	void Parse(rl::MessageBufferView& buffer)
	{
		effectHash = buffer.Read<uint32_t>(32);
		assetHash = buffer.Read<uint32_t>(32);

		int _posX = buffer.ReadSignedFloat(19, 27648.0f);
		int _posY = buffer.ReadSignedFloat(19, 27648.0f);
		int _posZ = buffer.ReadFloat(19, 4416.0f) - 1700.0f;

		rotX = buffer.ReadSignedFloat(19, 27648.0f) * toDegrees;
		rotY = buffer.ReadSignedFloat(19, 27648.0f) * toDegrees;
		rotZ = (buffer.ReadFloat(19, 4416.0f) - 1700.0f) * toDegrees;

		scale = (buffer.Read<int>(10) / 1023.0f) * 10.0f;

		axisBitset = buffer.Read<uint8_t>(3);

		isOnEntity = buffer.Read<uint8_t>(1);

		if (isOnEntity)
		{
			posX = 0.0f;
			posY = 0.0f;
			posZ = 0.0f;

			offsetX = _posX;
			offsetY = _posY;
			offsetZ = _posZ;

			entityNetId = buffer.Read<uint16_t>(13);
		}
		else
		{
			posX = _posX;
			posY = _posY;
			posZ = _posZ;

			offsetX = 0.0f;
			offsetY = 0.0f;
			offsetZ = 0.0f;

			entityNetId = 0;
		}

		f109 = buffer.Read<uint8_t>(1);

		if (f109)
		{
			f92 = buffer.Read<int>(32);
		}
		else
		{
			f92 = -1;
		}

		f110 = buffer.Read<uint8_t>(1);

		if (f110)
		{
			f105 = buffer.Read<uint16_t>(8);
			f106 = buffer.Read<uint16_t>(8);
			f107 = buffer.Read<uint16_t>(8);
		}
		else
		{
			f105 = 0;
			f107 = 0;
		}

		f111 = buffer.Read<uint8_t>(1);

		if (f111)
		{
			f100 = buffer.Read<int>(8) / 255.0f;
		}
		else
		{
			f100 = -1.0f;
		}
	}

	inline std::string GetName()
	{
		return "ptFxEvent";
	}

	double toDegrees = 180.0 / boost::math::constants::pi<double>();

	uint32_t effectHash;
	uint32_t assetHash;

	float posX;
	float posY;
	float posZ;

	float offsetX;
	float offsetY;
	float offsetZ;

	float rotX;
	float rotY;
	float rotZ;

	float scale;

	uint8_t axisBitset;

	bool isOnEntity;

	uint16_t entityNetId;

	bool f109;

	int f92;

	bool f110;

	int f105;
	int f106;
	int f107;

	bool f111;

	float f100;

	MSGPACK_DEFINE_MAP(effectHash, assetHash, posX, posY, posZ, offsetX, offsetY, offsetZ, rotX, rotY, rotZ, scale, axisBitset, isOnEntity, entityNetId, f109, f92, f110, f105, f106, f107, f111, f100);
};

struct CRequestNetworkSyncedSceneEvent
{
	uint32_t sceneId; // Increased from uint16_t, see "NetworkSynchronisedSceneHacks.cpp"

	void Parse(rl::MessageBufferView& buffer)
	{
		sceneId = buffer.Read<uint32_t>(32);
	}

	inline std::string GetName()
	{
		return "requestNetworkSyncedSceneEvent";
	}

	MSGPACK_DEFINE_MAP(sceneId);
};

struct CStartNetworkSyncedSceneEvent
{
private:
	struct PedEntityData
	{
		uint16_t objectId;
		uint32_t animPartialHash;
		float blendIn;
		float blendOut;
		float moverBlendIn;
		int flags;
		int ragdollBlockingFlags;
		int ikFlags;

		void Parse(rl::MessageBufferView& buffer)
		{
			objectId = buffer.Read<uint16_t>(13);
			animPartialHash = buffer.Read<uint32_t>(32);
			blendIn = buffer.ReadSignedFloat(30, 1001.0f);
			blendOut = buffer.ReadSignedFloat(30, 1001.0f);
			moverBlendIn = buffer.ReadSignedFloat(30, 1001.0f);
			flags = buffer.Read<int>(32);
			ragdollBlockingFlags = buffer.Read<int>(32);
			ikFlags = buffer.Read<int>(15);
		}

		MSGPACK_DEFINE_MAP(objectId, animPartialHash, blendIn, blendOut, moverBlendIn, flags, ragdollBlockingFlags, ikFlags);
	};

	struct NonPedEntityData
	{
		uint16_t objectId;
		uint32_t animHash;
		float blendIn;
		float blendOut;
		int flags;

		void Parse(rl::MessageBufferView& buffer)
		{
			objectId = buffer.Read<uint16_t>(13);
			animHash = buffer.Read<uint32_t>(32);
			blendIn = buffer.ReadSignedFloat(30, 1001.0f);
			blendOut = buffer.ReadSignedFloat(30, 1001.0f);
			flags = buffer.Read<int>(32);
		}

		MSGPACK_DEFINE_MAP(objectId, animHash, blendIn, blendOut, flags);
	};

	struct MapEntityData
	{
		uint32_t nameHash;
		float posX;
		float posY;
		float posZ;
		float blendIn;
		float blendOut;
		int flags;
		uint32_t animHash;

		void Parse(rl::MessageBufferView& buffer)
		{
			nameHash = buffer.Read<uint32_t>(32);
			posX = buffer.ReadSignedFloat(19, 27648.0f);
			posY = buffer.ReadSignedFloat(19, 27648.0f);
			posZ = buffer.ReadFloat(19, 4416.0f) - 1700.0f;
			blendIn = buffer.ReadSignedFloat(30, 1001.0f);
			blendOut = buffer.ReadSignedFloat(30, 1001.0f);
			flags = buffer.Read<int>(32);
			animHash = buffer.Read<uint32_t>(32);
		}

		MSGPACK_DEFINE_MAP(nameHash, posX, posY, posZ, blendIn, blendOut, flags, animHash);
	};

	template<typename TEntityData>
	static bool SanitizeEntity(fx::ServerGameState* sgs, const TEntityData& entityData, const uint32_t clientNetId)
	{
		const auto entity = sgs->GetEntity(0, entityData.objectId);
		const auto owner = entity ? entity->GetClient() : fx::ClientSharedPtr{};

		if (owner && clientNetId != owner->GetNetId() && !entity->allowRemoteSyncedScenes)
		{
			return false;
		}

		return true;
	}

public:
	uint32_t sceneId; // Increased from uint16_t, see "NetworkSynchronisedSceneHacks.cpp"
	uint32_t startTime;

	bool isActive;

	float scenePosX;
	float scenePosY;
	float scenePosZ;

	float sceneRotX;
	float sceneRotY;
	float sceneRotZ;
	float sceneRotW;

	bool hasAttachEntity;
	uint16_t attachEntityId;
	uint8_t attachEntityBone;

	float phaseToStopScene;
	float rate;

	bool holdLastFrame;
	bool isLooped;
	float phase;

	uint32_t cameraAnimHash;
	uint32_t animDictHash;

	std::vector<PedEntityData> pedEntities;
	std::vector<NonPedEntityData> nonPedEntities;
	std::vector<MapEntityData> mapEntities;

	void Parse(rl::MessageBufferView& buffer)
	{
		sceneId = buffer.Read<uint32_t>(32);

		startTime = buffer.Read<uint32_t>(32);

		isActive = buffer.ReadBit();

		scenePosX = buffer.ReadSignedFloat(26, 27648.0f);
		scenePosY = buffer.ReadSignedFloat(26, 27648.0f);
		scenePosZ = buffer.ReadFloat(26, 4416.0f) - 1700.0f;

		sceneRotX = buffer.ReadSignedFloat(30, 1.0f);
		sceneRotY = buffer.ReadSignedFloat(30, 1.0f);
		sceneRotZ = buffer.ReadSignedFloat(30, 1.0f);
		sceneRotW = buffer.ReadSignedFloat(30, 1.0f);

		hasAttachEntity = buffer.ReadBit();
		if (hasAttachEntity)
		{
			attachEntityId = buffer.Read<uint16_t>(13);
			attachEntityBone = buffer.Read<uint8_t>(8);
		}
		else
		{
			attachEntityId = 0;
			attachEntityBone = 0;
		}

		phaseToStopScene = buffer.ReadBit() ? 1.0f : buffer.ReadFloat(9, 1.0f);
		rate = buffer.ReadBit() ? 1.0f : buffer.ReadFloat(8, 2.0f);

		holdLastFrame = buffer.ReadBit();
		isLooped = buffer.ReadBit();
		phase = buffer.ReadFloat(9, 1.0f);

		cameraAnimHash = buffer.ReadBit() ? buffer.Read<uint32_t>(32) : 0;
		animDictHash = buffer.Read<uint32_t>(32);

		for (int i = 0; i < 10; i++) // ped entities
		{
			if (buffer.ReadBit())
			{
				PedEntityData entityData;
				entityData.Parse(buffer);
				pedEntities.push_back(entityData);
			}
		}


		const auto maxNonPedEntities = Is2189() ? 5 : Is2060() ? 4 : 3;
		for (int i = 0; i < maxNonPedEntities; i++) // non-ped entities
		{
			if (buffer.ReadBit())
			{
				NonPedEntityData entityData;
				entityData.Parse(buffer);
				nonPedEntities.push_back(entityData);
			}
		}

		if (Is2060())
		{
			for (int i = 0; i < 1; i++) // map entities
			{
				if (buffer.ReadBit())
				{
					MapEntityData entityData;
					entityData.Parse(buffer);
					mapEntities.push_back(entityData);
				}
			}
		}
	}

	inline std::string GetName()
	{
		return "startNetworkSyncedSceneEvent";
	}

	bool Sanitize(fx::ServerGameState* sgs, const fx::ClientSharedPtr& client) const
	{
		const auto clientNetId = client->GetNetId();

		const auto passedValidation = std::all_of(pedEntities.begin(), pedEntities.end(), [&sgs, clientNetId](const auto& pedEntity)
		{
			return SanitizeEntity<PedEntityData>(sgs, pedEntity, clientNetId);
		}) && std::all_of(nonPedEntities.begin(), nonPedEntities.end(), [&sgs, clientNetId](const auto& nonPedEntity)
		{
			return SanitizeEntity<NonPedEntityData>(sgs, nonPedEntity, clientNetId);
		});

		if (!passedValidation)
		{
			static std::chrono::milliseconds lastWarn{ -120 * 1000 };

			auto now = msec();

			if ((now - lastWarn) > std::chrono::seconds{ 120 })
			{
				console::PrintWarning("sync", "A client (netID %d) tried to use NetworkStartSynchronisedScene, but it was rejected.\n"
					"Synchronized Scenes that include remotely owned entities need to be allowlisted. To fix this, use \"SetEntityRemoteSyncedScenesAllowed(entityId, true)\".\n",
					client->GetNetId());

				lastWarn = now;
			}
		}

		return passedValidation;
	}

	MSGPACK_DEFINE_MAP(sceneId, startTime, isActive, scenePosX, scenePosY, scenePosZ, sceneRotX, sceneRotY, sceneRotZ, sceneRotW, hasAttachEntity, attachEntityId, attachEntityBone, phaseToStopScene, rate, holdLastFrame, isLooped, phase, cameraAnimHash, animDictHash, pedEntities, nonPedEntities, mapEntities);
};

struct CUpdateNetworkSyncedSceneEvent
{
	uint32_t sceneId; // Increased from uint16_t, see "NetworkSynchronisedSceneHacks.cpp"
	float rate;

	void Parse(rl::MessageBufferView& buffer)
	{
		sceneId = buffer.Read<uint32_t>(32);

		rate = (buffer.Read<uint8_t>(8) / 255.0f) * 2.0f;
	}

	inline std::string GetName()
	{
		return "updateNetworkSyncedSceneEvent";
	}

	MSGPACK_DEFINE_MAP(sceneId, rate);
};

struct CStopNetworkSyncedSceneEvent
{
	uint32_t sceneId; // Increased from uint16_t, see "NetworkSynchronisedSceneHacks.cpp"

	void Parse(rl::MessageBufferView& buffer)
	{
		sceneId = buffer.Read<uint32_t>(32);
	}

	inline std::string GetName()
	{
		return "stopNetworkSyncedSceneEvent";
	}

	MSGPACK_DEFINE_MAP(sceneId);
};

/*NETEV givePedScriptedTaskEvent SERVER
/#*
 * Triggered when a client requests to assign a scripted task to a remotely-controlled ped.
 *
 * @param sender - The network ID of the player initiating the event.
 * @param data - The event data.
 #/
declare function givePedScriptedTaskEvent(sender: number, data: {
	/#*
	 * The network ID of the target ped receiving the task.
	 #/
	entityNetId: number,
	/#*
	 * The ID of the assigned task. See [GetIsTaskActive](https://docs.fivem.net/natives/?_0xB0760331C7AA4155)
	 #/
	taskId: number,
	
}): void;
*/
struct CGivePedScriptedTaskEvent
{
	uint16_t entityNetId;
	uint16_t taskId;

	void Parse(rl::MessageBufferView& buffer)
	{
		entityNetId = buffer.Read<uint16_t>(16);
		taskId = buffer.Read<uint16_t>(10);
	}

	inline std::string GetName()
	{
		return "givePedScriptedTaskEvent";
	}

	MSGPACK_DEFINE_MAP(entityNetId, taskId);
};
#endif

#ifdef STATE_RDR3
struct CExplosionEvent
{
	void Parse(rl::MessageBufferView& buffer);

	inline std::string GetName()
	{
		return "explosionEvent";
	}

	uint16_t f218;
	uint16_t f240;
	int ownerNetId;
	uint16_t f246;
	int explosionType;
	float damageScale;

	float posX;
	float posY;
	float posZ;

	bool f274;
	uint16_t f120;
	float cameraShake;

	bool isAudible;
	bool f221;
	bool isInvisible;
	bool f142;
	bool f273;

	uint32_t unkHash1436;
	uint16_t attachEntityId;
	uint8_t f244;

	float unkX;
	float unkY;
	float unkZ;

	bool f222;
	bool f223;
	bool f225;
	bool f226;
	bool f276;

	uint32_t f180;
	uint32_t f184;

	float posX256;
	float posY256;
	float posZ256;

	bool f272;
	uint16_t f250;
	uint8_t f248;

	MSGPACK_DEFINE_MAP(f218, f240, ownerNetId, f246, explosionType, damageScale, posX, posY, posZ, f274, f120, cameraShake, isAudible, f221, isInvisible, f142, f273, unkHash1436, attachEntityId, f244, unkX, unkY, unkZ, f223, f225, f226, f276, f180, f184, posX256, posY256, posZ256, f272, f250, f248);
};

void CExplosionEvent::Parse(rl::MessageBufferView& buffer)
{
	f218 = buffer.Read<uint16_t>(16);
	f240 = buffer.Read<uint16_t>(13);
	ownerNetId = buffer.Read<uint16_t>(13);
	f246 = buffer.Read<uint16_t>(13);
	explosionType = buffer.ReadSigned<int>(7);
	damageScale = buffer.Read<int>(8) / 255.0f;

	posX = buffer.ReadSignedFloat(22, 27648.0f);
	posY = buffer.ReadSignedFloat(22, 27648.0f);
	posZ = buffer.ReadFloat(22, 4416.0f) - 1700.0f;

	f274 = buffer.ReadBit();
	f120 = buffer.Read<uint16_t>(16);
	cameraShake = buffer.Read<int>(8) / 127.0f;

	isAudible = buffer.ReadBit();
	f221 = buffer.ReadBit();
	isInvisible = buffer.ReadBit();
	f142 = buffer.ReadBit();
	f273 = buffer.ReadBit();

	unkHash1436 = buffer.Read<uint32_t>(32);

	attachEntityId = buffer.Read<uint16_t>(13);
	f244 = buffer.Read<uint8_t>(5); // 1311+

	unkX = buffer.ReadSignedFloat(16, 1.1f);
	unkY = buffer.ReadSignedFloat(16, 1.1f);
	unkZ = buffer.ReadSignedFloat(16, 1.1f);

	f222 = buffer.ReadBit();
	f223 = buffer.ReadBit();
	f225 = buffer.ReadBit();
	f226 = buffer.ReadBit();
	f276 = buffer.ReadBit();

	f180 = buffer.ReadBit() ? buffer.Read<uint32_t>(32) : 0;
	f184 = buffer.ReadBit() ? buffer.Read<uint32_t>(32) : 0;

	if (f274)
	{
		posX256 = buffer.ReadSignedFloat(31, 27648.0f);
		posY256 = buffer.ReadSignedFloat(31, 27648.0f);
		posZ256 = buffer.ReadFloat(31, 4416.0f) - 1700.0f;
	}
	else
	{
		posX256 = 0;
		posY256 = 0;
		posZ256 = 0;
	}

	f272 = buffer.ReadBit();
	if (f272)
	{
		f250 = buffer.Read<uint16_t>(16); // network id

		if (f250)
		{
			f248 = buffer.Read<uint8_t>(8); // player index
		}
	}
}

struct CWeaponDamageEvent
{
	void Parse(rl::MessageBufferView& buffer);

	void SetTargetPlayers(fx::ServerGameState* sgs, const std::vector<uint16_t>& targetPlayers);

	inline std::string GetName()
	{
		return "weaponDamageEvent";
	}

	uint8_t damageType;
	uint32_t weaponType; // weaponHash
	uint32_t f92;

	bool overrideDefaultDamage;
	bool hitEntityWeapon;
	bool hitWeaponAmmoAttachment;
	bool silenced;
	uint8_t f201;

	uint64_t damageFlags;
	bool hasActionResult;

	uint32_t actionResultName;
	uint16_t actionResultId;
	uint32_t f124;
	uint32_t f120;
	bool f136;
	uint32_t f132;

	float weaponDamage;
	float f100;

	bool isNetTargetPos;

	float localPosX;
	float localPosY;
	float localPosZ;

	uint32_t damageTime;
	uint8_t f184;
	uint16_t f156;

	bool hasVehicleData;
	uint16_t parentGlobalId; // Source entity?
	uint16_t hitGlobalId; // Target entity?

	uint32_t f140;
	uint32_t f144;
	uint8_t f148;

	std::vector<uint16_t> hitGlobalIds;

	uint8_t tyreIndex;
	uint8_t suspensionIndex;

	uint32_t f152;
	bool f202;
	bool f203;
	bool f204;
	bool f197;
	bool f205;
	bool f206;

	uint8_t f176;
	uint8_t f177;
	uint32_t f170;

	uint16_t f162;

	MSGPACK_DEFINE_MAP(damageType, weaponType, f92, overrideDefaultDamage, hitEntityWeapon, hitWeaponAmmoAttachment, silenced, f201, damageFlags, hasActionResult, actionResultName, actionResultId, f124, f120, f136, f132, weaponDamage, f100, isNetTargetPos, localPosX, localPosY, localPosZ, damageTime, f184, f156, hasVehicleData, parentGlobalId, hitGlobalId, f140, f144, f148, hitGlobalIds, tyreIndex, suspensionIndex, f152, f202, f203, f204, f197, f205, f206, f176, f177, f170, f162);
};

void CWeaponDamageEvent::Parse(rl::MessageBufferView& buffer)
{
	damageType = buffer.Read<uint8_t>(2);
	weaponType = buffer.ReadBit() ? buffer.Read<uint32_t>(32) : 0;
	f92 = buffer.ReadBit() ? buffer.Read<uint32_t>(32) : 0;

	overrideDefaultDamage = buffer.ReadBit();
	hitEntityWeapon = buffer.ReadBit();
	hitWeaponAmmoAttachment = buffer.ReadBit();
	silenced = buffer.ReadBit();
	f201 = buffer.Read<uint8_t>(1);

	damageFlags = buffer.ReadLong(46);
	hasActionResult = buffer.ReadBit();

	if (hasActionResult)
	{
		actionResultName = buffer.Read<uint32_t>(32);
		actionResultId = buffer.Read<uint16_t>(16);
		f124 = buffer.Read<uint32_t>(32);
		f120 = buffer.Read<uint32_t>(32);
		f136 = buffer.ReadBit();
		f132 = buffer.ReadBit() ? buffer.Read<uint32_t>(32) : 0;
	}

	if (overrideDefaultDamage)
	{
		weaponDamage = buffer.Read<uint32_t>(18) / 10.0f; // 10 is a default divisor it seems
		f100 = buffer.ReadFloat(20, 350.0f);
	}
	else
	{
		weaponDamage = 0.0f;
		f100 = 0.0f;
	}

	isNetTargetPos = buffer.ReadBit();

	if (isNetTargetPos)
	{
		localPosX = buffer.ReadSignedFloat(13, 20.0f);
		localPosY = buffer.ReadSignedFloat(13, 20.0f);
		localPosZ = buffer.ReadSignedFloat(13, 20.0f);
	}

	if (damageType == 3)
	{
		damageTime = buffer.Read<uint32_t>(32);
		f184 = buffer.Read<uint8_t>(2); // 1 = willKill

		if (hasActionResult)
		{
			hitGlobalId = buffer.Read<uint16_t>(13);
		}
		else
		{
			hitGlobalId = 0;
		}

		f156 = buffer.Read<uint16_t>(11);
	}
	else
	{
		parentGlobalId = buffer.Read<uint16_t>(13);
		hitGlobalId = buffer.Read<uint16_t>(13);
	}

	if (damageType == 2 || damageType == 3)
	{
		f140 = buffer.ReadBit() ? buffer.Read<uint32_t>(32) : 0;
		f144 = buffer.ReadBit() ? buffer.Read<uint32_t>(32) : 0;
		f148 = buffer.Read<uint8_t>(6); // 1311+
	}

	if (damageType < 2)
	{
		localPosX = buffer.ReadSignedFloat(13, 20.0f);
		localPosY = buffer.ReadSignedFloat(13, 20.0f);
		localPosZ = buffer.ReadSignedFloat(13, 20.0f);

		if (damageType == 1)
		{
			hasVehicleData = buffer.ReadBit();

			if (hasVehicleData)
			{
				tyreIndex = buffer.Read<uint8_t>(4);
				suspensionIndex = buffer.Read<uint8_t>(4);
			}
		}
	}

	f152 = buffer.Read<uint32_t>(32);

	f202 = buffer.ReadBit();
	f203 = buffer.ReadBit();
	f204 = buffer.ReadBit();
	f197 = buffer.ReadBit();
	f205 = buffer.ReadBit();
	f206 = buffer.ReadBit();

	bool has162 = buffer.ReadBit();
	bool has176 = buffer.ReadBit();

	if (has176)
	{
		f176 = buffer.Read<uint8_t>(8);
		f177 = buffer.Read<uint8_t>(3);
		f170 = buffer.Read<uint32_t>(32);
	}
	else
	{
		f176 = -1;
		f177 = 0;
		f170 = 0;
	}

	f162 = (has162) ? buffer.Read<uint16_t>(16) : 0;
}

void CWeaponDamageEvent::SetTargetPlayers(fx::ServerGameState* sgs, const std::vector<uint16_t>& targetPlayers)
{
	if (hitGlobalId == 0)
	{
		if (!targetPlayers.empty())
		{
			auto instance = sgs->GetServerInstance();
			auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

			for (uint16_t player : targetPlayers)
			{
				if (auto client = clientRegistry->GetClientByNetID(player))
				{
					auto clientDataUnlocked = GetClientDataUnlocked(sgs, client);

					fx::sync::SyncEntityPtr playerEntity;
					{
						std::shared_lock _lock(clientDataUnlocked->playerEntityMutex);
						playerEntity = clientDataUnlocked->playerEntity.lock();
					}

					if (playerEntity)
					{
						uint16_t objectId = playerEntity->handle & 0xFFFF;

						if (hitGlobalId == 0)
						{
							hitGlobalId = objectId;
						}

						hitGlobalIds.push_back(objectId);
					}
				}
			}
		}
	}
	else
	{
		hitGlobalIds.push_back(hitGlobalId);
	}
}

struct CWeaponDamageEventReply
{
	uint32_t health;
	uint32_t time;
	bool f185;
	bool f203;
	uint8_t f188;

	void Parse(rl::MessageBufferView& buffer)
	{
		health = buffer.Read<uint32_t>(14);
		time = buffer.Read<uint32_t>(32);
		f185 = buffer.ReadBit();
		f203 = buffer.ReadBit();
		f188 = (f185) ? buffer.Read<uint8_t>(5) : 0;
	}

	inline std::string GetName()
	{
		return "weaponDamageReply";
	}

	MSGPACK_DEFINE_MAP(health, time, f185, f203, f188);
};

struct CRespawnPlayerPedEvent
{
	float posX;
	float posY;
	float posZ;

	int f80;
	int f88;
	int f92;

	bool f96;
	bool f97;
	bool f99;
	bool f100;
	bool f101;

	void Parse(rl::MessageBufferView& buffer)
	{
		posX = buffer.ReadSignedFloat(19, 27648.0f);
		posY = buffer.ReadSignedFloat(19, 27648.0f);
		posZ = buffer.ReadFloat(19, 4416.0f) - 1700.0f;

		f80 = buffer.Read<uint32_t>(32);
		f88 = buffer.Read<uint16_t>(13);
		f92 = buffer.Read<uint32_t>(32);

		f96 = buffer.ReadBit();
		f97 = buffer.ReadBit();
		f99 = buffer.ReadBit();
		f100 = buffer.ReadBit();
		f101 = buffer.ReadBit();
	}

	inline std::string GetName()
	{
		return "respawnPlayerPedEvent";
	}

	MSGPACK_DEFINE_MAP(posX, posY, posZ, f80, f88, f92, f96, f97, f99, f100, f101);
};

struct CRespawnPlayerPedReply
{
	bool respawnFailedResult;

	void Parse(rl::MessageBufferView& buffer)
	{
		respawnFailedResult = buffer.ReadBit();
	}

	inline std::string GetName()
	{
		return "respawnPlayerPedReply";
	}

	MSGPACK_DEFINE_MAP(respawnFailedResult);
};

struct CLightningEvent
{
	uint8_t f84;
	float f88;

	void Parse(rl::MessageBufferView& buffer)
	{
		f84 = buffer.Read<uint8_t>(3);
		f88 = buffer.ReadFloat(10, 1.0f);
	}

	inline std::string GetName()
	{
		return "lightningEvent";
	}

	MSGPACK_DEFINE_MAP(f84, f88);
};

struct CClearPedTasksEvent
{
	uint16_t pedId;
	bool immediately;

	void Parse(rl::MessageBufferView& buffer)
	{
		pedId = buffer.Read<uint16_t>(13);
		immediately = buffer.Read<uint8_t>(1);
	}

	inline std::string GetName()
	{
		return "clearPedTasksEvent";
	}

	MSGPACK_DEFINE_MAP(pedId, immediately);
};

struct CEndLootEvent
{
	uint16_t targetId;
	bool f58;
	bool f59;
	uint8_t f60;

	void Parse(rl::MessageBufferView& buffer)
	{
		targetId = buffer.Read<uint16_t>(13);
		f58 = buffer.ReadBit();
		f59 = buffer.ReadBit();
		f60 = buffer.Read<uint8_t>(3);
	}

	inline std::string GetName()
	{
		return "endLootEvent";
	}

	MSGPACK_DEFINE_MAP(targetId, f58, f59, f60);
};

struct CSendCarriableUpdateCarryStateEvent
{
	uint16_t carriableId;
	uint16_t parentId;
	uint16_t carrierId;

	void Parse(rl::MessageBufferView& buffer)
	{
		carriableId = buffer.Read<uint16_t>(13);
		parentId = buffer.Read<uint16_t>(13);
		carrierId = buffer.Read<uint16_t>(13);
	}

	inline std::string GetName()
	{
		return "sendCarriableUpdateCarryStateEvent";
	}

	MSGPACK_DEFINE_MAP(carriableId, parentId, carrierId);
};

struct CCarriableVehicleStowStartEvent
{
	uint16_t carrierId;
	uint16_t carriableId;
	uint16_t vehicleId;

	void Parse(rl::MessageBufferView& buffer)
	{
		carrierId = buffer.Read<uint16_t>(13);
		carriableId = buffer.Read<uint16_t>(13);
		vehicleId = buffer.Read<uint16_t>(13);
	}

	inline std::string GetName()
	{
		return "carriableVehicleStowStartEvent";
	}

	MSGPACK_DEFINE_MAP(carrierId, carriableId, vehicleId);
};

struct CCarriableVehicleStowCompleteEvent
{
	uint16_t carrierId;
	uint16_t vehicleId;
	uint16_t carriableId;

	bool f70;
	uint32_t f72;
	uint32_t f76;

	float posX;
	float posY;
	float posZ;

	float f96;
	bool f100;

	void Parse(rl::MessageBufferView& buffer)
	{
		carrierId = buffer.Read<uint16_t>(13);
		vehicleId = buffer.Read<uint16_t>(13);
		carriableId = buffer.Read<uint16_t>(13);

		f70 = buffer.ReadBit();
		f72 = buffer.ReadBit() ? buffer.Read<uint32_t>(32) : 0;
		f76 = buffer.Read<uint32_t>(32);

		posX = buffer.ReadSignedFloat(32, 16000.0f);
		posY = buffer.ReadSignedFloat(32, 16000.0f);
		posZ = buffer.ReadSignedFloat(32, 16000.0f);

		f96 = buffer.ReadSignedFloat(16, 200.0f);
		f100 = buffer.ReadBit();
	}

	inline std::string GetName()
	{
		return "carriableVehicleStowCompleteEvent";
	}

	MSGPACK_DEFINE_MAP(carrierId, vehicleId, carriableId, f70, f72, f76, posX, posY, posZ, f96, f100);
};

struct CPickupCarriableEvent
{
	uint16_t carrierId;
	uint16_t carriableId;
	bool f60;
	uint16_t parentId;

	void Parse(rl::MessageBufferView& buffer)
	{
		carrierId = buffer.Read<uint16_t>(13);
		carriableId = buffer.Read<uint16_t>(13);
		f60 = buffer.ReadBit();
		parentId = buffer.Read<uint16_t>(13);
	}

	inline std::string GetName()
	{
		return "pickupCarriableEvent";
	}


	MSGPACK_DEFINE_MAP(carrierId, carriableId, f60, parentId);
};

struct CPlaceCarriableOntoParentEvent
{
	uint16_t carrierId;
	uint16_t carriableId;
	uint16_t parentId;

	uint8_t f62;
	bool f63;
	uint32_t f64;

	void Parse(rl::MessageBufferView& buffer)
	{
		carrierId = buffer.Read<uint16_t>(13);
		carriableId = buffer.Read<uint16_t>(13);
		parentId = buffer.Read<uint16_t>(13);

		f62 = buffer.Read<uint8_t>(4);
		f63 = buffer.ReadBit();
		f64 = buffer.ReadBit() ? buffer.Read<uint32_t>(32) : 0;
	}

	inline std::string GetName()
	{
		return "placeCarriableOntoParentEvent";
	}

	MSGPACK_DEFINE_MAP(carrierId, carriableId, parentId, f62, f63, f64);
};

struct CStartProjectileEvent
{
	int ownerId;
	int projectileHash;

	int weaponHash;
	float initialPositionX;
	float initialPositionY;
	float initialPositionZ;

	int targetEntity;
	float firePositionX;
	float firePositionY;
	float firePositionZ;

	int effectGroup;

	bool f148;
	bool f150;
	bool f149;

	float unkX80;
	float unkY80;
	float unkZ80;

	bool f136;
	float f152;

	uint16_t f146;
	int throwTaskSequence;
	bool f151;

	void Parse(rl::MessageBufferView& buffer)
	{
		ownerId = buffer.Read<uint16_t>(13);
		projectileHash = buffer.Read<uint32_t>(32);

		weaponHash = buffer.Read<uint32_t>(32);
		initialPositionX = buffer.ReadSignedFloat(32, 16000.0f);
		initialPositionY = buffer.ReadSignedFloat(32, 16000.0f);
		initialPositionZ = buffer.ReadSignedFloat(32, 16000.0f);

		targetEntity = buffer.Read<uint16_t>(13);
		firePositionX = buffer.ReadSignedFloat(16, 1.1f);
		firePositionY = buffer.ReadSignedFloat(16, 1.1f);
		firePositionZ = buffer.ReadSignedFloat(16, 1.1f);

		effectGroup = buffer.Read<uint8_t>(5);

		f148 = buffer.ReadBit();
		f150 = buffer.ReadBit();
		f149 = buffer.ReadBit();

		if (f149)
		{
			unkX80 = buffer.ReadSignedFloat(16, 400.0f);
			unkY80 = buffer.ReadSignedFloat(16, 400.0f);
			unkZ80 = buffer.ReadSignedFloat(16, 400.0f);
		}

		f136 = buffer.ReadBit();

		f152 = buffer.ReadBit() ? buffer.ReadSignedFloat(18, 8000.0f) : -1.0f;

		if (f136)
		{
			throwTaskSequence = buffer.Read<uint32_t>(32);
			f151 = buffer.ReadBit();
		}

		f146 = buffer.Read<uint16_t>(16);
	}

	inline std::string GetName()
	{
		return "startProjectileEvent";
	}

	MSGPACK_DEFINE_MAP(ownerId, projectileHash, weaponHash, initialPositionX, initialPositionY, initialPositionZ, targetEntity, firePositionX, firePositionY, firePositionZ, effectGroup, f148, f150, f149, unkX80, unkY80, unkZ80, f136, f152, f146, throwTaskSequence, f151);
};

#endif

inline bool ParseEvent(net::Buffer& buffer, rl::MessageBufferView* outBuffer)
{
	uint16_t length = buffer.Read<uint16_t>();

	if (length == 0)
	{
		return false;
	}

	*outBuffer = rl::MessageBufferView{ net::Span{const_cast<uint8_t*>(buffer.GetRemainingBytesPtr()), std::min(length, static_cast<uint16_t>(buffer.GetRemainingBytes()))} };
	return true;
}

template<typename TEvent>
static constexpr auto HasTargetPlayerSetter(char)
{
	return false;
}

template<typename TEvent>
static constexpr auto HasTargetPlayerSetter(int) -> decltype(std::is_same_v<decltype(std::declval<TEvent>().SetTargetPlayers(std::declval<fx::ServerGameState*>(), std::declval<const std::vector<uint16_t>&>())), void>)
{
	return true;
}

template<typename TEvent>
static constexpr auto HasSanitizer(char)
{
	return false;
}

template<typename TEvent>
static constexpr auto HasSanitizer(int) -> decltype(std::is_same_v<decltype(std::declval<TEvent>().Sanitize(std::declval<fx::ServerGameState*>(), std::declval<const fx::ClientSharedPtr&>())), void>)
{
	return true;
}

// todo: remove when msgNetGameEventV2 is the default handler for game events
template<typename TEvent>
inline auto GetHandler(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer&& buffer, const std::vector<uint16_t>& targetPlayers = {}) -> std::function<bool()>
{
	rl::MessageBufferView msgBuf;
	if (!ParseEvent(buffer, &msgBuf))
	{
		return []()
		{
			return false;
		};
	}

	auto ev = std::make_shared<TEvent>();
	if (msgBuf.GetLength())
	{
		ev->Parse(msgBuf);

		if constexpr (HasSanitizer<TEvent>(0))
		{
			if (!ev->Sanitize(instance->GetComponent<fx::ServerGameState>().GetRef(), client))
			{
				return []()
				{
					return false;
				};
			}
		}
	}

	if constexpr (HasTargetPlayerSetter<TEvent>(0))
	{
		ev->SetTargetPlayers(instance->GetComponent<fx::ServerGameState>().GetRef(), targetPlayers);
	}

	return [instance, client, ev = std::move(ev)]()
	{
		auto evComponent = instance->GetComponent<fx::ResourceManager>()->GetComponent<fx::ResourceEventManagerComponent>();
		return evComponent->TriggerEvent2(ev->GetName(), { }, std::to_string(client->GetNetId()), *ev);
	};
}

template<typename TEvent>
inline auto GetHandlerWithEvent(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::packet::ClientNetGameEventV2& netGameEvent, const std::vector<uint16_t>& targetPlayers = {}) -> std::function<bool()>
{
	auto ev = std::make_shared<TEvent>();
	if (!netGameEvent.data.GetValue().empty())
	{
		rl::MessageBufferView msgBuf { netGameEvent.data.GetValue() };
		ev->Parse(msgBuf);

		if constexpr (HasSanitizer<TEvent>(0))
		{
			if (!ev->Sanitize(instance->GetComponent<fx::ServerGameState>().GetRef(), client))
			{
				return []()
				{
					return false;
				};
			}
		}
	}

	if constexpr (HasTargetPlayerSetter<TEvent>(0))
	{
		ev->SetTargetPlayers(instance->GetComponent<fx::ServerGameState>().GetRef(), targetPlayers);
	}

	return [instance, client, ev = std::move(ev)]()
	{
		auto evComponent = instance->GetComponent<fx::ResourceManager>()->GetComponent<fx::ResourceEventManagerComponent>();
		return evComponent->TriggerEvent2(ev->GetName(), { }, std::to_string(client->GetNetId()), *ev);
	};
}

enum GTA_EVENT_IDS
{
#ifdef STATE_FIVE
	OBJECT_ID_FREED_EVENT,
	OBJECT_ID_REQUEST_EVENT,
	ARRAY_DATA_VERIFY_EVENT,
	SCRIPT_ARRAY_DATA_VERIFY_EVENT,
	REQUEST_CONTROL_EVENT,
	GIVE_CONTROL_EVENT,
	WEAPON_DAMAGE_EVENT,
	REQUEST_PICKUP_EVENT,
	REQUEST_MAP_PICKUP_EVENT,
	GAME_CLOCK_EVENT, // 2372: Removed (index now empty placeholder)
	GAME_WEATHER_EVENT, // 2372: Removed (index now empty placeholder)
	RESPAWN_PLAYER_PED_EVENT,
	GIVE_WEAPON_EVENT,
	REMOVE_WEAPON_EVENT,
	REMOVE_ALL_WEAPONS_EVENT,
	VEHICLE_COMPONENT_CONTROL_EVENT,
	FIRE_EVENT,
	EXPLOSION_EVENT,
	START_PROJECTILE_EVENT,
	UPDATE_PROJECTILE_TARGET_EVENT,
	REMOVE_PROJECTILE_ENTITY_EVENT,
	BREAK_PROJECTILE_TARGET_LOCK_EVENT,
	ALTER_WANTED_LEVEL_EVENT,
	CHANGE_RADIO_STATION_EVENT,
	RAGDOLL_REQUEST_EVENT,
	PLAYER_TAUNT_EVENT,
	PLAYER_CARD_STAT_EVENT,
	DOOR_BREAK_EVENT,
	SCRIPTED_GAME_EVENT,
	REMOTE_SCRIPT_INFO_EVENT,
	REMOTE_SCRIPT_LEAVE_EVENT,
	MARK_AS_NO_LONGER_NEEDED_EVENT,
	CONVERT_TO_SCRIPT_ENTITY_EVENT,
	SCRIPT_WORLD_STATE_EVENT,
	CLEAR_AREA_EVENT,
	CLEAR_RECTANGLE_AREA_EVENT,
	NETWORK_REQUEST_SYNCED_SCENE_EVENT,
	NETWORK_START_SYNCED_SCENE_EVENT,
	NETWORK_STOP_SYNCED_SCENE_EVENT,
	NETWORK_UPDATE_SYNCED_SCENE_EVENT,
	INCIDENT_ENTITY_EVENT,
	GIVE_PED_SCRIPTED_TASK_EVENT,
	GIVE_PED_SEQUENCE_TASK_EVENT,
	NETWORK_CLEAR_PED_TASKS_EVENT,
	NETWORK_START_PED_ARREST_EVENT,
	NETWORK_START_PED_UNCUFF_EVENT,
	NETWORK_SOUND_CAR_HORN_EVENT,
	NETWORK_ENTITY_AREA_STATUS_EVENT,
	NETWORK_GARAGE_OCCUPIED_STATUS_EVENT,
	PED_CONVERSATION_LINE_EVENT,
	SCRIPT_ENTITY_STATE_CHANGE_EVENT,
	NETWORK_PLAY_SOUND_EVENT,
	NETWORK_STOP_SOUND_EVENT,
	NETWORK_PLAY_AIRDEFENSE_FIRE_EVENT,
	NETWORK_BANK_REQUEST_EVENT,
	REQUEST_DOOR_EVENT,
	NETWORK_TRAIN_REPORT_EVENT,
	NETWORK_TRAIN_REQUEST_EVENT,
	NETWORK_INCREMENT_STAT_EVENT,
	MODIFY_VEHICLE_LOCK_WORD_STATE_DATA,
	MODIFY_PTFX_WORD_STATE_DATA_SCRIPTED_EVOLVE_EVENT,
	REQUEST_PHONE_EXPLOSION_EVENT,
	REQUEST_DETACHMENT_EVENT,
	KICK_VOTES_EVENT,
	GIVE_PICKUP_REWARDS_EVENT,
	NETWORK_CRC_HASH_CHECK_EVENT, // 2944: Removed completely
	BLOW_UP_VEHICLE_EVENT,
	NETWORK_SPECIAL_FIRE_EQUIPPED_WEAPON,
	NETWORK_RESPONDED_TO_THREAT_EVENT,
	NETWORK_SHOUT_TARGET_POSITION,
	VOICE_DRIVEN_MOUTH_MOVEMENT_FINISHED_EVENT,
	PICKUP_DESTROYED_EVENT,
	UPDATE_PLAYER_SCARS_EVENT,
	NETWORK_CHECK_EXE_SIZE_EVENT,
	NETWORK_PTFX_EVENT,
	NETWORK_PED_SEEN_DEAD_PED_EVENT,
	REMOVE_STICKY_BOMB_EVENT,
	NETWORK_CHECK_CODE_CRCS_EVENT,
	INFORM_SILENCED_GUNSHOT_EVENT,
	PED_PLAY_PAIN_EVENT,
	CACHE_PLAYER_HEAD_BLEND_DATA_EVENT,
	REMOVE_PED_FROM_PEDGROUP_EVENT,
	REPORT_MYSELF_EVENT,
	REPORT_CASH_SPAWN_EVENT,
	ACTIVATE_VEHICLE_SPECIAL_ABILITY_EVENT,
	BLOCK_WEAPON_SELECTION,
	NETWORK_CHECK_CATALOG_CRC,
#elif defined(STATE_RDR3)
	OBJECT_ID_FREED_EVENT,
	OBJECT_ID_REQUEST_EVENT,
	ARRAY_DATA_VERIFY_EVENT,
	SCRIPT_ARRAY_DATA_VERIFY_EVENT,
	REQUEST_CONTROL_EVENT,
	GIVE_CONTROL_EVENT,
	WEAPON_DAMAGE_EVENT,
	REQUEST_PICKUP_EVENT,
	REQUEST_MAP_PICKUP_EVENT,
	GAME_CLOCK_EVENT,
	GAME_WEATHER_EVENT,
	RESPAWN_PLAYER_PED_EVENT,
	GENERIC_COMPONENT_CONTROL_EVENT,
	FIRE_EVENT,
	FIRE_TRAIL_UPDATE_EVENT,
	EXPLOSION_EVENT,
	START_PROJECTILE_EVENT,
	ALTER_WANTED_LEVEL_EVENT,
	NETWORK_WANTED_EVENT,
	CHANGE_RADIO_STATION_EVENT,
	RAGDOLL_REQUEST_EVENT,
	PLAYER_CARD_STAT_EVENT,
	DOOR_BREAK_EVENT,
	SCRIPTED_GAME_EVENT,
	REMOTE_SCRIPT_INFO_EVENT,
	REMOTE_SCRIPT_LEAVE_EVENT,
	MARK_AS_NO_LONGER_NEEDED_EVENT,
	CONVERT_TO_SCRIPT_ENTITY_EVENT,
	UNUSED_EVENT_28,
	CLEAR_AREA_EVENT,
	CLEAR_VOLUME_EVENT,
	INCIDENT_EVENT,
	INCIDENT_ENTITY_EVENT,
	GIVE_PED_SCRIPTED_TASK_EVENT,
	GIVE_PED_SEQUENCE_TASK_EVENT,
	NETWORK_CLEAR_PED_TASKS_EVENT,
	NETWORK_IGNITE_BOMB_EVENT,
	NETWORK_SOUND_CAR_HORN_EVENT,
	NETWORK_ENTITY_AREA_STATUS_EVENT,
	NETWORK_GARAGE_OCCUPIED_STATUS_EVENT,
	PED_SPEECH_CREATE_EVENT,
	PED_SPEECH_PLAY_EVENT,
	PED_SPEECH_STOP_EVENT,
	PED_SPEECH_ASSIGN_VOICE_EVENT,
	SCRIPT_ENTITY_STATE_CHANGE_EVENT,
	NETWORK_PLAY_SCRIPT_SOUND_EVENT,
	NETWORK_PLAY_AUDIO_ENTITY_SOUND_EVENT,
	NETWORK_STOP_SCRIPT_SOUND_EVENT,
	NETWORK_STOP_AUDIO_ENTITY_SOUND_EVENT,
	UNUSED_EVENT_49,
	NETWORK_TRAIN_REQUEST_EVENT,
	NETWORK_INCREMENT_STAT_EVENT, // 1491.50: Removed completely
	UNUSED_EVENT_52,
	MODIFY_VEHICLE_LOCK_WORD_STATE_DATA,
	UNUSED_EVENT_54,
	INCAPACITATED_REVIVE_EVENT,
	INCAPACITATED_EXECUTE_EVENT,
	REQUEST_PHONE_EXPLOSION_EVENT,
	REQUEST_DETACHMENT_EVENT,
	KICK_VOTES_EVENT,
	GIVE_PICKUP_REWARDS_EVENT,
	BLOW_UP_VEHICLE_EVENT,
	NETWORK_SPECIAL_FIRE_EQUIPPED_WEAPON,
	NETWORK_RESPONDED_TO_THREAT_EVENT,
	NETWORK_SHOUT_TARGET_POSITION_EVENT,
	UNUSED_EVENT_65,
	PICKUP_DESTROYED_EVENT,
	NETWORK_CHECK_EXE_SIZE_EVENT,
	NETWORK_PTFX_EVENT,
	NETWORK_PED_SEEN_DEAD_PED_EVENT,
	UNUSED_EVENT_70,
	NETWORK_CHECK_CODE_CRCS_EVENT,
	PED_PLAY_PAIN_EVENT,
	ADD_OR_REMOVE_PED_FROM_PEDGROUP_EVENT,
	NETWORK_START_PED_HOGTIE_EVENT,
	NETWORK_SEND_PED_LASSO_ATTACH_EVENT,
	NETWORK_SEND_PED_LASSO_DETTACH_EVENT,
	NETWORK_BOLAS_HIT_EVENT,
	NETWORK_SEND_CARRIABLE_UPDATE_CARRY_STATE_EVENT,
	UNUSED_EVENT_79,
	UNUSED_EVENT_80,
	REQUEST_CONTROL_REQUESTER_EVENT,
	NETWORK_VOLUME_LOCK_REQUEST_EVENT,
	NETWORK_VOLUME_LOCK_REQUEST_FAILURE_EVENT,
	NETWORK_VOLUME_LOCK_KEEP_ALIVE_EVENT,
	NETWORK_VOLUME_LOCK_MOVE_EVENT,
	NETWORK_VOLUME_LOCK_ATTACH_EVENT,
	NETWORK_VOLUME_LOCK_DETACH_EVENT,
	NETWORK_VOLUME_LOCK_RESIZE_EVENT,
	NETWORK_VOLUME_LOCK_RELEASE_EVENT,
	NETWORK_VOLUME_LOCK_DATA_EVENT,
	NETWORK_MAKE_WITNESS_EVENT,
	NETWORK_REMOVE_WITNESS_EVENT,
	NETWORK_GANG_INVITE_PLAYER_EVENT,
	NETWORK_GANG_INVITE_RESPONSE_EVENT,
	NETWORK_GANG_INVITE_CANCEL_EVENT,
	NETWORK_GANG_JOIN_REQUEST_EVENT,
	UNUSED_EVENT_97,
	UNUSED_EVENT_98,
	UNUSED_EVENT_99,
	NETWORK_REQUEST_CONVERT_TO_SCRIPT_ENTITY_EVENT,
	NETWORK_REGISTER_CRIME_EVENT,
	NETWORK_CRIME_REPORT_EVENT,
	NETWORK_START_LOOT_EVENT,
	NETWORK_NEW_BUG_EVENT,
	UNUSED_EVENT_105,
	UNUSED_EVENT_106,
	REPORT_MYSELF_EVENT,
	REPORT_CASH_SPAWN_EVENT,
	NETWORK_CHEST_REQUEST_EVENT,
	NETWORK_CHEST_DATA_CHANGE_EVENT,
	NETWORK_CHEST_RELEASE_EVENT,
	NETWORK_CLEAR_GANG_BOUNTY_EVENT,
	NETWORK_PLAYER_REQUEST_CONTENTION_EVENT,
	NETWORK_START_FALLBACK_CARRY_ACTION_EVENT,
	NETWORK_GIVE_ENERGY_EVENT,
	REQUEST_IS_VOLUME_EMPTY,
	UNUSED_EVENT_117,
	NETWORK_DOOR_STATE_CHANGE,
	NETWORK_STAMINA_COST_EVENT,
	NETWORK_REMOVE_DOOR,
	SCRIPT_COMMAND_EVENT,
	NETWORK_KNOCK_PED_OFF_VEHICLE_EVENT,
	NETWORK_SPAWN_SEARCH_EVENT,
	NETWORK_ROPE_WORLD_STATE_DATA_BREAK_EVENT,
	NETWORK_PLAYER_HAT_EVENT,
	NETWORK_CRIME_SCENE_EVENT,
	NETWORK_POINT_OF_INTEREST_EVENT,
	UNUSED_EVENT_128,
	NETWORK_DESTROY_VEHICLE_LOCK_EVENT,
	NETWORK_APPLY_REACTION_EVENT,
	NETWORK_START_LOOT_ALIVE_EVENT,
	NETWORK_SET_ENTITY_GHOST_WITH_PLAYER_EVENT,
	NETWORK_COMBAT_DIRECTOR_EVENT,
	NETWORK_MELEE_ARBITRATION_FAIL_EVENT,
	NETWORK_PED_WHISTLE_EVENT,
	NETWORK_PED_MOTIVATION_CHANGE_EVENT,
	NETWORK_PED_SHARED_TARGETING_EVENT,
	NETWORK_REQUEST_COMBAT_GESTURE,
	LIGHTNING_EVENT,
	PED_TRIGGER_BULLET_FLINCH_EVENT,
	PED_TRIGGER_EXPLOSION_FLINCH_EVENT,
	NETWORK_PLAYER_WHISTLE_EVENT,
	NETWORK_PLAYER_SPURRING_EVENT,
	NETWORK_PLAYER_HORSE_TAMING_CALLOUT_EVENT,
	CONVERSATION_EVENT,
	UNUSED_EVENT_146,
	NETWORK_END_LOOT_EVENT,
	NETWORK_PICKUP_CARRIABLE_EVENT,
	NETWORK_PLACE_CARRIABLE_ONTO_PARENT_EVENT,
	PLAY_DEAD_EVENT,
	NETWORK_PLAYER_HORSE_SHOT_EVENT,
	ANIM_SCENE_ABORT_ENTITY_EVENT,
	NETWORK_REQUEST_ASSET_DETACHMENT,
	NETWORK_CARRIABLE_VEHICLE_STOW_START_EVENT,
	NETWORK_CARRIABLE_VEHICLE_STOW_COMPLETE_EVENT,
	NETWORK_UPDATE_ANIMATED_VEHICLE_PROP_EVENT,
	NETWORK_SET_CARRYING_FLAG_FOR_ENTITY,
	NETWORK_BOUNTY_HUNT_EVENT,
	NETWORK_DEBUG_REQUEST_ENTITY_POSITION,
	NETWORK_REMOVE_PROP_OWNERSHIP,
	NETWORK_DUMP_CARRIABLE_OFF_MOUNT_EVENT,
	NETWORK_FORCE_IN_SCOPE_FOR_DUPLICATE_OBJECT_OWNER, // Added in 1491.50(?)
#endif
};

#ifdef STATE_FIVE
namespace fx
{
template<RequestControlFilterMode Mode>
inline bool RequestControlHandler(fx::ServerGameState* sgs, const fx::ClientSharedPtr& client, uint32_t objectId, const char** reason = nullptr)
{
	auto entity = sgs->GetEntity(0, objectId);

	// nonexistent entities should be ignored, anyway
	if (!entity)
	{
		if (reason)
		{
			*reason = "Entity doesn't exist";
		}

		return false;
	}

	// silly, but the game will ignore this anyway
	if (entity->type == sync::NetObjEntityType::Player)
	{
		if (reason)
		{
			*reason = "Entity is a player";
		}

		return false;
	}

	// if the sender isn't in the same bucket as the entity, nope either
	{
		auto clientData = GetClientDataUnlocked(sgs, client);
		
		if (clientData->routingBucket != entity->routingBucket)
		{
			if (reason)
			{
				*reason = "Entity is in a different routing bucket";
			}

			return false;
		}
	}

	// if the entity is set to ignore the policy, allow
	if (entity->ignoreRequestControlFilter)
	{
		return true;
	}

	// if the sender is strict, nope
	if (sgs->GetEntityLockdownMode(client) == fx::EntityLockdownMode::Strict)
	{
		if (reason)
		{
			*reason = "Strict entity lockdown is active";
		}

		return false;
	}

	if constexpr (Mode == RequestControlFilterMode::FilterAll)
	{
		return false;
	}

	// if we need to, check if the entity is player-controlled
	// for now, this means a vehicle that's occupied by a player.
	// in the future, we could track e.g. attachment state or pending component control
	bool playerControlled = false;

	if constexpr (Mode == RequestControlFilterMode::FilterPlayer || Mode == RequestControlFilterMode::FilterPlayerSettled || Mode == RequestControlFilterMode::FilterPlayerPlusNonPlayerSettled)
	{
		// #TODO: turn this into a 'is this type a vehicle' helper
		if (entity->type == sync::NetObjEntityType::Automobile || entity->type == sync::NetObjEntityType::Bike || entity->type == sync::NetObjEntityType::Boat || entity->type == sync::NetObjEntityType::Heli || entity->type == sync::NetObjEntityType::Plane || entity->type == sync::NetObjEntityType::Submarine || entity->type == sync::NetObjEntityType::Trailer ||
#ifdef STATE_RDR3
			entity->type == sync::NetObjEntityType::DraftVeh ||
#endif
			entity->type == sync::NetObjEntityType::Train)
		{
			if (auto syncTree = entity->syncTree)
			{
				auto vehicleData = entity->syncTree->GetVehicleGameState();
				if (vehicleData->playerOccupants.any())
				{
					playerControlled = true;
				}
			}
		}
	}

	// verify the entity's age, if needed
	bool settled = false;

	if constexpr (Mode == RequestControlFilterMode::FilterPlayerSettled || Mode == RequestControlFilterMode::FilterPlayerPlusNonPlayerSettled)
	{
		auto entityAge = msec() - entity->createdAt;
		if (entityAge >= std::chrono::milliseconds{ g_requestControlSettleDelay })
		{
			settled = true;
		}
	}

	// check the policy
	if constexpr (Mode == RequestControlFilterMode::FilterPlayerPlusNonPlayerSettled)
	{
		if (playerControlled || settled)
		{
			if (reason)
			{
				if (playerControlled)
				{
					*reason = "Entity is controlled by a player";
				}
				else if (settled)
				{
					*reason = "Entity has been settled";
				}
			}

			return false;
		}
	}
	else if constexpr (Mode == RequestControlFilterMode::FilterPlayer)
	{
		if (playerControlled)
		{
			if (reason)
			{
				*reason = "Entity is controlled by a player";
			}

			return false;
		}
	}
	else if constexpr (Mode == RequestControlFilterMode::FilterPlayerSettled)
	{
		if (playerControlled && settled)
		{
			if (reason)
			{
				*reason = "Entity is controlled by a player and has been settled";
			}

			return false;
		}
	}

	// #whatever
	return true;
}
}
#endif

// todo: remove when msgNetGameEventV2 is the default handler for game events
std::function<bool()> fx::ServerGameState::GetRequestControlEventHandler(const fx::ClientSharedPtr& client, net::Buffer&& buffer)
{
#ifndef STATE_FIVE
	return {};
#else
	if (g_requestControlFilterState == RequestControlFilterMode::NoFilter)
	{
		return {};
	}

	uint32_t objectId = 0;
	rl::MessageBufferView msg;

	if (ParseEvent(buffer, &msg))
	{
		objectId = msg.Read<uint32_t>(13);
	}

	return [this, client, objectId]()
	{
		auto handler = []
		{
			switch (g_requestControlFilterState)
			{
				case RequestControlFilterMode::FilterPlayerPlusNonPlayerSettled:
					return &fx::RequestControlHandler<RequestControlFilterMode::FilterPlayerPlusNonPlayerSettled>;
				case RequestControlFilterMode::Default:
				case RequestControlFilterMode::FilterPlayer:
				default:
					return &fx::RequestControlHandler<RequestControlFilterMode::FilterPlayer>;
				case RequestControlFilterMode::FilterPlayerSettled:
					return &fx::RequestControlHandler<RequestControlFilterMode::FilterPlayerSettled>;
				case RequestControlFilterMode::FilterAll:
					return &fx::RequestControlHandler<RequestControlFilterMode::FilterAll>;
			}
		}();

		const char* reason = nullptr;
		bool result = handler(this, client, objectId, &reason);

		if (!result)
		{
			static std::chrono::milliseconds lastWarn{ -120 * 1000 };

			if (g_requestControlFilterState == RequestControlFilterMode::Default)
			{
				auto now = msec();

				if ((now - lastWarn) > std::chrono::seconds{ 120 })
				{
					console::PrintWarning("sync", "A client (slotID %d) tried to use NetworkRequestControlOfEntity (entity network ID %d), but it was rejected (%s).\n"
												  "NetworkRequestControlOfEntity is deprecated, and should not be used because of potential abuse by cheaters. To disable this check, set \"sv_filterRequestControl\" \"0\".\n"
												  "See https://aka.cfx.re/rcmitigation for more information.\n",
					client->GetSlotId(),
					objectId,
					reason);

					lastWarn = now;
				}
			}
		}

		return result;
	};
#endif
}

std::function<bool()> fx::ServerGameState::GetRequestControlEventHandlerWithEvent(const fx::ClientSharedPtr& client, net::packet::ClientNetGameEventV2& netGameEvent)
{
#ifndef STATE_FIVE
	return {};
#else
	if (g_requestControlFilterState == RequestControlFilterMode::NoFilter)
	{
		return {};
	}
	
	rl::MessageBufferView msg {netGameEvent.data.GetValue()};
	const uint32_t objectId = msg.Read<uint32_t>(13);

	return [this, client, objectId]()
	{
		auto handler = []
		{
			switch (g_requestControlFilterState)
			{
				case RequestControlFilterMode::FilterPlayerPlusNonPlayerSettled:
					return &fx::RequestControlHandler<RequestControlFilterMode::FilterPlayerPlusNonPlayerSettled>;
				case RequestControlFilterMode::Default:
				case RequestControlFilterMode::FilterPlayer:
				default:
					return &fx::RequestControlHandler<RequestControlFilterMode::FilterPlayer>;
				case RequestControlFilterMode::FilterPlayerSettled:
					return &fx::RequestControlHandler<RequestControlFilterMode::FilterPlayerSettled>;
				case RequestControlFilterMode::FilterAll:
					return &fx::RequestControlHandler<RequestControlFilterMode::FilterAll>;
			}
		}();

		const char* reason = nullptr;
		bool result = handler(this, client, objectId, &reason);

		if (!result)
		{
			static std::chrono::milliseconds lastWarn{ -120 * 1000 };

			if (g_requestControlFilterState == RequestControlFilterMode::Default)
			{
				auto now = msec();

				if ((now - lastWarn) > std::chrono::seconds{ 120 })
				{
					console::PrintWarning("sync", "A client (slotID %d) tried to use NetworkRequestControlOfEntity (entity network ID %d), but it was rejected (%s).\n"
												  "NetworkRequestControlOfEntity is deprecated, and should not be used because of potential abuse by cheaters. To disable this check, set \"sv_filterRequestControl\" \"0\".\n"
												  "See https://aka.cfx.re/rcmitigation for more information.\n",
					client->GetSlotId(),
					objectId,
					reason);

					lastWarn = now;
				}
			}
		}

		return result;
	};
#endif
}

// todo: remove when msgNetGameEventV2 is the default handler for game events
std::function<bool()> fx::ServerGameState::GetGameEventHandler(const fx::ClientSharedPtr& client, const std::vector<uint16_t>& targetPlayers, net::Buffer&& buffer)
{
	auto instance = m_instance;

	buffer.Read<uint16_t>(); // eventHeader
	bool isReply = buffer.Read<uint8_t>(); // is reply
	uint16_t eventType = buffer.Read<uint16_t>(); // event ID

#ifdef STATE_FIVE
	if (Is2060() && eventType > 55) // patch for 1868+ game build as `NETWORK_AUDIO_BARK_EVENT` was added
	{
		eventType--;
	}

	if (Is2944() && eventType >= 66) // patch for 2944+ game build as `NETWORK_CRC_HASH_CHECK_EVENT` was removed
	{
		eventType++;
	}
#endif

	// RDR3 remaps eventType on the client (netEventMgr_MapEventId)

#if defined(STATE_FIVE) || defined(STATE_RDR3)
#ifdef STATE_FIVE
	if (eventType == NETWORK_PLAY_SOUND_EVENT)
#else
	if (eventType == NETWORK_PLAY_SCRIPT_SOUND_EVENT)
#endif
	{
		return []()
		{
			return g_networkedSoundsEnabled;
		};
	}

	if (eventType == REQUEST_PHONE_EXPLOSION_EVENT)
	{
		return []()
		{
			return g_networkedPhoneExplosionsEnabled;
		};
	}

	// This event should *only* be triggered while a vehicle has a pending owner
	// change, i.e., old owner informing new owner that the vehicle should be
	// blown up. This does not apply to OneSync.
	if (eventType == BLOW_UP_VEHICLE_EVENT)
	{
		return []()
		{
			return false;
		};
	}

	if (eventType == SCRIPT_ENTITY_STATE_CHANGE_EVENT)
	{
		return []()
		{
			return g_networkedScriptEntityStatesEnabled;
		};
	}
#endif

#ifdef STATE_FIVE
	if (eventType == REQUEST_CONTROL_EVENT)
	{
		return GetRequestControlEventHandler(client, std::move(buffer));
	}

	if (isReply)
	{
		switch(eventType)
		{
			case WEAPON_DAMAGE_EVENT: return GetHandler<CWeaponDamageEventReply>(instance, client, std::move(buffer));
			case RESPAWN_PLAYER_PED_EVENT: return GetHandler<CRespawnPlayerPedReply>(instance, client, std::move(buffer));
			case VEHICLE_COMPONENT_CONTROL_EVENT: return GetHandler<CVehicleComponentControlReply>(instance, client, std::move(buffer));
			// All other events have no PrepareReply/HandleReply handlers.
			default:
				break;
		};

		return {};
	}

	switch(eventType)
	{
		case WEAPON_DAMAGE_EVENT: return GetHandler<CWeaponDamageEvent>(instance, client, std::move(buffer), targetPlayers);
		case RESPAWN_PLAYER_PED_EVENT: return GetHandler<CRespawnPlayerPedEvent>(instance, client, std::move(buffer));
		case GIVE_WEAPON_EVENT: return GetHandler<CGiveWeaponEvent>(instance, client, std::move(buffer));
		case REMOVE_WEAPON_EVENT: return GetHandler<CRemoveWeaponEvent>(instance, client, std::move(buffer));
		case REMOVE_ALL_WEAPONS_EVENT: return GetHandler<CRemoveAllWeaponsEvent>(instance, client, std::move(buffer));
		case VEHICLE_COMPONENT_CONTROL_EVENT: return GetHandler<CVehicleComponentControlEvent>(instance, client, std::move(buffer));
		case FIRE_EVENT: return GetHandler<CFireEvent>(instance, client, std::move(buffer));
		case EXPLOSION_EVENT: return GetHandler<CExplosionEvent>(instance, client, std::move(buffer));
		case START_PROJECTILE_EVENT: return GetHandler<CStartProjectileEvent>(instance, client, std::move(buffer));
		case NETWORK_CLEAR_PED_TASKS_EVENT: return GetHandler<CClearPedTasksEvent>(instance, client, std::move(buffer));
		case NETWORK_PTFX_EVENT: return GetHandler<CNetworkPtFXEvent>(instance, client, std::move(buffer));
		case NETWORK_REQUEST_SYNCED_SCENE_EVENT: return GetHandler<CRequestNetworkSyncedSceneEvent>(instance, client, std::move(buffer));
		case NETWORK_START_SYNCED_SCENE_EVENT: return GetHandler<CStartNetworkSyncedSceneEvent>(instance, client, std::move(buffer));
		case NETWORK_UPDATE_SYNCED_SCENE_EVENT: return GetHandler<CUpdateNetworkSyncedSceneEvent>(instance, client, std::move(buffer));
		case NETWORK_STOP_SYNCED_SCENE_EVENT: return GetHandler<CStopNetworkSyncedSceneEvent>(instance, client, std::move(buffer));
		case GIVE_PED_SCRIPTED_TASK_EVENT: return GetHandler<CGivePedScriptedTaskEvent>(instance, client, std::move(buffer));
		default:
			break;
	};
#endif

#ifdef STATE_RDR3
	if (isReply)
	{
		switch(eventType)
		{
			case WEAPON_DAMAGE_EVENT: return GetHandler<CWeaponDamageEventReply>(instance, client, std::move(buffer));
			case RESPAWN_PLAYER_PED_EVENT: return GetHandler<CRespawnPlayerPedReply>(instance, client, std::move(buffer));
			default:
				break;
		};

		return {};
	}

	switch(eventType)
	{
		case WEAPON_DAMAGE_EVENT: return GetHandler<CWeaponDamageEvent>(instance, client, std::move(buffer), targetPlayers);
		case RESPAWN_PLAYER_PED_EVENT: return GetHandler<CRespawnPlayerPedEvent>(instance, client, std::move(buffer));
		case EXPLOSION_EVENT: return GetHandler<CExplosionEvent>(instance, client, std::move(buffer));
		case LIGHTNING_EVENT: return GetHandler<CLightningEvent>(instance, client, std::move(buffer));
		case START_PROJECTILE_EVENT: return GetHandler<CStartProjectileEvent>(instance, client, std::move(buffer));
		case NETWORK_CLEAR_PED_TASKS_EVENT: return GetHandler<CClearPedTasksEvent>(instance, client, std::move(buffer));
		case NETWORK_END_LOOT_EVENT: return GetHandler<CEndLootEvent>(instance, client, std::move(buffer));
		case NETWORK_SEND_CARRIABLE_UPDATE_CARRY_STATE_EVENT: return GetHandler<CSendCarriableUpdateCarryStateEvent>(instance, client, std::move(buffer));
		case NETWORK_CARRIABLE_VEHICLE_STOW_START_EVENT: return GetHandler<CCarriableVehicleStowStartEvent>(instance, client, std::move(buffer));
		case NETWORK_CARRIABLE_VEHICLE_STOW_COMPLETE_EVENT: return GetHandler<CCarriableVehicleStowCompleteEvent>(instance, client, std::move(buffer));
		case NETWORK_PICKUP_CARRIABLE_EVENT: return GetHandler<CPickupCarriableEvent>(instance, client, std::move(buffer));
		case NETWORK_PLACE_CARRIABLE_ONTO_PARENT_EVENT: return GetHandler<CPlaceCarriableOntoParentEvent>(instance, client, std::move(buffer));
		default:
			break;
	};
#endif

	return {};
}

std::function<bool()> fx::ServerGameState::GetGameEventHandlerWithEvent(const fx::ClientSharedPtr& client, const std::vector<uint16_t>& targetPlayers, net::packet::ClientNetGameEventV2& netGameEvent)
{
	auto instance = m_instance;
	
	const bool isReply = netGameEvent.isReply;
	const uint32_t eventNameHash = netGameEvent.eventNameHash;

#if defined(STATE_FIVE) || defined(STATE_RDR3)
#ifdef STATE_FIVE
	if (eventNameHash == net::force_consteval<uint32_t, HashRageString("NETWORK_PLAY_SOUND_EVENT")>)
#else
	if (eventNameHash == net::force_consteval<uint32_t, HashRageString("NETWORK_PLAY_SCRIPT_SOUND_EVENT")>)
#endif
	{
		return []()
		{
			return g_networkedSoundsEnabled;
		};
	}

	if (eventNameHash == net::force_consteval<uint32_t, HashRageString("REQUEST_PHONE_EXPLOSION_EVENT")>)
	{
		return []()
		{
			return g_networkedPhoneExplosionsEnabled;
		};
	}

	// This event should *only* be triggered while a vehicle has a pending owner
	// change, i.e., old owner informing new owner that the vehicle should be
	// blown up. This does not apply to OneSync.
	if (eventNameHash == net::force_consteval<uint32_t, HashRageString("BLOW_UP_VEHICLE_EVENT")>)
	{
		return []()
		{
			return false;
		};
	}

	if (eventNameHash == net::force_consteval<uint32_t, HashRageString("SCRIPT_ENTITY_STATE_CHANGE_EVENT")>)
	{
		return []()
		{
			return g_networkedScriptEntityStatesEnabled;
		};
	}
#endif

#ifdef STATE_FIVE
	if (eventNameHash == net::force_consteval<uint32_t, HashRageString("REQUEST_CONTROL_EVENT")>)
	{
		return GetRequestControlEventHandlerWithEvent(client, netGameEvent);
	}

	if (isReply)
	{
		switch(eventNameHash)
		{
			case net::force_consteval<uint32_t, HashRageString("WEAPON_DAMAGE_EVENT")>: return GetHandlerWithEvent<CWeaponDamageEventReply>(instance, client, netGameEvent);
			case net::force_consteval<uint32_t, HashRageString("RESPAWN_PLAYER_PED_EVENT")>: return GetHandlerWithEvent<CRespawnPlayerPedReply>(instance, client, netGameEvent);
			case net::force_consteval<uint32_t, HashRageString("VEHICLE_COMPONENT_CONTROL_EVENT")>: return GetHandlerWithEvent<CVehicleComponentControlReply>(instance, client, netGameEvent);
			// All other events have no PrepareReply/HandleReply handlers.
			default:
				break;
		};

		return {};
	}

	switch(eventNameHash)
	{
		case net::force_consteval<uint32_t, HashRageString("WEAPON_DAMAGE_EVENT")>: return GetHandlerWithEvent<CWeaponDamageEvent>(instance, client, netGameEvent, targetPlayers);
		case net::force_consteval<uint32_t, HashRageString("RESPAWN_PLAYER_PED_EVENT")>: return GetHandlerWithEvent<CRespawnPlayerPedEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("GIVE_WEAPON_EVENT")>: return GetHandlerWithEvent<CGiveWeaponEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("REMOVE_WEAPON_EVENT")>: return GetHandlerWithEvent<CRemoveWeaponEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("REMOVE_ALL_WEAPONS_EVENT")>: return GetHandlerWithEvent<CRemoveAllWeaponsEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("VEHICLE_COMPONENT_CONTROL_EVENT")>: return GetHandlerWithEvent<CVehicleComponentControlEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("FIRE_EVENT")>: return GetHandlerWithEvent<CFireEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("EXPLOSION_EVENT")>: return GetHandlerWithEvent<CExplosionEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("START_PROJECTILE_EVENT")>: return GetHandlerWithEvent<CStartProjectileEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("NETWORK_CLEAR_PED_TASKS_EVENT")>: return GetHandlerWithEvent<CClearPedTasksEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("NETWORK_PTFX_EVENT")>: return GetHandlerWithEvent<CNetworkPtFXEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("NETWORK_REQUEST_SYNCED_SCENE_EVENT")>: return GetHandlerWithEvent<CRequestNetworkSyncedSceneEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("NETWORK_START_SYNCED_SCENE_EVENT")>: return GetHandlerWithEvent<CStartNetworkSyncedSceneEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("NETWORK_UPDATE_SYNCED_SCENE_EVENT")>: return GetHandlerWithEvent<CUpdateNetworkSyncedSceneEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("NETWORK_STOP_SYNCED_SCENE_EVENT")>: return GetHandlerWithEvent<CStopNetworkSyncedSceneEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("GIVE_PED_SCRIPTED_TASK_EVENT")>: return GetHandlerWithEvent<CGivePedScriptedTaskEvent>(instance, client, netGameEvent);
		default:
			break;
	};
#endif

#ifdef STATE_RDR3
	if (isReply)
	{
		switch(eventNameHash)
		{
			case net::force_consteval<uint32_t, HashRageString("WEAPON_DAMAGE_EVENT")>: return GetHandlerWithEvent<CWeaponDamageEventReply>(instance, client, netGameEvent);
			case net::force_consteval<uint32_t, HashRageString("RESPAWN_PLAYER_PED_EVENT")>: return GetHandlerWithEvent<CRespawnPlayerPedReply>(instance, client, netGameEvent);
			default:
				break;
		};

		return {};
	}

	switch(eventNameHash)
	{
		case net::force_consteval<uint32_t, HashRageString("WEAPON_DAMAGE_EVENT")>: return GetHandlerWithEvent<CWeaponDamageEvent>(instance, client, netGameEvent, targetPlayers);
		case net::force_consteval<uint32_t, HashRageString("RESPAWN_PLAYER_PED_EVENT")>: return GetHandlerWithEvent<CRespawnPlayerPedEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("EXPLOSION_EVENT")>: return GetHandlerWithEvent<CExplosionEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("LIGHTNING_EVENT")>: return GetHandlerWithEvent<CLightningEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("START_PROJECTILE_EVENT")>: return GetHandlerWithEvent<CStartProjectileEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("NETWORK_CLEAR_PED_TASKS_EVENT")>: return GetHandlerWithEvent<CClearPedTasksEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("NETWORK_END_LOOT_EVENT")>: return GetHandlerWithEvent<CEndLootEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("NETWORK_SEND_CARRIABLE_UPDATE_CARRY_STATE_EVENT")>: return GetHandlerWithEvent<CSendCarriableUpdateCarryStateEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("NETWORK_CARRIABLE_VEHICLE_STOW_START_EVENT")>: return GetHandlerWithEvent<CCarriableVehicleStowStartEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("NETWORK_CARRIABLE_VEHICLE_STOW_COMPLETE_EVENT")>: return GetHandlerWithEvent<CCarriableVehicleStowCompleteEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("NETWORK_PICKUP_CARRIABLE_EVENT")>: return GetHandlerWithEvent<CPickupCarriableEvent>(instance, client, netGameEvent);
		case net::force_consteval<uint32_t, HashRageString("NETWORK_PLACE_CARRIABLE_ONTO_PARENT_EVENT")>: return GetHandlerWithEvent<CPlaceCarriableOntoParentEvent>(instance, client, netGameEvent);
		default:
			break;
	};
#endif

	return {};
}

bool fx::ServerGameState::IsClientRelevantEntity(const fx::ClientSharedPtr& client, uint32_t objectId)
{
	if (auto val = GetClientDataUnlocked(this, client))
	{
		if (fx::sync::SyncEntityPtr syncEntity = val->playerEntity.lock())
		{
			std::shared_lock lock(syncEntity->guidMutex);
			return syncEntity->relevantTo.test(objectId);
		}
	}

	return false;
}

static InitFunction initFunction([]()
{
	g_scriptHandlePool = new CPool<fx::ScriptGuid>(1500, "fx::ScriptGuid");
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		if (!IsStateGame())
		{
			return;
		}

		g_networkedSoundsEnabledVar = instance->AddVariable<bool>("sv_enableNetworkedSounds", ConVar_None, true, &g_networkedSoundsEnabled);

		g_networkedPhoneExplosionsEnabledVar = instance->AddVariable<bool>("sv_enableNetworkedPhoneExplosions", ConVar_None, false, &g_networkedPhoneExplosionsEnabled);

		g_protectServerEntitiesDeletionVar = instance->AddVariable<bool>("sv_protectServerEntities", ConVar_Replicated, false, &g_protectServerEntitiesDeletion);

		g_networkedScriptEntityStatesEnabledVar = instance->AddVariable<bool>("sv_enableNetworkedScriptEntityStates", ConVar_None, true, &g_networkedScriptEntityStatesEnabled);

		g_requestControlVar = instance->AddVariable<int>("sv_filterRequestControl", ConVar_None, (int)RequestControlFilterMode::NoFilter, (int*)&g_requestControlFilterState);
		g_requestControlSettleVar = instance->AddVariable<int>("sv_filterRequestControlSettleTimer", ConVar_None, 30000, &g_requestControlSettleDelay);

		fx::SetOneSyncGetCallback([]()
		{
			return g_oneSyncEnabledVar->GetValue() || g_oneSyncVar->GetValue() != fx::OneSyncState::Off;
		});

		g_oneSyncVar = instance->AddVariable<fx::OneSyncState>("onesync", ConVar_ReadOnly, fx::OneSyncState::Off);
		g_oneSyncPopulation = instance->AddVariable<bool>("onesync_population", ConVar_ReadOnly, true);
		g_oneSyncARQ = instance->AddVariable<bool>("onesync_automaticResend", ConVar_None, false);

		// .. to infinity?
		g_oneSyncBigMode = instance->AddVariable<bool>("onesync_enableInfinity", ConVar_ReadOnly, false);

		// or maybe, beyond?
		g_oneSyncLengthHack = instance->AddVariable<bool>("onesync_enableBeyond", ConVar_ReadOnly, false);

		g_experimentalOneSyncPopulation = instance->AddVariable<bool>("sv_experimentalOneSyncPopulation", ConVar_None, true);
		g_experimentalNetGameEventHandler = instance->AddVariable<bool>("sv_experimentalNetGameEventHandler", ConVar_None, true);

		constexpr bool canLengthHack =
#ifdef STATE_RDR3
		false
#else
		true
#endif
		;

		fx::SetBigModeHack(g_oneSyncBigMode->GetValue(), canLengthHack && g_oneSyncLengthHack->GetValue());
		if (g_experimentalOneSyncPopulation->GetValue() || g_experimentalNetGameEventHandler->GetValue())
		{
			fx::SetOneSyncPopulation(g_oneSyncPopulation->GetValue());
		}

		if (g_oneSyncVar->GetValue() == fx::OneSyncState::On)
		{
			if (g_experimentalOneSyncPopulation->GetValue() || g_experimentalNetGameEventHandler->GetValue())
			{
				fx::SetBigModeHack(true, canLengthHack);
			}
			else
			{
				fx::SetBigModeHack(true, canLengthHack && g_oneSyncPopulation->GetValue());
			}

			g_oneSyncBigMode->GetHelper()->SetRawValue(true);
			g_oneSyncLengthHack->GetHelper()->SetRawValue(fx::IsLengthHack());
		}

		instance->OnInitialConfiguration.Connect([]()
		{
			if (g_oneSyncEnabledVar->GetValue() && g_oneSyncVar->GetValue() == fx::OneSyncState::Off)
			{
				g_oneSyncVar->GetHelper()->SetRawValue(fx::IsBigMode() ? fx::OneSyncState::On : fx::OneSyncState::Legacy);

				console::PrintWarning("server", "`onesync_enabled` is deprecated. Please use `onesync %s` instead.\n", 
					fx::IsBigMode() ? "on" : "legacy");
			}
			else if (!g_oneSyncEnabledVar->GetValue() && g_oneSyncVar->GetValue() != fx::OneSyncState::Off)
			{
				g_oneSyncEnabledVar->GetHelper()->SetRawValue(true);
			}
		});
	}, INT32_MIN);

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		if (!IsStateGame())
		{
			return;
		}

		g_oneSyncEnabledVar = instance->AddVariable<bool>("onesync_enabled", ConVar_ServerInfo, false);
		g_oneSyncCulling = instance->AddVariable<bool>("onesync_distanceCulling", ConVar_None, true);
		g_oneSyncVehicleCulling = instance->AddVariable<bool>("onesync_distanceCullVehicles", ConVar_None, false);
		g_oneSyncForceMigration = instance->AddVariable<bool>("onesync_forceMigration", ConVar_None, true);
		g_oneSyncRadiusFrequency = instance->AddVariable<bool>("onesync_radiusFrequency", ConVar_None, true);
		g_oneSyncLogVar = instance->AddVariable<std::string>("onesync_logFile", ConVar_None, "");
		g_oneSyncWorkaround763185 = instance->AddVariable<bool>("onesync_workaround763185", ConVar_None, false);

		fwRefContainer<fx::ServerGameState> sgs = new fx::ServerGameState();
		instance->SetComponent(sgs);
		instance->SetComponent<fx::ServerGameStatePublic>(sgs);

		instance->GetComponent<fx::GameServer>()->OnSyncTick.Connect([=]()
		{
			if (!fx::IsOneSync())
			{
				return;
			}

			instance->GetComponent<fx::ServerGameState>()->Tick(instance);
		});

		auto gameServer = instance->GetComponent<fx::GameServer>();

		gameServer->GetComponent<fx::HandlerMapComponent>()->Add(HashRageString("msgNetGameEvent"), { fx::ThreadIdx::Sync, [=](const fx::ClientSharedPtr& client, net::ByteReader& reader, fx::ENetPacketPtr packet)
		{
			if (g_experimentalNetGameEventHandler->GetValue())
			{
				return;
			}
			// this should match up with SendGameEventRaw on client builds
			// 1024 bytes is from the rlBuffer
			// 512 is from the max amount of players (2 * 256)
			// 2 is event id
			// 2 is from event type
			// 2 is from length of rlBuffer
			// 1 from target size
			// 1 from "is reply" field
			constexpr int kMaxPacketSize = 1024 + 512 + 2 + 2 + 2 + 1 + 1;

			net::Buffer buffer(reader.GetData(), reader.GetCapacity());
			buffer.Seek(reader.GetOffset());

			if (buffer.GetLength() > kMaxPacketSize)
			{
				return;
			}

			auto sgs = instance->GetComponent<fx::ServerGameState>();
			auto targetPlayerCount = buffer.Read<uint8_t>();
			std::vector<uint16_t> targetPlayers(targetPlayerCount);

			if (!buffer.Read(targetPlayers.data(), targetPlayers.size() * sizeof(uint16_t)))
			{
				return;
			}

			// de-duplicate targetPlayers, preventing the sending of a large number of events to a single client
			std::sort(targetPlayers.begin(), targetPlayers.end());
			targetPlayers.erase(std::unique(targetPlayers.begin(), targetPlayers.end()), targetPlayers.end());

			net::Buffer netBuffer;
			netBuffer.Write<uint32_t>(HashRageString("msgNetGameEvent"));
			netBuffer.Write<uint16_t>(client->GetNetId());
			buffer.ReadTo(netBuffer, buffer.GetRemainingBytes());

			auto sourceClientData = GetClientDataUnlocked(sgs.GetRef(), client);
			auto bucket = sourceClientData->routingBucket;

			auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

			auto routeEvent = [sgs, bucket, netBuffer, targetPlayers, clientRegistry]()
			{
				for (uint16_t player : targetPlayers)
				{
					auto targetClient = clientRegistry->GetClientByNetID(player);

					if (targetClient)
					{
						auto targetClientData = GetClientDataUnlocked(sgs.GetRef(), targetClient);

						if (targetClientData->routingBucket == bucket)
						{
							targetClient->SendPacket(1, netBuffer, NetPacketType_Reliable);
						}
					}
				}
			};

			auto copyBuf = netBuffer.Clone();
			copyBuf.Seek(6);

			auto eventHandler = sgs->GetGameEventHandler(client, targetPlayers, std::move(copyBuf));

			if (eventHandler)
			{
				gscomms_execute_callback_on_main_thread([eventHandler, routeEvent]()
				{
					if (eventHandler())
					{
						routeEvent();
					}
				});
			}
			else
			{
				routeEvent();
			}
		} });

		auto consoleCtx = instance->GetComponent<console::Context>();

		// start sessionmanager
		if (gameServer->GetGameName() == fx::GameName::RDR3)
		{
			consoleCtx->ExecuteSingleCommandDirect(ProgramArguments{ "start", "sessionmanager-rdr3" });
		}
		else if (!g_oneSyncEnabledVar->GetValue() && g_oneSyncVar->GetValue() == fx::OneSyncState::Off)
		{
			consoleCtx->ExecuteSingleCommandDirect(ProgramArguments{ "start", "sessionmanager" });
		}
	}, 999999);
});
