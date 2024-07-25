#pragma once

#include <om/OMClass.h>
#include <fxScripting.h>

namespace fx
{
class MemoryScriptBuffer : public OMClass<MemoryScriptBuffer, IScriptBuffer>
{
public:
	class MemoryScriptBufferPool
	{
		class Block
		{
		public:
			char* m_memory;
			size_t m_size;

			Block(char* memory, size_t size)
				: m_memory(memory), m_size(size)
			{
			}
		};

		char* m_pool;
		std::vector<Block> m_freeBlocks{};

	public:
		MemoryScriptBufferPool(size_t size)
			: m_pool(new char[size])
		{
			m_freeBlocks.emplace_back(m_pool, size);
		}

		~MemoryScriptBufferPool()
		{
			m_freeBlocks.clear();
			delete[] m_pool;
		}

		void FreeResultBuffer(char* block, size_t size)
		{
			std::vector<Block>::iterator left = m_freeBlocks.end();
			std::vector<Block>::iterator right = m_freeBlocks.end();

			for (auto it = m_freeBlocks.begin(); it != m_freeBlocks.end(); ++it)
			{
				if (it->m_memory + it->m_size == block)
				{
					left = it;
				}
				else if (it->m_memory == block + size)
				{
					right = it;
				}
			}

			if (right != m_freeBlocks.end() && left != m_freeBlocks.end())
			{
				// the start gains the size of the block and the size of the end
				left->m_size += size + right->m_size;
				m_freeBlocks.erase(right);
				return;
			}

			if (left != m_freeBlocks.end())
			{
				// the block is just appended at the end, so the start ptr does not need to move
				left->m_size += size;
				return;
			}

			if (right != m_freeBlocks.end())
			{
				// the block is prepended, so the start moves to the left (start of block) and the size is increased
				right->m_memory = block;
				right->m_size += size;
				return;
			}

			m_freeBlocks.emplace_back(block, size);
		}

		char* GetResultBuffer(const size_t size)
		{
			if (size == 0)
				return nullptr;

			for (uint32_t i = 0; i < m_freeBlocks.size(); i++)
			{
				Block block = m_freeBlocks[i];
				if (block.m_size < size)
					continue;

				m_freeBlocks.erase(m_freeBlocks.begin() + i);

				if (size < block.m_size)
				{
					// the remaining block start after the used block and has the size of the remaining data
					m_freeBlocks.emplace_back(block.m_memory + size, block.m_size - size);
				}

				return block.m_memory;
			}

			return nullptr;
		}

		size_t GetFreeBlockCount() const
		{
			return m_freeBlocks.size();
		}
	};

private:
	static MemoryScriptBufferPool& GetMemoryScriptBufferPool()
	{
		static MemoryScriptBufferPool pool{ 32ULL * 1024 * 1024 }; //32 MB pool
		return pool;
	}

	char* m_buffer;
	uint32_t m_bufferSize;

public:
	explicit MemoryScriptBuffer(char* data, const uint32_t size)
		: m_buffer(data), m_bufferSize(size)
	{
	}

	~MemoryScriptBuffer() override
	{
		GetMemoryScriptBufferPool().FreeResultBuffer(m_buffer, m_bufferSize);
	}

	NS_DECL_ISCRIPTBUFFER;

	static auto Make(const uint32_t size)
	{
		fx::OMPtr<IScriptBuffer> sb;
		char* buffer = GetMemoryScriptBufferPool().GetResultBuffer(size);
		if (buffer)
		{
			auto self = fx::MakeNew<MemoryScriptBuffer>(buffer, size);
			self.As(&sb);
		}

		return sb;
	}

	static auto Make(const char* data, const uint32_t size)
	{
		fx::OMPtr<IScriptBuffer> sb;
		char* buffer = GetMemoryScriptBufferPool().GetResultBuffer(size);
		if (buffer)
		{
			memcpy(buffer, data, size);
			auto self = fx::MakeNew<MemoryScriptBuffer>(buffer, size);
			self.As(&sb);
		}

		return sb;
	}
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
