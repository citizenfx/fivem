/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <algorithm>

#define RAGE_FORMATS_FILE pgContainers
#include "formats-header.h"

#ifdef RAGE_FORMATS_OK
#if defined(RAGE_FORMATS_GAME_NY)
#define RAGE_FORMATS_ny_pgContainers 1
#elif defined(RAGE_FORMATS_GAME_PAYNE)
#define RAGE_FORMATS_payne_pgContainers 1
#elif defined(RAGE_FORMATS_GAME_RDR3)
#define RAGE_FORMATS_rdr3_pgContainers 1
#endif

#if defined(RAGE_FORMATS_GAME_FIVE)
#define RAGE_FORMATS_five_pgContainers 1
#endif

template<typename TValue, typename TIndex = uint16_t>
class pgArray : public pgStreamableBase
{
private:
	pgPtr<TValue> m_offset;
	TIndex m_count;
	TIndex m_size;

public:
	pgArray()
	{
		m_offset = (TValue*)0;
		m_count = 0;
		m_size = 0;
	}

	pgArray(int capacity)
	{
#ifndef RAGE_FORMATS_IN_GAME
		m_offset = new TValue[capacity];
#else
		m_offset = (TValue*)rage::GetAllocator()->Allocate(sizeof(TValue) * capacity, 16, 0);
#endif
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

	TValue& Get(TIndex offset)
	{
		return (*m_offset)[offset];
	}

	inline auto& operator[](TIndex offset)
	{
		return Get(offset);
	}

	void Expand(TIndex newSize)
	{
		if (m_size >= newSize)
		{
			return;
		}

#ifndef RAGE_FORMATS_IN_GAME
		TValue* newOffset = new TValue[newSize];
		std::copy((*m_offset), (*m_offset) + m_count, newOffset);

		delete[] *m_offset;
#else
		auto newOffset = (TValue*)rage::GetAllocator()->Allocate(sizeof(TValue) * newSize, 16, 0);
		std::copy((*m_offset), (*m_offset) + m_count, newOffset);

		rage::GetAllocator()->Free(*m_offset);
#endif
		m_offset = newOffset;
		m_size = newSize;
	}

	pgArray* MakeSaveable()
	{
		return new(false) pgArray(*m_offset, m_count);
	}

	void Set(TIndex offset, const TValue& value)
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

	inline TIndex GetSize() const
	{
		return m_size;
	}

	inline TIndex GetCount() const
	{
		return m_count;
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_count = SwapShortRead(m_count);
		m_size = SwapShortRead(m_size);

		m_offset.Resolve(blockMap);
	}
};

#pragma pack(push, 1)
template<typename TValue>
class pgObjectArray : public pgStreamableBase
{
private:
	pgPtr<pgPtr<TValue>> m_objects;
	uint16_t m_count;
	uint16_t m_size;

	// ensure 8-byte padding
#if defined(RAGE_FORMATS_GAME_FIVE)
	uint32_t m_padding;
#endif

public:
	pgObjectArray()
	{
		m_objects = (pgPtr<TValue>*)0;
		m_count = 0;
		m_size = 0;
	}

	pgObjectArray(int capacity)
	{
#ifndef RAGE_FORMATS_IN_GAME
		m_objects = new pgPtr<TValue>[capacity];
#else
		m_objects = (pgPtr<TValue>*)rage::GetAllocator()->Allocate(sizeof(pgPtr<TValue>) * capacity, 16, 0);
#endif
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

			if (!object)
			{
				(*m_objects)[i] = nullptr;
				continue;
			}

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

#ifndef RAGE_FORMATS_IN_GAME
		pgPtr<TValue>* newObjects = new pgPtr<TValue>[newSize];
		std::copy((*m_objects), (*m_objects) + m_count, newObjects);

		delete[] *m_objects;
#else
		auto newObjects = (pgPtr<TValue>*)rage::GetAllocator()->Allocate(sizeof(pgPtr<TValue>) * newSize, 16, 0);
		std::copy((*m_objects), (*m_objects) + m_count, newObjects);

		rage::GetAllocator()->Free(*m_objects);
#endif

		m_objects = newObjects;
		m_size = newSize;
	}

	TValue* Get(uint16_t offset)
	{
		if (offset >= m_count)
		{
			return nullptr;
		}

		return *((*m_objects)[offset]);
	}

	inline auto& operator[](uint16_t offset)
	{
		return ((*m_objects)[offset]);
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

	inline uint16_t GetSize() const
	{
		return m_size;
	}

	inline uint16_t GetCount() const
	{
		return m_count;
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_count = SwapShortRead(m_count);
		m_size = SwapShortRead(m_size);

		m_objects.Resolve(blockMap);
		
		for (int i = 0; i < m_size; i++)
		{
			(*m_objects)[i].Resolve(blockMap);

			if (!(*m_objects)[i].IsNull())
			{
				(*m_objects)[i]->Resolve(blockMap);
			}
		}
	}
};
#pragma pack(pop)

template<typename TValue>
class pgDictionary : public pgBase
{
private:
	pgPtr<pgBase> m_parent;
	uint32_t m_usageCount;
	pgArray<uint32_t> m_hashes;
	pgObjectArray<TValue> m_values;

public:
	struct iterator : public std::iterator<std::forward_iterator_tag, std::pair<uint32_t, TValue*>>
	{
	private:
		pgDictionary* m_base;
		int m_index;

		std::pair<uint32_t, TValue*> m_value;

	private:
		inline std::pair<uint32_t, TValue*> GetValue()
		{
			return std::make_pair(m_base->m_hashes.Get(m_index), m_base->m_values.Get(m_index));
		}

	public:
		inline iterator(pgDictionary* base, int index)
			: m_base(base), m_index(index)
		{
			m_value = GetValue();
		}

		inline std::pair<uint32_t, TValue*> operator*() const
		{
			return m_value;
		}

		inline const std::pair<uint32_t, TValue*>* operator->() const
		{
			return &m_value;
		}

		inline const iterator& operator++()
		{
			m_index++;
			m_value = GetValue();

			return *this;
		}

		inline friend bool operator!=(const iterator& left, const iterator& right)
		{
			return (left.m_base != right.m_base || left.m_index != right.m_index);
		}

		inline friend bool operator==(const iterator& left, const iterator& right)
		{
			return !(left != right);
		}
	};

public:
	pgDictionary()
	{
		m_usageCount = 1;
	}

	inline iterator begin()
	{
		return iterator(this, 0);
	}

	inline iterator end()
	{
		return iterator(this, m_hashes.GetCount());
	}

	inline void Add(uint32_t keyHash, TValue* value)
	{
		for (size_t idx = 0; idx < m_hashes.GetCount(); idx++)
		{
			if (m_hashes[idx] == keyHash)
			{
				// dupe
				return;
			}
			else if (m_hashes[idx] > keyHash)
			{
				// expand array
				auto lastCount = m_hashes.GetCount();

				m_hashes.Set(m_hashes.GetCount(), 0);
				m_values.Set(m_values.GetCount(), nullptr);

				// move
				std::move_backward(&m_hashes[idx], &m_hashes[lastCount], &m_hashes[lastCount + 1]);
				std::move_backward(&m_values[idx], &m_values[lastCount], &m_values[lastCount + 1]);

				// insert
				m_hashes.Set(idx, keyHash);
				m_values.Set(idx, value);

				return;
			}
		}

		m_hashes.Set(m_hashes.GetCount(), keyHash);
		m_values.Set(m_values.GetCount(), value);
	}

	inline void Add(const char* key, TValue* value)
	{
		Add(HashString(key), value);
	}

	inline TValue* Get(uint32_t keyHash)
	{
		for (int i = 0; i < m_hashes.GetCount(); i++)
		{
			if (m_hashes.Get(i) == keyHash)
			{
				return m_values.Get(i);
			}
		}

		return nullptr;
	}

	inline TValue* Get(const char* key)
	{
		return Get(HashString(key));
	}

	inline TValue* GetAt(uint16_t index)
	{
		return m_values.Get(index);
	}

	inline uint16_t GetCount() const
	{
		return m_hashes.GetCount();
	}

	inline void SetFrom(pgDictionary* dictionary)
	{
		// allocate a temporary list of pairs to sort from
		std::vector<std::pair<uint32_t, TValue*>> values(dictionary->GetCount());

		if (dictionary->GetCount())
		{
			std::copy(dictionary->begin(), dictionary->end(), values.begin());
			std::sort(values.begin(), values.end(), [] (const auto& left, const auto& right)
			{
				return (left.first < right.first);
			});

			// copy each into a smaller array of values to pass to SetFrom
			std::vector<uint32_t> fromKeys(values.size());
			std::vector<pgPtr<TValue>> fromValues(values.size());

			int i = 0;

			for (auto& pair : values)
			{
				fromKeys[i] = pair.first;
				fromValues[i] = pair.second;

				i++;
			}

			// and set the local arrays from each
			m_hashes.SetFrom(&fromKeys[0], fromKeys.size());
			m_values.SetFrom(&fromValues[0], fromValues.size());
		}
	}

	inline void Resolve(BlockMap* blockMap = nullptr)
	{
		m_parent.Resolve(blockMap);
		m_hashes.Resolve(blockMap);
		m_values.Resolve(blockMap);
	}
};
#endif

#include "formats-footer.h"
