#pragma once

#include <StdInc.h>

#include <string>
#include <unordered_map>


class RageHashList
{
public:
	template<int Size>
	RageHashList(const char*(&list)[Size])
	{
		for (int i = 0; i < Size; i++)
		{
			m_lookupList.insert({ HashString(list[i]), list[i] });
		}
	}

	inline std::string LookupHash(uint32_t hash)
	{
		auto it = m_lookupList.find(hash);

		if (it != m_lookupList.end())
		{
			return std::string(it->second);
		}

		return fmt::sprintf("0x%08x", hash);
	}

	inline bool ContainsHash(uint32_t hash)
	{
		return m_lookupList.find(hash) != m_lookupList.end();
	}

private:
	std::unordered_map<uint32_t, std::string_view> m_lookupList;
};
