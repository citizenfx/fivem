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
	: m_frameIndex(1), m_entitiesById(MaxObjectId), m_entityLockdownMode(EntityLockdownMode::Inactive)
{
	m_tg = std::make_unique<ThreadPool>();
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

		net::Buffer netBuffer(reinterpret_cast<uint8_t*>(outData.data()), len + 4 + 8);
		netBuffer.Seek(len + 4 + 8); // since the buffer constructor doesn't actually set the offset

		GS_LOG("flushBuffer: sending %d bytes to %d\n", len + 4 + 8, client->GetNetId());

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
						FinalizeClone({}, entity->handle, 0, "Requested deletion");
						continue;
					}
					// it's a client-owned entity, let's check for a few things
					else if (entity->IsOwnedByClientScript())
					{
						// is the original owner offline?
						if (entity->firstOwnerDropped)
						{
							// we can delete
							FinalizeClone({}, entity->handle, 0, "First owner dropped");
							continue;
						}
					}
					// it's a script-less entity, we can collect it.
					else if (!entity->IsOwnedByScript() && (entity->type != sync::NetObjEntityType::Player || !entity->GetClient()))
					{
						FinalizeClone({}, entity->handle, 0, "Regular entity GC");
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

		glm::vec3 playerPos;

		if (playerEntity)
		{
			playerPos = GetPlayerFocusPos(playerEntity);
		}

		auto slotId = client->GetSlotId();

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

			if (!isRelevant)
			{
				if (playerEntity)
				{
					float diffX = entityPos.x - playerPos.x;
					float diffY = entityPos.y - playerPos.y;

					float distSquared = (diffX * diffX) + (diffY * diffY);
					if (distSquared < entity->GetDistanceCullingRadius())
					{
						isRelevant = true;
					}
				}
				else
				{
					// can't really say otherwise if the player entity doesn't exist
					isRelevant = !fx::IsBigMode();
				}
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

			// don't route entities that haven't passed filter to others
			if (!entity->passedFilter && !ownsEntity)
			{
				isRelevant = false;
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
					if (entity->IsOwnedByClientScript())
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

			auto entIdentifier = MakeHandleUniqifierPair(entity->handle, entity->uniqifier);

			// already syncing
			if (auto syncIt = currentSyncedEntities.find(entIdentifier); syncIt != currentSyncedEntities.end())
			{
				auto& entityData = syncIt->second;
				if (isRelevant)
				{
					const auto deltaTime = syncDelay - entityData.syncDelta;
					newSyncedEntities[entIdentifier] = { entityData.nextSync + deltaTime, syncDelay, entity, entityData.forceUpdate, entityData.hasCreated };
				}
				else if (entityData.hasCreated)
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

				newSyncedEntities[entIdentifier] = { 0ms, syncDelay, entity, true, false };
			}
		}

		clientDataUnlocked->syncedEntities = std::move(newSyncedEntities);
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

		// no
		if (client->GetSlotId() == -1)
		{
			return;
		}

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

		static fx::object_pool<sync::SyncCommandList, 512 * 1024> syncPool;
		auto scl = shared_reference<sync::SyncCommandList, &syncPool>::Construct(m_frameIndex);

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
						if (entity->IsOwnedByServerScript())
						{
							std::unique_lock _(entity->clientMutex);
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

										sec->TriggerClientEventReplayed("onPlayerDropped", fmt::sprintf("%d", client->GetNetId()), ownerNetId, ownerRef->GetName(), otherSlot);

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
										oldClientData->playerBag->RemoveRoutingTarget(client->GetSlotId());

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
				if (auto stateBag = entity->stateBag)
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

		entitiesToDestroy.clear();

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
					ReassignEntity(entity->handle, client);
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
								int slotId = 127;

								for (; slotId >= 0; slotId--)
								{
									if (slotId == 31)
									{
										slotId--;
									}

									if (!clientData->playersInScope.test(slotId))
									{
										break;
									}
								}

								if (slotId >= 0)
								{
									clientData->playersInScope[slotId] = entityClient->GetNetId();
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
				if (client->GetNetId() == entityClient->GetNetId() && !entity->onCreationRPC.empty())
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

			if (forceUpdate && !wasThisIt)
			{
				baseFrameIndex = 0;
				entity->lastFramesSent[slotId] = 0;
			}
			else
			{
				std::lock_guard _(entity->frameMutex);
				baseFrameIndex = entity->lastFramesSent[slotId];
			}

			ces.syncedEntities[entity->handle] = { entity, baseFrameIndex };

			// should we sync?
			if (forceUpdate || syncData.nextSync - curTime <= 0ms)
			{
				if (!forceUpdate)
				{
					syncData.nextSync = curTime + syncData.syncDelta;
				}

				if (auto stateBag = entity->stateBag)
				{
					stateBag->AddRoutingTarget(slotId);
				}

				bool wasForceUpdate = forceUpdate;
				forceUpdate = false;

				auto syncType = syncData.hasCreated ? 2 : 1;

				auto _ent = entity;

				auto runSync = [this, _ent, &syncType, curTime, &scl, baseFrameIndex, wasForceUpdate](auto&& preCb) 
				{
					scl->EnqueueCommand([this,
										entity = _ent,
										syncType,
										preCb = std::move(preCb),
										baseFrameIndex,
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
						static thread_local rl::MessageBuffer mb(1200);

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
								std::lock_guard _(entity->frameMutex);
								entity->lastFramesSent[slotId] = entity->lastFrameIndex;
							}

							auto len = (state.buffer.GetCurrentBit() / 8) + 1;

							auto startBit = cmdState.cloneBuffer.GetCurrentBit();
							cmdState.maybeFlushBuffer(3 + /* 13 */ 16 + 16 + 4 + 32 + 16 + 64 + 32 + 12 + (len * 8));
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

							cmdState.cloneBuffer.Write(32, (uint32_t)(frameIndex >> 32));
							cmdState.cloneBuffer.Write(32, (uint32_t)frameIndex);

							cmdState.cloneBuffer.Write<uint32_t>(32, (syncType == 1) ?
								curTime.count() :
								(isFirstFrameUpdate) ? (curTime.count() + 1) : entity->timestamp);

							bool mayWrite = true;

							if (syncType == 2 && !isFirstFrameUpdate && wasForceUpdate && entity->GetClient() == cmdState.client)
							{
								mayWrite = false;
							}

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
						syncData.hasCreated = true;

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

		scl->Execute(client);
	
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
	auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

	// clean any non-existent clients from world grid accel periodically
	static std::chrono::milliseconds nextWorldCheck;
	auto now = msec();

	if (now >= nextWorldCheck)
	{
		for (size_t x = 0; x < std::size(m_worldGridAccel.netIDs); x++)
		{
			for (size_t y = 0; y < std::size(m_worldGridAccel.netIDs[x]); y++)
			{
				if (m_worldGridAccel.netIDs[x][y] != 0xFFFF && !clientRegistry->GetClientByNetID(m_worldGridAccel.netIDs[x][y]))
				{
					m_worldGridAccel.netIDs[x][y] = 0xFFFF;
				}
			}
		}

		nextWorldCheck = now + 10s;
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
					if (m_worldGridAccel.netIDs[entry.sectorX][entry.sectorY] == netID)
					{
						m_worldGridAccel.netIDs[entry.sectorX][entry.sectorY] = -1;
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
				bool found = (m_worldGridAccel.netIDs[x][y] != 0xFFFF);

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

	auto oldClientRef = entity->GetClientUnsafe().lock();
	{
		entity->lastMigratedAt = msec();

		entity->GetLastOwnerUnsafe() = oldClientRef;
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
	clientRegistry->ForAllClients([&, uniqPair](const fx::ClientSharedPtr& crClient) {
		const auto slotId = crClient->GetSlotId();
		if (slotId == 0xFFFFFFFF)
		{
			return;
		}
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

		clientRegistry->ForAllClients([this, &client, &candidates, &entity, eh, pos](const fx::ClientSharedPtr& tgtClient)
		{
			if (client && tgtClient == client)
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
				candidates.emplace(distance, tgtClient);
			}
		});

		if (entity->type == sync::NetObjEntityType::Player)
		{
			candidates.clear();
		}

		if (candidates.empty()) // no candidate?
		{
			GS_LOG("no candidates for entity %d, assigning as unowned\n", entity->handle);

			if (entity->IsOwnedByServerScript())
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

void ServerGameState::HandleClientDrop(const fx::ClientSharedPtr& client, uint16_t netId, uint32_t slotId)
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
		clientRegistry->ForAllClients([this, &client, netId](const fx::ClientSharedPtr& tgtClient)
		{
			auto [lock, clientData] = GetClientData(this, tgtClient);

			auto si = clientData->playersToSlots.find(netId);

			if (si != clientData->playersToSlots.end())
			{
				fwRefContainer<ServerEventComponent> events = m_instance->GetComponent<ServerEventComponent>();
				events->TriggerClientEventReplayed("onPlayerDropped", fmt::sprintf("%d", tgtClient->GetNetId()), netId, client->GetName(), si->second);

				clientData->playersInScope.reset(si->second);
				clientData->playersToSlots.erase(si);
			}
		});
	}

	// clear the player's world grid ownership
	if (slotId != -1)
	{
		WorldGridState* gridState = &m_worldGrid[slotId];

		for (auto& entry : gridState->entries)
		{
			entry.netID = -1;
			entry.sectorX = 0;
			entry.sectorY = 0;
		}
	}

	{
		for (size_t x = 0; x < std::size(m_worldGridAccel.netIDs); x++)
		{
			for (size_t y = 0; y < std::size(m_worldGridAccel.netIDs[x]); y++)
			{
				if (m_worldGridAccel.netIDs[x][y] == netId)
				{
					m_worldGridAccel.netIDs[x][y] = 0xFFFF;
				}
			}
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

			auto firstOwner = entity->GetFirstOwner();
			if (firstOwner && firstOwner->GetNetId() == client->GetNetId())
			{
				entity->firstOwnerDropped = true;
			}

			if (slotId != -1)
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
	if (slotId != 0xFFFFFFFF)
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

	client->SetSyncData({});
}

void ServerGameState::ProcessCloneCreate(const fx::ClientSharedPtr& client, rl::MessageBuffer& inPacket, AckPacketWrapper& ackPacket)
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

void ServerGameState::FinalizeClone(const fx::ClientSharedPtr& client, uint16_t objectId, uint16_t uniqifier, std::string_view finalizeReason)
{
	sync::SyncEntityPtr entityRef;

	{
		std::shared_lock entitiesByIdLock(m_entitiesByIdMutex);
		entityRef = m_entitiesById[objectId].lock();
	}

	if (entityRef)
	{
		if (!entityRef->finalizing)
		{
			entityRef->finalizing = true;

			GS_LOG("%s: finalizing object %d (for reason %s)\n", __func__, objectId, finalizeReason);

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

			OnCloneRemove(entityRef, [this, objectId, entityRef]()
			{
				{
					std::unique_lock entitiesByIdLock(m_entitiesByIdMutex);
					m_entitiesById[objectId].reset();
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
	bool hadId = false;

	int id = fx::IsLengthHack() ? (MaxObjectId - 1) : 8191;

	{
		std::unique_lock objectIdsLock(m_objectIdsMutex);

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
	}

	fx::sync::SyncEntityPtr entity = fx::sync::SyncEntityPtr::Construct();
	entity->type = type;
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
		m_entityList.insert(entity);
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
		auto state = sync::SyncParseState{ { bitBytes }, parsingType, 0, timestamp, entity, m_frameIndex };

		auto syncTree = entity->syncTree;
		if (syncTree)
		{
			syncTree->Parse(state);

			if (parsingType == 2)
			{
				entity->hasSynced = true;
			}

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

		// well, yeah, it's their entity, they have it
		{
			auto [lock, data] = GetClientData(this, client);
			auto identPair = MakeHandleUniqifierPair(objectId, uniqifier);

			data->syncedEntities[identPair] = { 0ms, 10ms, entity, false, true };
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

	if (entity->stateBag)
	{
		entity->stateBag->AddRoutingTarget(slotId);
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

	FlushBuffer(ackPacket, HashRageString("msgPackedAcks"), 0, client, nullptr, true);

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
		std::unique_lock objectIdsLock(m_objectIdsMutex);

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
		gscomms_execute_callback_on_sync_thread([=]() 
		{
			RemoveClone({}, entity->handle);
		});
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
	bool hasActionResult;

	uint32_t actionResultName;
	uint16_t actionResultId;
	uint32_t f104;

	uint16_t weaponDamage;
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

	uint8_t tyreIndex;
	uint8_t suspensionIndex;
	uint8_t hitComponent;

	bool f133;
	bool hasImpactDir;

	float impactDirX;
	float impactDirY;
	float impactDirZ;

	MSGPACK_DEFINE_MAP(damageType, weaponType, overrideDefaultDamage, hitEntityWeapon, hitWeaponAmmoAttachment, silenced, damageFlags, hasActionResult, actionResultName, actionResultId, f104, weaponDamage, isNetTargetPos, localPosX, localPosY, localPosZ, f112, damageTime, willKill, f120, hasVehicleData, f112_1, parentGlobalId, hitGlobalId, tyreIndex, suspensionIndex, hitComponent, f133, hasImpactDir, impactDirX, impactDirY, impactDirZ);
};

void CWeaponDamageEvent::Parse(rl::MessageBuffer& buffer)
{
	if (Is2060()) {
		buffer.Read<uint16_t>(16);
	}

	damageType = buffer.Read<uint8_t>(2);
	weaponType = buffer.Read<uint32_t>(32);

	overrideDefaultDamage = buffer.Read<uint8_t>(1);
	hitEntityWeapon = buffer.Read<uint8_t>(1);
	hitWeaponAmmoAttachment = buffer.Read<uint8_t>(1);
	silenced = buffer.Read<uint8_t>(1);

	damageFlags = buffer.Read<uint32_t>(Is2060() ? 24 : 21);
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
		weaponDamage = buffer.Read<uint16_t>(14);
	}
	else
	{
		weaponDamage = 0;
	}

	if (Is2060()) {
		bool _f92 = buffer.Read<uint8_t>(1);
		if (_f92) {
			buffer.Read<uint8_t>(4);
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

		gameServer->GetComponent<fx::HandlerMapComponent>()->Add(HashRageString("gameStateNAck"), {
			fx::ThreadIdx::Sync, [=](const fx::ClientSharedPtr& client, net::Buffer& buffer) {
				auto sgs = instance->GetComponent<fx::ServerGameState>();
				
				auto slotId = client->GetSlotId();
				if (slotId == -1)
				{
					return;
				}

				const uint8_t flags = buffer.Read<uint8_t>();
				const auto thisFrame = buffer.Read<uint64_t>();
				
				if (flags & 1)
				{
					const auto firstMissingFrame = buffer.Read<uint64_t>();
					const auto lastMissingFrame = buffer.Read<uint64_t>();

					auto [lock, clientData] = GetClientData(sgs.GetRef(), client);
					auto& states = clientData->frameStates;

					for (uint64_t frame = lastMissingFrame; frame >= firstMissingFrame; --frame)
					{
						if (auto frameIt = states.find(frame); frameIt != states.end())
						{
							auto& [synced, deletions] = frameIt->second;
							for (auto& [objectId, entData] : synced)
							{
								if (auto ent = entData.entityWeak.lock())
								{
									const auto entIdentifier = MakeHandleUniqifierPair(objectId, ent->uniqifier);

									if (!entData.isCreated)
									{
										if (auto syncedIt = clientData->syncedEntities.find(entIdentifier); syncedIt != clientData->syncedEntities.end())
										{
											syncedIt->second.hasCreated = false;
										}
									}
									else
									{
										std::lock_guard _(ent->frameMutex);
										ent->lastFramesSent[slotId] = std::min(entData.lastSent, ent->lastFramesSent[slotId]);
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
							instance->GetComponent<fx::GameServer>()->DropClient(client, "Timed out after 60 seconds (1, %d)", lastMissingFrame - firstMissingFrame);
							return;
						}
					}
				}

				auto [lock, clientData] = GetClientData(sgs.GetRef(), client);
				auto& states = clientData->frameStates;

				if (auto frameIt = states.find(thisFrame); frameIt != states.end())
				{
					// ignore list
					if (flags & 2)
					{
						eastl::fixed_vector<uint16_t, 100> ignoredUpdates;
						uint8_t ignoreCount = buffer.Read<uint8_t>();
						for (int i = 0; i < ignoreCount; i++)
						{
							ignoredUpdates.push_back(buffer.Read<uint16_t>());
						}

						for (auto objectId : ignoredUpdates)
						{
							auto& [synced, deletions] = frameIt->second;
							if (auto entIter = synced.find(objectId); entIter != synced.end())
							{
								if (auto ent = entIter->second.entityWeak.lock())
								{
									std::lock_guard _(ent->frameMutex);
									ent->lastFramesSent[slotId] = std::min(entIter->second.lastSent, ent->lastFramesSent[slotId]);
								}
							}
						}
					}

					// recreate list
					if (flags & 4)
					{
						eastl::fixed_vector<uint16_t, 100> recreates;
						uint8_t recreateCount = buffer.Read<uint8_t>();
						for (int i = 0; i < recreateCount; i++)
						{
							recreates.push_back(buffer.Read<uint16_t>());
						}

						for (auto objectId : recreates)
						{
							GS_LOG("attempt recreate of id %d for client %d\n", objectId, client->GetNetId());
							auto& [synced, deletions] = frameIt->second;
							if (auto entIter = synced.find(objectId); entIter != synced.end())
							{
								if (auto ent = entIter->second.entityWeak.lock())
								{
									const auto entIdentifier = MakeHandleUniqifierPair(objectId, ent->uniqifier);
									if (auto syncedIt = clientData->syncedEntities.find(entIdentifier); syncedIt != clientData->syncedEntities.end())
									{
										GS_LOG("recreating id %d for client %d\n", objectId, client->GetNetId());
										syncedIt->second.hasCreated = false;
									}
								}
							}
						}
					}
				}
				else
				{
					instance->GetComponent<fx::GameServer>()->DropClient(client, "Timed out after 60 seconds (2)");
					return;
				}
			}
		});

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
