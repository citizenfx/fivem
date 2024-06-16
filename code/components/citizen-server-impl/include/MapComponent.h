#pragma once

#include <optional>

namespace fx
{
	template<typename TKey, typename TValue>
	class MapComponent : public fwRefCountable
	{
	public:
		inline void Add(const TKey& key, const TValue& value)
		{
			m_map.insert({ key, value });
		}

		inline void Add(const TKey& key, TValue&& value)
		{
			m_map.insert({ key, std::move(value) });
		}

		inline void Remove(const TKey& key)
		{
			m_map.erase(key);
		}

		inline std::optional<TValue> Get(const TKey& key) const
		{
			std::optional<TValue> value;
			auto it = m_map.find(key);

			if (it != m_map.end())
			{
				value = it->second;
			}

			return value;
		}

		inline auto begin()
		{
			return m_map.begin();
		}

		inline auto end()
		{
			return m_map.end();
		}

	private:
		std::unordered_map<TKey, TValue> m_map;
	};
}
