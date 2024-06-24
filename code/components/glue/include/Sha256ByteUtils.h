#pragma once
#include <cstdint>
#include <unordered_set>

namespace cfx
{
namespace puremode
{

	struct Sha256Result
	{
		uint64_t part1;
		uint64_t part2;
		uint64_t part3;
		uint64_t part4;

		Sha256Result()
		{
			part1 = 0;
			part2 = 0;
			part3 = 0;
			part4 = 0;
		}

		constexpr Sha256Result(uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4)
			: part1(p1), part2(p2), part3(p3), part4(p4)
		{
		}

		bool operator==(const Sha256Result& other) const
		{
			return part1 == other.part1 && part2 == other.part2 && part3 == other.part3 && part4 == other.part4;
		}
	};

	// Define a hash function for Sha256Result
	struct Sha256ResultHash
	{
		::std::size_t operator()(const Sha256Result& result) const
		{
			::std::size_t hash = 0;
			// Combine the hash values of the parts
			// 0x9e3779b9 is 32bit but doesn't matter for hash_set
			hash ^= ::std::hash<uint64_t>{}(result.part1) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= ::std::hash<uint64_t>{}(result.part2) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= ::std::hash<uint64_t>{}(result.part3) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= ::std::hash<uint64_t>{}(result.part4) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			return hash;
		}
	};

	constexpr uint8_t hexCharToByte(char c)
	{
		if (c >= '0' && c <= '9')
		{
			return c - '0';
		}
		else if (c >= 'a' && c <= 'f')
		{
			return c - 'a' + 10;
		}
		else if (c >= 'A' && c <= 'F')
		{
			return c - 'A' + 10;
		}
		else
		{
			return 0;
		}
	}

	constexpr uint8_t hexStrToByte(const char* str, size_t index)
	{
		return (hexCharToByte(str[index * 2]) << 4) | hexCharToByte(str[index * 2 + 1]);
	}

	template<typename T>
	constexpr T SwapEndian(T u)
	{
		union
		{
			T u;
			unsigned char u8[sizeof(T)];
		} source, dest;

		source.u = u;

		for (size_t k = 0; k < sizeof(T); k++)
		{
			dest.u8[k] = source.u8[sizeof(T) - k - 1];
		}

		return dest.u;
	}

	constexpr Sha256Result ShaUnpack(const char* hash)
	{
		uint64_t part1 = 0;
		uint64_t part2 = 0;
		uint64_t part3 = 0;
		uint64_t part4 = 0;

		for (size_t i = 0; i < 32; ++i)
		{
			uint8_t byte = hexStrToByte(hash, i);
			if (i < 8)
			{
				part1 = (part1 << 8) | byte;
			}
			else if (i < 16)
			{
				part2 = (part2 << 8) | byte;
			}
			else if (i < 24)
			{
				part3 = (part3 << 8) | byte;
			}
			else
			{
				part4 = (part4 << 8) | byte;
			}
		}

		return Sha256Result(SwapEndian<uint64_t>(part1), SwapEndian<uint64_t>(part2), SwapEndian<uint64_t>(part3), SwapEndian<uint64_t>(part4));
	}

}
}

// std::hash implementation for Sha256Result to make it usable with std::unordered_set
namespace std
{
template<>
struct hash<cfx::puremode::Sha256Result>
{
	std::size_t operator()(const cfx::puremode::Sha256Result& result) const
	{
		return cfx::puremode::Sha256ResultHash{}(result);
	}
};
}
