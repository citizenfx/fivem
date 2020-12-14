#pragma once

#include <atArray.h>

template<typename TEntry>
class atHashMap
{
private:
	struct Entry
	{
		uint32_t hash;
		TEntry* data;
		Entry* next;
	};

private:
	atArray<Entry*> m_data;

	char m_pad[3];
	bool m_initialized;

public:
	atHashMap()
	{
		m_initialized = false;
	}

	inline void ForAllEntries(const std::function<void(TEntry*)>& cb)
	{
		for (auto& entries : m_data)
		{
			for (auto i = entries; i; i = i->next)
			{
				cb(i->data);
			}
		}
	}

	inline void ForAllEntriesWithHash(const std::function<void(uint32_t hash, TEntry*)>& cb)
	{
		for (auto& entries : m_data)
		{
			for (auto i = entries; i; i = i->next)
			{
				cb(i->hash, i->data);
			}
		}
	}
};

template<typename TEntry>
class atHashMapReal
{
private:
	struct Entry
	{
		uint32_t hash;
		TEntry data;
		Entry* next;
	};

private:
	atArray<Entry*> m_data;

	char m_pad[3];
	bool m_initialized;

public:
	atHashMapReal()
	{
		m_initialized = false;
	}

	inline TEntry* find(const uint32_t& idx)
	{
		for (Entry* i = *(m_data.m_offset + (idx % m_data.GetCount())); i; i = i->next)
		{
			if (i->hash == idx)
			{
				return &i->data;
			}
		}

		return nullptr;
	}
};

template<typename TEntry>
using atMultiHashMap = atHashMap<atArray<TEntry>>;

template<typename TKey, typename TEntry>
class atMap
{
private:
	struct Entry
	{
		TKey hash;
		TEntry data;
		Entry* next;
	};

private:
	Entry** m_data;
	uint16_t m_size;
	uint16_t m_count;
	Entry* m_nextFree;

public:
	atMap()
	{
		m_initialized = false;
	}

	inline void ForAllEntriesWithHash(const std::function<void(TKey hash, TEntry*)>& cb)
	{
		for (int idx = 0; idx < m_size; idx++)
		{
			for (auto i = m_data[idx]; i; i = i->next)
			{
				cb(i->hash, &i->data);
			}
		}
	}
};
