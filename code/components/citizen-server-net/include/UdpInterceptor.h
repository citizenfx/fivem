#pragma once

#include <NetAddress.h>

#include <ServerInstanceBase.h>
#include <ComponentHolder.h>

namespace fx
{
	class UdpInterceptor : public fwRefCountable
	{
	public:
		//
		// Arguments:
		// - from address
		// - data
		// - data size
		// - intercepted
		//
		fwEvent<const net::PeerAddress&, const uint8_t*, size_t, bool*> OnIntercept;

		using TSendFn = std::function<void(const net::PeerAddress& address, const void* data, size_t length)>;

		inline void SetSendCallback(const TSendFn& fn)
		{
			m_sendCallback = fn;
		}

		//
		// Send a packet back to the owning socket.
		//
		inline void Send(const net::PeerAddress& address, const void* data, size_t length)
		{
			m_sendCallback(address, data, length);
		}

	private:
		TSendFn m_sendCallback;
	};
}

DECLARE_INSTANCE_TYPE(fx::UdpInterceptor);
