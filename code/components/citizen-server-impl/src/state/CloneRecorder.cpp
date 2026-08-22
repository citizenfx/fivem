#include "StdInc.h"

#include <state/CloneRecorder.h>

#ifdef STATE_RDR3

#include <ServerInstanceBase.h>

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

enum RecEntryType : uint8_t
{
	kRecClonePacket = 1,
	kRecClientInfo = 2,
};

constexpr size_t kSnapshotBufferSize = 4096;

std::mutex g_recMutex;
FILE* g_recFile;
std::string g_recPath;
std::chrono::steady_clock::time_point g_recStart;
uint32_t g_recPackets;
uint32_t g_recClients;
uint64_t g_recBytes;
std::set<uint16_t> g_seenClients;

fx::ServerInstanceBase* g_serverInstance;

uint32_t ElapsedMs()
{
	return uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - g_recStart).count());
}

void WriteEntry(uint8_t type, uint16_t netId, const void* payload, uint32_t length)
{
	if (!g_recFile)
	{
		return;
	}

	RecEntry entry{ type, ElapsedMs(), netId, length };

	fwrite(&entry, sizeof(entry), 1, g_recFile);

	if (length)
	{
		fwrite(payload, 1, length, g_recFile);
	}

	g_recBytes += sizeof(entry) + length;
}

void WriteClientInfo(uint16_t netId, uint16_t slotId, std::string_view name)
{
	uint8_t payload[2 + 1 + 255];
	uint8_t nameLength = uint8_t(std::min<size_t>(name.size(), 255));

	memcpy(&payload[0], &slotId, 2);
	payload[2] = nameLength;
	memcpy(&payload[3], name.data(), nameLength);

	WriteEntry(kRecClientInfo, netId, payload, 3 + nameLength);

	g_recClients++;
}

uint32_t WriteEntitySnapshot()
{
	if (!g_serverInstance)
	{
		return 0;
	}

	auto sgs = g_serverInstance->GetComponent<fx::ServerGameState>();

	if (!sgs.GetRef())
	{
		return 0;
	}

	uint32_t written = 0;

	sgs->ForAllSyncEntities([&written](const fx::sync::SyncEntityPtr& entity)
	{
		auto client = entity->GetClient();

		if (!client || !entity->syncTree)
		{
			return;
		}

		auto unparse = [&entity](int syncType, rl::MessageBuffer& buffer) -> uint32_t
		{
			fx::sync::SyncUnparseState state(buffer);
			state.syncType = syncType;
			state.objType = 0;
			state.timestamp = 0;
			state.targetSlotId = 0;
			state.lastFrameIndex = 0;
			state.isFirstUpdate = false;
			state.writeNodeLengths = true;

			if (!entity->syncTree->Unparse(state))
			{
				return 0;
			}

			auto length = (buffer.GetCurrentBit() / 8) + 1;

			return (length < 4096) ? length : 0;
		};

		rl::MessageBuffer createBuffer(kSnapshotBufferSize);
		auto createLength = unparse(1, createBuffer);

		if (!createLength)
		{
			return;
		}

		rl::MessageBuffer syncBuffer(kSnapshotBufferSize);
		auto syncLength = unparse(2, syncBuffer);

		rl::MessageBuffer packet((kSnapshotBufferSize * 2) + 32);
		packet.Write(3, 1);
		packet.Write(16, entity->uniqifier);
		packet.Write(13, uint16_t(entity->handle));
		packet.Write(32, entity->creationToken);
		packet.Write(kNetObjectTypeBitLength, uint8_t(entity->type));
		packet.Write(12, uint16_t(createLength));
		packet.WriteBits(createBuffer.GetBuffer().data(), createLength * 8);

		if (syncLength)
		{
			packet.Write(3, 2);
			packet.Write(16, entity->uniqifier);
			packet.Write(13, uint16_t(entity->handle));
			packet.Write(12, uint16_t(syncLength));
			packet.WriteBits(syncBuffer.GetBuffer().data(), syncLength * 8);
		}

		packet.Write(3, 7);

		auto netId = uint16_t(client->GetNetId());

		if (g_seenClients.insert(netId).second)
		{
			WriteClientInfo(netId, uint16_t(client->GetSlotId()), client->GetName());
		}

		WriteEntry(kRecClonePacket, netId, packet.GetBuffer().data(), uint32_t((packet.GetCurrentBit() / 8) + 1));

		written++;
	});

	return written;
}

void StopRecording(bool quiet)
{
	if (!g_recFile)
	{
		if (!quiet)
		{
			console::PrintError("recorder", "Not recording.\n");
		}

		return;
	}

	auto elapsed = ElapsedMs();

	fclose(g_recFile);
	g_recFile = nullptr;
	g_seenClients.clear();

	console::Printf("recorder", "Stopped recording %s: %u packets from %u clients, %llu bytes in %.1f s.\n",
		g_recPath,
		g_recPackets,
		g_recClients,
		(unsigned long long)g_recBytes,
		elapsed / 1000.0);
	console::Printf("recorder", "Verify it with: net_recDump %s\n", g_recPath);
}

void StartRecording(const std::string& fileName)
{
	std::lock_guard _(g_recMutex);

	if (g_recFile)
	{
		console::PrintError("recorder", "Already recording to %s - stop it first.\n", g_recPath);
		return;
	}

	auto file = fopen(fileName.c_str(), "wb");

	if (!file)
	{
		console::PrintError("recorder", "Couldn't open %s for writing.\n", fileName);
		return;
	}

	setvbuf(file, nullptr, _IOFBF, 1024 * 1024);

	RecHeader header{};
	header.magic[0] = 'R';
	header.magic[1] = 'D';
	header.magic[2] = 'S';
	header.magic[3] = 'R';
	header.version = kRecVersion;
	header.flags = fx::IsLengthHack() ? kRecFlagLengthHack : 0;
	header.startWallClockMs = uint64_t(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

	fwrite(&header, sizeof(header), 1, file);

	g_recFile = file;
	g_recPath = fileName;
	g_recStart = std::chrono::steady_clock::now();
	g_recPackets = 0;
	g_recClients = 0;
	g_recBytes = sizeof(header);
	g_seenClients.clear();

	auto snapshot = WriteEntitySnapshot();

	console::Printf("recorder", "Recording incoming clone packets to %s (%u entities in snapshot).\n", fileName, snapshot);
}

struct ObjectInfo
{
	uint16_t netId;
	uint16_t uniqifier;
	uint8_t entityType;
	uint32_t creates;
	uint32_t syncs;
	uint64_t payloadBytes;
};

struct DumpStats
{
	uint32_t packets = 0;
	uint32_t badPackets = 0;
	uint32_t creates = 0;
	uint32_t syncs = 0;
	uint32_t removes = 0;
	uint32_t takeovers = 0;
	uint32_t timestamps = 0;
	uint32_t indices = 0;
	uint64_t payloadBytes = 0;
	uint32_t firstTs = 0;
	uint32_t lastTs = 0;

	std::map<int, uint32_t> createsByType;
	std::map<uint16_t, ObjectInfo> objects;
	std::map<uint16_t, uint32_t> packetsByClient;
};

bool ParseClonePacket(const uint8_t* data, uint32_t len, uint16_t netId, DumpStats& stats, std::string& error, std::string* detail)
{
	rl::MessageBuffer buffer(data, len);

	bool end = false;

	while (!buffer.IsAtEnd() && !end)
	{
		uint8_t dataType;

		if (!buffer.Read<uint8_t>(3, &dataType))
		{
			error = "truncated data type";
			return false;
		}

		switch (dataType)
		{
			case 1:
			case 2:
			{
				uint16_t uniqifier, objectId, length;
				uint32_t creationToken = 0;
				uint8_t entityType = 0;

				if (!buffer.Read<uint16_t>(16, &uniqifier) || !buffer.Read<uint16_t>(13, &objectId))
				{
					error = "truncated clone header";
					return false;
				}

				if (dataType == 1)
				{
					if (!buffer.Read<uint32_t>(32, &creationToken) || !buffer.Read<uint8_t>(kNetObjectTypeBitLength, &entityType))
					{
						error = "truncated clone creation header";
						return false;
					}
				}

				if (!buffer.Read<uint16_t>(12, &length))
				{
					error = "truncated clone length";
					return false;
				}

				std::vector<uint8_t> cloneData(length);

				if (length && !buffer.ReadBits(cloneData.data(), length * 8))
				{
					error = fmt::sprintf("clone payload of %d bytes runs past the end of the packet", int(length));
					return false;
				}

				stats.payloadBytes += length;

				auto& object = stats.objects[objectId];
				object.netId = netId;
				object.uniqifier = uniqifier;
				object.payloadBytes += length;

				if (dataType == 1)
				{
					object.entityType = entityType;
					object.creates++;

					stats.creates++;
					stats.createsByType[entityType]++;
				}
				else
				{
					object.syncs++;
					stats.syncs++;
				}

				if (detail)
				{
					*detail += fmt::sprintf("\n    %s [obj:%d] uniq %d len %d%s",
						dataType == 1 ? "create" : "sync  ",
						int(objectId),
						int(uniqifier),
						int(length),
						dataType == 1 ? fmt::sprintf(" type %s", fx::sync::GetNetObjEntityName(entityType)) : std::string{});
				}

				break;
			}
			case 3:
			{
				uint16_t objectId, uniqifier;

				if (!buffer.Read<uint16_t>(13, &objectId) || !buffer.Read<uint16_t>(16, &uniqifier))
				{
					error = "truncated remove";
					return false;
				}

				stats.removes++;

				if (detail)
				{
					*detail += fmt::sprintf("\n    remove [obj:%d] uniq %d", int(objectId), int(uniqifier));
				}

				break;
			}
			case 4:
			{
				uint16_t clientId, objectId;

				if (!buffer.Read<uint16_t>(16, &clientId) || !buffer.Read<uint16_t>(13, &objectId))
				{
					error = "truncated takeover";
					return false;
				}

				stats.takeovers++;

				if (detail)
				{
					*detail += fmt::sprintf("\n    takeover [obj:%d] to %d", int(objectId), int(clientId));
				}

				break;
			}
			case 5:
			{
				uint32_t timestamp;

				if (!buffer.Read<uint32_t>(32, &timestamp))
				{
					error = "truncated timestamp";
					return false;
				}

				if (!stats.firstTs)
				{
					stats.firstTs = timestamp;
				}

				stats.lastTs = timestamp;
				stats.timestamps++;

				break;
			}
			case 6:
			{
				uint32_t index;

				if (!buffer.Read<uint32_t>(32, &index))
				{
					error = "truncated index";
					return false;
				}

				stats.indices++;

				break;
			}
			case 7:
				end = true;
				break;
			default:
				error = fmt::sprintf("unknown data type %d", int(dataType));
				return false;
		}
	}

	return true;
}

void DumpRecording(const std::string& fileName, int detailPackets)
{
	auto file = fopen(fileName.c_str(), "rb");

	if (!file)
	{
		console::PrintError("recorder", "Couldn't open %s for reading.\n", fileName);
		return;
	}

	RecHeader header{};

	if (fread(&header, sizeof(header), 1, file) != 1)
	{
		console::PrintError("recorder", "File is too small to hold a header.\n");
		fclose(file);
		return;
	}

	if (memcmp(header.magic, "RDSR", 4) != 0)
	{
		console::PrintError("recorder", "Not a clone recording (bad magic).\n");
		fclose(file);
		return;
	}

	if (header.version != kRecVersion)
	{
		console::PrintError("recorder", "Unsupported recording version %u (expected %u).\n", header.version, kRecVersion);
		fclose(file);
		return;
	}

	bool recordedLengthHack = (header.flags & kRecFlagLengthHack) != 0;

	if (recordedLengthHack != fx::IsLengthHack())
	{
		console::PrintWarning("recorder", "Recording was made with onesync_enableBeyond %s, this server runs with it %s - object ids use a different bit width, so the dump below will be garbage.\n",
			recordedLengthHack ? "on" : "off", fx::IsLengthHack() ? "on" : "off");
	}

	console::Printf("recorder", "^2%s^7\n", fileName);

	DumpStats stats;

	std::vector<uint8_t> payload;
	std::map<uint16_t, std::string> clients;

	uint32_t lastTime = 0;
	uint32_t entries = 0;
	uint32_t detailed = 0;
	bool truncated = false;

	while (true)
	{
		RecEntry entry;

		if (fread(&entry, sizeof(entry), 1, file) != 1)
		{
			break;
		}

		if (entry.length > 1024 * 1024)
		{
			console::PrintError("recorder", "Entry %u claims a length of %u bytes - file is corrupt.\n", entries, entry.length);
			truncated = true;
			break;
		}

		payload.resize(entry.length);

		if (entry.length && fread(payload.data(), 1, entry.length, file) != entry.length)
		{
			console::PrintError("recorder", "Entry %u is truncated (wanted %u bytes).\n", entries, entry.length);
			truncated = true;
			break;
		}

		entries++;
		lastTime = entry.timeMs;

		switch (entry.type)
		{
			case kRecClonePacket:
			{
				std::string error;
				std::string detail;
				bool wantDetail = int(detailed) < detailPackets;

				if (!ParseClonePacket(payload.data(), entry.length, entry.netId, stats, error, wantDetail ? &detail : nullptr))
				{
					stats.badPackets++;

					console::PrintError("recorder", "  [%u ms] packet %u from netId %d is unparseable: %s\n", entry.timeMs, stats.packets, int(entry.netId), error);
				}
				else if (wantDetail)
				{
					console::Printf("recorder", "  [%u ms] netId %d%s\n", entry.timeMs, int(entry.netId), detail);
					detailed++;
				}

				stats.packets++;
				stats.packetsByClient[entry.netId]++;

				break;
			}
			case kRecClientInfo:
			{
				if (entry.length < 3)
				{
					console::PrintError("recorder", "  malformed client entry at %u ms\n", entry.timeMs);
					break;
				}

				uint16_t slotId;
				memcpy(&slotId, payload.data(), 2);

				uint8_t nameLength = payload[2];
				std::string name(reinterpret_cast<char*>(payload.data() + 3), std::min<size_t>(nameLength, entry.length - 3));

				clients[entry.netId] = name;

				console::Printf("recorder", "  client netId %d slot %d %s\n", int(entry.netId), int(slotId), name);

				break;
			}
			default:
				console::PrintError("recorder", "Unknown entry type %d at %u ms - file is corrupt.\n", int(entry.type), entry.timeMs);
				truncated = true;
				break;
		}

		if (truncated)
		{
			break;
		}
	}

	fclose(file);

	console::Printf("recorder", "\n^3clone stream^7\n");
	console::Printf("recorder", "  duration      %.1f s (%u entries)\n", lastTime / 1000.0, entries);
	console::Printf("recorder", "  packets       %u (%u unparseable)\n", stats.packets, stats.badPackets);
	console::Printf("recorder", "  creates       %u\n", stats.creates);
	console::Printf("recorder", "  syncs         %u\n", stats.syncs);
	console::Printf("recorder", "  removes       %u\n", stats.removes);
	console::Printf("recorder", "  takeovers     %u\n", stats.takeovers);
	console::Printf("recorder", "  timestamps    %u\n", stats.timestamps);
	console::Printf("recorder", "  payload       %llu bytes\n", (unsigned long long)stats.payloadBytes);
	console::Printf("recorder", "  packet rate   %.1f/s\n", stats.packets / std::max(0.001, lastTime / 1000.0));
	console::Printf("recorder", "  client ts     %u ms span\n", stats.lastTs - stats.firstTs);
	console::Printf("recorder", "  unique objs   %d\n", int(stats.objects.size()));

	console::Printf("recorder", "\n^3packets by client^7\n");

	for (auto& [netId, count] : stats.packetsByClient)
	{
		auto it = clients.find(netId);

		console::Printf("recorder", "  netId %-5d %-24s %u packets\n", int(netId), it != clients.end() ? it->second : std::string{ "^1(no client info!)^7" }, count);
	}

	console::Printf("recorder", "\n^3creates by entity type^7\n");

	for (auto& [type, count] : stats.createsByType)
	{
		console::Printf("recorder", "  %-28s %u\n", fx::sync::GetNetObjEntityName(uint16_t(type)), count);
	}

	console::Printf("recorder", "\n^3player peds^7\n");

	bool anyPlayer = false;

	for (auto& [objectId, object] : stats.objects)
	{
		if (object.entityType != uint8_t(fx::sync::NetObjEntityType::Player) || object.creates == 0)
		{
			continue;
		}

		anyPlayer = true;

		console::Printf("recorder", "  [obj:%d] uniq %d owner netId %d - %u creates, %u syncs, %llu bytes\n",
			int(objectId),
			int(object.uniqifier),
			int(object.netId),
			object.creates,
			object.syncs,
			(unsigned long long)object.payloadBytes);
	}

	if (!anyPlayer)
	{
		console::PrintError("recorder", "  ^1none - this recording has no player ped and cannot drive a replay^7\n");
	}

	if (truncated)
	{
		console::PrintError("recorder", "\nThe recording is incomplete - it was likely not closed with net_recStop.\n");
	}
	else if (stats.badPackets == 0 && stats.packets > 0)
	{
		console::Printf("recorder", "\n^2Recording is complete and every packet re-parsed cleanly.^7\n");
	}
}
}

void CloneRecorder_OnClonePacket(const fx::ClientSharedPtr& client, const uint8_t* data, size_t len)
{
	std::lock_guard _(g_recMutex);

	if (!g_recFile)
	{
		return;
	}

	auto netId = uint16_t(client->GetNetId());

	if (g_seenClients.insert(netId).second)
	{
		WriteClientInfo(netId, uint16_t(client->GetSlotId()), client->GetName());
	}

	WriteEntry(kRecClonePacket, netId, data, uint32_t(len));

	g_recPackets++;
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		g_serverInstance = instance;

		static auto recStartAuto = instance->AddCommand("net_recStart", []()
		{
			StartRecording(fmt::sprintf("clonerec-%lld.bin", (long long)std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()));
		});

		static auto recStart = instance->AddCommand("net_recStart", [](const std::string& fileName)
		{
			StartRecording(fileName);
		});

		static auto recStop = instance->AddCommand("net_recStop", []()
		{
			std::lock_guard _(g_recMutex);
			StopRecording(false);
		});

		static auto recStatus = instance->AddCommand("net_recStatus", []()
		{
			std::lock_guard _(g_recMutex);

			if (!g_recFile)
			{
				console::Printf("recorder", "Not recording.\n");
				return;
			}

			console::Printf("recorder", "Recording to %s: %u packets from %u clients, %llu bytes, %.1f s.\n",
				g_recPath,
				g_recPackets,
				g_recClients,
				(unsigned long long)g_recBytes,
				ElapsedMs() / 1000.0);
		});

		static auto recDump = instance->AddCommand("net_recDump", [](const std::string& fileName)
		{
			DumpRecording(fileName, 0);
		});

		static auto recDumpDetail = instance->AddCommand("net_recDump", [](const std::string& fileName, int detailPackets)
		{
			DumpRecording(fileName, detailPackets);
		});
	});
});
#endif
