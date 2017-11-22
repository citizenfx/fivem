/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

namespace net
{
class
#ifdef COMPILING_NET_BASE
	DLL_EXPORT
#endif
	DatagramSink : public fwRefCountable
{
public:
	virtual ~DatagramSink() {}

	virtual void WritePacket(const std::vector<uint8_t>& packet) = 0;
};
}