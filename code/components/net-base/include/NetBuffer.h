/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <memory>

namespace net
{
class Buffer
{
private:
	std::shared_ptr<std::vector<uint8_t>> m_bytes;
	size_t m_curOff;

	bool m_end;

private:
	void Initialize();

protected:
	void EnsureWritableSize(size_t length);

	inline std::shared_ptr<std::vector<uint8_t>> GetBytes()
	{
		return m_bytes;
	}

	inline const std::shared_ptr<std::vector<uint8_t>> GetBytes() const
	{
		return m_bytes;
	}

public:
	Buffer();
	Buffer(const uint8_t* bytes, size_t length);
	Buffer(const std::vector<uint8_t>& origBytes);
	Buffer(size_t length);

	Buffer(const Buffer& other);

	bool IsAtEnd() const;

	bool Read(void* buffer, size_t length);
	void Write(const void* buffer, size_t length);

	template<typename T>
	T Read()
	{
		T tempValue;
		Read(&tempValue, sizeof(T));

		return tempValue;
	}

	template<typename T>
	void Write(T value)
	{
		Write(&value, sizeof(T));
	}

	bool ReadTo(Buffer& other, size_t length);

	inline void Reset()
	{
		m_curOff = 0;
	}

	inline const uint8_t* GetBuffer() const { return &(*m_bytes)[0]; }
	inline size_t GetLength() const { return m_bytes->size(); }
	inline size_t GetCurOffset() const { return m_curOff; }
	inline size_t GetRemainingBytes() const { return GetLength() - GetCurOffset(); }

	inline const std::vector<uint8_t>& GetData() const { return *m_bytes; }
};
}