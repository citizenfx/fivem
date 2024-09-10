#pragma once
#include "Client.h"

namespace fx
{
class ClientMetricData
{
public:
	fx::Client* thisptr;
	int channel;
	const net::Buffer buffer;
	NetPacketType flags;

	ClientMetricData(fx::Client* const thisptr, const int channel, const net::Buffer& buffer, const NetPacketType flags)
		: thisptr(thisptr),
		  channel(channel),
		  buffer(buffer),
		  flags(flags)
	{
	}
};
}
