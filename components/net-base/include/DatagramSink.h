/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
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