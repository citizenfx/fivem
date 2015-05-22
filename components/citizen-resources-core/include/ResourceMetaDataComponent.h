/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

namespace fx
{
class Resource;

template<typename IteratorType>
class IteratorView
{
private:
	IteratorType m_begin;
	IteratorType m_end;

public:
	inline IteratorView(const std::pair<IteratorType, IteratorType>& pair)
		: m_begin(pair.first), m_end(pair.second)
	{
		
	}

	inline const IteratorType& begin() const
	{
		return m_begin;
	}

	inline const IteratorType& end() const
	{
		return m_end;
	}
};

template<typename ContainerType>
using IteratorTypeView = IteratorView<ContainerType::iterator>;

template<typename PairType>
inline auto GetIteratorView(const PairType& pair)
{
	return IteratorView<decltype(pair.first)>(pair);
}

class ResourceMetaDataComponent : public fwRefCountable
{
private:
	Resource* m_resource;

	std::multimap<std::string, std::string> m_metaDataEntries;

public:
	inline auto GetEntries(const std::string& key)
	{
		return GetIteratorView(m_metaDataEntries.equal_range(key));
	}
};
}