#pragma once

#include <om/OMClass.h>
#include <fxScripting.h>

namespace fx
{
class MemoryScriptBuffer : public OMClass<MemoryScriptBuffer, IScriptBuffer>
{
	class Pool
	{
		class PoolElement
		{
			std::vector<char> m_buffer;
			bool m_isUsed;

		public:
			PoolElement() : m_buffer(8192), m_isUsed(false)
			{
			}

			PoolElement(const uint32_t size) : m_buffer(size), m_isUsed(true)
			{
			}

			bool Use()
			{
				if (m_isUsed)
				{
					return false;
				}

				m_isUsed = true;
				return true;
			}

			void Free()
			{
				m_isUsed = false;
			}

			void Require(const size_t size)
			{
				if (m_buffer.size() < size)
				{
					m_buffer.resize(size);
				}
			}

			char* GetData()
			{
				return m_buffer.data();
			}
		};

		std::vector<PoolElement> m_pool {};

	public:
		Pool()
		{
		}
		
		void FreeResultBuffer(const char* dataPtr)
		{
			for (auto& poolElement : m_pool)
			{
				if (poolElement.GetData() == dataPtr)
				{
					poolElement.Free();
					break;
				}
			}
		}

		char* GeResultBuffer(const size_t requiredSize)
		{
			for (auto& poolElement : m_pool)
			{
				if (poolElement.Use())
				{
					poolElement.Require(requiredSize);
					return poolElement.GetData();
				}
			}

			return m_pool.emplace_back(requiredSize).GetData();
		}
	};

	static inline Pool pool {};

public:
	explicit MemoryScriptBuffer(char* data, const uint32_t size)
		: m_buffer(data), m_bufferSize(size)
	{
	}

	~MemoryScriptBuffer() override
	{
		pool.FreeResultBuffer(m_buffer);
	}

	NS_DECL_ISCRIPTBUFFER;

	template<typename... T>
	static auto Make(const uint32_t size)
	{
		char* buffer = pool.GeResultBuffer(size);
		auto self = fx::MakeNew<MemoryScriptBuffer>(buffer, size);
		fx::OMPtr<IScriptBuffer> sb;
		self.As(&sb);

		return sb;
	}

	template<typename... T>
	static auto Make(const char* data, const uint32_t size)
	{
		char* buffer = pool.GeResultBuffer(size);
		memcpy(buffer, data, size);
		auto self = fx::MakeNew<MemoryScriptBuffer>(buffer, size);
		fx::OMPtr<IScriptBuffer> sb;
		self.As(&sb);

		return sb;
	}

private:
	char* m_buffer;
	uint32_t m_bufferSize;
};

inline char* MemoryScriptBuffer::GetBytes()
{
	return m_buffer;
}

inline uint32_t MemoryScriptBuffer::GetLength()
{
	return m_bufferSize;
}
}
