#pragma once

#include <om/OMClass.h>
#include <fxScripting.h>

namespace fx
{
class MemoryScriptBuffer : public OMClass<MemoryScriptBuffer, IScriptBuffer>
{
public:
	inline explicit MemoryScriptBuffer(std::vector<char>&& buffer)
		: m_buffer(std::move(buffer))
	{

	}

	inline explicit MemoryScriptBuffer(const std::vector<char>& buffer)
		: m_buffer(buffer)
	{
	}

	inline explicit MemoryScriptBuffer(const void* data, size_t size)
		: m_buffer(size)
	{
		memcpy(m_buffer.data(), data, m_buffer.size());
	}

	NS_DECL_ISCRIPTBUFFER;

	template<typename... T>
	inline static auto Make(T... args)
	{
		auto self = fx::MakeNew<MemoryScriptBuffer>(std::forward<T>(args)...);
		fx::OMPtr<IScriptBuffer> sb;
		self.As(&sb);

		return sb;
	}

private:
	std::vector<char> m_buffer;
};

inline char* MemoryScriptBuffer::GetBytes()
{
	return m_buffer.data();
}

inline uint32_t MemoryScriptBuffer::GetLength()
{
	return static_cast<uint32_t>(m_buffer.size());
}
}
