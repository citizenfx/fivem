/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

#include <NetBase.h>
#include <NetAddress.h>

namespace net
{
class
#ifdef COMPILING_NET_BASE
	DLL_EXPORT
#endif
	UdpSocket : public fwRefCountable
{
private:
	PlatformSocketType m_socket;

	AddressFamily m_addressFamily;

public:
	UdpSocket(AddressFamily addressFamily = AddressFamily::IPv4);

	virtual ~UdpSocket();

	inline bool IsValidSocket()
	{
		return m_socket != (PlatformSocketType)-1;
	}

	bool Bind(const PeerAddress& localAddress);

	PeerAddress GetLocalAddress();

	//void SetReceiveCallback(...);

	bool ReceiveFrom(std::vector<uint8_t>& outArray, int* outLength, PeerAddress* outAddress);

	boost::optional<std::vector<uint8_t>> ReceiveFrom(size_t size, PeerAddress* outAddress);

	bool SendTo(const std::vector<uint8_t>& data, const PeerAddress& outAddress);
};
}