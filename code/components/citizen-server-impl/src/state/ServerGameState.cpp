#include <StdInc.h>
#include <GameServer.h>

#include <state/ServerGameState.h>

#include <optional>

#include <NetBuffer.h>

#include <lz4.h>

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

#include <OneSyncVars.h>
#include <DebugAlias.h>

#include <StateBagComponent.h>

#include <citizen_util/object_pool.h>
#include <citizen_util/shared_reference.h>

static bool g_bigMode;
static bool g_lengthHack;

namespace fx
{
	bool IsBigMode()
	{
		return g_bigMode;
	}

	bool IsLengthHack()
	{
		return g_lengthHack;
	}
}

namespace rl
{
	bool MessageBuffer::GetLengthHackState()
	{
		return g_lengthHack;
	}
}

CPool<fx::ScriptGuid>* g_scriptHandlePool;
std::shared_mutex g_scriptHandlePoolMutex;

std::shared_ptr<ConVar<bool>> g_oneSyncEnabledVar;
std::shared_ptr<ConVar<bool>> g_oneSyncCulling;
std::shared_ptr<ConVar<bool>> g_oneSyncVehicleCulling;
std::shared_ptr<ConVar<bool>> g_oneSyncForceMigration;
std::shared_ptr<ConVar<bool>> g_oneSyncRadiusFrequency;
std::shared_ptr<ConVar<std::string>> g_oneSyncLogVar;
std::shared_ptr<ConVar<bool>> g_oneSyncWorkaround763185;
std::shared_ptr<ConVar<bool>> g_oneSyncBigMode;
std::shared_ptr<ConVar<bool>> g_oneSyncLengthHack;
std::shared_ptr<ConVar<fx::OneSyncState>> g_oneSyncVar;
std::shared_ptr<ConVar<bool>> g_oneSyncPopulation;

namespace fx
{
bool IsOneSync()
{
	return g_oneSyncEnabledVar->GetValue() || g_oneSyncVar->GetValue() != fx::OneSyncState::Off;
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
	: deleting(false)
{

}

inline std::shared_ptr<GameStateClientData> GetClientDataUnlocked(ServerGameState* state, const fx::ClientSharedPtr& client)
{
	// NOTE: static_pointer_cast typically will lead to an unneeded refcount increment+decrement
	// Doing this makes it so that there's only *one* increment for the fast case.
#ifndef _MSC_VER
	auto data = std::static_pointer_cast<GameStateClientData>(client->GetSyncData());
#else
	auto data = std::shared_ptr<GameStateClientData>{ reinterpret_cast<std::shared_ptr<GameStateClientData>&&>(client->GetSyncData()) };
#endif

	if (!data)
	{
		fx::ClientWeakPtr weakClient(client);

		data = std::make_shared<GameStateClientData>();
		data->client = weakClient;
		data->playerBag = state->GetStateBags()->RegisterStateBag(fmt::sprintf("player:%d", client->GetNetId()));

		if (fx::IsBigMode())
		{
			data->playerBag->AddRoutingTarget(client->GetSlotId());
		}

		data->playerBag->SetOwningPeer(client->GetSlotId());

		client->SetSyncData(data);
		client->OnDrop.Connect([weakClient, state]()
		{
			state->HandleClientDrop(weakClient.lock());
		});
	}

	return data;
}

inline std::tuple<std::unique_lock<std::mutex>, std::shared_ptr<GameStateClientData>> GetClientData(ServerGameState* state, const fx::ClientSharedPtr& client)
{
	auto val = GetClientDataUnlocked(state, client);

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
				if (std::any_cast<uint32_t>(client->GetData("playerEntity")) == guid)
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

glm::vec3 GetPlayerFocusPos(const fx::sync::SyncEntityPtr& entity)
{
	auto syncTree = entity->syncTree;

	if (!syncTree)
	{
		return { 0, 0, 0 };
	}

	float playerPos[3];
	syncTree->GetPosition(playerPos);

	auto camData = syncTree->GetPlayerCamera();

	if (!camData)
	{
		return { playerPos[0], playerPos[1], playerPos[2] };
	}

	switch (camData->camMode)
	{
	case 0:
	default:
		return { playerPos[0], playerPos[1], playerPos[2] };
	case 1:
		return { camData->freeCamPosX, camData->freeCamPosY, camData->freeCamPosZ };
	case 2:
		return { playerPos[0] + camData->camOffX, playerPos[1] + camData->camOffY, playerPos[2] + camData->camOffZ };
	}
}

ServerGameState::ServerGameState()
	: m_frameIndex(0), m_entitiesById(MaxObjectId), m_entityLockdownMode(EntityLockdownMode::Inactive)
{
	m_tg = std::make_unique<ThreadPool>();
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

static void FlushBuffer(rl::MessageBuffer& buffer, uint32_t msgType, uint64_t frameIndex, const fx::ClientSharedPtr& client, bool finalFlush = false)
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

		*(uint32_t*)(outData.data()) = msgType;
		*(uint64_t*)(outData.data() + 4) = frameIndex | ((finalFlush) ? (uint64_t(1) << 63) : 0);

		net::Buffer netBuffer(reinterpret_cast<uint8_t*>(outData.data()), len + 4 + 8);
		netBuffer.Seek(len + 4 + 8); // since the buffer constructor doesn't actually set the offset

		GS_LOG("flushBuffer: sending %d bytes to %d\n", len + 4 + 8, client->GetNetId());

		client->SendPacket(1, netBuffer);

		size_t oldCurrentBit = buffer.GetCurrentBit();

		debug::Alias(&oldCurrentBit);
		debug::Alias(&len);

		buffer.SetCurrentBit(0);
	}
}

static void MaybeFlushBuffer(rl::MessageBuffer& buffer, uint32_t msgType, uint64_t frameIndex, const fx::ClientSharedPtr& client, size_t size = 0)
{
	if (LZ4_compressBound(buffer.GetDataLength() + (size / 8)) > (1100 - 12 - 12))
	{
		FlushBuffer(buffer, msgType, frameIndex, client);
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

	scs.flushBuffer = [this, &scsSelf, &client](bool finalFlush)
	{
		FlushBuffer(scsSelf.cloneBuffer, HashRageString("msgPackedClones"), frameIndex, client, true);
	};

	scs.maybeFlushBuffer = [this, &scsSelf, &client](size_t plannedBits)
	{
		MaybeFlushBuffer(scsSelf.cloneBuffer, HashRageString("msgPackedClones"), frameIndex, client, plannedBits);
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

	// approximate amount of ticks per second, 120 is svSync from GameServer.cpp
	int effectiveTicksPerSecond = (120 / (3 * tickMul));

	{
		std::shared_lock<std::shared_mutex> lock(m_entityListMutex);
		for (auto& entity : m_entityList)
		{
			entity->frameIndex = m_frameIndex;
		}
	}

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
	> relevantEntities[MaxObjectId];

	//map of handles to entity indices
	static uint16_t entityHandleMap[MaxObjectId];

	int maxValidEntity = 0;

	{
		std::shared_lock<std::shared_mutex> lock(m_entityListMutex);

		for (auto& entity : m_entityList)
		{
			if (!entity->syncTree)
			{
				continue;
			}

			float position[3];
			entity->syncTree->GetPosition(position);

			glm::vec3 entityPosition(position[0], position[1], position[2]);

			sync::CVehicleGameStateNodeData* vehicleData = nullptr;

			if (entity->type == sync::NetObjEntityType::Automobile ||
				entity->type == sync::NetObjEntityType::Bike ||
				entity->type == sync::NetObjEntityType::Boat ||
				entity->type == sync::NetObjEntityType::Heli ||
				entity->type == sync::NetObjEntityType::Plane ||
				entity->type == sync::NetObjEntityType::Submarine ||
				entity->type == sync::NetObjEntityType::Trailer ||
				entity->type == sync::NetObjEntityType::Train)
			{
				vehicleData = entity->syncTree->GetVehicleGameState();
			}
			
			relevantEntities[maxValidEntity] = { entity, entityPosition, vehicleData, entity->GetClient() };
			entityHandleMap[entity->handle] = maxValidEntity;
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

		// this assumes resize won't *free* what already exists
		clientDataUnlocked->relevantEntities.resize(0);

		glm::vec3 playerPos;

		if (playerEntity)
		{
			playerPos = GetPlayerFocusPos(playerEntity);
		}

		auto slotId = client->GetSlotId();

		for (int entityIndex = 0; entityIndex < maxValidEntity; entityIndex++)
		{
			const auto& [entity, entityPos, vehicleData, entityClientWeak] = relevantEntities[entityIndex];

			bool hasCreated = false;

			{
				auto clientData = GetClientDataUnlocked(this, client);
				hasCreated = clientData->createdEntities.test(entity->handle);
			}

			bool shouldBeCreated = (g_oneSyncCulling->GetValue()) ? false : true;

			// players should always have their own player entities
			auto entityClient = entityClientWeak.lock();
			if (entityClient && client->GetNetId() == entityClient->GetNetId() && entity->type == sync::NetObjEntityType::Player)
			{
				shouldBeCreated = true;
			}
			
			// players should have their own entities, if nobody else cares about them
			if (entityClient && client->GetNetId() == entityClient->GetNetId())
			{
				auto clientCount = entity->relevantTo.count();

				// if there's *no* relevantTo, or if it's just the owning player
				if (clientCount < 1 || (entity->relevantTo.test(client->GetSlotId()) && clientCount < 2))
				{
					// server script-owned entities get special disown behavior
					if (!entity->IsOwnedByScript())
					{
						shouldBeCreated = true;
					}
				}
			}

			if (!shouldBeCreated)
			{
				if (playerEntity)
				{
					float diffX = entityPos.x - playerPos.x;
					float diffY = entityPos.y - playerPos.y;

					float distSquared = (diffX * diffX) + (diffY * diffY);
					if (distSquared < entity->GetDistanceCullingRadius())
					{
						shouldBeCreated = true;
					}
				}
				else
				{
					// can't really say otherwise if the player entity doesn't exist
					shouldBeCreated = !fx::IsBigMode();
				}
			}

			// #TODO1S: improve logic for what should and shouldn't exist based on game code
			bool isPlayerOrVehicle = false;

			if (!shouldBeCreated)
			{
				if (entity->type == sync::NetObjEntityType::Player)
				{
					isPlayerOrVehicle = true;

					if (!fx::IsBigMode())
					{
						shouldBeCreated = true;
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
					entity->type == sync::NetObjEntityType::Train)
				{
					if (vehicleData)
					{
						if (vehicleData->playerOccupants.any())
						{
							if (!g_oneSyncVehicleCulling->GetValue() && !fx::IsBigMode())
							{
								shouldBeCreated = true;
							}

							isPlayerOrVehicle = true;
						}
					}
				}
			}

			// don't route entities that haven't passed filter to others
			if (!entity->passedFilter)
			{
				if (!entityClient || entityClient->GetNetId() != client->GetNetId())
				{
					shouldBeCreated = false;
				}
			}

			auto syncDelay = 50ms;

			// only update sync delay if should-be-created
			// shouldBeCreated should **not** be updated after this
			if (shouldBeCreated && g_oneSyncRadiusFrequency->GetValue() && !isPlayerOrVehicle)
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
						auto dist = glm::distance2(position, playerPos);

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
					auto dist = glm::distance2(entityPos, playerPos);

					if (dist < 35.0f * 35.0f)
					{
						syncDelay /= 4;
					}
				}
			}

			bool isRelevant = false;

			// entities that should exist are relevant
			if (shouldBeCreated)
			{
				isRelevant = true;
			}

			// entities that should be removed are relevant
			if (!shouldBeCreated && hasCreated)
			{
				isRelevant = true;
			}

			// players that should be removed are relevant
			if (!shouldBeCreated && entity->type == sync::NetObjEntityType::Player && entityClient)
			{
				auto [clientDataLock, clientData] = GetClientData(this, client);

				auto plit = clientData->playersToSlots.find(entityClient->GetNetId());
				bool hasCreatedPlayer = (plit != clientData->playersToSlots.end());

				if (hasCreatedPlayer)
				{
					isRelevant = true;
				}
			}

			if (isRelevant)
			{
				clientDataUnlocked->relevantEntities.push_back({ entity->handle, syncDelay, shouldBeCreated });
			}
		}
	}

	lastUpdateSlot = slot;

	auto curTime = msec();

	// gather client refs
	eastl::fixed_vector<fx::ClientSharedPtr, MAX_CLIENTS> clientRefs;

	creg->ForAllClients([&clientRefs](const fx::ClientSharedPtr& clientRef)
	{
		clientRefs.push_back(clientRef);
	});

	tbb::parallel_for_each(clientRefs.begin(), clientRefs.end(), [this, curTime, effectiveTicksPerSecond, &sec, &evMan, &gs](const fx::ClientSharedPtr& clientRef)
	{
		// get our own pointer ownership
		auto client = clientRef;

		if (client->GetSlotId() == -1)
		{
			return;
		}

		uint64_t time = curTime.count();

		NetPeerStackBuffer stackBuffer;
		gscomms_get_peer(client->GetPeer(), stackBuffer);
		auto enPeer = stackBuffer.GetBase();

		auto resendDelay = 0ms;

		if (enPeer && enPeer->GetPing() != -1)
		{
			resendDelay = std::chrono::milliseconds(std::max(int(1), int(enPeer->GetPing() * 2) + int(enPeer->GetPingVariance())));
		}
		else
		{
			// no peer, no connection, no service
			return;
		}

		bool shouldSkip = false;

		{
			auto [lock, data] = GetClientData(this, client);

			if (!data->playerId)
			{
				return;
			}

			auto& ackPacket = data->ackBuffer;

			// any ACKs to send?
			data->FlushAcks();

			if (data->syncing)
			{
				shouldSkip = true;
			}
			else
			{
				data->syncing = true;
			}
		}

		if (shouldSkip)
		{
			return;
		}

		static fx::object_pool<sync::SyncCommandList, 512 * 1024> syncPool;
		auto scl = shared_reference<sync::SyncCommandList, &syncPool>::Construct(m_frameIndex);

		int numCreates = 0, numSyncs = 0, numSkips = 0;

		auto clientDataUnlocked = GetClientDataUnlocked(this, client);

		sync::SyncEntityPtr playerEntity;
		{
			std::shared_lock _lock(clientDataUnlocked->playerEntityMutex);
			playerEntity = clientDataUnlocked->playerEntity.lock();
		}

		glm::vec3 playerPos;

		if (playerEntity)
		{
			playerPos = GetPlayerFocusPos(playerEntity);
		}

		auto slotId = client->GetSlotId();

		// collect the intended entity state
		static thread_local EntityStateObject blankEntityState;

		auto newEntityStateRef = GameStateClientData::MakeEntityState();
		auto& newEntityState = *newEntityStateRef;

		for (const auto& entityIdTuple : clientDataUnlocked->relevantEntities)
		{
			auto [entityId, syncDelay, shouldBeCreated] = entityIdTuple;
			if (!shouldBeCreated)
			{
				continue;
			}

			const auto& [entity, entityPos, vehicleData, entityClientWeak] = relevantEntities[entityHandleMap[entityId]];

			if (!entity || entity->deleting)
			{
				continue;
			}

			auto entityClient = entityClientWeak.lock();
			if (!entityClient && entity->type != sync::NetObjEntityType::Player)
			{
				std::unique_lock _entityLock(entity->clientMutex);

				// assign it to the client, it's relevant!
				if (!entity->GetClientUnsafe())
				{
					ReassignEntity(entity->handle, client);
				}
			}

			ClientEntityState ces;
			ces.frameIndex = entity->frameIndex;
			ces.lastSend = 0ms;
			ces.uniqifier = entity->uniqifier;
			ces.syncDelay = syncDelay;
			ces.isPlayer = (entity->type == sync::NetObjEntityType::Player);
			ces.netId = (entityClient) ? entityClient->GetNetId() : -1;
			ces.overrideFrameIndex = false;

			newEntityState[entity->handle] = ces;
		}

		// get a delta path for the op list
		eastl::fixed_vector<std::tuple<uint64_t, const EntityStateObject*>, 5> deltaPath;

		{
			auto [lock, data] = GetClientData(this, client);

			{
				auto deltaAcks = m_frameIndex - data->lastAckIndex;

				// is the user 60 seconds behind?
				if (data->lastAckIndex > 0 && deltaAcks > (effectiveTicksPerSecond * 60))
				{
					// so we don't run more
					data->lastAckIndex = 0;

					gscomms_execute_callback_on_main_thread([client, gs, deltaAcks]()
					{
						gs->DropClient(client, "Didn't acknowledge %d sync packets.", deltaAcks);
					});

					return;
				}
			}

			for (auto lastIdx = data->lastAckIndex; lastIdx < m_frameIndex; lastIdx++)
			{
				auto lastIt = data->entityStates.find(lastIdx);

				if (lastIt != data->entityStates.end())
				{
					deltaPath.push_back({ lastIdx, lastIt->second.get() });
				}
			}
		}

		if (!deltaPath.size())
		{
			deltaPath.push_back({ 0, &blankEntityState });
		}

		// gather op list from delta path
		eastl::fixed_map<uint16_t, std::tuple<fx::ClientEntityState, const fx::ClientEntityState*>, 128> deletedKeys;
		eastl::fixed_map<uint16_t, std::tuple<fx::ClientEntityState, const fx::ClientEntityState*>, 128> liveKeys;

		for (size_t deltaIdx = 0; deltaIdx < deltaPath.size(); deltaIdx++)
		{
			auto lastEntityState = std::get<1>(deltaPath[deltaIdx]);
			auto nextEntityState = ((deltaIdx + 1) < deltaPath.size()) ? std::get<1>(deltaPath[deltaIdx + 1]) : &newEntityState;

			// removals (and uniqifier changes) go first
			eastl::fixed_vector<eastl::pair<uint16_t, fx::ClientEntityState>, 128> deletedKeysLocal;

			std::set_difference(lastEntityState->begin(), lastEntityState->end(), nextEntityState->begin(), nextEntityState->end(), std::back_inserter(deletedKeysLocal), [](const auto& left, const auto& right)
			{
				auto leftTup = std::make_tuple(left.first, left.second.uniqifier);
				auto rightTup = std::make_tuple(right.first, right.second.uniqifier);

				return leftTup < rightTup;
			});

			for (auto& pair : deletedKeysLocal)
			{
				auto oldEntity = lastEntityState->find(pair.first);

				if (oldEntity != lastEntityState->end())
				{
					deletedKeys[pair.first] = { pair.second, &oldEntity->second };
				}
			}

			for (auto& [objectId, objectState] : *nextEntityState)
			{
				auto& oneES = std::get<1>(deltaPath[0]);
				auto oldEntity = oneES->find(objectId);

				liveKeys[objectId] = { objectState, (oldEntity != oneES->end()) ? &oldEntity->second : nullptr };
			}
		}

		// process live removals
		const auto& clientRegistry = m_instance->GetComponent<fx::ClientRegistry>();

		for (const auto& deletionPair : deletedKeys)
		{
			uint16_t deletion = deletionPair.first;
			uint16_t uniqifier = std::get<0>(deletionPair.second).uniqifier;

			// delete player
			if (fx::IsBigMode())
			{
				auto oldEntity = std::get<1>(deletionPair.second);

				if (oldEntity->isPlayer)
				{
					if (oldEntity->netId != -1)
					{
						auto [clientDataLock, clientData] = GetClientData(this, client);

						auto plit = clientData->playersToSlots.find(oldEntity->netId);
						bool hasCreatedPlayer = (plit != clientData->playersToSlots.end());

						{
							if (hasCreatedPlayer)
							{
								auto entityClient = clientRegistry->GetClientByNetID(oldEntity->netId);
								int slotId = plit->second;

								sec->TriggerClientEventReplayed("onPlayerDropped", fmt::sprintf("%d", client->GetNetId()), oldEntity->netId, entityClient ? entityClient->GetName() : "", slotId);

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
								evMan->QueueEvent2("playerLeftScope", {}, std::map<std::string, std::string>{ { "player", fmt::sprintf("%d", oldEntity->netId) }, { "for", fmt::sprintf("%d", client->GetNetId()) } });

								if (entityClient)
								{
									auto oldClientData = GetClientDataUnlocked(this, entityClient);
									oldClientData->playerBag->RemoveRoutingTarget(client->GetSlotId());
								}

								clientData->slotsToPlayers.erase(slotId);
								clientData->playersToSlots.erase(oldEntity->netId);
							}
						}
					}
				}
			}

			
			// if the entity still exists, and this is the *owner* we're deleting it for, we should tell them their object ID is stolen
			// as ReassignEntity will mark the object ID as part of the steal pool
			bool shouldSteal = false;
			auto entity = GetEntity(0, deletion);

			if (entity && entity->uniqifier == uniqifier)
			{
				auto entityClient = entity->GetClient();
				if (entityClient && entityClient->GetNetId() == client->GetNetId())
				{
					std::unique_lock<std::mutex> lock(m_objectIdsMutex);

					if (m_objectIdsUsed.test(deletion))
					{
						shouldSteal = true;

						// mark the object as stolen already, in case we're not stealing it later
						m_objectIdsStolen.set(deletion);
					}
				}

				auto [_, clientData] = GetClientData(this, client);
				clientData->relevantEntities.erase(std::remove_if(clientData->relevantEntities.begin(), clientData->relevantEntities.end(), [deletion](const auto& tup)
				{
					return deletion == std::get<0>(tup);
				}),
				clientData->relevantEntities.end());
			}

			GS_LOG("Tick: deleting object %d@%d for %d\n", deletion, uniqifier, client->GetNetId());

			// delete object
			scl->EnqueueCommand([this, deletion, uniqifier, shouldSteal](sync::SyncCommandState& cmdState)
			{
				cmdState.maybeFlushBuffer(17 + 16);
				cmdState.cloneBuffer.Write(3, 3);

				cmdState.cloneBuffer.WriteBit(shouldSteal ? true : false);
				cmdState.cloneBuffer.Write(13, int32_t(deletion));
				cmdState.cloneBuffer.Write(16, uniqifier);
			});
		}

		eastl::fixed_set<uint16_t, 64> nah;

		// after deletions, handle updates
		for (auto& [objectId, objectStatePair] : liveKeys)
		{
			auto& [objectState, lastState] = objectStatePair;

			const auto& [entity, entityPos, vehicleData, entityClientWeak] = relevantEntities[entityHandleMap[objectId]];

			if (!entity)
			{
				continue;
			}

			auto entityClient = entityClientWeak.lock();

			bool hasCreated = false;

			if (lastState)
			{
				hasCreated = true;
			}
			else
			{
				auto data = GetClientDataUnlocked(this, client);
				hasCreated = data->createdEntities.test(objectId);
			}

			uint64_t lastFrameIndex = clientDataUnlocked->lastAckIndex;

			if (lastState && lastState->overrideFrameIndex)
			{
				lastFrameIndex = lastState->frameIndex;
			}

			// pending migration?
			{
				fx::ClientSharedPtr cl = entity->GetClient();
				fx::ClientSharedPtr lu = entity->GetLastUpdater();

				if (lu && cl && lu->GetNetId() != cl->GetNetId())
				{
					lastFrameIndex = 0;
				}
			}

			// is this a different entity in reality?
			if (lastState && lastState->uniqifier != entity->uniqifier)
			{
				hasCreated = false;
				lastFrameIndex = 0;
				lastState = nullptr;
			}

			bool shouldBeCreated = true;
			if (fx::IsBigMode())
			{
				if (entity->type == sync::NetObjEntityType::Player)
				{
					if (entityClient)
					{
						auto [clientDataLock, clientData] = GetClientData(this, client);

						auto plit = clientData->playersToSlots.find(entityClient->GetNetId());
						bool hasCreatedPlayer = (plit != clientData->playersToSlots.end());

						if (!hasCreatedPlayer)
						{
							int slotId = 127;

							for (; slotId >= 0; slotId--)
							{
								if (slotId == 31)
								{
									slotId--;
								}

								// #TODO1SBIG: bitset?
								if (clientData->slotsToPlayers.find(slotId) == clientData->slotsToPlayers.end())
								{
									break;
								}
							}

							if (slotId >= 0)
							{
								clientData->slotsToPlayers[slotId] = entityClient->GetNetId();
								clientData->playersToSlots[entityClient->GetNetId()] = slotId;

								sec->TriggerClientEventReplayed("onPlayerJoining", fmt::sprintf("%d", client->GetNetId()), entityClient->GetNetId(), entityClient->GetName(), slotId);
								
								auto ecData = GetClientDataUnlocked(this, entityClient);
								ecData->playerBag->AddRoutingTarget(client->GetSlotId());

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
								shouldBeCreated = false;
							}
						}
					}
				}
			}

			// if this is a ped and it is occupying a vehicle we want the vehicle acked first
			// (players don't matter as they won't migrate so they will not get some broken state from this)
#if 0
			if (entity->type == sync::NetObjEntityType::Ped)
			{
				sync::CPedGameStateNodeData* state = nullptr;

				if (entity->syncTree && (state = entity->syncTree->GetPedGameState()))
				{
					if (state->curVehicle >= 0 && GetEntity(0, state->curVehicle))
					{
						if (!lastEntityState || lastEntityState->find(state->curVehicle) == lastEntityState->end())
						{
							GS_LOG("ped [obj:%d] not getting created for client [cl:%d] as vehicle [obj:%d] does not exist.\n", entity->handle, client->GetNetId(), state->curVehicle);

							shouldBeCreated = false;
						}
					}
				}
			}
#endif

			if (!shouldBeCreated)
			{
				nah.insert(objectId);
				continue;
			}

			if (client->GetSlotId() == -1)
			{
				continue;
			}

			{
				std::lock_guard<std::shared_mutex> _lock(entity->guidMutex);
				entity->relevantTo.set(client->GetSlotId());
			}

			// default to it being a sync
			int syncType = 2;

			if (!hasCreated)
			{
				GS_LOG("Tick: creating object %d for %d\n", entity->handle, client->GetNetId());

				// make it a create
				syncType = 1;
			}
			else
			{
				// we know the entity has been created, so we can try sending some entity RPC to 'em
				if (client == entityClient && !entity->onCreationRPC.empty())
				{
					std::lock_guard<std::shared_mutex> _(entity->guidMutex);

					for (auto& entry : entity->onCreationRPC)
					{
						entry(client);
					}

					entity->onCreationRPC.clear();
				}
			}

			bool shouldSend = true;

			// TODO: proper hazards
			if (slotId == -1)
			{
				break;
			}

			resendDelay = std::min(objectState.syncDelay, resendDelay);

			auto lastResend = (lastState) ? lastState->lastSend : 0ms;
			auto resendDelta = (curTime - lastResend);

			if (lastResend != 0ms && resendDelta < resendDelay)
			{
				GS_LOG("%s: skipping resend for object %d (resend delay %dms, last resend %d)\n", __func__, entity->handle, resendDelay.count(), resendDelta.count());
				shouldSend = false;
			}

			if (shouldSend)
			{
				auto syncDelay = objectState.syncDelay;
				objectState.lastSend = curTime;

				((syncType == 1) ? numCreates : numSyncs)++;

				auto entClWeak = entityClientWeak; // structured bindings can't be captured, c++ is a good language i promise
				auto ent = std::move(entity);
				auto runSync = [this, ent, entClWeak, resendDelay, syncDelay, &syncType, lastFrameIndex, curTime, &scl](auto preCb)
				{
					scl->EnqueueCommand([
							this,
							entity = ent,
							entClWeak,
							resendDelay,
							syncDelay,
							syncType,
							origLastFrameIndex = lastFrameIndex,
							preCb,
							curTime
					](sync::SyncCommandState& cmdState)
					{
						if (!entity)
						{
							return;
						}

						auto entityClient = entClWeak.lock();
						if (!entityClient)
						{
							entityClient = entity->GetClient();

							if (!entityClient)
							{
								return;
							}
						}

						auto slotId = cmdState.client->GetSlotId();

						if (slotId == -1)
						{
							return;
						}

						auto lastFrameIndex = origLastFrameIndex;
						bool isFirstFrameUpdate = false;
						preCb(lastFrameIndex, isFirstFrameUpdate);

						// create a buffer once (per thread) to save allocations
						static thread_local rl::MessageBuffer mb(1200);

						// gather entity timestamps
						eastl::fixed_set<uint32_t, 10> timeSet;
						timeSet.insert(0);

						for (auto& ts : timeSet)
						{
							mb.SetCurrentBit(0);

							sync::SyncUnparseState state(mb);
							state.syncType = syncType;
							state.targetSlotId = slotId;
							state.timestamp = ts;
							state.lastFrameIndex = lastFrameIndex;

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

								auto len = (state.buffer.GetCurrentBit() / 8) + 1;

								if (len > 4096)
								{
									return;
								}

								auto startBit = cmdState.cloneBuffer.GetCurrentBit();
								cmdState.maybeFlushBuffer(3 + 13 + 16 + 4 + 32 + 16 + 32 + 12 + (len * 8));
								cmdState.cloneBuffer.Write(3, syncType);
								cmdState.cloneBuffer.Write(13, entity->handle);
								cmdState.cloneBuffer.Write(16, entityClient->GetNetId());

								if (syncType == 1)
								{
									cmdState.cloneBuffer.Write(4, (uint8_t)entity->type);

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

								cmdState.cloneBuffer.Write<uint32_t>(32, ts == 0 ? entity->timestamp : ts);

								bool mayWrite = true;

								if (syncType == 2 && entity->GetLastUpdater() == cmdState.client && !isFirstFrameUpdate)
								{
									mayWrite = false;
								}

								if (mayWrite)
								{
									cmdState.cloneBuffer.Write(12, len);

									if (!cmdState.cloneBuffer.WriteBits(state.buffer.GetBuffer().data(), len * 8))
									{
										cmdState.cloneBuffer.SetCurrentBit(startBit);

										// force a buffer flush, we're oversize
										cmdState.flushBuffer(false);
									}
									else
									{
										//entity->lastSyncs[slotId] = curTime;
										//entity->lastResends[slotId] = curTime;
									}
								}
								else
								{
									cmdState.cloneBuffer.Write(12, 0);
								}
							}
						}
					});
				};

				runSync([](uint64_t&, bool&) {});

				if (syncType == 1 && entity->type != sync::NetObjEntityType::Player)
				{
					syncType = 2;
					runSync([](uint64_t& lfi, bool& isLfi)
					{
						lfi = 0;
						isLfi = true;
					});
				}
			}
			else
			{
				numSkips++;
			}
		}

		// remove entities from the nah list
		for (uint16_t objectId : nah)
		{
			GS_LOG("object %d is in nah list for client %d\n", objectId, client->GetNetId());
			newEntityState.erase(objectId);
		}

		if (!g_oneSyncLogVar->GetValue().empty())
		{
			std::stringstream ss;
			ss << "entStates: ";

			for (auto& es : newEntityState)
			{
				ss << es.first << "@" << es.second.uniqifier << " ";
			}

			GS_LOG("%d %s\n", client->GetNetId(), ss.str());

			ss = {};

			ss << "delta path: " << deltaPath.size() << " -> ";

			for (auto& frame : deltaPath)
			{
				ss << std::get<0>(frame) << " ";
			}

			GS_LOG("%d %s\n", client->GetNetId(), ss.str());
		}

		{
			auto [lock, data] = GetClientData(this, client);
			data->entityStates.emplace(scl->frameIndex, std::move(newEntityStateRef));
		}

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

		GS_LOG("Tick: cl %d: frIdx %d; %d cr, %d sy, %d sk\n", client->GetNetId(), m_frameIndex, numCreates, numSyncs, numSkips);
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
		net::Buffer msg;
		msg.Write<uint32_t>(HashRageString("msgWorldGrid3"));

		uint32_t base = 0;
		uint32_t length = sizeof(m_worldGrid);

		if (entry)
		{
			base = ((WorldGridEntry*)entry - &m_worldGrid[0].entries[0]) * sizeof(WorldGridEntry);
			length = sizeof(WorldGridEntry);
		}

		if (client->GetSlotId() == -1)
		{
			return;
		}

		// really bad way to snap the world grid data to a client's own base
		uint32_t baseRef = sizeof(m_worldGrid[0]) * client->GetSlotId();
		uint32_t lengthRef = sizeof(m_worldGrid[0]);

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

		msg.Write(reinterpret_cast<char*>(m_worldGrid) + base, length);

		client->SendPacket(1, msg, NetPacketType_ReliableReplayed);
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
	instance->GetComponent<fx::ClientRegistry>()->ForAllClients([&](const fx::ClientSharedPtr& client)
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

		auto pos = GetPlayerFocusPos(playerEntity);

		int minSectorX = std::max((pos.x - 299.0f) + 8192.0f, 0.0f) / 150;
		int maxSectorX = std::max((pos.x + 299.0f) + 8192.0f, 0.0f) / 150;
		int minSectorY = std::max((pos.y - 299.0f) + 8192.0f, 0.0f) / 150;
		int maxSectorY = std::max((pos.y + 299.0f) + 8192.0f, 0.0f) / 150;

		if (minSectorX < 0 || minSectorX > std::size(m_worldGridAccel.netIDs) ||
			minSectorY < 0 || minSectorY > std::size(m_worldGridAccel.netIDs[0]))
		{
			return;
		}

		auto netID = client->GetNetId();

		WorldGridState* gridState = &m_worldGrid[slotID];

		// disown any grid entries that aren't near us anymore
		for (auto& entry : gridState->entries)
		{
			if (entry.netID != 0xFFFF)
			{
				if (entry.sectorX < (minSectorX - 1) || entry.sectorX >= (maxSectorX + 1) ||
					entry.sectorY < (minSectorY - 1) || entry.sectorY >= (maxSectorY + 1))
				{
					if (m_worldGridAccel.netIDs[entry.sectorX][entry.sectorY] == netID)
					{
						m_worldGridAccel.netIDs[entry.sectorX][entry.sectorY] = -1;
					}

					entry.sectorX = 0;
					entry.sectorY = 0;
					entry.netID = -1;

					SendWorldGrid(&entry, client);
				}
			}
		}

		for (int x = minSectorX; x <= maxSectorX; x++)
		{
			for (int y = minSectorY; y <= maxSectorY; y++)
			{
				// find if this x/y is owned by someone already
				bool found = (m_worldGridAccel.netIDs[x][y] != 0xFFFF);

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

							m_worldGridAccel.netIDs[x][y] = netID;

							SendWorldGrid(&entry, client);

							break;
						}
					}
				}
			}
		}
	});
}

// make sure you have a lock to the client mutex before calling this function!
void ServerGameState::ReassignEntity(uint32_t entityHandle, const fx::ClientSharedPtr& targetClient)
{
	auto entity = GetEntity(0, entityHandle);

	if (!entity)
	{
		return;
	}

	// can we even force control at all? (occupant state check)
	std::function<bool(const fx::sync::SyncEntityPtr&)> checkCanReassign = [this, targetClient, &checkCanReassign](const fx::sync::SyncEntityPtr& entity)
	{
		bool can = true;

		// no client?
		if (!targetClient)
		{
			// yes we can
			return true;
		}

		if (entity->type == sync::NetObjEntityType::Ped)
		{
			auto vehicleData = entity->syncTree->GetPedGameState();

			if (vehicleData)
			{
				auto cv = vehicleData->curVehicle;

				if (cv >= 0)
				{
					auto vehicleEntity = GetEntity(0, cv);

					if (vehicleEntity)
					{
						can = can && checkCanReassign(vehicleEntity);
					}
				}
			}
		}
		else if (entity->type == sync::NetObjEntityType::Automobile ||
				entity->type == sync::NetObjEntityType::Bike ||
				entity->type == sync::NetObjEntityType::Boat ||
				entity->type == sync::NetObjEntityType::Heli ||
				entity->type == sync::NetObjEntityType::Plane ||
				entity->type == sync::NetObjEntityType::Submarine ||
				entity->type == sync::NetObjEntityType::Trailer ||
				entity->type == sync::NetObjEntityType::Train)
		{
			auto curVehicleData = (entity && entity->syncTree) ? entity->syncTree->GetVehicleGameState() : nullptr;

			if (curVehicleData)
			{
				auto [_, clientData] = GetClientData(this, targetClient);
				auto ces = clientData->entityStates.find(clientData->lastAckIndex);

				if (ces != clientData->entityStates.end())
				{
					for (auto occupant : curVehicleData->occupants)
					{
						if (occupant > 0)
						{
							auto occupantEntity = GetEntity(0, occupant);

							if (occupantEntity)
							{
								auto es = ces->second->find(occupant);

								if (es != ces->second->end())
								{
									if (es->second.uniqifier == occupantEntity->uniqifier)
									{
										auto fi = es->second.frameIndex;
										auto ofi = occupantEntity->syncTree->GetPedGameStateFrameIndex();

										if (ofi > fi)
										{
											can = false;
										}
									}
									else
									{
										can = false;
									}
								}
								else
								{
									can = false;
								}
							}
						}
					}
				}
				else
				{
					can = false;
				}
			}
		}

		return can;
	};

	bool canReassign = checkCanReassign(entity);

	if (!canReassign)
	{
		return;
	}

	auto oldClientRef = entity->GetClientUnsafe().lock();

	{
		entity->GetClientUnsafe() = targetClient;
		
		if (entity->stateBag)
		{
			if (targetClient)
			{
				entity->stateBag->SetOwningPeer(targetClient->GetSlotId());
			}
			else
			{
				entity->stateBag->SetOwningPeer(-1);
			}
		}

		GS_LOG("%s: obj id %d, old client %d, new client %d\n", __func__, entityHandle, (!oldClientRef) ? -1 : oldClientRef->GetNetId(), (targetClient) ? targetClient->GetNetId() : -1);

		if (oldClientRef)
		{
			auto[lock, sourceData] = GetClientData(this, oldClientRef);
			sourceData->objectIds.erase(entityHandle);
		}
	}

	// #TODO1S: reassignment should also send a create if the player was out of scope
	{
		if (targetClient)
		{
			auto [lock, targetData] = GetClientData(this, targetClient);
			targetData->objectIds.insert(entityHandle);
		}
	}

	// when deleted, we want to make this object ID return to the global pool, not to the player who last owned it
	// therefore, mark it as stolen
	{
		std::unique_lock<std::mutex> lock(m_objectIdsMutex);

		if (m_objectIdsUsed.test(entityHandle))
		{
			m_objectIdsStolen.set(entityHandle);
		}
	}

	// allow this object to be synced instantly again so clients are aware of ownership changes as soon as possible
	m_instance->GetComponent<fx::ClientRegistry>()->ForAllClients([this, entityHandle](const fx::ClientSharedPtr& client)
	{
		if (client)
		{
			auto [lock, targetData] = GetClientData(this, client);

			auto it = targetData->entityStates.find(targetData->lastAckIndex);

			if (it != targetData->entityStates.end())
			{
				auto eIt = it->second->find(entityHandle);

				if (eIt != it->second->end())
				{
					eIt->second.frameIndex = 0;
					eIt->second.lastSend = 0ms;
					eIt->second.overrideFrameIndex = true;
				}
			}
		}
	});
}

bool ServerGameState::MoveEntityToCandidate(const fx::sync::SyncEntityPtr& entity, const fx::ClientSharedPtr& client)
{
	const auto& clientRegistry = m_instance->GetComponent<fx::ClientRegistry>();
	bool hasClient = true;

	{
		auto entityClient = entity->GetClient();

		if (!entityClient || !client)
		{
			hasClient = false;
		}
		else if (entityClient->GetNetId() == client->GetNetId())
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

		clientRegistry->ForAllClients([this, &client, &candidates, &entity, eh, pos](const fx::ClientSharedPtr& tgtClient)
		{
			if (tgtClient == client)
			{
				return;
			}

			if (tgtClient->GetSlotId() == 0xFFFFFFFF)
			{
				return;
			}

			float distance = std::numeric_limits<float>::max();

			try
			{
				auto data = GetClientDataUnlocked(this, tgtClient);
				sync::SyncEntityPtr playerEntity;
				{
					std::shared_lock _lock(data->playerEntityMutex);
					playerEntity = data->playerEntity.lock();
				}

				if (playerEntity)
				{
					auto tgt = GetPlayerFocusPos(playerEntity);

					if (pos.x != 0.0f)
					{
						distance = glm::distance2(tgt, pos);
					}
				}
			}
			catch (std::bad_any_cast&)
			{

			}

			if (entity->relevantTo.test(tgtClient->GetSlotId()))
			{
				auto [_, clientData] = GetClientData(this, tgtClient);
				auto lastAck = clientData->entityStates.find(clientData->lastAckIndex);

				if (lastAck == clientData->entityStates.end() || lastAck->second->find(eh) != lastAck->second->end())
				{
					candidates.emplace(distance, tgtClient);
				}
			}
		});

		if (entity->type == sync::NetObjEntityType::Player)
		{
			candidates.clear();
		}

		if (candidates.empty()) // no candidate?
		{
			GS_LOG("no candidates for entity %d, assigning as unowned\n", entity->handle);

			if (entity->IsOwnedByScript())
			{
				std::unique_lock _lock(entity->clientMutex);
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

			std::unique_lock _lock(entity->clientMutex);
			ReassignEntity(entity->handle, std::get<1>(candidate));
		}
	}

	return true;
}

void ServerGameState::HandleClientDrop(const fx::ClientSharedPtr& client)
{
	if (!IsOneSync())
	{
		return;
	}

	auto clientRegistry = m_instance->GetComponent<fx::ClientRegistry>();

#ifndef _MSC_VER
	GS_LOG("client drop - reassigning\n", 0);
#else
	GS_LOG("client drop - reassigning\n");
#endif

	if (fx::IsBigMode())
	{
		clientRegistry->ForAllClients([this, &client](const fx::ClientSharedPtr& tgtClient)
		{
			auto [lock, clientData] = GetClientData(this, tgtClient);

			auto si = clientData->playersToSlots.find(client->GetNetId());

			if (si != clientData->playersToSlots.end())
			{
				fwRefContainer<ServerEventComponent> events = m_instance->GetComponent<ServerEventComponent>();
				events->TriggerClientEventReplayed("onPlayerDropped", fmt::sprintf("%d", tgtClient->GetNetId()), client->GetNetId(), client->GetName(), si->second);

				clientData->slotsToPlayers.erase(si->second);
				clientData->playersToSlots.erase(si);
			}
		});
	}

	// clear the player's world grid ownership
	if (auto slotId = client->GetSlotId(); slotId != -1)
	{
		WorldGridState* gridState = &m_worldGrid[slotId];

		auto netId = client->GetNetId();

		for (auto& entry : gridState->entries)
		{
			if (m_worldGridAccel.netIDs[entry.sectorX][entry.sectorY] == netId)
			{
				m_worldGridAccel.netIDs[entry.sectorX][entry.sectorY] = 0xFFFF;
			}

			entry.netID = -1;
			entry.sectorX = 0;
			entry.sectorY = 0;
		}
	}

	std::set<uint32_t> toErase;

	{
		std::shared_lock<std::shared_mutex> lock(m_entityListMutex);

		for (auto& entity : m_entityList)
		{
			if (!entity->syncTree)
			{
				continue;
			}

			auto slotId = client->GetSlotId();

			if (slotId != -1)
			{
				std::lock_guard<std::shared_mutex> _(entity->guidMutex);
				entity->relevantTo.reset(slotId);
			}

			if (!MoveEntityToCandidate(entity, client))
			{
				toErase.insert(entity->handle);
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

		std::unique_lock<std::mutex> objectIdsLock(m_objectIdsMutex);

		for (auto& objectId : data->objectIds)
		{
			m_objectIdsSent.reset(objectId);
		}
	}

	// remove ACKs for this client
	if (client->GetSlotId() != 0xFFFFFFFF)
	{
		std::shared_lock<std::shared_mutex> lock(m_entityListMutex);

		for (auto& entity : m_entityList)
		{
			if (entity->syncTree)
			{
				entity->syncTree->Visit([&client](sync::NodeBase& node)
				{
					node.ackedPlayers.reset(client->GetSlotId());

					return true;
				});
			}
		}
	}
}

void ServerGameState::ProcessCloneCreate(const fx::ClientSharedPtr& client, rl::MessageBuffer& inPacket, AckPacketWrapper& ackPacket)
{
	uint16_t objectId = 0;
	uint16_t uniqifier = 0;
	if (ProcessClonePacket(client, inPacket, 1, &objectId, &uniqifier))
	{
		std::unique_lock<std::mutex> objectIdsLock(m_objectIdsMutex);
		m_objectIdsUsed.set(objectId);
	}

	ackPacket.Write(3, 1);
	ackPacket.Write(13, objectId);
	ackPacket.Write(16, uniqifier);
	ackPacket.flush();

	GS_LOG("%s: cl %d, id %d\n", __func__, client->GetNetId(), objectId);
}

void ServerGameState::ProcessCloneSync(const fx::ClientSharedPtr& client, rl::MessageBuffer& inPacket, AckPacketWrapper& ackPacket)
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

void ServerGameState::ProcessCloneTakeover(const fx::ClientSharedPtr& client, rl::MessageBuffer& inPacket)
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

		std::unique_lock _lock(entity->clientMutex);
		ReassignEntity(entity->handle, tgtCl);
	}
}

void ServerGameState::ProcessCloneRemove(const fx::ClientSharedPtr& client, rl::MessageBuffer& inPacket, AckPacketWrapper& ackPacket)
{
	auto playerId = 0;
	auto objectId = inPacket.Read<uint16_t>(13);
	auto uniqifier = inPacket.Read<uint16_t>(16);

	// TODO: verify ownership
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

		RemoveClone(client, objectId, uniqifier);
	}
}

void ServerGameState::RemoveClone(const fx::ClientSharedPtr& client, uint16_t objectId, uint16_t uniqifier)
{
	GS_LOG("%s: deleting object %d %d\n", __func__, (client) ? client->GetNetId() : 0, objectId);

	// defer deletion of the object so script has time to do things
	auto continueCloneRemoval = [this, objectId]()
	{
		bool stolen = false;

		{
			std::unique_lock<std::mutex> objectIdsLock(m_objectIdsMutex);
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
			std::shared_lock _lock(m_entitiesByIdMutex);
			auto entityRef = m_entitiesById[objectId].lock();

			if (entityRef)
			{
				fx::ClientSharedPtr clientRef = entityRef->GetClient();

				if (clientRef)
				{
					auto [lock, clientData] = GetClientData(this, clientRef);
					clientData->objectIds.erase(objectId);
				}
			}
		}

		{
			std::unique_lock<std::shared_mutex> entityListLock(m_entityListMutex);

			for (auto it = m_entityList.begin(), end = m_entityList.end(); it != end; ++it)
			{
				if (((*it)->handle) == objectId)
				{
					m_entityList.erase(it);
					break;
				}
			}
		}

		// unset weak pointer, as well
		{
			std::unique_lock entitiesByIdLock(m_entitiesByIdMutex);
			m_entitiesById[objectId].reset();
		}
	};

	{
		sync::SyncEntityPtr entityRef;

		{
			std::shared_lock entitiesByIdLock(m_entitiesByIdMutex);
			entityRef = m_entitiesById[objectId].lock();
		}

		if (entityRef && !entityRef->deleting)
		{
			entityRef->deleting = true;
			
			// in case uniqifier wasn't passed, set it
			uniqifier = entityRef->uniqifier;

			OnCloneRemove(entityRef, continueCloneRemoval);
		}
	}

	// since we know the owner won't have this ID anymore, make this the actual case
	if (client)
	{
		auto [_, data] = GetClientData(this, client);

		for (auto& es : data->entityStates)
		{
			auto esIt = es.second->find(objectId);

			if (esIt != es.second->end() && esIt->second.uniqifier == uniqifier)
			{
				es.second->erase(objectId);
			}
		}
	}
}

auto ServerGameState::CreateEntityFromTree(sync::NetObjEntityType type, const std::shared_ptr<sync::SyncTreeBase>& tree) -> fx::sync::SyncEntityPtr
{
	bool hadId = false;

	int id = fx::IsLengthHack() ? (MaxObjectId - 1) : 8191;

	for (; id >= 1; id--)
	{
		if (!m_objectIdsSent.test(id) && !m_objectIdsUsed.test(id))
		{
			break;
		}
	}

	m_objectIdsSent.set(id);
	m_objectIdsUsed.set(id);
	m_objectIdsStolen.set(id);

	fx::sync::SyncEntityPtr entity = fx::sync::SyncEntityPtr::Construct();
	entity->type = type;
	entity->guid = nullptr;
	entity->frameIndex = m_frameIndex;
	entity->lastFrameIndex = 0;
	entity->handle = MakeEntityHandle(id);
	entity->uniqifier = rand();
	entity->creationToken = msec().count();
	entity->passedFilter = true;

	entity->syncTree = tree;

	entity->lastReceivedAt = msec();

	entity->timestamp = msec().count();

	{
		std::unique_lock<std::shared_mutex> entityListLock(m_entityListMutex);

		m_entityList.push_back(entity);
	}

	{
		std::unique_lock entitiesByIdLock(m_entitiesByIdMutex);
		m_entitiesById[id] = entity;
	}

	return entity;
}

bool ServerGameState::ProcessClonePacket(const fx::ClientSharedPtr& client, rl::MessageBuffer& inPacket, int parsingType, uint16_t* outObjectId, uint16_t* outUniqifier)
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

		objectType = (sync::NetObjEntityType)inPacket.Read<uint8_t>(4);
	}

	auto length = inPacket.Read<uint16_t>(12);

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
			entity->GetClientUnsafe() = client;
			entity->type = objectType;
			entity->guid = nullptr;
			entity->frameIndex = m_frameIndex;
			entity->lastFrameIndex = 0;
			entity->handle = MakeEntityHandle(objectId);
			entity->uniqifier = uniqifier;
			entity->creationToken = creationToken;

			entity->syncTree = MakeSyncTree(objectType);

			{
				std::unique_lock<std::shared_mutex> entityListLock(m_entityListMutex);

				m_entityList.push_back(entity);
			}

			createdHere = true;

			{
				std::unique_lock entitiesByIdLock(m_entitiesByIdMutex);
				m_entitiesById[objectId] = entity;
			}
		}
		else // duplicate create? that's not supposed to happen
		{
			auto lcl = entity->GetClient();

			if (objectType != entity->type)
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
		GS_LOG("%s: wrong uniqifier (%d)!\n", __func__, objectId);

		return false;
	}

	if (client->GetSlotId() < 0 || client->GetSlotId() > MAX_CLIENTS)
	{
		return false;
	}

	{
		std::lock_guard<std::shared_mutex> _(entity->guidMutex);
		entity->relevantTo.set(client->GetSlotId());
	}

	fx::ClientSharedPtr entityClient = entity->GetClient();

	if (!entityClient)
	{
		return false;
	}

	if (entityClient->GetNetId() != client->GetNetId())
	{
		GS_LOG("%s: wrong owner (%d)!\n", __func__, objectId);

		return false;
	}

	{
		std::unique_lock _clientLock(entity->clientMutex);
		entity->GetLastUpdaterUnsafe() = entityClient;
	}
	entity->lastReceivedAt = msec();

	// force the client to have the new entity so we don't send duplicate creations
	{
		auto [lock, clientData] = GetClientData(this, client);

		for (auto& state : clientData->entityStates)
		{
			ClientEntityState ces;
			ces.frameIndex = entity->frameIndex;
			ces.lastSend = 0ms;
			ces.uniqifier = entity->uniqifier;
			ces.syncDelay = 0ms;
			ces.isPlayer = (entity->type == sync::NetObjEntityType::Player);
			ces.netId = (client) ? client->GetNetId() : -1;
			ces.overrideFrameIndex = false;

			(*state.second)[entity->handle] = ces;
		}
	}

	if (length > 0)
	{
		entity->timestamp = timestamp;

		auto state = sync::SyncParseState{ { bitBytes }, parsingType, 0, timestamp, entity, m_frameIndex };

		auto syncTree = entity->syncTree;
		if (syncTree)
		{
			syncTree->Parse(state);

			if (parsingType == 2)
			{
				entity->hasSynced = true;
			}

			// reset resends to 0
			//entity->lastResends = {};

			if (parsingType == 1)
			{
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

		// if we're in entity lockdown, validate the entity first
		if (m_entityLockdownMode != EntityLockdownMode::Inactive && entity->type != sync::NetObjEntityType::Player)
		{
			if (!ValidateEntity(entity))
			{
				// yeet
				RemoveClone({}, entity->handle);
				return false;
			}
		}

		if (!OnEntityCreate(entity))
		{
			RemoveClone({}, entity->handle);
			return false;
		}

		// players love their entities.
		// don't take them away.
		// that's incredibly mean.
		{
			auto [lock, clientData] = GetClientData(this, client);
			clientData->relevantEntities.push_back({ entity->handle, 0ms, true });
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

	if (entity->stateBag)
	{
		entity->stateBag->AddRoutingTarget(client->GetSlotId());
	}

	return true;
}

bool ServerGameState::ValidateEntity(const fx::sync::SyncEntityPtr& entity)
{
	bool allowed = false;
	// allow auto-generated population in non-strict lockdown
	if (m_entityLockdownMode != EntityLockdownMode::Strict)
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

static std::tuple<std::optional<net::Buffer>, uint32_t> UncompressClonePacket(const std::vector<uint8_t>& packetData)
{
	net::Buffer readBuffer(packetData);
	auto type = readBuffer.Read<uint32_t>();

	if (type != HashString("netClones") && type != HashString("netAcks"))
	{
		return { std::optional<net::Buffer>{}, type };
	}

	uint8_t bufferData[16384] = { 0 };
	int bufferLength = LZ4_decompress_safe(reinterpret_cast<const char*>(&readBuffer.GetData()[4]), reinterpret_cast<char*>(bufferData), readBuffer.GetRemainingBytes(), sizeof(bufferData));

	if (bufferLength <= 0)
	{
		return { std::optional<net::Buffer>{}, type };
	}

	return { { {bufferData, size_t(bufferLength)} }, type };
}

void ServerGameState::ParseGameStatePacket(const fx::ClientSharedPtr& client, const std::vector<uint8_t>& packetData)
{
	if (!IsOneSync())
	{
		return;
	}

	auto [packet, type] = UncompressClonePacket(packetData);

	if (!packet)
	{
		return;
	}

	switch (type)
	{
	case HashString("netClones"):
		ParseClonePacket(client, *packet);
		break;
	}
}

void ServerGameState::ParseClonePacket(const fx::ClientSharedPtr& client, net::Buffer& buffer)
{
	rl::MessageBuffer msgBuf(buffer.GetData().data() + buffer.GetCurOffset(), buffer.GetRemainingBytes());

	rl::MessageBuffer ackPacket;

	{
		auto[lock, clientData] = GetClientData(this, client);

		ackPacket = std::move(clientData->ackBuffer);
	}

	AckPacketWrapper ackPacketWrapper{ ackPacket };
	ackPacketWrapper.flush = [&ackPacket, &client]()
	{
		MaybeFlushBuffer(ackPacket, HashRageString("msgPackedAcks"), 0, client);
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
		case 7: // end
			end = true;
			break;
		default:
			end = true;
			break;
		}
	}

	{
		auto [lock, clientData] = GetClientData(this, client);

		clientData->ackBuffer = std::move(ackPacket);
	}
}

void ServerGameState::SendObjectIds(const fx::ClientSharedPtr& client, int numIds)
{
	// first, gather IDs
	std::vector<int> ids;

	{
		auto [lock, data] = GetClientData(this, client);
		std::unique_lock<std::mutex> objectIdsLock(m_objectIdsMutex);

		int id = 1;

		for (int i = 0; i < numIds; i++)
		{
			bool hadId = false;

			for (; id < m_objectIdsSent.size(); id++)
			{
				if (!m_objectIdsSent.test(id) && !m_objectIdsUsed.test(id))
				{
					hadId = true;

					data->objectIds.insert(id);

					ids.push_back(id);
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

	// compress and send

	// adapted from https://stackoverflow.com/a/1081776
	std::vector<std::tuple<int, int>> pairs;

	int last = -1, len = 0;

	for (int i = 0; i < ids.size(); )
	{
		int gap = ids[i] - 2 - last;
		int size = 0;

		while (++i < ids.size() && ids[i] == ids[i - 1] + 1) size++;

		last = ids[i - 1];

		pairs.emplace_back(gap, size);
	}

	net::Buffer outBuffer;
	outBuffer.Write<uint32_t>(HashRageString("msgObjectIds"));
	outBuffer.Write<uint16_t>(pairs.size());

	for (auto& pair : pairs)
	{
		auto [gap, size] = pair;

		outBuffer.Write<uint16_t>(gap);
		outBuffer.Write<uint16_t>(size);
	}

	client->SendPacket(1, outBuffer, NetPacketType_ReliableReplayed);
}

void ServerGameState::DeleteEntity(const fx::sync::SyncEntityPtr& entity)
{
	if (entity->type != sync::NetObjEntityType::Player && entity->syncTree)
	{
		RemoveClone({}, entity->handle);
	}
}

void ServerGameState::SendPacket(int peer, std::string_view data)
{
	auto creg = m_instance->GetComponent<fx::ClientRegistry>();

	if (peer < 0)
	{
		return;
	}

	auto client = creg->GetClientBySlotID(peer);

	if (client)
	{
		net::Buffer buffer;
		buffer.Write<uint32_t>(HashRageString("msgStateBag"));
		buffer.Write(data.data(), data.size());

		client->SendPacket(1, buffer, NetPacketType_ReliableReplayed);
	}
}

void ServerGameState::AttachToObject(fx::ServerInstanceBase* instance)
{
	m_instance = instance;

	auto sbac = fx::StateBagComponent::Create(fx::StateBagRole::Server);
	sbac->SetGameInterface(this);

	instance->GetComponent<fx::GameServer>()->GetComponent<fx::HandlerMapComponent>()->Add(HashRageString("msgStateBag"), 
		{ fx::ThreadIdx::Sync, [sbac](const fx::ClientSharedPtr& client, net::Buffer& buffer) {
		if (client->GetSlotId() != -1)
		{
			sbac->HandlePacket(client->GetSlotId(), std::string_view{ reinterpret_cast<const char*>(buffer.GetBuffer() + buffer.GetCurOffset()), buffer.GetRemainingBytes() });
		}
	} });

	instance->GetComponent<fx::ResourceManager>()->SetComponent(sbac);

	auto creg = instance->GetComponent<fx::ClientRegistry>();
	m_globalBag = sbac->RegisterStateBag("global");
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
		std::shared_lock<std::shared_mutex> lock(m_entityListMutex);

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
}
}

#include <ResourceManager.h>
#include <ServerInstanceBase.h>
#include <ServerInstanceBaseRef.h>
#include <ScriptEngine.h>

struct CFireEvent
{
	void Parse(rl::MessageBuffer& buffer);

	inline std::string GetName()
	{
		return "fireEvent";
	}

	struct fire
	{
		int v1;
		bool v2;
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
		int v13;
		uint16_t fireId;

		MSGPACK_DEFINE_MAP(v1,v2,v3,v4,v5X,v5Y,v5Z,posX,posY,posZ,v7,v8,maxChildren,v10,v11,v12,v13,fireId);
	};

	std::vector<fire> fires;

	MSGPACK_DEFINE(fires);
};

void CFireEvent::Parse(rl::MessageBuffer& buffer)
{
	int count = buffer.Read<int>(3);
	if (count > 5)
		count = 5;

	for (int i = 0; i < count; i++)
	{
		fire f;
		f.v1 = buffer.Read<int>(4);
		f.v2 = buffer.Read<uint8_t>(1);
		if (f.v2)
			f.v3 = buffer.Read<uint16_t>(13);
		else
			f.v3 = 0;
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
		if (buffer.Read<uint8_t>(1))
			f.v13 = buffer.Read<int>(32);
		else
			f.v13 = 0;
		f.fireId = buffer.Read<uint16_t>(16);
		fires.push_back(f);
	}
}

struct CExplosionEvent
{
	void Parse(rl::MessageBuffer& buffer);

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

void CExplosionEvent::Parse(rl::MessageBuffer& buffer)
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

struct CWeaponDamageEvent
{
	void Parse(rl::MessageBuffer& buffer);

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
	bool f80_1;

	uint32_t f100;
	uint16_t f116;
	uint32_t f104;

	uint16_t weaponDamage;
	bool f135;

	uint16_t f48;
	uint16_t f52;
	uint16_t f56;

	bool f112;

	uint32_t damageTime;
	bool willKill;
	uint32_t f120;
	bool hasVehicleData;

	uint16_t f112_1;

	uint16_t parentGlobalId; // Source entity?
	uint16_t hitGlobalId; // Target entity?

	uint8_t tyreIndex;
	uint8_t suspensionIndex;
	uint8_t hitComponent;

	bool f133;
	bool f125;

	uint16_t f64;
	uint16_t f68;
	uint16_t f72;

	MSGPACK_DEFINE_MAP(damageType, weaponType, overrideDefaultDamage, hitEntityWeapon, hitWeaponAmmoAttachment, silenced, damageFlags, f80_1, f100, f116, f104, weaponDamage, f135, f48, f52, f56, f112, damageTime, willKill, f120, hasVehicleData, f112_1, parentGlobalId, hitGlobalId, tyreIndex, suspensionIndex, hitComponent, f133, f125, f64, f68, f72);
};

void CWeaponDamageEvent::Parse(rl::MessageBuffer& buffer)
{
	damageType = buffer.Read<uint8_t>(2);
	weaponType = buffer.Read<uint32_t>(32);

	overrideDefaultDamage = buffer.Read<uint8_t>(1);
	hitEntityWeapon = buffer.Read<uint8_t>(1);
	hitWeaponAmmoAttachment = buffer.Read<uint8_t>(1);
	silenced = buffer.Read<uint8_t>(1);

	damageFlags = buffer.Read<uint32_t>(21);
	// (damageFlags >> 1) & 1
	f80_1 = buffer.Read<uint8_t>(1);

	if (f80_1)
	{
		f100 = buffer.Read<uint32_t>(32);
		f116 = buffer.Read<uint16_t>(16);
		f104 = buffer.Read<uint32_t>(32);
	}

	if (overrideDefaultDamage)
	{
		weaponDamage = buffer.Read<uint16_t>(14);
	}
	else
	{
		weaponDamage = 0;
	}

	f135 = buffer.Read<uint8_t>(1);

	if (f135)
	{
		f48 = buffer.Read<uint16_t>(16);
		f52 = buffer.Read<uint16_t>(16);
		f56 = buffer.Read<uint16_t>(16);
	}

	if (damageType == 3)
	{
		damageTime = buffer.Read<uint32_t>(32);
		willKill = buffer.Read<uint8_t>(1);

		if (f80_1)
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
		parentGlobalId = buffer.Read<uint16_t>(13);
		hitGlobalId = buffer.Read<uint16_t>(13);

		if (damageType < 2)
		{
			f48 = buffer.Read<uint16_t>(16);
			f52 = buffer.Read<uint16_t>(16);
			f56 = buffer.Read<uint16_t>(16);

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
	}

	f133 = buffer.Read<uint8_t>(1);
	f125 = buffer.Read<uint8_t>(1);

	if (f125)
	{
		f64 = buffer.Read<uint16_t>(16);
		f68 = buffer.Read<uint16_t>(16);
		f72 = buffer.Read<uint16_t>(16);
	}
}

struct CVehicleComponentControlEvent
{
	void Parse(rl::MessageBuffer& buffer)
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

struct CClearPedTasksEvent
{
	void Parse(rl::MessageBuffer& buffer)
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
	void Parse(rl::MessageBuffer& buffer)
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

struct CGiveWeaponEvent
{
    void Parse(rl::MessageBuffer& buffer)
    {
        pedId = buffer.Read<uint16_t>(13);
        weaponType = buffer.Read<uint32_t>(32);
        unk1 = buffer.Read<uint8_t>(1);
        ammo = buffer.Read<uint16_t>(15);
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
    void Parse(rl::MessageBuffer& buffer)
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

template<typename TEvent>
inline auto GetHandler(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer&& buffer) -> std::function<bool()>
{
	uint16_t length = buffer.Read<uint16_t>();

	if (length == 0)
	{
		return []() {
			return false;
		};
	}

	std::vector<uint8_t> data(length);
	buffer.Read(data.data(), data.size());

	rl::MessageBuffer msgBuf(data);
	auto ev = std::make_shared<TEvent>();
	ev->Parse(msgBuf);

	return [instance, client, ev = std::move(ev)]()
	{
		auto evComponent = instance->GetComponent<fx::ResourceManager>()->GetComponent<fx::ResourceEventManagerComponent>();
		return evComponent->TriggerEvent2(ev->GetName(), { }, fmt::sprintf("%d", client->GetNetId()), *ev);
	};
}

enum GTA_EVENT_IDS
{
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
	NETWORK_CRC_HASH_CHECK_EVENT,
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
};

static std::function<bool()> GetEventHandler(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer&& buffer)
{
	buffer.Read<uint16_t>(); // eventHeader
	bool isReply = buffer.Read<uint8_t>(); // is reply
	uint16_t eventType = buffer.Read<uint16_t>(); // event ID

	switch(eventType)
	{
		case WEAPON_DAMAGE_EVENT: return GetHandler<CWeaponDamageEvent>(instance, client, std::move(buffer));
		case RESPAWN_PLAYER_PED_EVENT: return GetHandler<CRespawnPlayerPedEvent>(instance, client, std::move(buffer));
		case GIVE_WEAPON_EVENT: return GetHandler<CGiveWeaponEvent>(instance, client, std::move(buffer));
		case REMOVE_WEAPON_EVENT: return GetHandler<CRemoveWeaponEvent>(instance, client, std::move(buffer));
		case VEHICLE_COMPONENT_CONTROL_EVENT: return GetHandler<CVehicleComponentControlEvent>(instance, client, std::move(buffer));
		case FIRE_EVENT: return GetHandler<CFireEvent>(instance, client, std::move(buffer));
		case EXPLOSION_EVENT: return GetHandler<CExplosionEvent>(instance, client, std::move(buffer));
		case NETWORK_CLEAR_PED_TASKS_EVENT: return GetHandler<CClearPedTasksEvent>(instance, client, std::move(buffer));
	};

	return {};
}

static InitFunction initFunction([]()
{
	g_scriptHandlePool = new CPool<fx::ScriptGuid>(1500, "fx::ScriptGuid");

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		g_oneSyncVar = instance->AddVariable<fx::OneSyncState>("onesync", ConVar_ReadOnly, fx::OneSyncState::Off);
		g_oneSyncPopulation = instance->AddVariable<bool>("onesync_population", ConVar_ReadOnly, true);

		// .. to infinity?
		g_oneSyncBigMode = instance->AddVariable<bool>("onesync_enableInfinity", ConVar_ReadOnly, false);

		g_bigMode = g_oneSyncBigMode->GetValue();

		// or maybe, beyond?
		g_oneSyncLengthHack = instance->AddVariable<bool>("onesync_enableBeyond", ConVar_ReadOnly, false);

		g_lengthHack = g_oneSyncLengthHack->GetValue();

		if (g_oneSyncVar->GetValue() == fx::OneSyncState::On)
		{
			g_bigMode = true;
			g_lengthHack = g_oneSyncPopulation->GetValue();

			g_oneSyncBigMode->GetHelper()->SetRawValue(true);
			g_oneSyncLengthHack->GetHelper()->SetRawValue(g_lengthHack);
		}

		instance->OnInitialConfiguration.Connect([]()
		{
			if (g_oneSyncEnabledVar->GetValue() && g_oneSyncVar->GetValue() == fx::OneSyncState::Off)
			{
				g_oneSyncVar->GetHelper()->SetRawValue(g_bigMode ? fx::OneSyncState::On : fx::OneSyncState::Legacy);

				console::PrintWarning("server", "`onesync_enabled` is deprecated. Please use `onesync %s` instead.\n", 
					g_bigMode ? "on" : "legacy");
			}
			else if (!g_oneSyncEnabledVar->GetValue() && g_oneSyncVar->GetValue() != fx::OneSyncState::Off)
			{
				g_oneSyncEnabledVar->GetHelper()->SetRawValue(true);
			}
		});
	}, INT32_MIN);

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		g_oneSyncEnabledVar = instance->AddVariable<bool>("onesync_enabled", ConVar_ServerInfo, false);
		g_oneSyncCulling = instance->AddVariable<bool>("onesync_distanceCulling", ConVar_None, true);
		g_oneSyncVehicleCulling = instance->AddVariable<bool>("onesync_distanceCullVehicles", ConVar_None, false);
		g_oneSyncForceMigration = instance->AddVariable<bool>("onesync_forceMigration", ConVar_None, false);
		g_oneSyncRadiusFrequency = instance->AddVariable<bool>("onesync_radiusFrequency", ConVar_None, true);
		g_oneSyncLogVar = instance->AddVariable<std::string>("onesync_logFile", ConVar_None, "");
		g_oneSyncWorkaround763185 = instance->AddVariable<bool>("onesync_workaround763185", ConVar_None, false);

		instance->SetComponent(new fx::ServerGameState);

		instance->GetComponent<fx::GameServer>()->OnSyncTick.Connect([=]()
		{
			if (!fx::IsOneSync())
			{
				return;
			}

			instance->GetComponent<fx::ServerGameState>()->Tick(instance);
		});

		auto gameServer = instance->GetComponent<fx::GameServer>();

		gameServer->GetComponent<fx::HandlerMapComponent>()->Add(HashRageString("msgNetGameEvent"), { fx::ThreadIdx::Sync, [=](const fx::ClientSharedPtr& client, net::Buffer& buffer)
		{
			auto targetPlayerCount = buffer.Read<uint8_t>();
			std::vector<uint16_t> targetPlayers(targetPlayerCount);

			if (!buffer.Read(targetPlayers.data(), targetPlayers.size() * sizeof(uint16_t)))
			{
				return;
			}

			net::Buffer netBuffer;
			netBuffer.Write<uint32_t>(HashRageString("msgNetGameEvent"));
			netBuffer.Write<uint16_t>(client->GetNetId());
			buffer.ReadTo(netBuffer, buffer.GetRemainingBytes());

			auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

			auto routeEvent = [netBuffer, targetPlayers, clientRegistry]()
			{
				for (uint16_t player : targetPlayers)
				{
					auto targetClient = clientRegistry->GetClientByNetID(player);

					if (targetClient)
					{
						targetClient->SendPacket(1, netBuffer, NetPacketType_Reliable);
					}
				}
			};

			auto copyBuf = netBuffer.Clone();
			copyBuf.Seek(6);

			auto eventHandler = GetEventHandler(instance, client, std::move(copyBuf));

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

		gameServer->GetComponent<fx::HandlerMapComponent>()->Add(HashRageString("msgRequestObjectIds"), { fx::ThreadIdx::Sync, [=](const fx::ClientSharedPtr& client, net::Buffer& buffer)
		{
			instance->GetComponent<fx::ServerGameState>()->SendObjectIds(client, fx::IsBigMode() ? 6 : 32);
		} });

		gameServer->GetComponent<fx::HandlerMapComponent>()->Add(HashRageString("gameStateAck"), { fx::ThreadIdx::Sync, [=](const fx::ClientSharedPtr& client, net::Buffer& buffer)
		{
			uint64_t frameIndex = buffer.Read<uint64_t>();

			eastl::fixed_set<uint16_t, 100, false> ignoreEntities;
			uint8_t ignoreCount = buffer.Read<uint8_t>();

			for (int i = 0; i < std::min(ignoreCount, uint8_t(100)); i++)
			{
				ignoreEntities.insert(buffer.Read<uint16_t>());
			}

			eastl::fixed_set<uint16_t, 100, false> recreateEntities;
			uint8_t recreateCount = buffer.Read<uint8_t>();

			for (int i = 0; i < std::min(recreateCount, uint8_t(100)); i++)
			{
				auto objectId = buffer.Read<uint16_t>();
				ignoreEntities.erase(objectId);
				recreateEntities.insert(objectId);
			}

			auto sgs = instance->GetComponent<fx::ServerGameState>();
			
			eastl::fixed_vector<std::tuple<fx::sync::SyncEntityPtr, bool>, 16> relevantExits;

			{
				auto slotId = client->GetSlotId();
				auto [lock, clientData] = GetClientData(sgs.GetRef(), client);

				auto origAckIndex = clientData->lastAckIndex;

				// if duplicate/out-of-order, ignore
				if (origAckIndex >= frameIndex)
				{
					GS_LOG("%s ack skipped - %d >= %d\n", client->GetName(), origAckIndex, frameIndex);
					return;
				}

				// generate delta path for this ack (inclusive)
				eastl::fixed_vector<std::tuple<uint64_t, fx::EntityStateObject*>, 5> deltaPath;

				for (auto lastIdx = origAckIndex; lastIdx <= frameIndex; lastIdx++)
				{
					auto lastIt = clientData->entityStates.find(lastIdx);

					if (lastIt != clientData->entityStates.end())
					{
						deltaPath.push_back({ lastIdx, lastIt->second.get() });
					}
				}

				// use delta path
				auto lastAck = clientData->entityStates.find(origAckIndex);
				auto es = clientData->entityStates.find(frameIndex);

				clientData->lastAckIndex = frameIndex;

				GS_LOG("%s ack is now %lld\n", client->GetName(), frameIndex);

				// diff current and last ack
				if (deltaPath.size() > 0)
				{
					eastl::fixed_map<uint16_t, fx::ClientEntityState, 128> deletedKeys;

					for (size_t deltaIdx = 0; deltaIdx < (deltaPath.size() - 1); deltaIdx++)
					{
						eastl::fixed_vector<eastl::pair<uint16_t, fx::ClientEntityState>, 128> deletedKeysLocal;

						std::set_difference(std::get<1>(deltaPath[deltaIdx])->begin(), std::get<1>(deltaPath[deltaIdx])->end(),
							std::get<1>(deltaPath[deltaIdx + 1])->begin(), std::get<1>(deltaPath[deltaIdx + 1])->end(),
							std::back_inserter(deletedKeysLocal), [](const auto& left, const auto& right)
						{
							return left.first < right.first;
						});

						for (auto& pair : deletedKeysLocal)
						{
							deletedKeys[pair.first] = pair.second;
						}
					}

					for (auto& entityPair : deletedKeys)
					{
						auto entity = sgs->GetEntity(0, entityPair.first);

						if (!entity)
						{
							continue;
						}

						// that's an entirely different entity
						if (entity->uniqifier != entityPair.second.uniqifier)
						{
							continue;
						}

						auto entityClient = entity->GetClient();

						{
							std::lock_guard<std::shared_mutex> _(entity->guidMutex);
							entity->relevantTo.reset(client->GetSlotId());
						}

						if (entity->stateBag)
						{
							entity->stateBag->RemoveRoutingTarget(client->GetSlotId());
						}

						std::shared_lock<std::shared_mutex> sharedLock(entity->guidMutex);
						// poor entity, it's relevant to nobody :( disown/delete it
						if (entity->relevantTo.none() && entityClient && entity->type != fx::sync::NetObjEntityType::Player)
						{
							relevantExits.push_back({ entity, false });
						}
						// yeah, ofc I deleted my own entity, that's what we do right?
						else if (entityClient == client)
						{
							relevantExits.push_back({ entity, true });
						}
					}

					for (auto& entityPair : *es->second)
					{
						auto entity = sgs->GetEntity(0, entityPair.first);

						if (!entity)
						{
							continue;
						}

						if (entity->stateBag)
						{
							entity->stateBag->AddRoutingTarget(client->GetSlotId());
						}
					}
				}

				if (es != clientData->entityStates.end())
				{
					for (auto& entity : ignoreEntities)
					{
						GS_LOG("%s is ignoring entity %d\n", client->GetName(), entity);

						for (auto& ackEntry : deltaPath)
						{
							if (lastAck != clientData->entityStates.end() && lastAck->second->find(entity) != lastAck->second->end())
							{
								// #TODO1SACK: better frame index handling to allow ignoring on node granularity
								auto eIt = std::get<1>(ackEntry)->find(entity);

								if (eIt != std::get<1>(ackEntry)->end())
								{
									eIt->second.frameIndex = 0;
									eIt->second.lastSend = 0ms;
									eIt->second.overrideFrameIndex = true;
								}
							}
							else
							{
								std::get<1>(ackEntry)->erase(entity);
							}
						}
					}

					for (auto& entity : recreateEntities)
					{
						GS_LOG("%s is requesting recreate of creating entity %d\n", client->GetName(), entity);

						for (auto& ackEntry : deltaPath)
						{
							std::get<1>(ackEntry)->erase(entity);
						}
					}
				}

				client->SetData("syncFrameIndex", frameIndex);

				// erase all states up to (and including) the one we just approved
				if (lastAck != clientData->entityStates.end())
				{
					clientData->entityStates.erase(clientData->entityStates.begin(), ++lastAck);
				}
			}

			for (auto& [ entity, relevantToSome ] : relevantExits)
			{
				// give the entity a chance
				if (!entity->hasSynced)
				{
					continue;
				}

				auto cl = entity->GetClient();
				if (relevantToSome)
				{
					if (sgs->MoveEntityToCandidate(entity, cl))
					{
						continue;
					}
				}

				if (entity->IsOwnedByScript())
				{
					std::unique_lock _lock(entity->clientMutex);
					sgs->ReassignEntity(entity->handle, {});
				}
				else
				{
					if (!sgs->MoveEntityToCandidate(entity, cl))
					{
						sgs->DeleteEntity(entity);
					}
				}
			}
		} });

		gameServer->GetComponent<fx::HandlerMapComponent>()->Add(HashRageString("msgTimeSyncReq"), { fx::ThreadIdx::Net, [=](const fx::ClientSharedPtr& client, net::Buffer& buffer)
		{
			auto reqTime = buffer.Read<uint32_t>();
			auto reqSeq = buffer.Read<uint32_t>();

			net::Buffer netBuffer;
			netBuffer.Write<uint32_t>(HashRageString("msgTimeSync"));
			netBuffer.Write<uint32_t>(reqTime);
			netBuffer.Write<uint32_t>(reqSeq);
			netBuffer.Write<uint32_t>((msec().count()) & 0xFFFFFFFF);

			client->SendPacket(1, netBuffer, NetPacketType_ReliableReplayed);
		} });
	}, 999999);
});
