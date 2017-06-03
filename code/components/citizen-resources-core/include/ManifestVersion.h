#pragma once

#include <fxcore.h>
#include <stdint.h>

#include <string_view>

// guid functions adapted from https://github.com/wine-mirror/wine/blob/b399bafa121aa9358d03c55d6eed1e762b3c535d/dlls/ole32/compobj.c
static inline constexpr bool IsValidHex(const char c)
{
	return (((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F')));
}

static constexpr const uint8_t guid_conv_table[256] =
{
	0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x00 */
	0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x10 */
	0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x20 */
	0,   1,   2,   3,   4,   5,   6, 7, 8, 9, 0, 0, 0, 0, 0, 0, /* 0x30 */
	0, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x40 */
	0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x50 */
	0, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf                             /* 0x60 */
};

static constexpr guid_t ParseGuid(const std::string_view& str)
{
	guid_t guid = { 0 };

	guid.data1 = 0;
	for (size_t i = 0; i < 8; i++)
	{
		if (!IsValidHex(str[i]))
		{
			return guid_t{ 0 };
		}

		guid.data1 = (guid.data1 << 4) | guid_conv_table[str[i]];
	}

	if (str[8] != '-')
	{
		return guid_t{ 0 };
	}

	guid.data2 = 0;
	for (size_t i = 9; i < 13; i++)
	{
		if (!IsValidHex(str[i]))
		{
			return guid_t{ 0 };
		}

		guid.data2 = (guid.data2 << 4) | guid_conv_table[str[i]];
	}

	if (str[13] != '-')
	{
		return guid_t{ 0 };
	}

	guid.data3 = 0;
	for (size_t i = 14; i < 18; i++)
	{
		if (!IsValidHex(str[i]))
		{
			return guid_t{ 0 };
		}

		guid.data3 = (guid.data3 << 4) | guid_conv_table[str[i]];
	}

	if (str[18] != '-')
	{
		return guid_t{ 0 };
	}

	for (size_t i = 19; i < 36; i += 2)
	{
		if (i == 23)
		{
			if (str[i] != '-')
			{
				return guid_t{ 0 };
			}

			i++;
		}

		if (!IsValidHex(str[i]) || !IsValidHex(str[i + 1]))
		{
			return guid_t{ 0 };
		}

		guid.data4[(i - 19) / 2] = guid_conv_table[str[i]] << 4 | guid_conv_table[str[i + 1]];
	}

	return guid;
}

struct ManifestVersion
{
	guid_t guid;

	inline constexpr ManifestVersion(const guid_t& guid)
		: guid(guid)
	{

	}

	template<int N>
	inline constexpr ManifestVersion(char const (&str)[N])
		: guid(ParseGuid(std::string_view(str, N)))
	{

	}

	inline constexpr ManifestVersion(const std::string_view& str)
		: guid(ParseGuid(str))
	{

	}
};

inline bool operator==(const ManifestVersion& left, const ManifestVersion& right)
{
	return (left.guid == right.guid);
}