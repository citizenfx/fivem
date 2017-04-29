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
	SequencedInputDatagramChannel : public SequencedDatagramChannel
{
public:
	SequencedInputDatagramChannel();

	void ProcessPacket(const std::vector<uint8_t>& packet);
};
}