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

	inline explicit MessageBuffer(size_t size)
		: m_data(size), m_curBit(0), m_maxBit(size * 8)
	{

	}

	inline MessageBuffer(const void* data, size_t size)
		: m_data(reinterpret_cast<const uint8_t*>(data), reinterpret_cast<const uint8_t*>(data) + size), m_curBit(0), m_maxBit(size * 8)
	{

	}

	inline bool ReadBitsSingle(void* out, int length)
	{
		int startIdx = m_curBit / 8;
		int shift = m_curBit % 8;
		
		if ((startIdx + (length / 8) > m_data.size()))
		{
			m_curBit += length;
			return false;
		}

		uint32_t retval = (uint8_t)(m_data[startIdx] << shift);
		startIdx++;

		if ((startIdx + (length / 8) > m_data.size()))
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

		ReadBits(retVal.data(), length);

		return retVal;
	}

	inline void ReadBits(void* data, int length)
	{
		auto byteData = (uint8_t*)data;

		for (int i = 0; i < length; i++)
		{
			int bit = ReadBit();

			// write bit
			{
				int startIdx = i / 8;
				int shift = (7 - (i % 8));

				byteData[startIdx] = (uint8_t)(((byteData[startIdx]) & ~(1 << shift)) | (bit << shift));
			}
		}
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

	inline void WriteBits(const void* data, int length)
	{
		auto byteData = (const uint8_t*)data;

		for (int i = 0; i < length; i++)
		{
			int startIdx = i / 8;
			int shift = (7 - (i % 8));

			WriteBit((byteData[startIdx] >> shift) & 1);
		}
	}

	inline void WriteBitsSingle(const void* data, int length)
	{
		auto a1 = m_data.data();
		auto a2 = *(uint32_t*)data;
		auto a3 = length;
		int a4 = m_curBit;

		m_curBit += length;

		int64_t v4; // r10
		int v5; // er9
		uint8_t *v6; // r10
		unsigned int v7; // er11
		uint64_t result; // rax
		unsigned int v9; // er11
		unsigned int v10; // ebx
		uint8_t *v11; // r10
		uint64_t v12; // rdx
		unsigned int v13; // eax
		int v14; // ecx

		v4 = (int64_t)a4 >> 3;
		v5 = a4 & 7;
		v6 = (uint8_t *)(a1 + v4);
		v7 = a2 << (32 - a3);
		result = v7 >> 24 >> v5;
		v9 = v7 << (8 - v5);
		v10 = -1 << (32 - a3) << (8 - v5);
		*v6 = result | *v6 & ~(uint8_t)((unsigned int)(-1 << (32 - a3)) >> 24 >> v5);
		v11 = v6 + 1;
		if (8 - v5 < a3)
		{
			v12 = ((unsigned int)(a3 - (8 - v5) - 1) >> 3) + 1;
			do
			{
				v13 = v9;
				v9 <<= 8;
				result = v13 >> 24;
				v14 = (uint64_t)v10 >> 24;
				v10 <<= 8;
				*v11 = result | *v11 & ~(uint8_t)v14;
				++v11;
				--v12;
			} while (v12);
		}
	}

	inline bool WriteBit(uint8_t bit)
	{
		int startIdx = m_curBit / 8;
		int shift = (7 - (m_curBit % 8));

		if (startIdx >= m_data.size())
		{
			return false;
		}

		m_data[startIdx] = (m_data[startIdx] & ~(1 << shift)) | (bit << shift);

		m_curBit++;

		return true;
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

	template<typename T>
	inline T ReadSigned(int length)
	{
		int sign = Read<int>(1);
		int data = Read<int>(length - 1);

		return T{ sign + (data ^ -sign) };
	}

	template<typename T>
	inline void Write(int length, T data)
	{
		static_assert(sizeof(T) <= 4, "maximum of 32 bit write");

		WriteBitsSingle(&data, length);
	}

	inline float ReadFloat(int length, float divisor)
	{
		auto integer = Read<int>(length);

		float max = (1 << length) - 1;
		return ((float)integer / max) * divisor;
	}

	inline float ReadSignedFloat(int length, float divisor)
	{
		auto integer = ReadSigned<int>(length);

		float max = (1 << (length - 1)) - 1;
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

	inline size_t GetDataLength()
	{
		char leftoverBit = (m_curBit % 8) ? 1 : 0;

		return (m_curBit / 8) + leftoverBit;
	}

private:
	std::vector<uint8_t> m_data;
	int m_curBit;
	int m_maxBit;
};
}
