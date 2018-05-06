/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "MumbleClientImpl.h"
#include "MumbleMessageHandler.h"
#include "PacketDataStream.h"

static MumbleMessageHandler handler(MumbleMessageType::UDPTunnel, [] (const uint8_t* data, size_t size)
{
	auto client = MumbleClient::GetCurrent();

	PacketDataStream pds(reinterpret_cast<const char*>(data), size);
	
	uint8_t header;
	uint64_t sessionId;
	uint64_t sequenceNumber;

	header = pds.next8();
	pds >> sessionId;
	pds >> sequenceNumber;

	if (header == 0x20 || (header >> 5) != 4)
	{
		return;
	}

	auto user = client->GetState().GetUser(uint32_t(sessionId));

	if (!user)
	{
		return;
	}

	uint64_t packetLength = 0;

	do 
	{
		pds >> packetLength;

		size_t len = (packetLength & 0x1FFF);
		std::vector<uint8_t> bytes(len);

		if (len > pds.left())
		{
			break;
		}

		for (size_t i = 0; i < len; i++)
		{
			if (pds.left() == 0)
			{
				return;
			}

			uint8_t b = pds.next8();
			bytes[i] = b;
		}

		if (bytes.empty())
		{
			break;
		}

		client->GetOutput().HandleClientVoiceData(*user, sequenceNumber, bytes.data(), bytes.size());

		break;
	} while ((packetLength & 0x2000) == 0);

	if (pds.left() >= 12)
	{
		float pos[3];
		pds >> pos[0];
		pds >> pos[1];
		pds >> pos[2];

		client->GetOutput().HandleClientPosition(*user, pos);
	}

	printf("\n");
});
