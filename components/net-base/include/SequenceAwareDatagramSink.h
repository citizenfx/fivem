/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "SequencedDatagramChannel.h"

namespace net
{
class
#ifdef COMPILING_NET_BASE
	DLL_EXPORT
#endif
	SequenceAwareDatagramSink : public DatagramSink
{
private:
	SequencedDatagramChannel* m_channel;

protected:
	inline uint32_t GetSequence()
	{
		return m_channel->GetSequence();
	}

public:
	inline void SetChannel(const fwRefContainer<SequencedDatagramChannel>& channel)
	{
		m_channel = channel.GetRef();
	}
};
}