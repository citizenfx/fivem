/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NetPeerBase.h"

namespace net
{
PeerBase::PeerBase(const fwRefContainer<DatagramSink>& outSink)
	: m_outSink(outSink), m_inputChannel(new SequencedInputDatagramChannel()), m_outputChannel(new SequencedOutputDatagramChannel()), m_components(new RefInstanceRegistry())
{
	m_inSink = new FunctionDatagramSink([=] (const std::vector<uint8_t>& packet)
	{
		return ProcessEncapsulatedPacket(packet);
	});

	m_outputChannel->SetSink(outSink);
}

void PeerBase::ProcessPacket(const std::vector<uint8_t>& buffer)
{
	m_inputChannel->ProcessPacket(buffer);
}

int PeerBase::ReadCompressedType(Buffer& buffer)
{
	uint8_t lead = buffer.Read<uint8_t>();
	int result;

	if (lead == 0)
	{
		result = -1;
	}
	else if (lead & 0x80)
	{
		result = buffer.Read<uint8_t>() << 7;
		result |= (lead & ~0x80);
	}

	return result;
}

void PeerBase::ProcessEncapsulatedPacket(const std::vector<uint8_t>& buffer)
{
	Buffer netBuffer(buffer);

	// if we don't have a list of remote trusted packets, only expect such
	if (m_remoteToLocalMapping.empty())
	{
		int type = ReadCompressedType(netBuffer);

		if (type == 1)
		{
			ProcessMappingPacket(netBuffer);
		}
	}
}

void PeerBase::ProcessMappingPacket(Buffer& buffer)
{
	// while the packet type isn't -1
	int type;

	do
	{
		type = ReadCompressedType(buffer);

		if (type >= 0)
		{
			uint32_t mappedType = buffer.Read<uint32_t>();

			if (m_processors.find(mappedType) == m_processors.end())
			{
				trace("Peer %s knows to send mapped type 0x%08x, but we don't know to handle it...\n", GetName().c_str(), mappedType);
			}

			m_remoteToLocalMapping[type] = mappedType;
		}
	} while (type >= 0);
}
}