#pragma once

class atPoolBase
{
protected:
#if IS_RDR3
	int8_t* m_unk;
#endif
	char* m_data;
	int8_t* m_flags;
	uint32_t m_count;
	uint32_t m_entrySize;
	uint32_t m_pad;
	uint32_t m_pad2;
	uint32_t m_bitCount;

private:
	struct VirtualDtorBase
	{
		virtual ~VirtualDtorBase() = 0;
	};

public:
	template<typename T>
	T* GetAt(int index) const
	{
		if (m_flags[index] < 0)
		{
			return nullptr;
		}

		return reinterpret_cast<T*>(m_data + (index * m_entrySize));
	}

	template<typename T>
	T* GetAtHandle(int handle) const
	{
		int index = handle >> 8;
		int8_t flag = handle & 0xFF;

		if (m_flags[index] < 0 || m_flags[index] != flag)
		{
			return nullptr;
		}

		return reinterpret_cast<T*>(m_data + (index * m_entrySize));
	}

	size_t GetCountDirect()
	{
		// R* code does << 2 >> 2, but that makes no sense
		return m_bitCount & 0x3FFFFFFF;
	}

	size_t GetCount()
	{
		size_t count = std::count_if(m_flags, m_flags + m_count, [] (int8_t flag)
		{
			return (flag >= 0);
		});

		return count;
	}

	size_t GetSize()
	{
		return m_count;
	}

	void Clear()
	{
		for (int i = 0; i < m_count; i++)
		{
			if (m_flags[i] >= 0)
			{
				delete GetAt<VirtualDtorBase>(i);
			}
		}
	}
};

template<typename T>
class atPool : public atPoolBase
{
public:
	struct iterator : public std::iterator<std::forward_iterator_tag, T*>
	{
		iterator(atPool<T>* pool, int index)
			: m_basePool(pool), m_index(index)
		{

		}

		inline void operator++()
		{
			m_index = m_basePool->FindNext(m_index);
		}

		inline T* operator*()
		{
			return m_basePool->GetAt(m_index);
		}

		inline T& operator->()
		{
			return *m_basePool->GetAt(m_index);
		}

		inline bool operator==(iterator right)
		{
			return (m_basePool == right.m_basePool && m_index && right.m_index);
		}

		inline bool operator!=(iterator right)
		{
			return !(*this == right);
		}

	private:
		atPool<T>* m_basePool;
		int m_index;
	};

public:
	T* GetAt(int index)
	{
		return atPoolBase::GetAt<T>(index);
	}

	int FindNext(int index)
	{
		for (int i = (index + 1); i < m_count; i++)
		{
			if (m_flags[i] >= 0)
			{
				return i;
			}
		}

		return m_count;
	}

	inline iterator begin()
	{
		return iterator(this, 0);
	}

	inline iterator end()
	{
		return iterator(this, m_count);
	}
};
