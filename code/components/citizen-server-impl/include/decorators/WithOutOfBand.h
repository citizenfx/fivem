#pragma once

namespace fx
{
	namespace ServerDecorators
	{
		template<typename... TOOB>
		const fwRefContainer<fx::GameServer>& WithOutOfBand(const fwRefContainer<fx::GameServer>& server)
		{
			static std::map<ENetHost*, std::function<int(ENetHost*)>> interceptionWrappers;

			std::map<std::string, std::function<void(const fwRefContainer<fx::GameServer>& server, const AddressPair& from, const std::string_view& data)>, std::less<>> processors;

			pass{ ([&]()
			{
				auto oob = TOOB();
				processors.insert({ oob.GetName(), std::bind(&std::remove_reference_t<decltype(oob)>::Process, &oob, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });
			}(), 1)... };

			server->OnHostRegistered.Connect([=](ENetHost* enHost)
			{
				interceptionWrappers[enHost] = [=](ENetHost* host)
				{
					// workaround a VS15.7 compiler bug that drops `const` qualifier in the std::function
					fwRefContainer<fx::GameServer> tempServer = server;

					if (host->receivedDataLength >= 4 && *reinterpret_cast<int*>(host->receivedData) == -1)
					{
						auto begin = host->receivedData + 4;
						auto len = host->receivedDataLength - 4;

						//ProcessOOB({ host, GetPeerAddress(host->receivedAddress) }, { begin, end });
						std::string_view sv(reinterpret_cast<const char*>(begin), len);
						auto from = GetPeerAddress(host->receivedAddress);

						// also a workaround
						AddressPair fromPair{ host, from };

						auto key = sv.substr(0, sv.find_first_of(" \n"));
						auto data = sv.substr(sv.find_first_of(" \n") + 1);

						auto it = processors.find(key);

						if (it != processors.end())
						{
							it->second(tempServer, fromPair, data);
						}

						return 1;
					}

					return 0;
				};

				// set an interceptor callback
				enHost->intercept = [](ENetHost* host, ENetEvent* event)
				{
					return interceptionWrappers[host](host);
				};
			});

			return server;
		}
	}
}
