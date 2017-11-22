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