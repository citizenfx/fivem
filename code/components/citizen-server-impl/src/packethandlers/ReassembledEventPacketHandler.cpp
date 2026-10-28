#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>

#include <ByteReader.h>

#include "packethandlers/ReassembledEventPacketHandler.h"

#include "ClientDropReasons.h"
#include "EventReassemblyComponent.h"
#include "GameServer.h"
#include "KeyedRateLimiter.h"
#include "LatentEventAckDiagnostics.h"
#include <json.hpp>
#include "ReassembledEventPacket.h"
#include "ResourceManager.h"

namespace fx
{
std::shared_ptr<ConVar<bool>> g_enableNetEventReassemblyConVar;
}

namespace
{
void ObserveAck(const fx::ClientSharedPtr& client, const net::packet::ReassembledEventV2& packet,
	const fx::EventReassemblyAckInfo& info, bool softLimited, bool overflow)
{
	static fx::LatentEventAckLogBudget logBudget;
	static const std::string dataKey = "__latentAckDiagnostics";
	try
	{
		auto data = client->GetData(dataKey);
		if (!data)
		{
			client->SetData(dataKey, fx::LatentEventAckWindow{});
			data = client->GetData(dataKey);
		}
		auto& window = fx::AnyCast<fx::LatentEventAckWindow>(data);
		const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
		window.Observe(now, info.reason, softLimited, overflow);
		if (!softLimited)
		{
			return;
		}
		if (!window.ShouldLog(now, overflow) || !logBudget.Consume(now, overflow))
		{
			return;
		}

		nlohmann::json counts = nlohmann::json::object();
		for (size_t i = 0; i < window.counts.size(); ++i)
		{
			counts[fx::kEventReassemblyAckReasonNames[i]] = window.counts[i];
		}
		nlohmann::json record{
			{ "schema", 1 }, { "kind", overflow ? "overflow" : "soft_limit" },
			{ "timestamp_ms", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() },
			{ "client", client->GetNetId() }, { "event_id", std::to_string(packet.eventId.GetValue()) },
			{ "packet_index", packet.packetIdx.GetValue() }, { "packet_count", packet.totalPackets.GetValue() },
			{ "reason", fx::kEventReassemblyAckReasonNames[static_cast<size_t>(info.reason)] },
			{ "window_ms", now - window.startedAt }, { "last_ack_gap_ms", window.lastGapMs },
			{ "max_ack_gap_ms", window.maxGapMs },
			{ "counts", counts }, { "soft_limits", window.softLimits }, { "overflows", window.overflows }
		};
		if (info.activeOutboundEvents >= 0)
		{
			record["active_outbound_events"] = info.activeOutboundEvents;
		}
		if (info.hasTransfer)
		{
			record["transfer"] = {
				{ "event_name", info.eventName.data() }, { "payload_bytes", info.payloadBytes },
				{ "bps", info.bytesPerSecond }, { "pending_targets", info.pendingTargets },
				{ "expected_fragments", info.expectedFragments }, { "acked_fragments", info.acknowledgedFragments },
				{ "observed_sends", info.observedSends }, { "observed_retries", info.observedRetries },
				{ "last_send_age_ms", info.lastSendAgeMs }, { "no_progress_age_ms", info.noProgressAgeMs },
				{ "retired_age_ms", info.retiredAgeMs }
			};
		}
		console::Printf("net", "latent_ack_diagnostic %s\n", record.dump());
	}
	catch (...)
	{
	}
}
}

fx::ServerDecorators::ReassembledEventPacketHandler::ReassembledEventPacketHandler(fx::ServerInstanceBase* instance)
	: m_rac(instance->GetComponent<fx::ResourceManager>()->GetComponent<fx::EventReassemblyComponent>())
{
}

bool fx::ServerDecorators::ReassembledEventPacketHandler::Process(ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader, ENetPacketPtr& packet)
{
	if (!g_enableNetEventReassemblyConVar->GetValue())
	{
		return false;
	}

	return ProcessPacket(reader, [](net::packet::ReassembledEvent& reassembledEvent, fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, fwRefContainer<fx::EventReassemblyComponent>& rac, ENetPacketPtr& packet)
	{
		gscomms_execute_callback_on_main_thread([rac, reassembledEvent, client, packet]()
		{
			rac->HandlePacket(client->GetNetId(), std::string_view{ reinterpret_cast<const char*>(reassembledEvent.data.GetValue().data()), reassembledEvent.data.GetValue().size() });
			// packet needs to be moved to prevent packet memory from being freed
			(void)packet;
		});
	},
	instance, client, m_rac, packet);
}

fx::ServerDecorators::ReassembledEventV2PacketHandler::ReassembledEventV2PacketHandler(fx::ServerInstanceBase* instance)
	: m_rac(instance->GetComponent<fx::ResourceManager>()->GetComponent<fx::EventReassemblyComponent>())
{
	m_debugAcks = instance->AddVariable<bool>("sv_debugLatentEventAcks", ConVar_None, false);
	m_rac->SetAckDiagnosticsEnabled(m_debugAcks->GetValue());
	if (!g_enableNetEventReassemblyConVar)
	{
		g_enableNetEventReassemblyConVar = instance->AddVariable<bool>("sv_enableNetEventReassembly", ConVar_None, true);
	}
}

bool fx::ServerDecorators::ReassembledEventV2PacketHandler::Process(ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader, ENetPacketPtr& packet)
{
	static fx::RateLimiterStore<uint32_t, false> latentEventRateLimiterStore{
		instance->GetComponent<console::Context>().GetRef()
	};

	constexpr double kLatentEventRateLimit = 75.f;
	constexpr double kLatentEventRateLimitBurst = 125.f;
	static auto latentEventRateLimiter = latentEventRateLimiterStore.GetRateLimiter(
	"latentEvent", fx::RateLimiterDefaults{ kLatentEventRateLimit, kLatentEventRateLimitBurst });

	constexpr double kLatentEventRateFloodLimit = 150.f;
	constexpr double kLatentEventRateFloodLimitBurst = 175.f;
	static auto latentEventRateFloodLimiter = latentEventRateLimiterStore.GetRateLimiter(
	"latentEventFlood", fx::RateLimiterDefaults{ kLatentEventRateFloodLimit, kLatentEventRateFloodLimitBurst });

	static auto latentEventAckAbuseRateLimiter = latentEventRateLimiterStore.GetRateLimiter(
	"latentEventAckAbuse", fx::RateLimiterDefaults{ 100.f, 200.f });
	static auto latentEventAckAbuseFloodLimiter = latentEventRateLimiterStore.GetRateLimiter(
	"latentEventAckAbuseFlood", fx::RateLimiterDefaults{ 500.f, 1000.f });

	if (!g_enableNetEventReassemblyConVar->GetValue())
	{
		return false;
	}

	const bool debugAcks = m_debugAcks->GetValue();
	m_rac->SetAckDiagnosticsEnabled(debugAcks);
	return ProcessPacket(reader, [](net::packet::ReassembledEventV2& reassembledEvent, fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, fwRefContainer<fx::EventReassemblyComponent>& rac, ENetPacketPtr& packet, bool debugAcks)
	{
		const uint32_t netId = client->GetNetId();
		if (reassembledEvent.IsAck())
		{
			fx::EventReassemblyAckInfo info;
			const auto result = rac->HandlePacketV2Ack(netId, reassembledEvent, &info);
			const bool softLimited = result != fx::EventReassemblyAckResult::Accepted && !latentEventAckAbuseRateLimiter->Consume(netId);
			const bool overflow = softLimited && !latentEventAckAbuseFloodLimiter->Consume(netId);
			if (debugAcks)
			{
				ObserveAck(client, reassembledEvent, info, softLimited, overflow);
			}
			if (overflow)
			{
				instance->GetComponent<fx::GameServer>()->DropClientWithReason(client, fx::serverDropResourceName, fx::ClientDropReason::LATENT_NET_EVENT_RATE_LIMIT, "latent event ACK overflow.");
			}

			return;
		}

		latentEventRateLimiter->Consume(netId);
		const bool withinFloodRate = latentEventRateFloodLimiter->Consume(netId);
		if (!withinFloodRate)
		{
			instance->GetComponent<fx::GameServer>()->DropClientWithReason(client, fx::serverDropResourceName, fx::ClientDropReason::LATENT_NET_EVENT_RATE_LIMIT, "latent event packet overflow.");
			return;
		}

		gscomms_execute_callback_on_main_thread([rac, reassembledEvent, client, packet]()
		{
			rac->HandlePacketV2(client->GetNetId(), reassembledEvent);
			// packet needs to be moved to prevent packet memory from being freed
			(void)packet;
		});
	},
	instance, client, m_rac, packet, debugAcks);
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		fx::g_enableNetEventReassemblyConVar = instance->AddVariable<bool>("sv_enableNetEventReassembly", ConVar_None, true);
	});
});
