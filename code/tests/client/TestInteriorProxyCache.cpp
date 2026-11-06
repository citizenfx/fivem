#include <catch_amalgamated.hpp>

#include "../../components/gta-streaming-five/include/InteriorProxyCache.h"

#include <array>

TEST_CASE("Cached interiors reconcile current pool ownership", "[streaming][cache]")
{
	struct Proxy
	{
		uint32_t mapData;
	};

	Proxy stock{ 42 };
	Proxy unrelated{ 13 };
	std::array<Proxy*, 4> slots{ nullptr, &stock, nullptr, &unrelated };
	auto lookup = [&](uint32_t mapData, int64_t orderedSlot = -1)
	{
		return LookupInteriorProxyForCache<Proxy>(slots.size(), [&](size_t i) { return slots.at(i); }, mapData, orderedSlot);
	};

	SECTION("An existing ordered proxy is updated without allocating")
	{
		const auto result = lookup(42, 1);
		REQUIRE(result.action == InteriorProxyCacheAction::Update);
		REQUIRE(result.proxy == &stock);
		REQUIRE(lookup(42, 1).proxy == result.proxy);
	}

	SECTION("Unordered custom proxies are also reused on repeated cache loads")
	{
		REQUIRE(lookup(13).action == InteriorProxyCacheAction::Update);
		REQUIRE(lookup(13).proxy == &unrelated);
	}

	SECTION("Only absent proxies take the original allocation path")
	{
		REQUIRE(lookup(99).action == InteriorProxyCacheAction::Add);
		REQUIRE(lookup(99, 2).action == InteriorProxyCacheAction::Add);
	}

	SECTION("Resource stop and restart consult current occupancy, not a stale pointer")
	{
		REQUIRE(lookup(42, 1).proxy == &stock);
		slots[1] = nullptr;
		REQUIRE(lookup(42, 1).action == InteriorProxyCacheAction::Add);
		Proxy replacement{ 42 };
		slots[1] = &replacement;
		REQUIRE(lookup(42, 1).proxy == &replacement);
	}

	SECTION("An occupied slot belonging to another map cannot be allocated or overwritten")
	{
		REQUIRE(lookup(42, 3).action == InteriorProxyCacheAction::Conflict);
		REQUIRE(unrelated.mapData == 13);
	}

	SECTION("Invalid ordering cannot index outside the pool")
	{
		REQUIRE(lookup(42, 4).action == InteriorProxyCacheAction::Conflict);
		REQUIRE(lookup(42, INT64_MAX).action == InteriorProxyCacheAction::Conflict);
	}
}
