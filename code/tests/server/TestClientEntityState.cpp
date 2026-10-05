#include <StdInc.h>

#include <catch_amalgamated.hpp>

#define COMPILING_STATE
#define STATE_FIVE
#include "state/ServerGameState.h"
#undef STATE_FIVE
#undef COMPILING_STATE

namespace
{
struct VectorGrowthObservation
{
	size_t allocations = 0;
	size_t relocatedElements = 0;
};

VectorGrowthObservation FillClientEntityState(fx::ClientEntityState& state, size_t count)
{
	VectorGrowthObservation observation;
	auto& entities = state.GetSyncedEntities();
	size_t previousCapacity = entities.capacity();

	for (size_t i = 0; i < count; ++i)
	{
		const size_t sizeBeforeInsert = entities.size();
		state.Insert(static_cast<uint16_t>(i), fx::ClientEntityData{});

		if (entities.capacity() != previousCapacity)
		{
			++observation.allocations;
			observation.relocatedElements += sizeBeforeInsert;
			previousCapacity = entities.capacity();
		}
	}

	return observation;
}
}

TEST_CASE("Client entity state reserves its per-tick working set")
{
	static constexpr size_t kEntityCount = 256;

	fx::ClientEntityState unreserved;
	const auto unreservedGrowth = FillClientEntityState(unreserved, kEntityCount);

	fx::ClientEntityState reserved;
	reserved.Reserve(kEntityCount);
	const auto reservedGrowth = FillClientEntityState(reserved, kEntityCount);

	CAPTURE(unreservedGrowth.allocations);
	CAPTURE(unreservedGrowth.relocatedElements);
	CAPTURE(reservedGrowth.allocations);
	CAPTURE(reservedGrowth.relocatedElements);
	CAPTURE(sizeof(fx::ClientEntityData));

	REQUIRE(unreservedGrowth.allocations == 9);
	REQUIRE(unreservedGrowth.relocatedElements == 255);
	REQUIRE(reservedGrowth.allocations == 0);
	REQUIRE(reservedGrowth.relocatedElements == 0);
	REQUIRE(reserved.GetSyncedEntities().capacity() >= kEntityCount);
}
