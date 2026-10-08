#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include <ServerPerfComponent.h>

using namespace std::chrono_literals;

TEST_CASE("Server performance metrics track a rolling window")
{
	fx::ServerPerfComponent component;

	SECTION("an empty component returns zero values")
	{
		auto snapshot = component.GetPerformanceSnapshot();

		REQUIRE(snapshot.tick.currentMilliseconds == 0.0);
		REQUIRE(snapshot.tick.averageMilliseconds == 0.0);
		REQUIRE(snapshot.tick.maximumMilliseconds == 0.0);
		REQUIRE(snapshot.tick.sampleCount == 0);
		REQUIRE(snapshot.script.sampleCount == 0);
	}

	SECTION("tick metrics retain the latest 64 samples")
	{
		for (int milliseconds = 1; milliseconds <= 64; ++milliseconds)
		{
			component.ObserveServerTick(std::chrono::milliseconds(milliseconds));
		}

		auto snapshot = component.GetPerformanceSnapshot();
		REQUIRE(snapshot.tick.currentMilliseconds == 64.0);
		REQUIRE(snapshot.tick.averageMilliseconds == 32.5);
		REQUIRE(snapshot.tick.maximumMilliseconds == 64.0);
		REQUIRE(snapshot.tick.sampleCount == 64);

		component.ObserveServerTick(100ms);
		snapshot = component.GetPerformanceSnapshot();

		REQUIRE(snapshot.tick.currentMilliseconds == 100.0);
		REQUIRE(snapshot.tick.averageMilliseconds == Catch::Approx(34.046875));
		REQUIRE(snapshot.tick.maximumMilliseconds == 100.0);
		REQUIRE(snapshot.tick.sampleCount == 65);
	}

	SECTION("script metrics are tracked independently")
	{
		component.ObserveServerTick(4ms);
		component.ObserveScriptTick(1500us);

		auto snapshot = component.GetPerformanceSnapshot();
		REQUIRE(snapshot.tick.currentMilliseconds == 4.0);
		REQUIRE(snapshot.script.currentMilliseconds == 1.5);
		REQUIRE(snapshot.script.averageMilliseconds == 1.5);
		REQUIRE(snapshot.script.maximumMilliseconds == 1.5);
		REQUIRE(snapshot.script.sampleCount == 1);
	}
}
