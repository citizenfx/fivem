/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "SequencedInputDatagramChannel.h"

namespace net
{
SequencedInputDatagramChannel::SequencedInputDatagramChannel()
	: SequencedDatagramChannel()
{

}

void SequencedInputDatagramChannel::ProcessPacket(const std::vector<uint8_t>& packet)
{
	if (packet.size() <= 4)
	{
		return;
	}

	uint32_t thisSequence = *reinterpret_cast<const uint32_t*>(&packet[0]);

	if (thisSequence == 0xFFFFFFFF)
	{
		// TODO: handle OOB requests
		return;
	}

	uint32_t lastSequence = GetSequence();

	if (thisSequence <= lastSequence)
	{
		trace("out-of-order or duplicate packet (%u, %u)\n", thisSequence, lastSequence);
		return;
	}

	if (thisSequence != (lastSequence + 1))
	{
		trace("dropped packet (%u, %u)\n", thisSequence, lastSequence);
	}

	SetSequence(thisSequence);

	// copy packet and write it back
	std::vector<uint8_t> nextPacket(packet.size() - 4);
	memcpy(&nextPacket[0], &packet[4], nextPacket.size());

	GetSink()->WritePacket(nextPacket);
}
}

#ifdef fake_tests
#include "SequencedOutputDatagramChannel.h"

class StoreOutDatagramSink : public net::DatagramSink
{
private:
	std::vector<uint8_t> m_lastPacket;

public:
	virtual void WritePacket(const std::vector<uint8_t>& packet) override
	{
		m_lastPacket = packet;
	}

	const std::vector<uint8_t>& GetPacket()
	{
		return m_lastPacket;
	}
};

static InitFunction initFunction([] ()
{
	net::SequencedInputDatagramChannel recvChannel;
	net::SequencedOutputDatagramChannel sendChannel;

	fwRefContainer<StoreOutDatagramSink> sendSink(new StoreOutDatagramSink());
	fwRefContainer<StoreOutDatagramSink> recvSink(new StoreOutDatagramSink());

	recvChannel.SetSink(recvSink);
	sendChannel.SetSink(sendSink);

	std::vector<uint8_t> dummyPacket(4);

	// sequence: 1
	sendChannel.WritePacket(dummyPacket);
	recvChannel.ProcessPacket(sendSink->GetPacket());

	// sequence: 2
	sendChannel.WritePacket(dummyPacket);
	recvChannel.ProcessPacket(sendSink->GetPacket());

	// sequence: 2 - again? OOPS
	recvChannel.ProcessPacket(sendSink->GetPacket());

	std::vector<uint8_t> seqTwo = sendSink->GetPacket();

	// sequence: 3 - drop it
	sendChannel.WritePacket(dummyPacket);

	// sequence: 4 - send it this time
	sendChannel.WritePacket(dummyPacket);
	recvChannel.ProcessPacket(sendSink->GetPacket());

	// sequence: 2 - THIS GUY SURE LOVES SEQUENCE 2
	recvChannel.ProcessPacket(seqTwo);

	__debugbreak();
});
#endif