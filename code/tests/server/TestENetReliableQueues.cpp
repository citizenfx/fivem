#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include <enet/enet.h>

extern "C" int enet_initialize(void)
{
	return 0;
}

extern "C" enet_uint32 enet_time_get(void)
{
	return 1000;
}

extern "C" enet_uint32 enet_host_random_seed(void)
{
	return 1;
}

extern "C" ENetSocket enet_socket_create(ENetSocketType)
{
	return 1;
}

extern "C" int enet_socket_bind(ENetSocket, const ENetAddress*)
{
	return -1;
}

extern "C" int enet_socket_get_address(ENetSocket, ENetAddress*)
{
	return -1;
}

extern "C" int enet_socket_send(ENetSocket, const ENetAddress*, const ENetBuffer* buffers, size_t count)
{
	size_t bytes = 0;
	for (size_t i = 0; i < count; ++i)
	{
		bytes += buffers[i].dataLength;
	}
	return static_cast<int>(bytes);
}

extern "C" int enet_socket_receive(ENetSocket, ENetAddress*, ENetBuffer*, size_t)
{
	return 0;
}

extern "C" int enet_socket_wait(ENetSocket, enet_uint32*, enet_uint32)
{
	return 0;
}

extern "C" int enet_socket_set_option(ENetSocket, ENetSocketOption, int)
{
	return 0;
}

extern "C" void enet_socket_destroy(ENetSocket)
{
}

TEST_CASE("ENet reliable ACK lookup avoids quadratic list scans")
{
	constexpr size_t kCommandCount = 32768;

	ENetHost host{};
	ENetPeer peer{};
	ENetChannel channels[2]{};
	std::vector<ENetOutgoingCommand> commands(kCommandCount);
	std::vector<ENetOutgoingCommand*> lookupBuckets(kCommandCount);

	peer.host = &host;
	peer.channels = channels;
	peer.channelCount = std::size(channels);
	peer.reliableCommandLookup = lookupBuckets.data();
	peer.reliableCommandLookupBucketCount = lookupBuckets.size();
	enet_list_clear(&peer.sentReliableCommands);
	enet_list_clear(&peer.outgoingReliableCommands);

	for (size_t i = 0; i < commands.size(); ++i)
	{
		auto& command = commands[i];
		command.command.header.command = ENET_PROTOCOL_COMMAND_SEND_RELIABLE | ENET_PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
		command.command.header.channelID = static_cast<enet_uint8>(i % std::size(channels));
		enet_peer_setup_outgoing_command(&peer, &command);
		enet_list_insert(
			enet_list_end(&peer.sentReliableCommands),
			enet_list_remove(&command.outgoingCommandList));
		enet_peer_mark_reliable_command_sent(&peer, &command);
	}

	REQUIRE(peer.reliableCommandCount == kCommandCount);
	REQUIRE(peer.sentReliableCommandCount == kCommandCount);
	REQUIRE(peer.reliableCommandLookupBucketCount >= kCommandCount / 2);

	const size_t probesBefore = host.reliableCommandLookupProbes;
	for (size_t i = commands.size(); i-- > 0;)
	{
		int wasSent = 0;
		auto found = enet_peer_find_reliable_command(
			&peer,
			commands[i].reliableSequenceNumber,
			commands[i].command.header.channelID,
			&wasSent);

		REQUIRE(found == &commands[i]);
		REQUIRE(wasSent == 1);
	}

	const size_t indexedProbes = host.reliableCommandLookupProbes - probesBefore;
	const size_t legacyLinearProbes = kCommandCount * (kCommandCount + 1) / 2;

	CHECK(host.reliableCommandLookups == kCommandCount);
	CHECK(host.reliableCommandLookupFallbacks == 0);
	CHECK(host.reliableCommandLookupMisses == 0);
	CHECK(indexedProbes == 49097);
	CHECK(indexedProbes < kCommandCount * 4);
	CHECK(legacyLinearProbes / indexedProbes > 1000);

	peer.reliableCommandTimeoutCursor = &commands[kCommandCount / 2];
	for (auto& command : commands)
	{
		enet_peer_untrack_reliable_command(&peer, &command);
		enet_list_remove(&command.outgoingCommandList);
	}

	CHECK(peer.reliableCommandCount == 0);
	CHECK(peer.sentReliableCommandCount == 0);
	CHECK(peer.reliableCommandTimeoutCursor == nullptr);
	peer.reliableCommandLookup = nullptr;
	peer.reliableCommandLookupBucketCount = 0;
}

TEST_CASE("ENet reliable queue scan work is bounded per peer")
{
	constexpr size_t kQueuedPackets = 10000;
	constexpr size_t kPeerScanBudget = 2048;

	ENetHost host{};
	ENetList queue{};
	std::vector<ENetOutgoingCommand> commands(kQueuedPackets);
	enet_list_clear(&queue);

	for (auto& command : commands)
	{
		enet_list_insert(enet_list_end(&queue), &command);
	}

	host.maximumReliableCommandScansPerPass = 4096;
	host.maximumReliableCommandScansPerPeer = kPeerScanBudget;
	host.reliableCommandScansRemaining = host.maximumReliableCommandScansPerPass;

	size_t peerScans = 0;
	size_t visitedCommands = 0;
	for (auto current = enet_list_begin(&queue); current != enet_list_end(&queue); current = enet_list_next(current))
	{
		if (!enet_host_consume_reliable_command_scan(&host, &peerScans))
		{
			break;
		}
		++visitedCommands;
	}

	const size_t legacyScans = kQueuedPackets;

	CHECK(visitedCommands == kPeerScanBudget);
	CHECK(host.reliableCommandScans == kPeerScanBudget);
	CHECK(host.reliableCommandPeerScanDeferrals == 1);
	CHECK(legacyScans / visitedCommands >= 4);
}

TEST_CASE("ENet reliable queue scan work is globally bounded across captured population")
{
	constexpr size_t kCapturedPeers = 589;
	constexpr size_t kCommandsPerPeer = 10000;
	constexpr size_t kHostScanBudget = 64 * 1024;
	constexpr size_t kPeerScanBudget = 2 * 1024;

	ENetHost host{};
	host.maximumReliableCommandScansPerPass = kHostScanBudget;
	host.maximumReliableCommandScansPerPeer = kPeerScanBudget;
	host.reliableCommandScansRemaining = kHostScanBudget;

	size_t visitedCommands = 0;
	for (size_t peer = 0; peer < kCapturedPeers; ++peer)
	{
		size_t peerScans = 0;
		for (size_t command = 0; command < kCommandsPerPeer; ++command)
		{
			if (!enet_host_consume_reliable_command_scan(&host, &peerScans))
			{
				break;
			}
			++visitedCommands;
		}
	}

	const size_t legacyScans = kCapturedPeers * kCommandsPerPeer;
	CHECK(visitedCommands == kHostScanBudget);
	CHECK(host.reliableCommandScans == kHostScanBudget);
	CHECK(host.reliableCommandScanBudgetExhaustions == 1);
	CHECK(legacyScans / visitedCommands >= 89);
}

namespace
{
ENetPeer* CreateIsolatedPeer(ENetHost* host)
{
	ENetAddress address{};
	auto peer = enet_host_connect(host, &address, 2, 0);
	REQUIRE(peer != nullptr);
	peer->state = ENET_PEER_STATE_CONNECTED;
	peer->outgoingPeerID = peer->incomingPeerID;
	peer->lastReceiveTime = enet_time_get();
	return peer;
}

void QueueReliableTestPackets(ENetPeer* peer, size_t count, size_t bytes = 8)
{
	const std::vector<uint8_t> payload(bytes, 42);
	for (size_t i = 0; i < count; ++i)
	{
		REQUIRE(enet_peer_send(peer, 0, enet_packet_create(payload.data(), payload.size(), ENET_PACKET_FLAG_RELIABLE)) == 0);
	}
}
}

TEST_CASE("ENet reliable scan resumes without bypassing a closed sequence window")
{
	auto host = enet_host_create(nullptr, 1, 2, 0, 0);
	REQUIRE(host != nullptr);
	auto peer = CreateIsolatedPeer(host);
	enet_host_flush(host);
	peer->channels[0].outgoingReliableSequenceNumber = 9 * ENET_PEER_RELIABLE_WINDOW_SIZE - 1;
	peer->channels[0].usedReliableWindows = 1;
	peer->channels[0].reliableWindows[0] = 1;
	host->maximumReliableCommandScansPerPass = 1;
	host->maximumReliableCommandScansPerPeer = 1;
	QueueReliableTestPackets(peer, 16);
	const auto sentBefore = peer->sentReliableCommandCount;
	for (size_t pass = 0; pass < 8; ++pass)
	{
		enet_host_flush(host);
	}
	CHECK(peer->sentReliableCommandCount == sentBefore);
	CHECK(peer->outgoingReliableCommandCount == 16);
	enet_host_destroy(host);
}

TEST_CASE("ENet reliable per-peer scan budget survives packet assembly iterations")
{
	auto host = enet_host_create(nullptr, 1, 2, 0, 0);
	REQUIRE(host != nullptr);
	auto peer = CreateIsolatedPeer(host);
	host->maximumReliableCommandScansPerPeer = 32;
	QueueReliableTestPackets(peer, 1000, 256);
	const auto before = host->reliableCommandScans;
	enet_host_flush(host);
	CHECK(host->reliableCommandScans - before <= 32);
	CHECK(peer->sentReliableCommandCount > 0);
	enet_host_destroy(host);
}

TEST_CASE("ENet reliable host budget resumes after the serviced peer on sparse hosts")
{
	auto host = enet_host_create(nullptr, 2081, 2, 0, 0);
	REQUIRE(host != nullptr);
	host->maximumReliableCommandScansPerPass = 32;
	host->maximumReliableCommandScansPerPeer = 8;
	for (size_t i = 0; i < 8; ++i)
	{
		auto peer = CreateIsolatedPeer(host);
		QueueReliableTestPackets(peer, 64);
		peer->reliableDataInTransit = 65536;
	}
	auto healthy = CreateIsolatedPeer(host);
	QueueReliableTestPackets(healthy, 1);
	host->reliableCommandScanPeerCursor = 100;
	for (size_t pass = 0; pass < 3; ++pass)
	{
		enet_host_flush(host);
	}
	CHECK(healthy->sentReliableCommandCount > 0);
	enet_host_destroy(host);
}
