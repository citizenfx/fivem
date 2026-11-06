#pragma once

#include <cstddef>
#include <cstdint>

struct CInteriorProxy
{
	virtual ~CInteriorProxy() = 0;

	uint32_t mapData;
};

enum class InteriorProxyCacheAction
{
	Add,
	Update,
	Conflict
};

template<typename Proxy>
struct InteriorProxyCacheLookup
{
	InteriorProxyCacheAction action;
	Proxy* proxy;
};

// Inspect the current pool on each load. A resource stop may have freed a previous proxy.
// An ordered slot must never be allocated while another map owns it.
template<typename Proxy, typename GetAt>
InteriorProxyCacheLookup<Proxy> LookupInteriorProxyForCache(size_t poolSize, GetAt getAt, uint32_t mapData, int64_t orderedSlot)
{
	if (orderedSlot >= 0)
	{
		if (static_cast<uint64_t>(orderedSlot) >= poolSize)
		{
			return { InteriorProxyCacheAction::Conflict, nullptr };
		}

		if (auto proxy = getAt(static_cast<size_t>(orderedSlot)))
		{
			return { proxy->mapData == mapData ? InteriorProxyCacheAction::Update : InteriorProxyCacheAction::Conflict, proxy };
		}
	}

	for (size_t i = 0; i < poolSize; ++i)
	{
		if (auto proxy = getAt(i); proxy && proxy->mapData == mapData)
		{
			return { InteriorProxyCacheAction::Update, proxy };
		}
	}

	return { InteriorProxyCacheAction::Add, nullptr };
}

InteriorProxyCacheLookup<CInteriorProxy> FindInteriorProxyForCache(uint32_t mapData);
