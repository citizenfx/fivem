#pragma once

namespace net
{
// todo: replace with std::byteswap when cpp23 is used

inline uint64_t htonll(uint64_t hostLongLong)
{
	return ((((hostLongLong) >> 56) & 0x00000000000000FFLL) |
		(((hostLongLong) >> 40) & 0x000000000000FF00LL) |
		(((hostLongLong) >> 24) & 0x0000000000FF0000LL) |
		(((hostLongLong) >> 8) & 0x00000000FF000000LL) |
		(((hostLongLong) << 8) & 0x000000FF00000000LL) |
		(((hostLongLong) << 24) & 0x0000FF0000000000LL) |
		(((hostLongLong) << 40) & 0x00FF000000000000LL) |
		(((hostLongLong) << 56) & 0xFF00000000000000LL));
}

inline uint32_t htonl(uint32_t hostLong)
{
	return ((hostLong >> 24) & 0x000000FF) |
		   ((hostLong >> 8) & 0x0000FF00) |
		   ((hostLong << 8) & 0x00FF0000) |
		   ((hostLong << 24) & 0xFF000000);
}

inline uint16_t htons(const uint16_t hostShort)
{
	return ((((hostShort) >> 8) & 0xff) | (((hostShort) & 0xff) << 8));
}

template<typename Type>
inline Type hton(Type net)
{
	if constexpr (std::is_same_v<decltype(net), uint16_t>)
	{
		return net::htons(net);
	}
	else if constexpr (std::is_same_v<decltype(net), uint32_t>)
	{
		return net::htonl(net);
	}
	else if constexpr (std::is_same_v<decltype(net), uint64_t>)
	{
		return net::htonll(net);
	}
	else
	{
		static_assert(!std::is_same_v<Type,Type>, "Unsupported type for hton");
		return Type{};
	}
}

inline uint64_t ntohll(uint64_t netLongLong)
{
	return htonll(netLongLong);
}

inline uint32_t ntohl(uint32_t netLong)
{
	return htonl(netLong);
}

inline uint16_t ntohs(uint16_t netShort)
{
	return htons(netShort);
}

template<typename Type>
inline Type ntoh(Type net)
{
	if constexpr (std::is_same_v<decltype(net), uint16_t>)
	{
		return net::ntohs(net);
	}
	else if constexpr (std::is_same_v<decltype(net), uint32_t>)
	{
		return net::ntohl(net);
	}
	else if constexpr (std::is_same_v<decltype(net), uint64_t>)
	{
		return net::ntohll(net);
	}
	else
	{
		static_assert(!std::is_same_v<Type,Type>, "Unsupported type for ntoh");
		return Type{};
	}
}
}
