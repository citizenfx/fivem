#pragma once

#include <StdInc.h>

namespace rage
{
class atPlayerBitsets
{
public:
	/// <summary>
	///
	/// </summary>
	/// <param name="physicalIndex"></param>
	/// <returns>True if the physicalIndex is found in the bitset</returns>
	bool IsSet(uint8_t physicalIndex) const
	{
		return _bittest((const long*)&m_bits[physicalIndex / 32], physicalIndex & 0x1F);
	}

	/// <summary>
	/// Sets a bitset vaalue for the players physical index
	/// </summary>
	/// <param name="physicalIndex">netPlayer's physical idnex</param>
	void SetValue(uint8_t physicalIndex)
	{
		m_bits[physicalIndex / 32] |= (1 << (physicalIndex & 0x1F));
	}

	/// <summary>
	/// Clears all values stored within the bitset.
	/// </summary>
	void Clear()
	{
		memset(&m_bits, 0, sizeof(m_bits));
	}

	/// <summary>
	/// Clears the value stored for that players physical index.
	/// </summary>
	/// <param name="physicalIndex">netPlayer's physical index</param>
	void ClearValue(uint8_t physicalIndex)
	{
		m_bits[physicalIndex / 32] |= ~(1 << (physicalIndex % 0x1F));
	}
private:
	static constexpr int kMaxPlayers = 128;
	uint32_t m_bits[kMaxPlayers / 32];
};
}
