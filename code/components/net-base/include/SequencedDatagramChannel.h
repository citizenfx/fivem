/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

#include "DatagramSink.h"

namespace net
{
class
#ifdef COMPILING_NET_BASE
	DLL_EXPORT
#endif
	SequencedDatagramChannel : public fwRefCountable
{
private:
	fwRefContainer<DatagramSink> m_sink;

	uint32_t m_sequence;

protected:
	inline fwRefContainer<DatagramSink>& GetSink()
	{
		return m_sink;
	}

	inline void SetSequence(uint32_t sequence)
	{
		m_sequence = sequence;
	}

public:
	SequencedDatagramChannel();

	virtual ~SequencedDatagramChannel();

	inline void Reset()
	{
		m_sequence = 0;
	}

	inline void SetSink(const fwRefContainer<DatagramSink>& sink)
	{
		m_sink = sink;
	}

	inline uint32_t GetSequence()
	{
		return m_sequence;
	}
};
}