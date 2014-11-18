#include "StdInc.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "EventCore.h"

TEST(EventCoreTest, StaticFwAction)
{
	static fwAction<bool> action;
	static bool calledFlag;

	calledFlag = false;

	action = [] (bool)
	{
		calledFlag = true;
	};

	action(true);

	EXPECT_TRUE(calledFlag);
}

TEST(EventCoreTest, RunAndConnectEvent)
{
	static fwEvent<int, int> event;
	static bool calledFlag;

	calledFlag = false;

	event.Connect([=] (int a1, int a2)
	{
		EXPECT_EQ(2, a1);
		EXPECT_EQ(42, a2);

		calledFlag = true;
	});

	event(2, 42);

	EXPECT_TRUE(calledFlag);
}

TEST(EventCoreTest, RunAndConnectEventMultiple)
{
	static fwEvent<int, int> event;
	static bool calledFlag1, calledFlag2;

	calledFlag1 = false;
	calledFlag2 = false;

	event.Connect([=] (int a1, int a2)
	{
		EXPECT_EQ(2, a1);
		EXPECT_EQ(42, a2);

		calledFlag1 = true;
	});

	event.Connect([=] (int a1, int a2)
	{
		EXPECT_EQ(2, a1);
		EXPECT_EQ(42, a2);

		calledFlag2 = true;
	});

	event(2, 42);

	EXPECT_TRUE(calledFlag1);
	EXPECT_TRUE(calledFlag2);
}

TEST(EventCoreTest, OrderEventCallbacks)
{
	static fwEvent<> event;
	std::vector<int> resultArray;
	
	// initialize the event with a few callbacks
	int initializationOrder[] = { 10, 15, 13, 12, 14 };

	for (auto& index : initializationOrder)
	{
		event.Connect([&resultArray, index] ()
		{
			resultArray.push_back(index);
		}, index);
	}

	// invoke the event to fill the array
	event();

	// verify the array contents
	EXPECT_THAT(resultArray, testing::ElementsAre(10, 12, 13, 14, 15));
}