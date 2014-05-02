#include "StdInc.h"
#include "NetLibrary.h"

NetChannel::NetChannel()
{
	NetAddress dummyAddress;

	Reset(dummyAddress, nullptr);
}

void NetChannel::Reset(NetAddress& target, NetLibrary* netLibrary)
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

	bool fragmented = (sequence & 0x80000000);
	uint16_t fragmentStart = 0;
	uint16_t fragmentLength = 0;

	if (fragmented)
	{
		fragmentStart = *(uint16_t*)(message + 4);
		fragmentStart = *(uint16_t*)(message + 6);

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
			m_fragmentBuffer = "";
		}

		if (fragmentStart != m_fragmentLength)
		{
			return false;
		}

		// append the buffer
		m_fragmentBuffer += std::string(message, size - 8);
		m_fragmentLength += (size - 8);

		if (fragmentLength != FRAGMENT_SIZE)
		{
			return false;
		}

		m_inSequence = sequence;
		m_fragmentLength = 0;

		*buffer = new NetBuffer(m_fragmentBuffer.c_str(), m_fragmentBuffer.size());

		return true;
	}
	else
	{
		*buffer = new NetBuffer(message, size - 4);

		m_inSequence = sequence;

		return true;
	}
}