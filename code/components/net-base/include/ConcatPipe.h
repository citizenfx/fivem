/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "NetPipe.h"

namespace net
{
class
#ifdef COMPILING_NET_BASE
	DLL_EXPORT
#endif
	ConcatInputPipe : public NetPipe
{
private:
	fwRefContainer<NetPipe> m_pipe1;
	fwRefContainer<NetPipe> m_pipe2;

public:
	ConcatInputPipe(const fwRefContainer<NetPipe>& p1, const fwRefContainer<NetPipe>& p2);

	inline fwRefContainer<NetPipe> GetLeftPipe()
	{
		return m_pipe1;
	}

	inline fwRefContainer<NetPipe> GetRightPipe()
	{
		return m_pipe2;
	}

	virtual void Reset() override;

	virtual void PassPacket(Buffer data) override;
};

class
#ifdef COMPILING_NET_BASE
	DLL_EXPORT
#endif
	ConcatOutputPipe : public NetPipe
{
private:
	fwRefContainer<NetPipe> m_pipe;

	Buffer m_savedFirst;

	bool m_tickTock;

public:
	ConcatOutputPipe(const fwRefContainer<NetPipe>& p1);

	inline fwRefContainer<NetPipe> GetTargetPipe()
	{
		return m_pipe;
	}

	virtual void Reset() override;

	virtual void PassPacket(Buffer data) override;
};
}