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
};

template<typename TEntry>
using atMultiHashMap = atHashMap<atArray<TEntry>>;
