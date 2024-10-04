#pragma once

#include <UdpInterceptor.h>

#include <ForceConsteval.h>

namespace fx
{
	namespace ServerDecorators
	{
		template <typename ServerImpl, typename... TOutOfBandHandler>
		const fwRefContainer<ServerImpl>& WithOutOfBandImpl(const fwRefContainer<ServerImpl>& server)
		{
			server->AddRawInterceptor([server](const uint8_t* receivedData, size_t receivedDataLength,
			                                   const net::PeerAddress& receivedAddress)
			{
				static auto interceptor = server->GetInstance()->template GetComponent<fx::UdpInterceptor>();
				static bool setCb{false};

				if (!setCb)
				{
					interceptor->SetSendCallback(
						[server](const net::PeerAddress& address, const void* data, size_t length)
						{
							server->SendOutOfBand(
								address, std::string_view{reinterpret_cast<const char*>(data), length}, false);
						});

					setCb = true;
				}

				// workaround a VS15.7 compiler bug that drops `const` qualifier in the std::function
				fwRefContainer<ServerImpl> tempServer = server;

				if (receivedDataLength >= 4 && *reinterpret_cast<const int*>(receivedData) == -1)
				{
					const unsigned char* begin = receivedData + 4;
					const size_t len = receivedDataLength - 4;

					if (!len)
					{
						// no need to continue processing the message if it has no key and data
						// when checking receivedDataLength for length to be at least 5 the udp interceptor would be triggered for the -1 prefix.
						return true;
					}
					const std::string_view sv(reinterpret_cast<const char*>(begin), len);
					auto from = receivedAddress;

					size_t keyEnd = sv.find_first_of(" \n");
					// when keyEnd is 0 the message does not contain a processor key and can be skipped
					if (!keyEnd)
					{
						// behavior seems to be that out of band messages are always returning true when they have the -1 prefix.
						return true;
					}
					// when keyEnd is npos the message does not contain the \n seperator an the data is empty
					if (keyEnd == std::string::npos)
					{
						keyEnd = len;
					}
					auto key = HashRageString(sv.substr(0, keyEnd));

					// we don't skip messages with no data, but we make sure we keep substr inside bounds
					auto data = sv.substr(std::min(keyEnd + 1, len));

					bool handled = false;
					([&]
					{
						if (!handled && key == net::force_consteval<uint32_t, HashRageString(TOutOfBandHandler::GetName())>)
						{
							static TOutOfBandHandler outOfBandHandler (tempServer);
							outOfBandHandler.Process(tempServer, from, data);
							handled = true;
						}
					}(), ...);

					// when false, gives a other raw udp interceptor the chance to parse it, even when we don't have such
					return handled;
				}

				// allow external components to have a say
				bool intercepted = false;
				interceptor->OnIntercept(receivedAddress, receivedData, receivedDataLength, &intercepted);

				return intercepted;
			});

			return server;
		}

		template <typename... TOutOfBandHandler>
		const fwRefContainer<fx::GameServer>& WithOutOfBand(const fwRefContainer<fx::GameServer>& server)
		{
			return WithOutOfBandImpl<fx::GameServer, TOutOfBandHandler...>(server);
		}
	}
}
