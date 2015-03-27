/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NetUdpSocket.h"

namespace net
{
UdpSocket::UdpSocket(AddressFamily addressFamily)
{
	EnsureNetInitialized();

	m_addressFamily = addressFamily;

	m_socket = socket((int)addressFamily, SOCK_DGRAM, IPPROTO_UDP);

	if (!IsValidSocket())
	{
		trace("Failed to create socket - error code %d\n", GetLastNetError());
	}
}

UdpSocket::~UdpSocket()
{
	closesocket(m_socket);

	m_socket = (PlatformSocketType)-1;
}

bool UdpSocket::Bind(const PeerAddress& localAddress)
{
	if (localAddress.GetAddressFamily() != (int)m_addressFamily)
	{
		trace("Failed to bind socket - address family %d does not match %d.\n", m_addressFamily, localAddress.GetAddressFamily());
		return false;
	}

	if (!IsValidSocket())
	{
		trace("Failed to bind socket - socket is invalid.\n");
		return false;
	}

	int result = bind(m_socket, localAddress.GetSocketAddress(), localAddress.GetSocketAddressLength());

	if (result != 0)
	{
		trace("Failed to bind socket - result code %d.\n", GetLastNetError());
		return false;
	}

	return true;
}

PeerAddress UdpSocket::GetLocalAddress()
{
	sockaddr_storage addr = { 0 };
	socklen_t addrlen = sizeof(addr);

	if (IsValidSocket())
	{
		getsockname(m_socket, reinterpret_cast<sockaddr*>(&addr), &addrlen);
	}

	return PeerAddress(reinterpret_cast<sockaddr*>(&addr), addrlen);
}

bool UdpSocket::ReceiveFrom(std::vector<uint8_t>& outArray, int* outLength, PeerAddress* outAddress)
{
	if (!IsValidSocket())
	{
		trace("Failed to receive from socket - socket is not valid.\n");

		return false;
	}

	sockaddr_storage fromAddr = { 0 };
	socklen_t fromLen = sizeof(fromAddr);

	int len = recvfrom(m_socket, reinterpret_cast<char*>(&outArray[0]), outArray.size(), 0, reinterpret_cast<sockaddr*>(&fromAddr), &fromLen);

	if (len < 0)
	{
		int lastError = GetLastNetError();

		if (lastError != EAGAIN)
		{
			trace("Failed to receive from socket - error code %d.\n", lastError);
		}

		return false;
	}

	if (outLength)
	{
		*outLength = len;
	}

	if (outAddress)
	{
		*outAddress = PeerAddress(reinterpret_cast<sockaddr*>(&fromAddr), fromLen);
	}

	return true;
}

boost::optional<std::vector<uint8_t>> UdpSocket::ReceiveFrom(size_t size, PeerAddress* outAddress)
{
	std::vector<uint8_t> outBuffer(size);
	int outLength;

	bool result = ReceiveFrom(outBuffer, &outLength, outAddress);

	boost::optional<decltype(outBuffer)> retval;

	if (result)
	{
		outBuffer.resize(outLength);

		retval = outBuffer;
	}

	return retval;
}

bool UdpSocket::SendTo(const std::vector<uint8_t>& data, const PeerAddress& outAddress)
{
	if (!IsValidSocket())
	{
		trace("Failed to send to socket - socket is not valid.\n");

		return false;
	}

	int len = sendto(m_socket, reinterpret_cast<const char*>(&data[0]), data.size(), 0, outAddress.GetSocketAddress(), outAddress.GetSocketAddressLength());

	if (len < 0)
	{
		int lastError = GetLastNetError();

		if (lastError != EAGAIN)
		{
			trace("Failed to send to socket - error code %d.\n");

			return false;
		}
	}

	return true;
}
}