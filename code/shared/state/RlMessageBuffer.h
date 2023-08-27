#pragma once

namespace rl
{
class MessageBuffer
{
private:
	static bool GetLengthHackState();

public:
	inline MessageBuffer()
		: m_curBit(0), m_maxBit(0)
	{

	}

	inline MessageBuffer(const std::vector<uint8_t>& data)
		: m_data(data), m_curBit(0), m_maxBit(data.size() * 8)
	{
		
	}

	inline MessageBuffer(std::vector<uint8_t>&& data)
		: m_data(std::move(data)), m_curBit(0), m_maxBit(m_data.size() * 8)
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

	/// <summary>
	/// rage::datBitBuffer::ReadUnsigned = 0x14128DCF8 (1604)
	/// </summary>
	template<typename T>
	inline bool ReadBitsSingle(T* out, int length)
	{
		if (length == 13 && GetLengthHackState())
		{
			length = 16;
		}

		static_assert(std::is_integral_v<T>, "ReadBitsSingle wants an int value");

		if ((m_curBit + length) > m_maxBit)
		{
			m_curBit += length;
			return false;
		}

		int startIdx = m_curBit / 8;
		int shift = m_curBit % 8;

		uint32_t retval = (uint8_t)(m_data[startIdx] << shift);
		startIdx++;

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

		if (shift)
		{
			auto leftover = (startIdx < m_data.size()) ? m_data[startIdx] : 0;
			retval |= (uint8_t)(leftover >> (8 - shift));
		}

		retval = retval >> (((length + 7) & 0xF8) - length);

		m_curBit += length;

		// hack to prevent an out-of-bounds write of `out`
		*out = *(T*)&retval;

		return true;
	}


	inline uint8_t ReadBit()
	{
		int startIdx = m_curBit / 8;

		if (startIdx >= m_data.size())
		{
			return 0;
		}

		int shift = (7 - (m_curBit % 8));
		uint32_t retval = (uint8_t)(m_data[startIdx] >> shift);

		m_curBit++;

		return (uint8_t)(retval & 1);
	}

	inline void WriteBitsOld(const void* data, int length)
	{
		if (length == 13)
		{
			length = 16;
		}

		auto byteData = (const uint8_t*)data;

		for (int i = 0; i < length; i++)
		{
			int startIdx = i / 8;
			int shift = (7 - (i % 8));

			WriteBit((byteData[startIdx] >> shift) & 1);
		}
	}

	// copied IDA code, please improve!
	inline bool CopyBits(const void* dest, const void* source, int length, int destBitOffset, int sourceBitOffset)
	{
		auto result = (uint64_t)dest;
		auto a2 = source;
		auto a3 = length;
		int a4 = destBitOffset;
		auto a5 = sourceBitOffset;

		uint64_t v5; // x21
		int v6; // w20
		uint64_t v7; // x19
		int v8; // w11
		uint8_t* v9; // x22
		int v10; // w8
		int v11; // w10
		unsigned int v12; // w9
		unsigned int v13; // w14
		unsigned int v14; // w9
		unsigned int v15; // w10
		int v16; // w11
		uint64_t v17; // x11
		uint8_t* v18; // x12
		int v19; // w13
		unsigned int v20; // w8
		char v21; // w9
		char* v22; // x0
		int v23; // w14
		char v24; // t1
		char v25; // w10
		unsigned int v26; // w11
		int64_t v27; // x23
		uint8_t* v28; // x13
		int v29; // w16
		int v30; // t1
		unsigned int v31; // w10
		uint64_t v32; // x11
		int v33; // w8
		unsigned int v34; // w9
		unsigned int v35; // w10
		unsigned int v36; // w9
		unsigned int v37; // w10
		uint8_t* v38; // x11
		int v39; // w12
		int64_t v40; // x11
		int v41; // w12
		unsigned int v42; // w8
		unsigned int v43; // w9
		char v44; // w13
		unsigned int v45; // w8
		unsigned int v46; // w9
		uint64_t v47; // x10
		uint8_t* v48; // x11
		int v49; // w12

		v5 = a4;
		v6 = (signed int)a3;
		v7 = result;
		v8 = a5 & 7;
		v9 = (uint8_t*)a2 + (a5 >> 3);
		if (a5 & 7)
		{
			v10 = 8 - v8;
			if (8 - v8 > (signed int)a3)
				v10 = (signed int)a3;
			v11 = 8 - (v5 & 7);
			v12 = -1 << (32 - v10);
			v13 = ((*v9 << v8) & 0xFFu) >> (8 - v10) << (32 - v10);
			*(uint8_t*)(result + ((int64_t)((uint64_t)a4 << 32) >> 35)) = (v13 >> 24 >> (v5 & 7)) | *(uint8_t*)(result + ((int64_t)((uint64_t)a4 << 32) >> 35)) & ~(v12 >> 24 >> (v5 & 7));
			if (v11 < v10)
			{
				v14 = v12 << v11;
				v15 = v13 << v11;
				v16 = v8 - 9;
				if (v16 < ~(uint32_t)a3)
					v16 = !(uint32_t)a3;
				v17 = (((unsigned int)(v5 & 7) - 10 - v16) >> 3) + 1;
				v18 = (uint8_t*)(result + ((int64_t)((uint64_t)a4 << 32) >> 35) + 1);
				do
				{
					--v17;
					v19 = *v18 & ~(v14 >> 24) | (v15 >> 24);
					v15 <<= 8;
					*v18++ = v19;
					v14 <<= 8;
				}
				while (v17);
			}
			++v9;
			v6 = (uint32_t)a3 - v10;
			v5 = (unsigned int)(v10 + v5);
		}
		if (v6 >= 1)
		{
			v20 = (unsigned int)v6 >> 3;
			if ((unsigned int)v6 >> 3)
			{
				v21 = v5 & 7;
				v22 = (char*)(result + (v5 << 32 >> 35));
				if (v5 & 7)
				{
					v24 = *v22;
					result = (uint64_t)(v22 + 1);
					v23 = v24;
					v25 = 8 - v21;
					v26 = 0xFFu >> v21;
					v27 = v20 - 1 + 1LL;
					v28 = v9;
					do
					{
						v29 = *(int8_t*)result;
						--v20;
						*(uint8_t*)(result - 1) = ((unsigned int)*v28 >> v21) | (255 << (8 - v21)) & v23;
						v30 = *v28++;
						v23 = (v30 << v25) | v26 & v29;
						*(uint8_t*)result++ = ((uint8_t)v30 << v25) | v26 & v29;
					}
					while (v20);
					v31 = v6 & 7;
					if (!(v6 & 7))
						return true;
				}
				else
				{
					v27 = v20;
					result = (long long)memcpy(v22, v9, v20);
					v31 = v6 & 7;
					if (!(v6 & 7))
						return true;
				}
				v40 = (int64_t)((uint64_t)((unsigned int)v5 + (v6 & 0xFFFFFFF8)) << 32) >> 35;
				v41 = ((uint8_t)v5 + (v6 & 0xF8)) & 7;
				v42 = -1 << (32 - v31);
				v43 = v9[v27] >> (8 - v31) << (32 - v31);
				v44 = 8 - v41;
				*(uint8_t*)(v7 + v40) = (v43 >> 24 >> v41) | *(uint8_t*)(v7 + v40) & ~(v42 >> 24 >> v41);
				if (8 - v41 < v31)
				{
					v45 = v42 << v44;
					v46 = v43 << v44;
					v47 = ((v31 + v41 - 9) >> 3) + 1;
					v48 = (uint8_t*)(v7 + v40 + 1);
					do
					{
						--v47;
						v49 = *v48 & ~(v45 >> 24) | (v46 >> 24);
						v46 <<= 8;
						*v48++ = v49;
						v45 <<= 8;
					}
					while (v47);
				}
			}
			else
			{
				v32 = v5 << 32 >> 35;
				v33 = 8 - (v5 & 7);
				v34 = -1 << (32 - v6);
				v35 = (unsigned int)*v9 >> (8 - v6) << (32 - v6);
				*(uint8_t*)(result + v32) = (v35 >> 24 >> (v5 & 7)) | *(uint8_t*)(result + v32) & ~(v34 >> 24 >> (v5 & 7));
				if (v33 < v6)
				{
					v36 = v34 << v33;
					v37 = v35 << v33;
					v38 = (uint8_t*)(result + v32 + 1);
					do
					{
						v33 += 8;
						v39 = *v38 & ~(v36 >> 24) | (v37 >> 24);
						v37 <<= 8;
						*v38++ = v39;
						v36 <<= 8;
					}
					while (v33 < v6);
				}
			}
		}

		return true;
	}


	// copied IDA code, please improve!
	inline bool ReadBits(void* data, int length)
	{
		if (length == 0)
		{
			return true;
		}

		if ((m_curBit + length) > m_maxBit)
		{
			return false;
		}

		auto rv = CopyBits(data, m_data.data(), length, 0, m_curBit);
		
		m_curBit += length;

		return rv;
	}

	// copied IDA code, please improve!
	inline bool WriteBits(const void* data, int length)
	{
		if ((m_curBit + length) > m_maxBit)
		{
			return false;
		}

		auto rv =  CopyBits(m_data.data(), data, length, m_curBit, 0);
		
		m_curBit += length;

		return rv;
	}

	// copied IDA code, eh
	template<typename T>
	inline bool WriteBitsSingle(const T* data, int length)
	{
		if (length == 13 && GetLengthHackState())
		{
			length = 16;
		}

		static_assert(std::is_integral_v<T>, "WriteBitsSingle wants an int value");

		auto a1 = m_data.data();

		// hack to prevent an out-of-bounds read of `data` tripping analyzers
		uint32_t a2 = 0;
		*(T*)&a2 = *data;

		auto a3 = length;
		int a4 = m_curBit;

		if ((m_curBit + length) > m_maxBit)
		{
			return false;
		}

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

		return true;
	}

	inline bool WriteBit(uint8_t bit)
	{
		int startIdx = m_curBit / 8;

		if (startIdx >= m_data.size())
		{
			return false;
		}

		int shift = (7 - (m_curBit % 8));
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

		uint32_t val = 0;
		ReadBitsSingle(&val, length);

		return T(val);
	}


	template<typename T>
	inline bool Read(int length, T* out)
	{
		static_assert(sizeof(T) <= 4, "maximum of 32 bit read");

		uint32_t val = 0;
		bool success = ReadBitsSingle(&val, length);

		if (success)
		{
			*out = T(val);
		}
		else
		{
			*out = T();
		}

		return success;
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

	template<typename T>
	inline void WriteSigned(int length, T data)
	{
		int sign = data < 0;
		int signEx = (data < 0) ? 0xFFFFFFFF : 0;
		int d = (data ^ signEx);

		Write<int>(1, sign);
		Write<int>(length - 1, d);
	}

	inline float ReadFloat(int length, float divisor)
	{
		auto integer = Read<int>(length);

		float max = (1 << length) - 1;
		return ((float)integer / max) * divisor;
	}

	inline void WriteFloat(int length, float divisor, float value)
	{
		float max = (1 << length) - 1;
		int integer = (int)((value / divisor) * max);

		Write<int>(length, integer);
	}

	inline float ReadSignedFloat(int length, float divisor)
	{
		auto integer = ReadSigned<int>(length);

		float max = (1 << (length - 1)) - 1;
		return ((float)integer / max) * divisor;
	}

	inline void WriteSignedFloat(int length, float divisor, float value)
	{
		float max = (1 << (length - 1)) - 1;
		int integer = (int)((value / divisor) * max);

		WriteSigned<int>(length, integer);
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
		auto s = m_maxBit - std::min(m_curBit, m_maxBit);
		auto c = (s / 8) + (s % 8 != 0) ? 1 : 0;

		std::vector<uint8_t> newData(c);
		ReadBits(newData.data(), s);
		return MessageBuffer{newData};
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

	inline std::vector<uint8_t>& GetBuffer()
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
