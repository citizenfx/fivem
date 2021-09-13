#pragma once

#include <UdpInterceptor.h>

namespace fx
{
	namespace ServerDecorators
	{
		template<typename... TOOB>
		const fwRefContainer<fx::GameServer>& WithOutOfBand(const fwRefContainer<fx::GameServer>& server)
		{
			// this is static because GCC seems to get really confused when things are wrapped in a lambda
			static std::map<std::string, std::function<void(const fwRefContainer<fx::GameServer>& server, const net::PeerAddress& from, const std::string_view& data)>, std::less<>> processors;

			([&]()
			{
				auto oob = TOOB();
				processors.insert({ oob.GetName(), std::bind(&std::remove_reference_t<decltype(oob)>::Process, &oob, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });
			}(), ...);

			server->AddRawInterceptor([server](const uint8_t* receivedData, size_t receivedDataLength, const net::PeerAddress& receivedAddress)
			{
				static auto interceptor = server->GetInstance()->GetComponent<fx::UdpInterceptor>();
				static bool setCb;
				
				if (!setCb)
				{
					interceptor->SetSendCallback([server](const net::PeerAddress& address, const void* data, size_t length)
					{
						server->SendOutOfBand(address, std::string_view{ reinterpret_cast<const char*>(data), length }, false);
					});

					setCb = true;
				}

				// workaround a VS15.7 compiler bug that drops `const` qualifier in the std::function
				fwRefContainer<fx::GameServer> tempServer = server;

				if (receivedDataLength >= 4 && *reinterpret_cast<const int*>(receivedData) == -1)
				{
					auto begin = receivedData + 4;
					auto len = receivedDataLength - 4;

					std::string_view sv(reinterpret_cast<const char*>(begin), len);
					auto from = receivedAddress;

					auto key = sv.substr(0, sv.find_first_of(" \n"));
					auto data = sv.substr(sv.find_first_of(" \n") + 1);

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
