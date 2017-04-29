/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifdef DispatchMessage
#undef DispatchMessage
#endif

namespace terminal
{
class Dispatcher : public fwRefCountable
{
public:
	virtual void DispatchMessage(uint32_t messageType, uint32_t messageID, const std::vector<uint8_t>& messageData) = 0;
};
}