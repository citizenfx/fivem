#include <StdInc.h>

#include <sstream>
#include <IteratorView.h>

#include <boost/algorithm/string.hpp>

#include <CoreConsole.h>

#include <NetAddress.h>

#define FOLLY_NO_CONFIG

#ifdef _WIN32
#undef ssize_t
#pragma comment(lib, "iphlpapi.lib")
#else
#include <sys/types.h>
#endif

#include <folly/IPAddress.h>
#include <folly/String.h>

class NetworkList
{
public:
	NetworkList()
	{
	
	}

	explicit NetworkList(std::string_view str)
	{
		// make_split_iterator currently does not understand string_view
		auto strRef = std::string{
			str
		};

		for (auto item :
			fx::GetIteratorView(
				std::make_pair(
					boost::algorithm::make_split_iterator(
						strRef,
						boost::algorithm::token_finder(
							boost::algorithm::is_space(),
							boost::algorithm::token_compress_on
						)
					),
					boost::algorithm::split_iterator<std::string::iterator>()
				)
			)
		)
		{
			auto network = folly::IPAddress::tryCreateNetwork(folly::range(&*item.begin(), &*item.end()));
				
			if (network)
			{
				networks.push_back(*network);
			}
		}
	}

	bool ContainsIP(const net::PeerAddress& ip) const
	{
		auto sockaddr = ip.GetSocketAddress();
		folly::ByteRange ipRange;

		if (sockaddr->sa_family == AF_INET)
		{
			sockaddr_in* inAddr = (sockaddr_in*)sockaddr;
			ipRange = folly::ByteRange{ (uint8_t*)&inAddr->sin_addr, (uint8_t*)&inAddr->sin_addr + sizeof(inAddr->sin_addr) };
		}
		else if (sockaddr->sa_family == AF_INET6)
		{
			sockaddr_in6* inAddr = (sockaddr_in6*)sockaddr;
			ipRange = folly::ByteRange{ (uint8_t*)&inAddr->sin6_addr, (uint8_t*)&inAddr->sin6_addr + sizeof(inAddr->sin6_addr) };
		}

		if (!ipRange.empty())
		{
			auto ipAddress = folly::IPAddress::tryFromBinary(ipRange);

			if (ipAddress)
			{
				for (const auto& network : networks)
				{
					if (ipAddress->inSubnet(network.first, network.second))
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	bool ContainsIP(std::string_view ip) const
	{
		auto ipAddress = folly::IPAddress::tryFromString(ip);

		if (ipAddress)
		{
			for (const auto& network : networks)
			{
				if (ipAddress->inSubnet(network.first, network.second))
				{
					return true;
				}
			}
		}

		return false;
	}

	std::string ToString() const
	{
		std::stringstream ss;

		for (auto& network : networks)
		{
			ss << folly::IPAddress::networkToString(network) << " ";
		}

		// strip the final space
		auto s = ss.str();

		if (!s.empty())
		{
			s = s.substr(0, s.length() - 1);
		}

		return s;
	}

	inline bool operator==(const NetworkList& right) const
	{
		return networks == right.networks;
	}

	inline bool operator!=(const NetworkList& right) const
	{
		return !(*this == right);
	}

private:
	std::vector<folly::CIDRNetwork> networks;
};

template<>
struct ConsoleArgumentType<NetworkList>
{
	static std::string Unparse(const NetworkList& input)
	{
		return input.ToString();
	}

	static bool Parse(const std::string& input, NetworkList* out)
	{
		*out = NetworkList{
			input
		};

		return true;
	}
};

template<>
struct ConsoleArgumentName<NetworkList>
{
	inline static const char* Get()
	{
		return "NetworkList";
	}
};

ConVar<NetworkList>* g_networkListVar;

namespace fx
{
bool DLL_EXPORT IsProxyAddress(std::string_view ep)
{
	return g_networkListVar->GetValue().ContainsIP(ep);
}

bool DLL_EXPORT IsProxyAddress(const net::PeerAddress& ep)
{
	return g_networkListVar->GetValue().ContainsIP(ep);
}
}

static InitFunction initFunction([]()
{
	static ConVar<NetworkList> allowedIpCidr("sv_proxyIPRanges", ConVar_None, NetworkList{ "10.0.0.0/8 127.0.0.0/8 192.168.0.0/16 172.16.0.0/12" });

	g_networkListVar = &allowedIpCidr;
});
