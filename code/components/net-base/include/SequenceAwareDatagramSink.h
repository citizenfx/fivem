/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

#include "SequencedDatagramChannel.h"

namespace net
{
template<typename TDerived>
class
	SequenceAwareDatagramSink : public DatagramSink
{
private:
	SequencedDatagramChannel* m_channel;

public:
	inline uint32_t GetSequence()
	{
		return m_channel->GetSequence();
	}

public:
	inline void SetChannel(const fwRefContainer<SequencedDatagramChannel>& channel)
	{
		m_channel = channel.GetRef();
	}

	template<typename... Args>
	fwRefContainer<TDerived> Clone(const Args&& args)
	{
		fwRefContainer<TDerived> secondThing(new TDerived(args...));
		secondThing->SetChannel(m_channel);

		return secondThing;
	}
};
}