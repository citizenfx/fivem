#include "StdInc.h"

#ifdef STATE_RDR3

#include <ClientRegistry.h>
#include <GameServer.h>
#include <NetAddress.h>
#include <ServerEventComponent.h>
#include <ServerInstanceBase.h>
#include <ServerTime.h>

#include <state/ServerGameState.h>
#include <state/RlMessageBuffer.h>
#include <net/NetObjEntityType.h>

#include <CoreConsole.h>

#include <chrono>
#include <map>
#include <mutex>
#include <set>
#include <vector>

namespace
{
constexpr uint32_t kRecVersion = 1;
constexpr uint16_t kRecFlagLengthHack = 1;
constexpr int kNetObjectTypeBitLength = 5;
constexpr int kFakePeerBase = 1000000;
constexpr size_t kPacketBufferSize = 16384;
constexpr uint16_t kLocalPlayerSlot = 16;
constexpr uint16_t kSentinelSlot = 31;

#pragma pack(push, 1)
struct RecHeader
{
	char magic[4];
	uint32_t version;
	uint16_t flags;
	uint16_t pad;
	uint64_t startWallClockMs;
};

struct RecEntry
{
	uint8_t type;
	uint32_t timeMs;
	uint16_t netId;
	uint32_t length;
};
#pragma pack(pop)

static_assert(sizeof(RecHeader) == 20, "RecHeader has to match the recorder layout");
static_assert(sizeof(RecEntry) == 11, "RecEntry has to match the recorder layout");

struct RecMsg
{
	uint8_t type;
	uint16_t objectId;
	uint32_t creationToken;
	uint8_t entityType;
	std::vector<uint8_t> payload;
};

struct RecFrame
{
	uint32_t timeMs;
	std::vector<RecMsg> msgs;
};

struct Bot
{
	fx::ClientSharedPtr client;
	std::map<uint16_t, uint16_t> objectIds;
	std::map<uint16_t, uint16_t> uniqifiers;
	size_t cursor;
	uint32_t startOffset;
	uint32_t lastLoop;
	uint16_t virtualSlot;
	bool looped;
	uint32_t emitted;
};

std::mutex g_mutex;

fx::ServerInstanceBase* g_serverInstance;

std::vector<RecFrame> g_frames;
std::string g_loadedPath;
uint32_t g_duration;
uint32_t g_playerObjects;

std::vector<Bot> g_bots;
int g_botCounter;
bool g_active;
std::chrono::steady_clock::time_point g_replayStart;
uint32_t g_spreadMs;

class BitReader
{
public:
	BitReader(const uint8_t* data, size_t size)
		: m_data(data), m_bit(0), m_maxBit(size * 8)
	{
	}

	bool Read(int length, uint32_t* out)
	{
		if (length == 13 && rl::MessageBufferLengthHack::GetState())
		{
			length = 16;
		}

		if (m_bit + length > m_maxBit)
		{
			return false;
		}

		uint32_t value = 0;

		for (int i = 0; i < length; i++)
		{
			auto byte = m_data[(m_bit + i) >> 3];
			value = (value << 1) | ((byte >> (7 - ((m_bit + i) & 7))) & 1);
		}

		m_bit += length;
		*out = value;

		return true;
	}

	bool ReadBytes(std::vector<uint8_t>& out, uint32_t byteCount)
	{
		if (m_bit + (byteCount * 8) > m_maxBit)
		{
			return false;
		}

		out.resize(byteCount);

		for (uint32_t i = 0; i < byteCount; i++)
		{
			uint32_t value;
			Read(8, &value);
			out[i] = uint8_t(value);
		}

		return true;
	}

	bool IsAtEnd() const
	{
		return m_bit >= m_maxBit;
	}

private:
	const uint8_t* m_data;
	size_t m_bit;
	size_t m_maxBit;
};

bool ParsePacketInto(const uint8_t* data, uint32_t len, std::vector<RecMsg>& out, std::map<uint16_t, uint8_t>& typesById)
{
	BitReader reader(data, len);

	while (!reader.IsAtEnd())
	{
		uint32_t dataType;

		if (!reader.Read(3, &dataType))
		{
			return false;
		}

		if (dataType == 7)
		{
			break;
		}

		if (dataType == 1 || dataType == 2)
		{
			uint32_t uniqifier, objectId, creationToken = 0, entityType = 0, length;

			if (!reader.Read(16, &uniqifier) || !reader.Read(13, &objectId))
			{
				return false;
			}

			if (dataType == 1)
			{
				if (!reader.Read(32, &creationToken) || !reader.Read(kNetObjectTypeBitLength, &entityType))
				{
					return false;
				}

				typesById[uint16_t(objectId)] = uint8_t(entityType);
			}

			if (!reader.Read(12, &length))
			{
				return false;
			}

			RecMsg msg;
			msg.type = uint8_t(dataType);
			msg.objectId = uint16_t(objectId);
			msg.creationToken = creationToken;
			msg.entityType = uint8_t(entityType);

			if (!reader.ReadBytes(msg.payload, length))
			{
				return false;
			}

			out.push_back(std::move(msg));
		}
		else if (dataType == 3)
		{
			uint32_t objectId, uniqifier;

			if (!reader.Read(13, &objectId) || !reader.Read(16, &uniqifier))
			{
				return false;
			}

			RecMsg msg;
			msg.type = 3;
			msg.objectId = uint16_t(objectId);
			msg.creationToken = 0;
			msg.entityType = 0;

			out.push_back(std::move(msg));
		}
		else if (dataType == 4)
		{
			uint32_t clientId, objectId;

			if (!reader.Read(16, &clientId) || !reader.Read(13, &objectId))
			{
				return false;
			}
		}
		else if (dataType == 5 || dataType == 6)
		{
			uint32_t value;

			if (!reader.Read(32, &value))
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool LoadRecording(const std::string& fileName)
{
	auto file = fopen(fileName.c_str(), "rb");

	if (!file)
	{
		console::PrintError("replay", "Couldn't open %s for reading.\n", fileName);
		return false;
	}

	RecHeader header;

	if (fread(&header, sizeof(header), 1, file) != 1 || memcmp(header.magic, "RDSR", 4) != 0 || header.version != kRecVersion)
	{
		console::PrintError("replay", "%s is not a valid clone recording.\n", fileName);
		fclose(file);
		return false;
	}

	bool recordedLengthHack = (header.flags & kRecFlagLengthHack) != 0;

	if (recordedLengthHack != fx::IsLengthHack())
	{
		console::PrintError("replay", "%s was recorded with onesync_enableBeyond %s, this server runs with it %s - object ids would be read at the wrong bit width.\n",
			fileName, recordedLengthHack ? "on" : "off", fx::IsLengthHack() ? "on" : "off");
		fclose(file);
		return false;
	}

	std::vector<RecFrame> frames;
	std::map<uint16_t, uint8_t> typesById;
	std::vector<uint8_t> payload;

	uint32_t lastTime = 0;
	uint32_t badPackets = 0;

	while (true)
	{
		RecEntry entry;

		if (fread(&entry, sizeof(entry), 1, file) != 1)
		{
			break;
		}

		if (entry.length > 1024 * 1024)
		{
			break;
		}

		payload.resize(entry.length);

		if (entry.length && fread(payload.data(), 1, entry.length, file) != entry.length)
		{
			break;
		}

		lastTime = entry.timeMs;

		if (entry.type != 1)
		{
			continue;
		}

		RecFrame frame;
		frame.timeMs = entry.timeMs;

		if (!ParsePacketInto(payload.data(), entry.length, frame.msgs, typesById))
		{
			badPackets++;
			continue;
		}

		if (!frame.msgs.empty())
		{
			frames.push_back(std::move(frame));
		}
	}

	fclose(file);

	std::set<uint16_t> playerObjects;

	for (auto& [objectId, type] : typesById)
	{
		if (type == uint8_t(fx::sync::NetObjEntityType::Player))
		{
			playerObjects.insert(objectId);
		}
	}

	if (badPackets)
	{
		console::PrintError("replay", "%u of %d packets failed to parse.\n", badPackets, int(frames.size() + badPackets));
	}

	if (playerObjects.empty())
	{
		console::PrintError("replay", "%s has no player ped with a create - nothing to replay.\n", fileName);

		if (typesById.empty())
		{
			console::PrintError("replay", "No creates were parsed at all - the file layout doesn't match what this build expects.\n");
		}
		else
		{
			std::map<uint8_t, uint32_t> seen;

			for (auto& [objectId, type] : typesById)
			{
				seen[type]++;
			}

			console::PrintError("replay", "Types that were found instead:\n");

			for (auto& [type, count] : seen)
			{
				console::PrintError("replay", "  %s x%u\n", fx::sync::GetNetObjEntityName(uint16_t(type)), count);
			}
		}

		return false;
	}

	std::vector<RecFrame> filtered;

	for (auto& frame : frames)
	{
		RecFrame kept;
		kept.timeMs = frame.timeMs;

		for (auto& msg : frame.msgs)
		{
			if (playerObjects.find(msg.objectId) != playerObjects.end())
			{
				kept.msgs.push_back(std::move(msg));
			}
		}

		if (!kept.msgs.empty())
		{
			filtered.push_back(std::move(kept));
		}
	}

	g_frames = std::move(filtered);
	g_loadedPath = fileName;
	g_duration = lastTime;
	g_playerObjects = uint32_t(playerObjects.size());

	uint32_t creates = 0, syncs = 0, removes = 0;

	for (auto& frame : g_frames)
	{
		for (auto& msg : frame.msgs)
		{
			if (msg.type == 1) creates++;
			else if (msg.type == 2) syncs++;
			else removes++;
		}
	}

	console::Printf("replay", "Loaded %s: %d frames over %.1f s, %u player objects (%u creates, %u syncs, %u removes).\n",
		fileName,
		int(g_frames.size()),
		g_duration / 1000.0,
		g_playerObjects,
		creates,
		syncs,
		removes);

	return true;
}

uint16_t MapObjectId(Bot& bot, const fwRefContainer<fx::ServerGameState>& sgs, const RecMsg& msg)
{
	auto it = bot.objectIds.find(msg.objectId);

	if (it != bot.objectIds.end())
	{
		return it->second;
	}

	if (msg.type != 1)
	{
		return 0;
	}

	std::vector<uint16_t> freeIds;
	sgs->GetFreeObjectIds(bot.client, 1, freeIds);

	if (freeIds.empty())
	{
		return 0;
	}

	bot.objectIds[msg.objectId] = freeIds[0];
	bot.uniqifiers[msg.objectId] = uint16_t((rand() % 0xFFFE) + 1);

	return freeIds[0];
}

void EmitFrame(Bot& bot, const fwRefContainer<fx::ServerGameState>& sgs, const RecFrame& frame)
{
	rl::MessageBuffer packet(kPacketBufferSize);

	packet.Write(3, 5);
	packet.Write<uint32_t>(32, uint32_t(msec().count()));

	bool wroteAny = false;

	for (auto& msg : frame.msgs)
	{
		if (msg.type == 1 && bot.looped)
		{
			continue;
		}

		auto objectId = MapObjectId(bot, sgs, msg);

		if (!objectId)
		{
			continue;
		}

		auto uniqifier = bot.uniqifiers[msg.objectId];

		if (msg.type == 1)
		{
			packet.Write(3, 1);
			packet.Write(16, uniqifier);
			packet.Write(13, objectId);
			packet.Write<uint32_t>(32, msg.creationToken);
			packet.Write(kNetObjectTypeBitLength, msg.entityType);
			packet.Write(12, uint16_t(msg.payload.size()));

			if (!msg.payload.empty())
			{
				packet.WriteBits(msg.payload.data(), int(msg.payload.size() * 8));
			}
		}
		else if (msg.type == 2)
		{
			packet.Write(3, 2);
			packet.Write(16, uniqifier);
			packet.Write(13, objectId);
			packet.Write(12, uint16_t(msg.payload.size()));

			if (!msg.payload.empty())
			{
				packet.WriteBits(msg.payload.data(), int(msg.payload.size() * 8));
			}
		}
		else
		{
			packet.Write(3, 3);
			packet.Write(13, objectId);
			packet.Write(16, uniqifier);

			bot.objectIds.erase(msg.objectId);
		}

		wroteAny = true;
	}

	if (!wroteAny)
	{
		return;
	}

	packet.Write(3, 7);

	auto length = (packet.GetCurrentBit() / 8) + 1;

	sgs->InjectClonePacket(bot.client, packet.GetBuffer().data(), length);

	bot.emitted++;
}

void Tick()
{
	std::lock_guard _(g_mutex);

	if (!g_active || g_frames.empty() || !g_serverInstance)
	{
		return;
	}

	auto sgs = g_serverInstance->GetComponent<fx::ServerGameState>();

	if (!sgs.GetRef())
	{
		return;
	}

	auto now = uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - g_replayStart).count());

	for (auto& bot : g_bots)
	{
		if (now < bot.startOffset)
		{
			continue;
		}

		auto elapsed = now - bot.startOffset;
		auto loops = (g_duration > 0) ? (elapsed / g_duration) : 0;
		auto localTime = (g_duration > 0) ? (elapsed % g_duration) : elapsed;

		if (loops != bot.lastLoop)
		{
			bot.lastLoop = loops;
			bot.cursor = 0;
			bot.looped = true;
		}

		while (bot.cursor < g_frames.size() && g_frames[bot.cursor].timeMs <= localTime)
		{
			EmitFrame(bot, sgs, g_frames[bot.cursor]);
			bot.cursor++;
		}
	}
}

void StopReplay(bool quiet)
{
	if (!g_active)
	{
		if (!quiet)
		{
			console::PrintError("replay", "Not replaying.\n");
		}

		return;
	}

	auto clientRegistry = g_serverInstance->GetComponent<fx::ClientRegistry>();
	auto events = g_serverInstance->GetComponent<fx::ServerEventComponent>();
	auto sgs = g_serverInstance->GetComponent<fx::ServerGameState>();

	for (auto& bot : g_bots)
	{
		if (!bot.client)
		{
			continue;
		}

		if (!bot.objectIds.empty() && sgs.GetRef())
		{
			rl::MessageBuffer packet(kPacketBufferSize);

			packet.Write(3, 5);
			packet.Write<uint32_t>(32, uint32_t(msec().count()));

			for (auto& [recordedId, objectId] : bot.objectIds)
			{
				packet.Write(3, 3);
				packet.Write(13, objectId);
				packet.Write(16, bot.uniqifiers[recordedId]);
			}

			packet.Write(3, 7);

			sgs->InjectClonePacket(bot.client, packet.GetBuffer().data(), (packet.GetCurrentBit() / 8) + 1);
		}

		events->TriggerClientEvent("onPlayerDropped", std::optional<std::string_view>(), bot.client->GetNetId(), bot.client->GetName(), uint32_t(bot.virtualSlot));

		clientRegistry->RemoveClient(bot.client);
	}

	auto count = g_bots.size();

	g_bots.clear();
	g_active = false;

	console::Printf("replay", "Stopped replaying %d bots.\n", int(count));
}

uint16_t NextFreeSlot()
{
	std::set<uint16_t> used;

	for (auto& bot : g_bots)
	{
		used.insert(bot.virtualSlot);
	}

	uint16_t slot = 0;

	while (slot == kLocalPlayerSlot || slot == kSentinelSlot || used.find(slot) != used.end())
	{
		slot++;
	}

	return slot;
}

static int GetPlayerPopulation()
{
	auto clientRegistry = g_serverInstance->GetComponent<fx::ClientRegistry>();

	int population = 0;

	clientRegistry->ForAllClients([&population](const fx::ClientSharedPtr& client)
	{
		if (client->GetSlotId() != -1)
		{
			population++;
		}
	});

	return population;
}

int AddBots(int count, uint32_t spreadMs, uint32_t baseOffset)
{
	auto clientRegistry = g_serverInstance->GetComponent<fx::ClientRegistry>();
	auto events = g_serverInstance->GetComponent<fx::ServerEventComponent>();

	constexpr int kTrackableSlots = int(kGamePlayerCap) - 2;
	constexpr int kMaxPopulation = kTrackableSlots + 1;

	int population = GetPlayerPopulation();
	int added = 0;

	for (int i = 0; i < count; i++)
	{
		if (population >= kMaxPopulation)
		{
			console::PrintError("replay", "Reached the maximum concurrent player count (%d). Each client can only track %d others, as slots %d and %d are reserved.\n", kMaxPopulation, kTrackableSlots, int(kLocalPlayerSlot), int(kSentinelSlot));
			break;
		}

		auto slot = NextFreeSlot();

		if (slot >= kGamePlayerCap)
		{
			console::PrintError("replay", "Ran out of client slots (game player cap is %d).\n", int(kGamePlayerCap));
			break;
		}

		auto index = g_botCounter++;
		auto client = clientRegistry->MakeFakeClient(fmt::sprintf("replay:%d", index), fmt::sprintf("Bot %d", index + 1), kFakePeerBase + index);

		if (client->GetSlotId() == -1)
		{
			console::PrintError("replay", "Ran out of server slots.\n");
			clientRegistry->RemoveClient(client);
			break;
		}

		Bot bot;
		bot.client = client;
		bot.cursor = 0;
		bot.startOffset = baseOffset + (uint32_t(i) * spreadMs);
		bot.lastLoop = 0;
		bot.virtualSlot = slot;
		bot.looped = false;
		bot.emitted = 0;

		events->TriggerClientEvent("onPlayerJoining", std::optional<std::string_view>(), client->GetNetId(), client->GetName(), uint32_t(bot.virtualSlot));

		console::Printf("replay", "  %s: netId %d, client slot %d, server slot %d, starts at %u ms\n",
			client->GetName(),
			int(client->GetNetId()),
			int(bot.virtualSlot),
			client->GetSlotId(),
			bot.startOffset);

		g_bots.push_back(std::move(bot));

		population++;
		added++;
	}

	return added;
}

void StartReplay(int count, uint32_t spreadMs)
{
	std::lock_guard _(g_mutex);

	if (g_active)
	{
		console::PrintError("replay", "Already replaying %d bots - use net_repAdd to add more, or net_repStop first.\n", int(g_bots.size()));
		return;
	}

	if (g_frames.empty())
	{
		console::PrintError("replay", "No recording loaded. Use net_repLoad first.\n");
		return;
	}

	if (count < 1 || count > 256)
	{
		console::PrintError("replay", "Bot count has to be between 1 and 256.\n");
		return;
	}

	g_bots.clear();
	g_botCounter = 0;
	g_replayStart = std::chrono::steady_clock::now();
	g_spreadMs = spreadMs;

	console::Printf("replay", "Replaying %d bots from %s, %u ms apart.\n", count, g_loadedPath, spreadMs);

	if (!AddBots(count, spreadMs, 0))
	{
		console::PrintError("replay", "Couldn't create any bots.\n");
		return;
	}

	g_active = true;
}
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		g_serverInstance = instance;

		instance->GetComponent<fx::GameServer>()->OnSyncTick.Connect([]()
		{
			Tick();
		});

		instance->GetComponent<fx::ClientRegistry>()->OnConnectedClient.Connect([](fx::Client* client)
		{
			std::lock_guard _(g_mutex);

			if (!g_active || g_bots.empty())
			{
				return;
			}

			auto events = g_serverInstance->GetComponent<fx::ServerEventComponent>();
			auto target = fmt::sprintf("%d", client->GetNetId());

			for (auto& bot : g_bots)
			{
				events->TriggerClientEvent("onPlayerJoining", std::string_view{ target }, bot.client->GetNetId(), bot.client->GetName(), uint32_t(bot.virtualSlot));
			}

			console::Printf("replay", "Announced %d bots to %s.\n", int(g_bots.size()), client->GetName());
		});

		static auto repLoad = instance->AddCommand("net_repLoad", [](const std::string& fileName)
		{
			std::lock_guard _(g_mutex);

			if (g_active)
			{
				console::PrintError("replay", "Stop the current replay first.\n");
				return;
			}

			LoadRecording(fileName);
		});

		static auto repStart = instance->AddCommand("net_repStart", [](int count)
		{
			uint32_t spread = 500;

			{
				std::lock_guard _(g_mutex);

				if (!g_frames.empty() && count > 1)
				{
					spread = std::max<uint32_t>(250, g_duration / uint32_t(count));
				}
			}

			StartReplay(count, spread);
		});

		static auto repStartSpread = instance->AddCommand("net_repStart", [](int count, int spreadMs)
		{
			StartReplay(count, uint32_t(std::max(0, spreadMs)));
		});

		static auto repAdd = instance->AddCommand("net_repAdd", [](int count)
		{
			std::lock_guard _(g_mutex);

			if (!g_active)
			{
				console::PrintError("replay", "Not replaying. Use net_repStart first.\n");
				return;
			}

			if (count < 1 || count > 256)
			{
				console::PrintError("replay", "Bot count has to be between 1 and 256.\n");
				return;
			}

			auto now = uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - g_replayStart).count());

			console::Printf("replay", "Adding %d bots.\n", count);

			auto added = AddBots(count, g_spreadMs, now);

			console::Printf("replay", "Now replaying %d bots (%d added).\n", int(g_bots.size()), added);
		});

		static auto repStop = instance->AddCommand("net_repStop", []()
		{
			std::lock_guard _(g_mutex);
			StopReplay(false);
		});

		static auto repStatus = instance->AddCommand("net_repStatus", []()
		{
			std::lock_guard _(g_mutex);

			if (!g_loadedPath.empty())
			{
				console::Printf("replay", "Loaded %s: %d frames, %.1f s.\n", g_loadedPath, int(g_frames.size()), g_duration / 1000.0);
			}
			else
			{
				console::Printf("replay", "No recording loaded.\n");
			}

			if (!g_active)
			{
				console::Printf("replay", "Not replaying.\n");
				return;
			}

			auto now = uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - g_replayStart).count());

			console::Printf("replay", "Replaying %d bots, %.1f s in.\n", int(g_bots.size()), now / 1000.0);

			auto sgs = g_serverInstance->GetComponent<fx::ServerGameState>();

			for (auto& bot : g_bots)
			{
				console::Printf("replay", "  %s: netId %d client slot %d, frame %d/%d, %u packets, %d objects\n",
					bot.client->GetName(),
					int(bot.client->GetNetId()),
					int(bot.virtualSlot),
					int(bot.cursor),
					int(g_frames.size()),
					bot.emitted,
					int(bot.objectIds.size()));

				if (!sgs.GetRef())
				{
					continue;
				}

				std::set<uint16_t> wanted;

				for (auto& [recordedId, objectId] : bot.objectIds)
				{
					wanted.insert(objectId);
				}

				sgs->ForAllSyncEntities([&wanted](const fx::sync::SyncEntityPtr& entity)
				{
					if (wanted.find(uint16_t(entity->handle)) == wanted.end() || !entity->syncTree)
					{
						return;
					}

					float position[3] = { 0 };
					entity->syncTree->GetPosition(position);

					size_t relevant = 0;

					{
						std::shared_lock _(entity->guidMutex);
						relevant = entity->relevantTo.count();
					}

					auto owner = entity->GetClient();

					console::Printf("replay", "    [obj:%d] %s at %.1f %.1f %.1f, owner %s, relevant to %d clients, synced %s\n",
						int(entity->handle),
						fx::sync::GetNetObjEntityName(uint16_t(entity->type)),
						position[0],
						position[1],
						position[2],
						owner ? owner->GetName() : std::string{ "(nobody)" },
						int(relevant),
						entity->hasSynced ? "yes" : "no");
				});
			}
		});
	});
});
#endif
