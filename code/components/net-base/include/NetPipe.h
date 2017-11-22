/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
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