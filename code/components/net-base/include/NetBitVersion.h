#pragma once

namespace net
{
	constexpr int HexIntToDecimal(int x)
	{
		int decimalValue = 0;
		int base = 1;

		while (x > 0)
		{
			const int lastDigit = x % 10;
			decimalValue += lastDigit * base;
			base *= 16;
			x /= 10;
		}

		return decimalValue;
	}

	constexpr uint64_t BuildNetVersion(const uint16_t year, const uint8_t month, const uint8_t day, const uint8_t hour,
	                                   const uint8_t minute)
	{
		return (static_cast<uint64_t>(HexIntToDecimal(year)) << 32) |
			(static_cast<uint64_t>(HexIntToDecimal(month)) << 24) |
			(static_cast<uint64_t>(HexIntToDecimal(day)) << 16) |
			(static_cast<uint64_t>(HexIntToDecimal(hour)) << 8) |
			static_cast<uint64_t>(HexIntToDecimal(minute));
	}

	enum class NetBitVersion : uint64_t
	{
		/// <summary>
		/// Last Net Version that has support.
		/// </summary>
		netVersion1 = BuildNetVersion(2021, 03, 29, 20, 50),

		/// <summary>
		/// Added new stateBagHandlerV2.
		/// </summary>
		netVersion2 = BuildNetVersion(2024, 05, 01, 00, 00),

		/// <summary>
		/// Improve onesync_population variable.
		/// </summary>
		netVersion3 = BuildNetVersion(2024, 07, 01, 00, 00),

		/// <summary>
		/// Added new netGameEventV2.
		/// </summary>
		netVersion4 = BuildNetVersion(2024, 8, 01, 00, 00),

		/// <summary>
		/// Added new msgReassembledEventV2.
		/// </summary>
		netVersion5 = BuildNetVersion(2025, 01, 01, 00, 00)
	};
}
