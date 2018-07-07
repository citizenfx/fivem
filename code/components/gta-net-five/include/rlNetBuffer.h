#pragma once

namespace rage
{
class netBuffer
{
public:
	inline netBuffer(void* data, size_t size)
	{
		m_data = data;
		m_f8 = 0;
		m_maxBit = size * 8;
		m_unkBit = 0;
		m_curBit = 0;
		m_unk2Bit = 0;
		m_f1C = 0;
	}

	inline uint32_t GetPosition()
	{
		return m_unkBit;
	}

	inline bool Seek(int bits)
	{
		if (bits >= 0)
		{
			uint32_t length = (m_f1C & 1) ? m_maxBit : m_curBit;

			if (bits <= length)
			{
				m_unkBit = bits;
			}
		}

		return false;
	}

	inline size_t GetDataLength()
	{
		char leftoverBit = (m_curBit % 8) ? 1 : 0;

		return (m_curBit / 8) + leftoverBit;
	}

	bool ReadInteger(uint32_t* integer, int bits);

	bool WriteInteger(uint32_t integer, int bits);

	bool WriteBit(bool bit);

public:
	void* m_data;
	uint32_t m_f8;
	uint32_t m_maxBit;
	uint32_t m_unkBit;
	uint32_t m_curBit;
	uint32_t m_unk2Bit;
	uint8_t m_f1C;
};
}
