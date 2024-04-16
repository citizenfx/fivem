#pragma once

#include <UdpInterceptor.h>

namespace fx
{
	namespace ServerDecorators
	{
		template <typename... TOOB>
		const fwRefContainer<fx::GameServer>& WithOutOfBand(const fwRefContainer<fx::GameServer>& server)
		{
			return WithOutOfBand<TOOB...>(server);
		}

		template <typename ServerImpl, typename... TOOB>
		const fwRefContainer<ServerImpl>& WithOutOfBandImpl(const fwRefContainer<ServerImpl>& server)
		{
			// this is static because GCC seems to get really confused when things are wrapped in a lambda
			static std::map<std::string, std::function<void(const fwRefContainer<ServerImpl>& server,
			                                                const net::PeerAddress& from,
			                                                const std::string_view& data)>, std::less<>> processors;

			([&]()
			{
				auto oob = TOOB();
				processors.insert({
					oob.GetName(),
					std::bind(&std::remove_reference_t<decltype(oob)>::Process, &oob, std::placeholders::_1,
					          std::placeholders::_2, std::placeholders::_3)
				});
			}(), ...);

			server->AddRawInterceptor([server](const uint8_t* receivedData, size_t receivedDataLength,
			                                   const net::PeerAddress& receivedAddress)
			{
				static auto interceptor = server->GetInstance()->GetComponent<fx::UdpInterceptor>();
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
					auto key = sv.substr(0, keyEnd);

					// we don't skip messages with no data, but we make sure we keep substr inside bounds
					auto data = sv.substr(std::min(keyEnd + 1, len));

					auto it = processors.find(key);

					if (it != processors.end())
					{
						it->second(tempServer, from, data);
					}

					return true;
				}

				// allow external components to have a say
				bool intercepted = false;
				interceptor->OnIntercept(receivedAddress, receivedData, receivedDataLength, &intercepted);

				return intercepted;
			});

			return server;
		}
	}
}
