#include <StdInc.h>
#include <EventReassemblyComponent.h>

#include <ResourceManager.h>
#include <ResourceEventComponent.h>

#include <EASTL/bitvector.h>
#include <state/RlMessageBuffer.h>

#include <shared_mutex>
#include <atomic>
#include <new>

#include <EASTL/unordered_map.h>

#include "ByteReader.h"
#include "ByteWriter.h"

#if IS_FXSERVER
#include <StructuredTrace.h>
#endif

namespace rl
{
bool MessageBufferLengthHack::GetState()
{
	return false;
}
}

namespace fx
{
class CRC_EXPORT EventReassemblyComponentImpl : public EventReassemblyComponent
{
public:
	EventReassemblyComponentImpl();

	~EventReassemblyComponentImpl() override = default;

	void AttachToObject(ResourceManager* object) override;

	void SetSink(EventReassemblySink* sink) override;

	void RegisterTarget(int id, uint8_t maxPendingEvents) override;

	void UnregisterTarget(int id) override;

	void HandlePacket(int source, std::string_view data) override;

	void HandlePacketV2(int source, const net::packet::ReassembledEventV2& packet) override;

	EventReassemblyAckResult HandlePacketV2Ack(int source, const net::packet::ReassembledEventV2& packet, EventReassemblyAckInfo* info = nullptr) override;

	void SetAckDiagnosticsEnabled(bool enabled) override;

#ifndef IS_FXSERVER
	void TriggerEvent(int target, std::string_view eventName, std::string_view eventPayload, int bytesPerSecond) override;
#endif

	void TriggerEventV2(int target, std::string_view eventName, std::string_view eventPayload, int bytesPerSecond) override;

	void NetworkTick() override;

private:
	using EventId = uint64_t;

	struct SendEvent
	{
		struct PerTargetData
		{
			std::chrono::milliseconds lastSend{ 0 };
			std::chrono::milliseconds delayNextSend{ 0 };
			std::chrono::milliseconds lastProgress{ 0 };
			eastl::bitvector<> ackBits;
			eastl::bitvector<> sentBits;
			size_t acknowledgedPackets = 0;
			size_t lastBit = 0;
			uint64_t diagnosticEpoch = 0;
			uint64_t observedSends = 0;
			uint64_t observedRetries = 0;
		};

		std::set<int> targets;
		int bytesPerSecond;
		std::vector<uint8_t> sendPayload;
		std::unordered_map<int, std::shared_ptr<PerTargetData>> targetData;
		int nextTarget = std::numeric_limits<int>::min();
	};

	struct ReceiveEvent
	{
		int source;
		eastl::bitvector<> ackedBits;
		size_t receivedPackets = 0;

		// completion information (used to handle lingering packets after full reassembly)
		bool completed;
		std::chrono::milliseconds timeLastAck;
		std::chrono::milliseconds lastActivity;

		// we don't reassemble in-flight since a malicious source may try to get us to allocate many in-flight packets to cause memory exhaustion
		std::map<uint32_t, std::tuple<size_t, std::unique_ptr<uint8_t[]>>> packetData;
	};

	using SendList = std::map<EventId, std::shared_ptr<SendEvent>>;

	struct Target
	{
		int id;
		uint8_t maxPendingEvents;
	};

private:
	void HandleReceivedPacket(int source, const std::shared_ptr<ReceiveEvent>& event, uint32_t fragmentSize, bool v2 = false);

	void UnregisterSendList(int target, SendList& sendList, bool v2 = false);

	EventReassemblyAckInfo DescribeSend(const SendEvent& event, const SendEvent::PerTargetData& target, std::chrono::milliseconds now) const;

	void RememberSend(EventId eventId, int source, const SendEvent& event, const SendEvent::PerTargetData& target, EventReassemblyAckReason reason, std::chrono::milliseconds now);

	void NetworkTickSendList(const std::chrono::milliseconds& timeNow, const std::chrono::milliseconds& dT, SendList& sendList, uint32_t fragmentSize, size_t& fragmentBudget, EventId& nextEvent, bool v2 = false);

private:
	SendList m_sendList;
	SendList m_sendListV2;

	std::unordered_map<int, std::unordered_map<EventId, std::shared_ptr<ReceiveEvent>>> m_receiveList;

	std::unordered_map<int, Target> m_targets;

	std::shared_mutex m_listMutex;
	std::atomic<bool> m_ackDiagnosticsEnabled{ false };
	std::unique_ptr<EventReassemblyAckHistory> m_ackHistory;
	uint64_t m_ackDiagnosticEpoch = 0;

	ResourceManager* m_resourceManager;

	EventReassemblySink* m_sink;

	EventId m_eventId;

	std::chrono::milliseconds m_lastTime;

	// Time of last m_receiveList cleanup
	std::chrono::milliseconds m_lastCleanup;

	EventId m_nextSendEvent = 0;
	EventId m_nextSendEventV2 = 0;
};

struct EventPacket
{
	uint64_t eventId;
	uint32_t packetIdx;
	uint32_t totalPackets;
	uint32_t thisBytes;

	std::array<uint8_t, net::packet::ReassembledEvent::kFragmentSize> payload;

	bool Parse(rl::MessageBufferView& buffer);

	bool Unparse(rl::MessageBufferView& buffer);

	bool IsAck() const
	{
		return thisBytes == 0;
	}
};

bool EventPacket::Parse(rl::MessageBufferView& buffer)
{
	static_assert(net::packet::ReassembledEvent::kFragmentSize <= (1 << net::packet::ReassembledEvent::kFragmentSizeBits), "Too little kFragmentSizeBits.");

	uint32_t eventIdLow = 0;
	uint32_t eventIdHigh = 0;

	if (!buffer.Read(32, &eventIdLow) || !buffer.Read(32, &eventIdHigh))
	{
		return false;
	}

	eventId = (static_cast<uint64_t>(eventIdHigh) << 32) | eventIdLow;

	if (!buffer.Read(net::packet::ReassembledEvent::kPacketSizeBits, &packetIdx))
	{
		return false;
	}

	if (!buffer.Read(net::packet::ReassembledEvent::kPacketSizeBits, &totalPackets))
	{
		return false;
	}

	if (!buffer.Read(net::packet::ReassembledEvent::kFragmentSizeBits, &thisBytes))
	{
		return false;
	}

	if (thisBytes > 0)
	{
		buffer.ReadBits(payload.data(), std::min(static_cast<size_t>(thisBytes), payload.size()) * 8);
	}

	return true;
}

bool EventPacket::Unparse(rl::MessageBufferView& buffer)
{
	buffer.Write(32, static_cast<uint32_t>(eventId & 0xFFFFFFFF));
	buffer.Write(32, static_cast<uint32_t>(eventId >> 32));
	buffer.Write(net::packet::ReassembledEvent::kPacketSizeBits, packetIdx);
	buffer.Write(net::packet::ReassembledEvent::kPacketSizeBits, totalPackets);
	buffer.Write(net::packet::ReassembledEvent::kFragmentSizeBits, thisBytes);

	if (thisBytes > 0)
	{
		buffer.WriteBits(payload.data(), thisBytes * 8);
	}

	return true;
}

EventReassemblyComponentImpl::EventReassemblyComponentImpl()
	: m_eventId(0), m_resourceManager(nullptr), m_sink(nullptr), m_lastTime(0), m_lastCleanup(0)
{
	m_lastTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
	m_lastCleanup = m_lastTime;
}

void EventReassemblyComponentImpl::AttachToObject(ResourceManager* object)
{
	m_resourceManager = object;
}

void EventReassemblyComponentImpl::SetSink(EventReassemblySink* sink)
{
	m_sink = sink;
}

void EventReassemblyComponentImpl::RegisterTarget(const int id, const uint8_t maxPendingEvents)
{
	std::unique_lock lock(m_listMutex);
	m_targets[id] = Target{ id, maxPendingEvents };
}

void EventReassemblyComponentImpl::UnregisterSendList(int target, SendList& sendList, bool v2)
{
	for (auto& [eventId, sendPacket] : sendList)
	{
		if (v2 && m_ackHistory)
		{
			const auto targetIt = sendPacket->targetData.find(target);
			if (targetIt != sendPacket->targetData.end() && targetIt->second)
			{
				RememberSend(eventId, target, *sendPacket, *targetIt->second, EventReassemblyAckReason::RetiredDisconnected,
					std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()));
			}
		}
		sendPacket->targetData.erase(target);
		sendPacket->targets.erase(target);
	}
}

void EventReassemblyComponentImpl::UnregisterTarget(const int id)
{
	std::unique_lock lock(m_listMutex);

	if (m_targets.find(id) != m_targets.end())
	{
		m_targets.erase(id);

		// drop any sends/receives from this target
		m_receiveList.erase(id);

		UnregisterSendList(id, m_sendList);
		UnregisterSendList(id, m_sendListV2, true);
	}
}

#ifndef IS_FXSERVER
void EventReassemblyComponentImpl::TriggerEvent(const int target, const std::string_view eventName, const std::string_view eventPayload, int bytesPerSecond)
{
	const auto timeNow = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());

	// default BPS if it's 0/negative so we won't end up with weird calculation artifacts later on
	if (bytesPerSecond <= 0)
	{
		bytesPerSecond = 25000;
	}

	// skip oversized packets
	uint32_t totalSize = eventPayload.size() + eventName.size() + sizeof(uint16_t);
	if (totalSize >= net::packet::ReassembledEvent::kMaxPacketSize)
	{
		trace("Event %s is too large to be sent. Total size: %u, max size: %u.\n", eventName, totalSize, net::packet::ReassembledEvent::kMaxPacketSize);
		return;
	}

	// 10+MB/s bitrate to send 50+MB object.
	if (bytesPerSecond >= 10000000 && eventPayload.size() >= 50000000)
	{
		static std::chrono::milliseconds lastWarning{ 0 };
		if (timeNow - lastWarning >= std::chrono::seconds(5))
		{
			trace("Warning: sending large event %s (%u bytes) with almost unrestricted bitrate (%u b/s). This may cause performance issues. Consider decreasing objects sizes or bitrate.\n",
			eventName, eventPayload.size(), bytesPerSecond);
			lastWarning = timeNow;
		}
	}

	std::set<int> targets;

	if (target == -1)
	{
		std::shared_lock _(m_listMutex);
		for (const auto& currTarget : m_targets)
		{
			targets.insert(currTarget.first);
		}
	}
	else
	{
		std::shared_lock _(m_listMutex);

		if (m_targets.find(target) == m_targets.end())
		{
			return;
		}

		targets.insert(target);
	}

	rl::MessageBuffer payloadBuf(eventName.size() + sizeof(uint16_t) + eventPayload.size());

	payloadBuf.Write(16, static_cast<uint32_t>(eventName.size()));
	payloadBuf.WriteBits(eventName.data(), eventName.size() * 8);

	memcpy(payloadBuf.GetBuffer().data() + (payloadBuf.GetCurrentBit() / 8), eventPayload.data(), eventPayload.size());

	auto sendPacket = std::make_shared<SendEvent>();
	sendPacket->bytesPerSecond = bytesPerSecond;
	sendPacket->targets = targets;
	sendPacket->sendPayload = std::move(payloadBuf.GetBuffer());

	auto packetCount = sendPacket->sendPayload.size() / net::packet::ReassembledEvent::kFragmentSize;

	if ((sendPacket->sendPayload.size() % net::packet::ReassembledEvent::kFragmentSize) != 0)
	{
		packetCount += 1;
	}

	for (auto target : targets)
	{
		auto targetData = std::make_shared<SendEvent::PerTargetData>();
		targetData->ackBits.resize(packetCount);
		targetData->sentBits.resize(packetCount);
		targetData->lastSend = std::chrono::milliseconds{ 0 };
		targetData->lastProgress = timeNow;

		sendPacket->targetData[target] = targetData;
	}

	std::unique_lock lock(m_listMutex);
	m_sendList.insert({ m_eventId++, sendPacket });
}
#endif

void EventReassemblyComponentImpl::TriggerEventV2(const int target, std::string_view eventName, std::string_view eventPayload, int bytesPerSecond)
{
	const auto timeNow = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());

	// default BPS if it's 0/negative so we won't end up with weird calculation artifacts later on
	if (bytesPerSecond <= 0)
	{
		bytesPerSecond = 25000;
	}

	// skip oversized packets
	uint32_t totalSize = eventPayload.size() + eventName.size() + sizeof(uint16_t);
	if (totalSize >= net::packet::ReassembledEventV2::kMaxPacketSize)
	{
		trace("Event %s is too large to be sent. Total size: %u, max size: %u.\n", eventName, totalSize, net::packet::ReassembledEventV2::kMaxPacketSize);
		return;
	}

	// 10+MB/s bitrate to send 50+MB object.
	if (bytesPerSecond >= 10000000 && eventPayload.size() >= 50000000)
	{
		static std::chrono::milliseconds lastWarning{ 0 };
		if (timeNow - lastWarning >= std::chrono::seconds(5))
		{
#if IS_FXSERVER
			StructuredTrace({ "type", "large_event_warning" }, { "event_type", "latent" }, { "event_name", eventName }, { "event_size", eventPayload.size() }, { "event_bps", bytesPerSecond });
#endif
			trace("Warning: sending large event %s (%u bytes) with almost unrestricted bitrate (%u b/s). This may cause performance issues. Consider decreasing objects sizes or bitrate.\n",
			eventName, eventPayload.size(), bytesPerSecond);
			lastWarning = timeNow;
		}
	}

	std::set<int> targets;

	if (target == -1)
	{
		std::shared_lock _(m_listMutex);
		for (const auto& currTarget : m_targets)
		{
			targets.insert(currTarget.first);
		}
	}
	else
	{
		std::shared_lock _(m_listMutex);

		if (m_targets.find(target) == m_targets.end())
		{
			return;
		}

		targets.insert(target);
	}

	auto sendPacket = std::make_shared<SendEvent>();
	sendPacket->bytesPerSecond = bytesPerSecond;
	sendPacket->targets = targets;
	sendPacket->sendPayload.resize(eventName.size() + sizeof(uint16_t) + eventPayload.size());

	net::ByteWriter writer{ sendPacket->sendPayload.data(), sendPacket->sendPayload.size() };
	uint16_t eventNameSize = static_cast<uint16_t>(eventName.size());
	writer.Field(eventNameSize);
	writer.Field(eventName, eventNameSize);
	writer.Field(eventPayload, eventPayload.size());

	auto packetCount = sendPacket->sendPayload.size() / net::packet::ReassembledEventV2::kFragmentSize;

	if ((sendPacket->sendPayload.size() % net::packet::ReassembledEventV2::kFragmentSize) != 0)
	{
		packetCount += 1;
	}

	for (auto targetId : targets)
	{
		auto targetData = std::make_shared<SendEvent::PerTargetData>();
		targetData->ackBits.resize(packetCount);
		targetData->sentBits.resize(packetCount);
		targetData->lastSend = std::chrono::milliseconds{ 0 };
		targetData->lastProgress = timeNow;

		sendPacket->targetData[targetId] = targetData;
	}

	std::unique_lock lock(m_listMutex);
	m_sendListV2.insert({ m_eventId++, sendPacket });
}

/// <summary>
/// Called when a packet is fully received
/// </summary>
/// <param name="source">net id from where we fully received the packet, the server net id is always 0 on the client</param>
/// <param name="event">ReceiveEvent that contains the infos about the packet to receive. It contains the map of packets to reassemble which should be complete now</param>
void EventReassemblyComponentImpl::HandleReceivedPacket(int source, const std::shared_ptr<ReceiveEvent>& event, uint32_t fragmentSize, const bool v2)
{
	if (m_sink->LimitEvent(source))
	{
		event->packetData.clear();
		return;
	}

	// reassemble the buffer
	std::vector<uint8_t> eventPayload(event->ackedBits.size() * fragmentSize);
	size_t readSize = 0;

	for (auto& packet : event->packetData)
	{
		auto offset = packet.first * fragmentSize;
		memcpy(eventPayload.data() + offset, std::get<1>(packet.second).get(), std::get<0>(packet.second));

		readSize += std::get<0>(packet.second);

	}

	event->packetData.clear();

	// parse event data
	static char eventName[65536];

	std::string_view name;
	// todo: replace with std::string_view when v1 protocol is removed
	std::string data;
	if (v2)
	{
		net::ByteReader reader{ eventPayload.data(), readSize };
		uint16_t nameLength = 0;
		if (!reader.Field(nameLength) || !reader.Field(name, nameLength))
		{
			return;
		}

		// Strip trailing null terminator if present (much older clients include it in the length)
		if (!name.empty() && name.back() == '\0')
		{
			name = name.substr(0, name.size() - 1);
		}

		if (!reader.Field(data, reader.GetRemaining()))
		{
			return;
		}
	}
	else
	{
		rl::MessageBufferView buffer(net::Span<uint8_t>(eventPayload.data(), readSize));

		uint16_t nameLength = buffer.Read<uint16_t>(16);
		if (nameLength == 0 || nameLength >= std::size(eventName) || !buffer.ReadBits(eventName, nameLength * 8))
		{
			return;
		}
		eventName[nameLength] = '\0';

		name = { eventName, static_cast<size_t>(nameLength - 1) };
		data = std::string(std::string_view{
		reinterpret_cast<const char*>(buffer.GetBuffer().data() + (buffer.GetCurrentBit() / 8)),
		buffer.GetBuffer().size() - (buffer.GetCurrentBit() / 8) });
	}

	// convert the source net ID to a string
	std::string sourceStr = "net:" + std::to_string(source);

	// get the resource manager and eventing component
	fwRefContainer<fx::ResourceEventManagerComponent> eventManager = m_resourceManager->GetComponent<fx::ResourceEventManagerComponent>();

	// and queue the event
	eventManager->QueueEvent(
	std::string(name),
	data,
	sourceStr);
}

void EventReassemblyComponentImpl::NetworkTickSendList(const std::chrono::milliseconds& timeNow, const std::chrono::milliseconds& dT, SendList& sendList, const uint32_t fragmentSize, size_t& fragmentBudget, EventId& nextEvent, const bool v2)
{
	static constexpr size_t kMaxFragmentsPerTargetPerTick = 64;
	static constexpr std::chrono::milliseconds kMinimumNoProgressTimeout{ 120000 };
	std::set<EventId> dones;
	if (sendList.empty())
	{
		return;
	}

	auto eventIt = sendList.lower_bound(nextEvent);
	if (eventIt == sendList.end())
	{
		eventIt = sendList.begin();
	}

	size_t eventsRemaining = sendList.size();
	while (eventsRemaining-- > 0)
	{
		const auto currentEventIt = eventIt;
		if (++eventIt == sendList.end())
		{
			eventIt = sendList.begin();
		}

		const EventId eventId = currentEventIt->first;
		nextEvent = eventIt->first;
		auto& sendPacket = currentEventIt->second;

		double pps = (sendPacket->bytesPerSecond / static_cast<double>(fragmentSize));
		std::chrono::milliseconds latency{ std::max<uint64_t>(1, static_cast<uint64_t>(1000 / pps)) };

		std::set<int> doneTargets;

		auto targetIt = sendPacket->targets.lower_bound(sendPacket->nextTarget);
		if (targetIt == sendPacket->targets.end())
		{
			targetIt = sendPacket->targets.begin();
		}

		size_t targetsRemaining = sendPacket->targets.size();
		while (targetsRemaining-- > 0)
		{
			const auto currentTargetIt = targetIt;
			if (++targetIt == sendPacket->targets.end())
			{
				targetIt = sendPacket->targets.begin();
			}

			const int target = *currentTargetIt;
			if (fragmentBudget == 0)
			{
				sendPacket->nextTarget = target;
				break;
			}
			sendPacket->nextTarget = *targetIt;

			// if the target is gone, get rid of them
			if (m_targets.find(target) == m_targets.end())
			{
				doneTargets.insert(target);
				continue;
			}

			auto targetData = sendPacket->targetData[target];
			const auto noProgressTimeout = std::max(kMinimumNoProgressTimeout, latency * 4);
			if (targetData && (timeNow - targetData->lastProgress) >= noProgressTimeout)
			{
				if (v2)
				{
					RememberSend(eventId, target, *sendPacket, *targetData, EventReassemblyAckReason::RetiredExpired, timeNow);
				}
				sendPacket->targetData[target] = {};
				doneTargets.insert(target);
				continue;
			}

			if (targetData && (targetData->lastSend + latency) < timeNow && targetData->delayNextSend < timeNow)
			{
				// burst loop so we don't 'slow down' too much at a lower tick rate
				auto resTime = dT;
				auto& ackBits = targetData->ackBits;
				size_t fragmentsSent = 0;

				do
				{
					const auto bc = ackBits.size();
					if (targetData->acknowledgedPackets == bc)
					{
						if (v2)
						{
							RememberSend(eventId, target, *sendPacket, *targetData, EventReassemblyAckReason::RetiredCompleted, timeNow);
						}
						sendPacket->targetData[target] = {};
						doneTargets.insert(target);
						break;
					}

					size_t packetIdx = bc;
					for (size_t bit = targetData->lastBit; bit < bc; bit++)
					{
						if (!ackBits[bit])
						{
							packetIdx = bit;
							break;
						}
					}

					if (packetIdx == bc)
					{
						for (size_t bit = 0; bit < targetData->lastBit; bit++)
						{
							if (!ackBits[bit])
							{
								targetData->lastBit = bit;
								break;
							}
						}

						targetData->delayNextSend = timeNow + std::chrono::milliseconds(250);
						break;
					}

					targetData->lastBit = (packetIdx + 1) % bc;
					if (v2 && m_ackHistory)
					{
						if (targetData->diagnosticEpoch != m_ackDiagnosticEpoch)
						{
							targetData->diagnosticEpoch = m_ackDiagnosticEpoch;
							targetData->observedSends = targetData->observedRetries = 0;
						}
						++targetData->observedSends;
						targetData->observedRetries += targetData->sentBits[packetIdx] ? 1 : 0;
					}
					targetData->sentBits.set(packetIdx, true);

					if (v2)
					{
						net::packet::ReassembledEventV2Packet packet;
						packet.data.eventId = eventId;
						packet.data.packetIdx = static_cast<uint16_t>(packetIdx);
						packet.data.totalPackets = static_cast<uint16_t>(ackBits.size());

						size_t offset = (packetIdx * fragmentSize);
						size_t size = std::min(sendPacket->sendPayload.size() - offset, static_cast<size_t>(fragmentSize));

						packet.data.data.SetValue({ sendPacket->sendPayload.data() + offset, size });
						m_sink->SendPacketV2(target, packet);
					}
					else
					{
						EventPacket packet;
						packet.eventId = eventId;
						packet.packetIdx = packetIdx;
						packet.totalPackets = ackBits.size();

						size_t offset = (packetIdx * fragmentSize);
						size_t size = std::min(sendPacket->sendPayload.size() - offset, size_t(fragmentSize));

						memcpy(packet.payload.data(), sendPacket->sendPayload.data() + offset, size);
						packet.thisBytes = size;

						uint8_t buf[1536];
						rl::MessageBufferView view(net::Span<uint8_t>(buf, 1536));
						packet.Unparse(view);

						m_sink->SendPacket(target, std::string_view{ reinterpret_cast<char*>(buf), view.GetDataLength() });
					}

					++fragmentsSent;
					--fragmentBudget;

					if (resTime > latency)
					{
						resTime -= latency;
					}

					if (targetData->lastBit == 0)
					{
						targetData->delayNextSend = timeNow + std::chrono::milliseconds(250);
						break;
					}

					if (fragmentsSent >= kMaxFragmentsPerTargetPerTick || fragmentBudget == 0)
					{
						break;
					}
				} while (resTime > latency);

				targetData->lastSend = timeNow;
			}
			// update send timer if we are delaying (to prevent oversize bursts)
			else if (targetData && targetData->delayNextSend >= timeNow)
			{
				targetData->lastSend = timeNow;
			}
		}

		for (auto done : doneTargets)
		{
			sendPacket->targets.erase(done);
		}

		// if no targets, complete send
		if (sendPacket->targets.empty())
		{
			dones.insert(eventId);
		}

		if (fragmentBudget == 0)
		{
			break;
		}
	}

	// remove dones
	for (auto done : dones)
	{
		sendList.erase(done);
	}
}

void EventReassemblyComponentImpl::NetworkTick()
{
	auto timeNow = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
	auto dT = std::min(timeNow - m_lastTime, std::chrono::milliseconds{ 100 });
	m_lastTime = timeNow;

	// handle the send list
	{
		std::unique_lock lock(m_listMutex);
		static constexpr size_t kMaxFragmentsPerNetworkTick = 1024;
		size_t fragmentBudget = kMaxFragmentsPerNetworkTick;

		NetworkTickSendList(timeNow, dT, m_sendList, net::packet::ReassembledEvent::kFragmentSize, fragmentBudget, m_nextSendEvent);
		NetworkTickSendList(timeNow, dT, m_sendListV2, net::packet::ReassembledEventV2::kFragmentSize, fragmentBudget, m_nextSendEventV2, true);

		static constexpr std::chrono::milliseconds gcInterval{ 30000 };
		static constexpr std::chrono::milliseconds completedLifetime{ 120000 };
		static constexpr std::chrono::milliseconds incompleteIdleTimeout{ 120000 };
		if (timeNow - m_lastCleanup >= gcInterval)
		{
			m_lastCleanup = timeNow;
			for (auto it = m_receiveList.begin(); it != m_receiveList.end(); /* no increment */)
			{
				for (auto clientIt = it->second.begin(); clientIt != it->second.end(); /* no increment */)
				{
					const auto& receiveEvent = clientIt->second;
					const bool completedExpired = receiveEvent->completed && (timeNow - receiveEvent->timeLastAck) >= completedLifetime;
					const bool incompleteExpired = !receiveEvent->completed && (timeNow - receiveEvent->lastActivity) >= incompleteIdleTimeout;
					if (completedExpired || incompleteExpired)
					{
						if (incompleteExpired && receiveEvent->source != 0)
						{
							auto target = m_targets.find(receiveEvent->source);
							if (target != m_targets.end() && target->second.maxPendingEvents != 0xFF)
							{
								++target->second.maxPendingEvents;
							}
						}

						clientIt = it->second.erase(clientIt);
					}
					else
					{
						++clientIt;
					}
				}
				if (it->second.empty())
				{
					it = m_receiveList.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
	}
}

/// <summary>
/// Called when a packet is received from the network endpoint
/// </summary>
/// <param name="source">net id from where we received this packet, the server net id is always 0 on the client</param>
/// <param name="data">The packet data we received</param>
void EventReassemblyComponentImpl::HandlePacket(int source, std::string_view data)
{
#ifndef IS_FXSERVER
	rl::MessageBufferView buffer(net::Span<uint8_t>(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(data.data())), static_cast<size_t>(data.size())));

	EventPacket packet;
	if (!packet.Parse(buffer))
	{
		return;
	}

	// packet that has no total amount of packet is invalid
	if (packet.totalPackets == 0)
	{
		return;
	}

	if (packet.packetIdx >= packet.totalPackets)
	{
		return;
	}

	if (packet.IsAck())
	{
		// received a ack packet to indicate the remote side received a payload packet
		std::unique_lock lock(m_listMutex);
		auto entryIt = m_sendList.find(packet.eventId);
		if (entryIt != m_sendList.end())
		{
			std::shared_ptr<SendEvent> sendData = entryIt->second;

			auto targetDataIt = sendData->targetData.find(source);

			if (targetDataIt != sendData->targetData.end() && targetDataIt->second)
			{
				auto& targetData = targetDataIt->second;
				auto& ackBits = targetData->ackBits;

				if (packet.totalPackets == ackBits.size() && packet.packetIdx < ackBits.size() && targetData->sentBits[packet.packetIdx])
				{
					// marks the packetIdx (0, ackBitsSize] as arrived
					if (!ackBits[packet.packetIdx])
					{
						ackBits.set(packet.packetIdx, true);
						++targetData->acknowledgedPackets;
						targetData->lastProgress = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
					}
				}
			}
		}
	}
	else
	{
		// queue event if we are 'full'
		bool hasAll = false;
		std::shared_ptr<ReceiveEvent> receiveData;
		{
			std::unique_lock lock(m_listMutex);
			// check if there is already a pending event with this id from the remote side
			auto clientIt = m_receiveList.find(source);
			std::unordered_map<EventId, std::shared_ptr<ReceiveEvent>>* clientReceiveMap;
			if (clientIt == m_receiveList.end())
			{
				clientReceiveMap = &m_receiveList.insert({ source, std::unordered_map<EventId, std::shared_ptr<ReceiveEvent>>{} }).first->second;
			}
			else
			{
				clientReceiveMap = &clientIt->second;
			}

			auto entryIt = clientReceiveMap->find(packet.eventId);
			if (entryIt == clientReceiveMap->end())
			{
				// targets are protected by m_listMutex as well
				auto target = m_targets.find(source);
				if (target == m_targets.end())
				{
					// discard packet when its received from a none registered target
					return;
				}

				switch (target->second.maxPendingEvents)
				{
					case 0:
						// more then maxPendingEvents are not accepted at the same time
						return;
					case 0xFF:
						// when maxPendingEvents is set to 255 the target has infinite amount of events
						// used on the client side for the remote server
						break;
					default:
						--target->second.maxPendingEvents;
						break;
				}

				// started receiving a new event from remote
				receiveData = std::make_shared<ReceiveEvent>();
				// the remote defines the amount of packets that the event will be split to
				receiveData->ackedBits.resize(packet.totalPackets);
				receiveData->source = source;
				receiveData->completed = false;
				receiveData->timeLastAck = std::chrono::milliseconds{ 0 };
				receiveData->lastActivity = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());

				clientReceiveMap->insert({ packet.eventId, receiveData });
			}
			else
			{
				// received some payload for this event already from the remote
				receiveData = entryIt->second;
			}

			// note down as acked
			auto& ackBits = receiveData->ackedBits;

			if (packet.totalPackets != ackBits.size())
			{
				return;
			}

			// packetIdx (0, ackBitsSize] is the relative index of the packet inside the event to receive
			bool ackedPacket = ackBits[packet.packetIdx];

			// Event has already been completed or acked: just send an ACK.
			if (receiveData->source == source && (receiveData->completed || ackedPacket))
			{
				// TODO: why allocate a std::vector with 1536 byte when this has a maximum of 108 bit

				uint8_t buf[1536];
				rl::MessageBufferView view(net::Span<uint8_t>(buf, 1536));
				// thisBytes = 0 makes sure the receive ack to remote does not repeat the packet payload and only the meta data.
				packet.thisBytes = 0;
				packet.Unparse(view);

				m_sink->SendPacket(source, std::string_view{ reinterpret_cast<char*>(buf), view.GetDataLength() });
				receiveData->timeLastAck = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
			}
			// Still rebuilding event...,
			// receiveData->source == source is always true
			else if (receiveData->source == source)
			{
				ackBits.set(packet.packetIdx, true);
				++receiveData->receivedPackets;
				receiveData->lastActivity = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());

				// copy the payload from the packet to our ReceiveEvent to assemble it when all data is received
				{
					auto newPayload = std::unique_ptr<uint8_t[]>(new uint8_t[packet.thisBytes]);
					memcpy(newPayload.get(), packet.payload.data(), packet.thisBytes);

					receiveData->packetData[packet.packetIdx] = { size_t(packet.thisBytes), std::move(newPayload) };
				}

				// send ack
				{
					// TODO: why allocate a std::vector with 1536 byte when this has a maximum of 108 bit
					uint8_t buf[1536];
					rl::MessageBufferView view(net::Span<uint8_t>(buf, 1536));
					// thisBytes = 0 makes sure the receive ack to remote does not repeat the packet payload and only the meta data.
					packet.thisBytes = 0;
					packet.Unparse(view);

					m_sink->SendPacket(source, std::string_view{ (char*)buf, view.GetDataLength() });
					receiveData->timeLastAck = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
				}

				hasAll = receiveData->receivedPackets == ackBits.size();

				if (hasAll)
				{
					// marks the ReceiveEvent as completed to remove it after 2 minutes from memory
					receiveData->completed = true;

					// the client handles the server as id 0 and there no limiting is required
					if (source != 0)
					{
						// targets are protected by m_listMutex as well
						const auto target = m_targets.find(source);
						if (target != m_targets.end() && target->second.maxPendingEvents != 0xFF)
						{
							// count up max pending event count, because a event is fully received
							++target->second.maxPendingEvents;
						}
					}

					// TODO: remove at least the payload of the fully received packet from the memory to keep the meta data to discard the duplicate packets

					// Cleanup will now happen in NetworkTick to prevent any lingering packets from recreating
					// the ReceiveEvent.
					// m_receiveList.erase({ source, packet.eventId });
				}
			}
		}

		if (hasAll && receiveData != nullptr)
		{
			HandleReceivedPacket(source, receiveData, net::packet::ReassembledEvent::kFragmentSize);
		}
	}
#endif
}

/// <summary>
/// Called when a packet v2 is received from the network endpoint
/// </summary>
/// <param name="source">net id from where we received this packet, the server net id is always 0 on the client</param>
/// <param name="data">The packet data we received</param>
void EventReassemblyComponentImpl::HandlePacketV2(int source, const net::packet::ReassembledEventV2& packet)
{
	// packet that has no total amount of packet is invalid
	if (packet.totalPackets == static_cast<uint16_t>(0))
	{
		return;
	}

	if (packet.packetIdx >= packet.totalPackets)
	{
		return;
	}

	if (packet.IsAck())
	{
		HandlePacketV2Ack(source, packet);
	}
	else
	{
		// queue event if we are 'full'
		bool hasAll = false;
		std::shared_ptr<ReceiveEvent> receiveData;
		{
			std::unique_lock lock(m_listMutex);
			// check if there is already a pending event with this id from the remote side
			auto clientIt = m_receiveList.find(source);
			std::unordered_map<EventId, std::shared_ptr<ReceiveEvent>>* clientReceiveMap;
			if (clientIt == m_receiveList.end())
			{
				clientReceiveMap = &m_receiveList.insert({ source, std::unordered_map<EventId, std::shared_ptr<ReceiveEvent>>{} }).first->second;
			}
			else
			{
				clientReceiveMap = &clientIt->second;
			}

			auto entryIt = clientReceiveMap->find(packet.eventId);
			if (entryIt == clientReceiveMap->end())
			{
				// targets are protected by m_listMutex as well
				auto target = m_targets.find(source);
				if (target == m_targets.end())
				{
					// discard packet when its received from a none registered target
					return;
				}

				switch (target->second.maxPendingEvents)
				{
					case 0:
						// more then maxPendingEvents are not accepted at the same time
						return;
					case 0xFF:
						// when maxPendingEvents is set to 255 the target has infinite amount of events
						// used on the client side for the remote server
						break;
					default:
						--target->second.maxPendingEvents;
						break;
				}

				// started receiving a new event from remote
				receiveData = std::make_shared<ReceiveEvent>();
				// the remote defines the amount of packets that the event will be split to
				receiveData->ackedBits.resize(packet.totalPackets);
				receiveData->source = source;
				receiveData->completed = false;
				receiveData->timeLastAck = std::chrono::milliseconds{ 0 };
				receiveData->lastActivity = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());

				clientReceiveMap->insert({ packet.eventId, receiveData });
			}
			else
			{
				// received some payload for this event already from the remote
				receiveData = entryIt->second;
			}

			// note down as acked
			auto& ackBits = receiveData->ackedBits;

			if (packet.totalPackets != ackBits.size() )
			{
				return;
			}

			// packetIdx (0, ackBitsSize] is the relative index of the packet inside the event to receive
			bool ackedPacket = ackBits[packet.packetIdx];

			// Event has already been completed or acked: just send an ACK.
			if (receiveData->source == source && (receiveData->completed || ackedPacket))
			{
				net::packet::ReassembledEventV2Packet ack;
				ack.data.eventId = packet.eventId;
				ack.data.packetIdx = packet.packetIdx;
				ack.data.totalPackets = packet.totalPackets;
				m_sink->SendPacketV2(source, ack);
				receiveData->timeLastAck = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
			}
			// Still rebuilding event...,
			// receiveData->source == source is always true
			else if (receiveData->source == source)
			{

				ackBits.set(packet.packetIdx, true);
				++receiveData->receivedPackets;
				receiveData->lastActivity = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());

				// copy the payload from the packet to our ReceiveEvent to assemble it when all data is received
				{
					auto newPayload = std::unique_ptr<uint8_t[]>(new uint8_t[packet.data.GetValue().size()]);
					memcpy(newPayload.get(), packet.data.GetValue().data(), packet.data.GetValue().size());

					receiveData->packetData[packet.packetIdx] = { packet.data.GetValue().size(), std::move(newPayload) };
				}

				// send ack
				{
					net::packet::ReassembledEventV2Packet ack;
					ack.data.eventId = packet.eventId;
					ack.data.packetIdx = packet.packetIdx;
					ack.data.totalPackets = packet.totalPackets;
					m_sink->SendPacketV2(source, ack);
					receiveData->timeLastAck = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
				}

				hasAll = receiveData->receivedPackets == ackBits.size();

				if (hasAll)
				{
					// marks the ReceiveEvent as completed to remove it after 2 minutes from memory
					receiveData->completed = true;

					// the client handles the server as id 0 and there no limiting is required
					if (source != 0)
					{
						// targets are protected by m_listMutex as well
						const auto target = m_targets.find(source);
						if (target != m_targets.end() && target->second.maxPendingEvents != 0xFF)
						{
							// count up max pending event count, because a event is fully received
							++target->second.maxPendingEvents;
						}
					}

					// TODO: remove at least the payload of the fully received packet from the memory to keep the meta data to discard the duplicate packets

					// Cleanup will now happen in NetworkTick to prevent any lingering packets from recreating
					// the ReceiveEvent.
					// m_receiveList.erase({ source, packet.eventId });
				}
			}
		}

		if (hasAll && receiveData != nullptr)
		{
			HandleReceivedPacket(source, receiveData, net::packet::ReassembledEventV2::kFragmentSize, true);
		}
	}
}

void EventReassemblyComponentImpl::SetAckDiagnosticsEnabled(bool enabled)
{
	if (m_ackDiagnosticsEnabled.load(std::memory_order_acquire) == enabled)
	{
		return;
	}
	std::unique_lock lock(m_listMutex);
	if (m_ackDiagnosticsEnabled.load(std::memory_order_relaxed) == enabled)
	{
		return;
	}
	if (enabled)
	{
		m_ackHistory.reset(new (std::nothrow) EventReassemblyAckHistory());
		++m_ackDiagnosticEpoch;
	}
	else
	{
		m_ackHistory.reset();
	}
	m_ackDiagnosticsEnabled.store(enabled, std::memory_order_release);
}

EventReassemblyAckInfo EventReassemblyComponentImpl::DescribeSend(const SendEvent& event, const SendEvent::PerTargetData& target, std::chrono::milliseconds now) const
{
	EventReassemblyAckInfo info;
	info.hasTransfer = true;
	info.activeOutboundEvents = static_cast<int64_t>(m_sendListV2.size());
	info.payloadBytes = event.sendPayload.size();
	info.bytesPerSecond = event.bytesPerSecond;
	info.pendingTargets = static_cast<uint32_t>(event.targets.size());
	info.expectedFragments = static_cast<uint32_t>(target.ackBits.size());
	info.acknowledgedFragments = static_cast<uint32_t>(target.acknowledgedPackets);
	info.lastSendAgeMs = target.lastSend.count() ? std::max<int64_t>(0, (now - target.lastSend).count()) : 0;
	info.noProgressAgeMs = std::max<int64_t>(0, (now - target.lastProgress).count());
	if (target.diagnosticEpoch == m_ackDiagnosticEpoch)
	{
		info.observedSends = target.observedSends;
		info.observedRetries = target.observedRetries;
	}

	net::ByteReader reader{ event.sendPayload.data(), event.sendPayload.size() };
	uint16_t nameLength = 0;
	if (reader.Field(nameLength))
	{
		const auto length = std::min({ static_cast<size_t>(nameLength), reader.GetRemaining(), info.eventName.size() - 1 });
		for (size_t i = 0; i < length; ++i)
		{
			const auto ch = event.sendPayload[sizeof(nameLength) + i];
			info.eventName[i] = (ch >= 32 && ch < 127) ? static_cast<char>(ch) : '?';
		}
	}
	return info;
}

void EventReassemblyComponentImpl::RememberSend(EventId eventId, int source, const SendEvent& event, const SendEvent::PerTargetData& target, EventReassemblyAckReason reason, std::chrono::milliseconds now)
{
	if (!m_ackHistory)
	{
		return;
	}
	auto info = DescribeSend(event, target, now);
	info.reason = reason;
	m_ackHistory->Remember(eventId, source, now.count(), info);
}

EventReassemblyAckResult EventReassemblyComponentImpl::HandlePacketV2Ack(const int source, const net::packet::ReassembledEventV2& packet, EventReassemblyAckInfo* info)
{
	if (info)
	{
		*info = {};
	}
	const auto finish = [info](EventReassemblyAckResult result, EventReassemblyAckReason reason)
	{
		if (info)
		{
			info->reason = reason;
		}
		return result;
	};
	if (!packet.IsAck() || packet.totalPackets == static_cast<uint16_t>(0) || packet.packetIdx >= packet.totalPackets)
	{
		return finish(EventReassemblyAckResult::Invalid, EventReassemblyAckReason::InvalidEnvelope);
	}

	std::unique_lock lock(m_listMutex);
	const auto describeMissing = [&](EventReassemblyAckReason reason)
	{
		if (info && m_ackHistory && m_ackHistory->Find(packet.eventId, source, packet.totalPackets,
			std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count(), *info))
		{
			info->activeOutboundEvents = static_cast<int64_t>(m_sendListV2.size());
			return EventReassemblyAckResult::Invalid;
		}
		return finish(EventReassemblyAckResult::Invalid, reason);
	};
	if (info)
	{
		info->activeOutboundEvents = static_cast<int64_t>(m_sendListV2.size());
	}
	const auto entryIt = m_sendListV2.find(packet.eventId);
	if (entryIt == m_sendListV2.end())
	{
		return describeMissing(EventReassemblyAckReason::UnknownEvent);
	}

	const auto targetDataIt = entryIt->second->targetData.find(source);
	if (targetDataIt == entryIt->second->targetData.end() || !targetDataIt->second)
	{
		return describeMissing(EventReassemblyAckReason::UnknownTarget);
	}

	auto& targetData = targetDataIt->second;
	auto& ackBits = targetData->ackBits;
	if (info && m_ackHistory)
	{
		*info = DescribeSend(*entryIt->second, *targetData,
			std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()));
	}
	if (packet.totalPackets != ackBits.size() || packet.packetIdx >= ackBits.size())
	{
		return finish(EventReassemblyAckResult::Invalid, EventReassemblyAckReason::FragmentCountMismatch);
	}
	if (!targetData->sentBits[packet.packetIdx])
	{
		return finish(EventReassemblyAckResult::Invalid, EventReassemblyAckReason::UnsentFragment);
	}

	if (ackBits[packet.packetIdx])
	{
		return finish(EventReassemblyAckResult::Duplicate, EventReassemblyAckReason::Duplicate);
	}

	ackBits.set(packet.packetIdx, true);
	++targetData->acknowledgedPackets;
	targetData->lastProgress = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
	return finish(EventReassemblyAckResult::Accepted, EventReassemblyAckReason::Accepted);
}

fwRefContainer<EventReassemblyComponent> EventReassemblyComponent::Create()
{
	return new EventReassemblyComponentImpl();
}
}
