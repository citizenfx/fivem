/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NetLibrary.h"

NetChannel::NetChannel()
{
	NetAddress dummyAddress;

	Reset(dummyAddress, nullptr);
}

void NetChannel::Reset(NetAddress& target, NetLibraryImplBase* netLibrary)
{
	m_fragmentBuffer = "";
	m_fragmentLength = 0;
	m_fragmentSequence = -1;

	m_inSequence = 0;
	m_outSequence = 0;

	m_targetAddress = target;
	m_netLibrary = netLibrary;
}

void NetChannel::Send(NetBuffer& buffer)
{
	if (buffer.GetCurLength() > FRAGMENT_SIZE)
	{
		return SendFragmented(buffer);
	}

	static char msgBuffer[FRAGMENT_SIZE + 100];
	*(uint32_t*)(msgBuffer) = m_outSequence;
	memcpy(&msgBuffer[4], buffer.GetBuffer(), buffer.GetCurLength());

	m_netLibrary->SendData(m_targetAddress, msgBuffer, buffer.GetCurLength() + 4);

	m_outSequence++;
}

void NetChannel::SendFragmented(NetBuffer& buffer)
{
	uint32_t outSequence = m_outSequence | 0x80000000;
	uint32_t remaining = buffer.GetCurLength();
	uint16_t i = 0;

	assert(buffer.GetCurLength() < 65536);

	while (remaining >= 0)
	{
		uint16_t thisSize = min(remaining, FRAGMENT_SIZE);

		// build this packet
		static char msgBuffer[FRAGMENT_SIZE + 100];
		*(uint32_t*)(&msgBuffer[0]) = outSequence;
		*(uint16_t*)(&msgBuffer[4]) = i;
		*(uint16_t*)(&msgBuffer[6]) = thisSize;
		memcpy(&msgBuffer[8], buffer.GetBuffer() + i, thisSize);

		m_netLibrary->SendData(m_targetAddress, msgBuffer, thisSize + 8);

		// decrement counters
		remaining -= thisSize;
		i += thisSize;

		// if the last packet didn't measure the fragment size
		if (remaining == 0 && thisSize != FRAGMENT_SIZE)
		{
			break;
		}
	}

	m_outSequence++;
}

bool NetChannel::Process(const char* message, size_t size, NetBuffer** buffer)
{
	uint32_t sequence = *(uint32_t*)(message);

	bool fragmented = ((sequence & 0x80000000) != 0);
	uint16_t fragmentStart = 0;
	uint16_t fragmentLength = 0;

	if (fragmented)
	{
		fragmentStart = *(uint16_t*)(message + 4);
		fragmentLength = *(uint16_t*)(message + 6);

		sequence &= ~0x80000000;

		message += 8;
	}
	else
	{
		message += 4;
	}

	if (sequence <= m_inSequence && m_inSequence != 0)
	{
		trace("out of order packet (%d, %d)\n", sequence, m_inSequence);

		return false;
	}

	if (sequence > (m_inSequence + 1))
	{
		trace("dropped packet (%d, %d)\n", sequence, m_inSequence);

		// don't return, we still accept these
	}

	if (fragmented)
	{
		if (sequence != m_fragmentSequence)
		{
			m_fragmentLength = 0;
			m_fragmentSequence = sequence;
			m_fragmentBuffer = new char[65536];
			m_fragmentValidSet.reset();
			m_fragmentLastBit = -1;
		}

		int fragmentBit = fragmentStart / FRAGMENT_SIZE;

		if (fragmentBit > ((65536 / FRAGMENT_SIZE) - 1))
		{
			return false;
		}

		if (m_fragmentValidSet.test(fragmentBit))
		{
			return false;
		}

		m_fragmentValidSet.set(fragmentBit, true);

		// append the buffer
		memcpy(&m_fragmentBuffer[fragmentBit * FRAGMENT_SIZE], message, size - 8);
		m_fragmentLength += (size - 8);

		if (fragmentLength != FRAGMENT_SIZE)
		{
			m_fragmentLastBit = fragmentBit;
		}

		// check all bits up to fragmentLastBit for validity
		if (m_fragmentLastBit == -1)
		{
			return false;
		}

		if ((int)m_fragmentValidSet.count() <= m_fragmentLastBit)
		{
			return false;
		}

		m_inSequence = sequence;

		*buffer = new NetBuffer(m_fragmentBuffer, m_fragmentLength);

		m_fragmentLength = 0;

		return true;
	}
	else
	{
		*buffer = new NetBuffer(message, size - 4);

		m_inSequence = sequence;

		return true;
	}
}