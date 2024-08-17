#include <StdInc.h>
#include <EventReassemblyComponent.h>

#include <ResourceManager.h>
#include <ResourceEventComponent.h>

#include <EASTL/bitvector.h>
#include <state/RlMessageBuffer.h>

#include <shared_mutex>

namespace rl
{
	bool MessageBufferLengthHack::GetState()
	{
		return false;
	}
}

constexpr const uint32_t kPacketSizeBits = 17;
constexpr const uint32_t kFragmentSize = 1024 - 1;
constexpr const uint32_t kFragmentSizeBits = 10;
constexpr const uint32_t kMaxPacketSize = (1 << kPacketSizeBits) * kFragmentSize;

namespace fx
{
class CRC_EXPORT EventReassemblyComponentImpl : public EventReassemblyComponent
{
public:
	EventReassemblyComponentImpl();

	virtual ~EventReassemblyComponentImpl() override = default;

	virtual void AttachToObject(ResourceManager* object) override;

	virtual void SetSink(EventReassemblySink* sink) override;

	virtual void RegisterTarget(int id) override;

	virtual void UnregisterTarget(int id) override;

	virtual void HandlePacket(int source, std::string_view data) override;

	virtual void TriggerEvent(int target, std::string_view eventName, std::string_view eventPayload, int bytesPerSecond) override;

	virtual void NetworkTick() override;

private:
	using EventId = uint64_t;

	struct SendEvent
	{
		struct PerTargetData
		{
			std::chrono::milliseconds lastSend{ 0 };
			std::chrono::milliseconds delayNextSend{ 0 };
			eastl::bitvector<> ackBits;
			size_t lastBit = 0;
		};

		std::set<int> targets;
		int bytesPerSecond;
		std::vector<uint8_t> sendPayload;
		std::unordered_map<int, std::shared_ptr<PerTargetData>> targetData;
	};

	struct ReceiveEvent
	{
		int source;
		eastl::bitvector<> ackedBits;

		// completion information (used to handle lingering packets after full reassembly)
		bool completed;
		std::chrono::milliseconds timeLastAck;

		// we don't reassemble in-flight since a malicious source may try to get us to allocate many in-flight packets to cause memory exhaustion
		std::map<uint32_t, std::tuple<size_t, std::unique_ptr<uint8_t[]>>> packetData;
	};

private:
	void HandleReceivedPacket(int source, const std::shared_ptr<ReceiveEvent>& event);

private:
	std::unordered_map<EventId, std::shared_ptr<SendEvent>> m_sendList;

	std::map<std::tuple<int, EventId>, std::shared_ptr<ReceiveEvent>> m_receiveList;

	std::set<int> m_targets;

	std::shared_mutex m_listMutex;

	ResourceManager* m_resourceManager;

	EventReassemblySink* m_sink;

	EventId m_eventId;

	std::chrono::milliseconds m_lastTime;

	// Time of last m_receiveList cleanup
	std::chrono::milliseconds m_lastCleanup;
};

struct EventPacket
{
	uint64_t eventId;
	uint32_t packetIdx;
	uint32_t totalPackets;
	uint32_t thisBytes;

	std::array<uint8_t, kFragmentSize> payload;

public:
	bool Parse(rl::MessageBuffer& buffer);

	bool Unparse(rl::MessageBuffer& buffer);

	inline bool IsAck()
	{
		return thisBytes == 0;
	}
};

bool EventPacket::Parse(rl::MessageBuffer& buffer)
{
	static_assert(kFragmentSize <= (1 << kFragmentSizeBits), "Too little kFragmentSizeBits.");

	uint32_t eventIdLow = 0;
	uint32_t eventIdHigh = 0;

	if (!buffer.Read(32, &eventIdLow) || !buffer.Read(32, &eventIdHigh))
	{
		return false;
	}

	eventId = ((uint64_t)eventIdHigh << 32) | eventIdLow;

	if (!buffer.Read(kPacketSizeBits, &packetIdx))
	{
		return false;
	}

	if (!buffer.Read(kPacketSizeBits, &totalPackets))
	{
		return false;
	}

	if (!buffer.Read(kFragmentSizeBits, &thisBytes))
	{
		return false;
	}
	
	if (thisBytes > 0)
	{
		buffer.ReadBits(payload.data(), std::min(size_t(thisBytes), payload.size()) * 8);
	}

	return true;
}

bool EventPacket::Unparse(rl::MessageBuffer& buffer)
{
	buffer.Write(32, uint32_t(eventId & 0xFFFFFFFF));
	buffer.Write(32, uint32_t(eventId >> 32));
	buffer.Write(kPacketSizeBits, packetIdx);
	buffer.Write(kPacketSizeBits, totalPackets);
	buffer.Write(kFragmentSizeBits, thisBytes);

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

void EventReassemblyComponentImpl::RegisterTarget(int id)
{
	std::unique_lock lock(m_listMutex);

	m_targets.insert(id);
}

void EventReassemblyComponentImpl::UnregisterTarget(int id)
{
	std::unique_lock lock(m_listMutex);

	if (m_targets.find(id) != m_targets.end())
	{
		m_targets.erase(id);

		// drop any sends/receives from this target
		m_receiveList.erase(m_receiveList.lower_bound({ id, 0 }), m_receiveList.upper_bound({ id, std::numeric_limits<EventId>::max() }));
		
		for (auto& [ _, sendPacket ] : m_sendList)
		{
			sendPacket->targetData.erase(id);
			sendPacket->targets.erase(id);
		}
	}
}

void EventReassemblyComponentImpl::TriggerEvent(int target, std::string_view eventName, std::string_view eventPayload, int bytesPerSecond)
{
	// default BPS if it's 0/negative so we won't end up with weird calculation artifacts later on
	if (bytesPerSecond <= 0)
	{
		bytesPerSecond = 25000;
	}

	// skip oversized packets
	if ((eventPayload.size() + eventName.size() + sizeof(uint16_t)) >= kMaxPacketSize)
	{
		return;
	}

	std::set<int> targets;

	if (target == -1)
	{
		std::shared_lock _(m_listMutex);
		targets = m_targets;
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

	payloadBuf.Write(16, uint32_t(eventName.size()));
	payloadBuf.WriteBits(eventName.data(), eventName.size() * 8);

	memcpy(payloadBuf.GetBuffer().data() + (payloadBuf.GetCurrentBit() / 8), eventPayload.data(), eventPayload.size());

	auto sendPacket = std::make_shared<SendEvent>();
	sendPacket->bytesPerSecond = bytesPerSecond;
	sendPacket->targets = targets;
	sendPacket->sendPayload = std::move(payloadBuf.GetBuffer());

	auto packetCount = sendPacket->sendPayload.size() / kFragmentSize;

	if ((sendPacket->sendPayload.size() % kFragmentSize) != 0)
	{
		packetCount += 1;
	}

	for (auto target : targets)
	{
		auto targetData = std::make_shared<SendEvent::PerTargetData>();
		targetData->ackBits.resize(packetCount);
		targetData->lastSend = std::chrono::milliseconds{ 0 };

		sendPacket->targetData[target] = targetData;
	}

	std::unique_lock lock(m_listMutex);
	m_sendList.insert({ m_eventId++, sendPacket });
}

void EventReassemblyComponentImpl::HandleReceivedPacket(int source, const std::shared_ptr<ReceiveEvent>& event)
{
	// reassemble the buffer
	std::vector<uint8_t> eventPayload(event->ackedBits.size() * kFragmentSize);
	size_t readSize = 0;

	for (auto& packet : event->packetData)
	{
		auto offset = packet.first * kFragmentSize;
		memcpy(eventPayload.data() + offset, std::get<1>(packet.second).get(), std::get<0>(packet.second));

		readSize += std::get<0>(packet.second);

		packet.second = std::tuple<size_t, std::unique_ptr<uint8_t[]>>{};
	}

	// parse event data
	static char eventName[65536];

	// TODO: no copying please
	rl::MessageBuffer buffer(eventPayload.data(), readSize);

	uint16_t nameLength = buffer.Read<uint16_t>(16);
	buffer.ReadBits(eventName, nameLength * 8);
	eventName[nameLength] = '\0';

	// convert the source net ID to a string
	std::string sourceStr = "net:" + std::to_string(source);

	// get the resource manager and eventing component
	fwRefContainer<fx::ResourceEventManagerComponent> eventManager = m_resourceManager->GetComponent<fx::ResourceEventManagerComponent>();

	if (m_sink->LimitEvent(source))
	{
		return;
	}

	// and queue the event
	eventManager->QueueEvent(
		std::string{ eventName },
		std::string{
			reinterpret_cast<const char*>(buffer.GetBuffer().data() + (buffer.GetCurrentBit() / 8)),
			buffer.GetBuffer().size() - (buffer.GetCurrentBit() / 8)
		},
		sourceStr
	);
}

void EventReassemblyComponentImpl::NetworkTick()
{
	auto timeNow = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
	auto dT = timeNow - m_lastTime;
	m_lastTime = timeNow;

	// handle the send list
	{
		std::unique_lock lock(m_listMutex);

		std::set<EventId> dones;

		for (auto& [ eventId, sendPacket ] : m_sendList)
		{
			double pps = (sendPacket->bytesPerSecond / (double)kFragmentSize);
			std::chrono::milliseconds latency{ uint64_t(1000 / pps) };

			std::set<int> doneTargets;

			for (int target : sendPacket->targets)
			{
				// if the target is gone, get rid of them
				if (m_targets.find(target) == m_targets.end())
				{
					doneTargets.insert(target);
					continue;
				}

				auto targetData = sendPacket->targetData[target];

				if (targetData && (targetData->lastSend + latency) < timeNow && targetData->delayNextSend < timeNow)
				{
					// burst loop so we don't 'slow down' too much at a lower tick rate
					auto resTime = dT;
					auto& ackBits = targetData->ackBits;
					auto startBit = targetData->lastBit;

					do
					{
						// All packets have been processed
						bool hasAll = true;

						auto bc = ackBits.size();

						// check if any 'early' bits still need to be acked.
						size_t firstPacketIdx = -1;
						for (size_t bit = 0; bit < targetData->lastBit; bit++)
						{
							if (!ackBits[bit])
							{
								hasAll = false;
								firstPacketIdx = bit;
								break;
							}
						}

						// find the first packet we have to send
						size_t packetIdx = -1;
						for (size_t bit = targetData->lastBit; bit < bc; bit++)
						{
							if (!ackBits[bit])
							{
								hasAll = false;
								packetIdx = bit;
								break;
							}
						}

						bool forceBurstDelay = false;
						if (hasAll)
						{
							// hey, we're done with this target!
							sendPacket->targetData[target] = {};
							doneTargets.insert(target);
							break;
						}
						else if (packetIdx == -1)
						{
							forceBurstDelay = true;

							// reset the loop to first packet that has not been ack'd
							targetData->lastBit = firstPacketIdx;
						}
						else
						{
							targetData->lastBit = (packetIdx + 1) % bc;

							// try to send them a packet
							EventPacket packet;
							packet.eventId = eventId;
							packet.packetIdx = packetIdx;
							packet.totalPackets = ackBits.size();

							size_t offset = (packetIdx * kFragmentSize);
							size_t size = std::min(sendPacket->sendPayload.size() - offset, size_t(kFragmentSize));

							memcpy(packet.payload.data(), sendPacket->sendPayload.data() + offset, size);
							packet.thisBytes = size;

							rl::MessageBuffer buf(1536);
							packet.Unparse(buf);

							m_sink->SendPacket(target, std::string_view{ (char*)buf.GetBuffer().data(), buf.GetDataLength() });

							// cut down time
							if (resTime > latency)
							{
								resTime -= latency;
							}
						}

						// if we've cycled around fully, set next send time and break out
						if (targetData->lastBit <= startBit || forceBurstDelay)
						{
							targetData->delayNextSend = timeNow + std::chrono::milliseconds(250);
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
		}

		// remove dones
		for (auto done : dones)
		{
			m_sendList.erase(done);
		}

		// Cleanup ReceiveEvents 2 minutes after completion (& last ACK).
		static const std::chrono::milliseconds gc_time(120000);
		for (auto it = m_receiveList.cbegin(); it != m_receiveList.cend(); /* no increment */)
		{
			if (it->second->completed && (timeNow - it->second->timeLastAck) >= gc_time)
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

void EventReassemblyComponentImpl::HandlePacket(int source, std::string_view data)
{
	rl::MessageBuffer buffer(data.data(), data.size());

	EventPacket packet;
	if (!packet.Parse(buffer))
	{
		return;
	}

	if (packet.IsAck())
	{
		std::unique_lock lock(m_listMutex);
		auto entryIt = m_sendList.find(packet.eventId);

		if (entryIt != m_sendList.end())
		{
			std::shared_ptr<SendEvent> sendData = entryIt->second;

			auto targetDataIt = sendData->targetData.find(source);
			
			if (targetDataIt != sendData->targetData.end() && targetDataIt->second)
			{
				auto& ackBits = targetDataIt->second->ackBits;

				if (packet.packetIdx < ackBits.size())
				{
					ackBits.set(packet.packetIdx, true);
				}
			}
		}
	}
	else
	{
		std::unique_lock lock(m_listMutex);
		auto entryIt = m_receiveList.find({ source, packet.eventId });

		std::shared_ptr<ReceiveEvent> receiveData;

		if (entryIt == m_receiveList.end())
		{
			receiveData = std::make_shared<ReceiveEvent>();
			receiveData->ackedBits.resize(packet.totalPackets);
			receiveData->source = source;
			receiveData->completed = false;
			receiveData->timeLastAck = std::chrono::milliseconds{ 0 };

			m_receiveList.insert({ { source, packet.eventId }, receiveData });
		}
		else
		{
			receiveData = entryIt->second;
		}

		// note down as acked
		auto& ackBits = receiveData->ackedBits;
		bool ackedPacket = (packet.packetIdx < ackBits.size()) ? ackBits[packet.packetIdx] : false;

		// Event has already been completed or acked: just send an ACK.
		if (receiveData->source == source && (receiveData->completed || ackedPacket))
		{
			rl::MessageBuffer buf(1536);
			packet.thisBytes = 0;
			packet.Unparse(buf);

			m_sink->SendPacket(source, std::string_view{ (char*)buf.GetBuffer().data(), buf.GetDataLength() });
			receiveData->timeLastAck = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
		}
		// Still rebuilding event...
		else if (receiveData->source == source)
		{
			if (packet.packetIdx < ackBits.size())
			{
				ackBits.set(packet.packetIdx, true);
			}

			// copy payload
			auto newPayload = std::unique_ptr<uint8_t[]>(new uint8_t[packet.thisBytes]);
			memcpy(newPayload.get(), packet.payload.data(), packet.thisBytes);

			receiveData->packetData[packet.packetIdx] = { size_t(packet.thisBytes), std::move(newPayload) };

			// send ack
			{
				rl::MessageBuffer buf(1536);
				packet.thisBytes = 0;
				packet.Unparse(buf);

				m_sink->SendPacket(source, std::string_view{ (char*)buf.GetBuffer().data(), buf.GetDataLength() });
				receiveData->timeLastAck = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
			}

			// queue event if we are 'full'
			bool hasAll = true;

			for (size_t bit = 0; bit < ackBits.size(); bit++)
			{
				if (!ackBits[bit])
				{
					hasAll = false;
					break;
				}
			}

			if (hasAll)
			{
				receiveData->completed = true;

				HandleReceivedPacket(source, receiveData);

				// Cleanup will now happen in NetworkTick to prevent any lingering packets from recreating
				// the ReceiveEvent.
				//m_receiveList.erase({ source, packet.eventId });
			}
		}
	}
}

fwRefContainer<EventReassemblyComponent> EventReassemblyComponent::Create()
{
	return new EventReassemblyComponentImpl();
}
}
