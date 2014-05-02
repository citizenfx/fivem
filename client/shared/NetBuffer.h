#pragma once

class NetBuffer
{
private:
	char* m_bytes;
	size_t m_curOff;
	size_t m_length;

	bool m_end;
	bool m_bytesManaged;

public:
	NetBuffer(const char* bytes, size_t length);
	NetBuffer(size_t length);

	virtual ~NetBuffer();

	bool End();

	void Read(void* buffer, size_t length);
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

	inline const char* GetBuffer() { return m_bytes; }
	inline size_t GetLength() { return m_length; }
	inline size_t GetCurLength() { return m_curOff; }
};