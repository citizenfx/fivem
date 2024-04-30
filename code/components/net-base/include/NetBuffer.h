/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <memory>

#ifdef COMPILING_NET_BASE
#define NET_BASE_EXPORT DLL_EXPORT
#else
#define NET_BASE_EXPORT DLL_IMPORT
#endif

namespace net
{
class NET_BASE_EXPORT Buffer
{
private:
	std::shared_ptr<std::vector<uint8_t>> m_bytes;
	size_t m_curOff;

	bool m_end;

private:
	void Initialize();

	void EnsureWritableSize(size_t length);

public:
	inline std::shared_ptr<std::vector<uint8_t>> GetBytes()
	{
		return m_bytes;
	}

	inline const std::shared_ptr<std::vector<uint8_t>> GetBytes() const
	{
		return m_bytes;
	}

	Buffer();
	Buffer(const uint8_t* bytes, size_t length);
	Buffer(const std::vector<uint8_t>& origBytes);
	Buffer(size_t length);

	Buffer(const Buffer& other);
	Buffer(Buffer&& other);

	Buffer& operator=(const Buffer& other);
	Buffer& operator=(Buffer&& other);

	Buffer Clone() const;

	bool IsAtEnd() const;

	bool Read(void* buffer, size_t length);
	void Write(const void* buffer, size_t length);

	bool CanRead(size_t length) const
	{
		return m_curOff + length <= m_bytes->size();
	}

	bool EndsAfterRead(size_t length) const
	{
		return m_curOff + length >= m_bytes->size();
	}

	template<typename T>
	T Read()
	{
		T tempValue;
		Read(&tempValue, sizeof(T));

		return tempValue;
	}

	/// <summary>
	/// Reads a std::string[_view] from the buffer. std::string_view is read allocation free
	/// </summary>
	/// <param name="length">length of the string to read in bytes</param>
	/// <returns>when the requested length can be read, it returns a std::string[_view] containing the data of the buffer at the current read position with the requested length, otherwise an empty std::string[_view] is returned.</returns>
	template <typename T, typename = std::enable_if_t<std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>>>
	T Read(size_t length)
	{
		if (EndsAfterRead(length))
		{
			m_end = true;

			if (!CanRead(length))
			{
				return T();
			}
		}

		T tempValue = T((char*)(GetRemainingBytesPtr()), length);
		m_curOff += length;
		return tempValue;
	}

	template<typename T>
	void Write(T value)
	{
		Write(&value, sizeof(T));
	}

	bool ReadTo(Buffer& other, size_t length);

	void Reset()
	{
		m_curOff = 0;
	}

	const uint8_t* GetBuffer() const { return &(*m_bytes)[0]; }
	size_t GetLength() const { return m_bytes->size(); }
	size_t GetCurOffset() const { return m_curOff; }
	size_t GetRemainingBytes() const { return GetLength() - GetCurOffset(); }
	const uint8_t* GetRemainingBytesPtr() const { return GetBuffer() + m_curOff; }
	void Seek(size_t position) { if (position <= GetLength()) { m_curOff = position; } }

	const std::vector<uint8_t>& GetData() const { return *m_bytes; }
};
}
