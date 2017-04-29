/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "NetBuffer.h"

namespace net
{
class NetPipe : public fwRefCountable
{
public:
	virtual void Reset() = 0;

	virtual void PassPacket(Buffer data) = 0;
};
}