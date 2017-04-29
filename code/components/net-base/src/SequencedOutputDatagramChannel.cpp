/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "SequencedOutputDatagramChannel.h"

namespace net
{
SequencedOutputDatagramChannel::SequencedOutputDatagramChannel()
	: SequencedDatagramChannel()
{

}

void SequencedOutputDatagramChannel::WritePacket(const std::vector<uint8_t>& packet)
{
	// copy packet and write it back
	std::vector<uint8_t> nextPacket(packet.size() + 4);
	memcpy(&nextPacket[4], &packet[0], nextPacket.size());

	// write sequence to the packet
	SetSequence(GetSequence() + 1);
	*reinterpret_cast<uint32_t*>(&nextPacket[0]) = GetSequence();

	GetSink()->WritePacket(nextPacket);
}
}