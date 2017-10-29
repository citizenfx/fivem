#pragma once

namespace rl
{
class MessageBuffer
{
public:
	inline MessageBuffer()
		: m_curBit(0), m_maxBit(0)
	{

	}

	inline MessageBuffer(const std::vector<uint8_t>& data)
		: m_data(data), m_curBit(0), m_maxBit(data.size() * 8)
	{
		
	}

	inline bool ReadBitsSingle(void* out, int length)
	{
		int startIdx = m_curBit / 8;
		int shift = m_curBit % 8;
		
		uint32_t retval = (uint8_t)(m_data[startIdx] << shift);
		startIdx++;

		if (startIdx >= m_data.size())
		{
			m_curBit += length;
			return false;
		}

		if (length > 8)
		{
			int remaining = ((length - 9) / 8) + 1;

			while (remaining > 0)
			{
				uint32_t thisVal = (uint32_t)(m_data[startIdx] << shift);
				startIdx++;

				retval = thisVal | (retval << 8);

				remaining--;
			}
		}

		auto leftover = (startIdx < m_data.size()) ? m_data[startIdx] : 0;

		retval = (uint32_t)(retval | (uint8_t)(leftover >> (8 - shift))) >> (((length + 7) & 0xF8) - length);

		m_curBit += length;

		*(uint32_t*)out = retval;

		return true;
	}

	inline std::vector<uint8_t> ReadBits(int length)
	{
		std::vector<uint8_t> retVal((length / 8) + ((length % 8 != 0) ? 1 : 0));

		// for reading as int the offset is needed however
		int oi = 0;

		for (int i = 0; i < length; i++)
		{
			int bit = ReadBit();

			// write bit
			{
				int startIdx = oi / 8;
				int shift = (7 - (oi % 8));

				retVal[startIdx] = (uint8_t)((retVal[startIdx]) | (bit << shift));
			}

			oi++;
		}

		return retVal;
	}

	inline uint8_t ReadBit()
	{
		int startIdx = m_curBit / 8;
		int shift = (7 - (m_curBit % 8));

		if (startIdx >= m_data.size())
		{
			return 0;
		}

		uint32_t retval = (uint8_t)(m_data[startIdx] >> shift);

		m_curBit++;

		return (uint8_t)(retval & 1);
	}

	inline bool RequireLength(int length)
	{
		return ((m_curBit + length) < m_maxBit);
	}

	template<typename T>
	inline T Read(int length)
	{
		static_assert(sizeof(T) <= 4, "maximum of 32 bit read");

		T val = T{};
		ReadBitsSingle(&val, length);

		return val;
	}

	inline float ReadFloat(int length, float divisor)
	{
		auto integer = Read<int>(length);

		float max = (1 << length) - 1;
		return ((float)integer / max) * divisor;
	}

	inline uint64_t ReadLong(int length)
	{
		if (length <= 32)
		{
			return Read<uint32_t>(length);
		}
		else
		{
			return Read<uint32_t>(32) | (((uint64_t)Read<uint32_t>(length - 32)) << 32);
		}
	}

	inline MessageBuffer Clone()
	{
		auto leftovers = ReadBits(m_maxBit - m_curBit);

		return MessageBuffer{ leftovers };
	}

	inline void Align()
	{
		int r = m_curBit % 8;

		if (r != 0)
		{
			m_curBit += (8 - r);
		}
	}

	inline uint32_t GetCurrentBit()
	{
		return m_curBit;
	}

	inline void SetCurrentBit(uint32_t bit)
	{
		m_curBit = bit;
	}

	inline bool IsAtEnd()
	{
		return m_curBit >= m_maxBit;
	}

	inline const std::vector<uint8_t>& GetBuffer()
	{
		return m_data;
	}

	inline size_t GetLength()
	{
		return m_data.size();
	}

private:
	std::vector<uint8_t> m_data;
	int m_curBit;
	int m_maxBit;
};
}
