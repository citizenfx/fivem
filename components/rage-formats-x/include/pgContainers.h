/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#define RAGE_FORMATS_FILE pgContainers
#include <formats-header.h>

#ifdef RAGE_FORMATS_OK
#if defined(RAGE_FORMATS_GAME_NY)
#define RAGE_FORMATS_ny_pgContainers 1
#endif

template<typename TValue>
class pgArray : public pgStreamableBase
{
private:
	pgPtr<TValue> m_offset;
	uint16_t m_count;
	uint16_t m_size;

public:
	pgArray()
	{
		m_offset = (TValue*)0xDEADC0DE;
		m_count = 0;
		m_size = 0;
	}

	pgArray(int capacity)
	{
		m_offset = new TValue[capacity];
		m_count = 0;
		m_size = capacity;
	}

	pgArray(TValue* values, int count)
	{
		SetFrom(values, count);
	}

	void SetFrom(TValue* values, int count)
	{
		m_offset = (TValue*)pgStreamManager::Allocate(count * sizeof(TValue), false, nullptr);
		std::copy(values, values + count, *m_offset);

		m_count = count;
		m_size = count;
	}

	TValue Get(uint16_t offset)
	{
		if (offset >= m_count)
		{
			return TValue(0);
		}

		return (*m_offset)[offset];
	}

	void Expand(uint16_t newSize)
	{
		if (m_size >= newSize)
		{
			return;
		}

		TValue* newOffset = new TValue[newSize];
		std::copy(m_offset, m_offset + m_count, newOffset);

		delete[] *m_offset;
		m_offset = newOffset;
	}

	pgArray* MakeSaveable()
	{
		return new(false) pgArray(*m_offset, m_count);
	}

	void Set(uint16_t offset, const TValue& value)
	{
		if (offset >= m_size)
		{
			Expand(offset + 1);
		}

		if (offset >= m_count)
		{
			m_count = offset + 1;
		}

		(*m_offset)[offset] = value;
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_count = SwapShortRead(m_count);
		m_size = SwapShortRead(m_size);

		m_offset.Resolve(blockMap);
	}
};

template<typename TValue>
class pgObjectArray : public pgStreamableBase
{
private:
	pgPtr<pgPtr<TValue>> m_objects;
	uint16_t m_count;
	uint16_t m_size;

public:
	pgObjectArray()
	{
		m_objects = (pgPtr<TValue>*)0xDEADC0DE;
		m_count = 0;
		m_size = 0;
	}

	pgObjectArray(int capacity)
	{
		m_objects = new pgPtr<TValue>[capacity];
		m_count = 0;
		m_size = capacity;
	}

	pgObjectArray(pgPtr<TValue>* objects, int count)
	{
		SetFrom(objects, count);
	}

	void SetFrom(pgPtr<TValue>* objects, int count)
	{
		m_objects = (pgPtr<TValue>*)pgStreamManager::Allocate(count * sizeof(pgPtr<TValue>), false, nullptr);

		for (int i = 0; i < count; i++)
		{
			auto object = *(objects[i]);

			if (pgStreamManager::IsInBlockMap(object, nullptr, false))
			{
				(*m_objects)[i] = object;
			}
			else
			{
				(*m_objects)[i] = new(pgStreamManager::Allocate(sizeof(*object), false, nullptr)) TValue(*object);
			}
		}

		m_count = count;
		m_size = count;
	}

	void Expand(uint16_t newSize)
	{
		if (m_size >= newSize)
		{
			return;
		}

		pgPtr<TValue>* newObjects = new pgPtr<TValue>[newSize];
		std::copy(m_objects, m_objects + m_count, newObjects);

		delete[] *m_objects;
		m_objects = newObjects;
	}

	TValue* Get(uint16_t offset)
	{
		if (offset >= m_count)
		{
			return nullptr;
		}

		return *((*m_objects)[offset]);
	}

	pgObjectArray* MakeSaveable()
	{
		return new(false) pgObjectArray(*m_objects, m_count);
	}

	void Set(uint16_t offset, TValue* value)
	{
		if (offset >= m_size)
		{
			Expand(offset + 1);
		}

		if (offset >= m_count)
		{
			m_count = offset + 1;
		}

		(*m_objects)[offset] = value;
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_count = SwapShortRead(m_count);
		m_size = SwapShortRead(m_size);

		m_objects.Resolve(blockMap);
		
		for (int i = 0; i < m_size; i++)
		{
			(*m_objects)[i].Resolve(blockMap);
			(*m_objects)[i]->Resolve(blockMap);
		}
	}
};

template<typename TValue>
class pgDictionary : public pgBase
{
private:
	pgPtr<pgBase> m_parent;
	uint32_t m_usageCount;
	pgArray<uint32_t> m_hashes;
	pgObjectArray<TValue> m_values;

public:
	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_parent.Resolve(blockMap);
		m_hashes.Resolve(blockMap);
		m_values.Resolve(blockMap);
	}
};
#endif

#include <formats-footer.h>