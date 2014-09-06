#include "StdInc.h"
#include <gtest/gtest.h>

#include <ComponentLoader.h>

TEST(ComponentIdTest, ParsingCategories)
{
	ComponentId componentId = ComponentId::Parse("citizen:game:client");

	EXPECT_EQ("citizen", componentId.GetCategory());
	EXPECT_EQ("game", componentId.GetSubCategory());

	EXPECT_EQ(componentId.GetCategory(), componentId.GetCategory(0));
	EXPECT_EQ(componentId.GetSubCategory(), componentId.GetCategory(1));
	EXPECT_EQ("client", componentId.GetCategory(2));
}

TEST(ComponentIdTest, ParsingVersions)
{
	ComponentId componentId = ComponentId::Parse("[1.2.3.46]");
	auto versions = componentId.GetVersions();

	ASSERT_NE(nullptr, versions);

	EXPECT_EQ(1, versions[0]);
	EXPECT_EQ(2, versions[1]);
	EXPECT_EQ(3, versions[2]);
	EXPECT_EQ(46, versions[3]);
}

TEST(ComponentIdTest, ParsingCombined)
{
	ComponentId componentId = ComponentId::Parse("citizen:game:client[1.2.3.46]");

	EXPECT_EQ("citizen", componentId.GetCategory());
	EXPECT_EQ("game", componentId.GetSubCategory());

	EXPECT_EQ(componentId.GetCategory(), componentId.GetCategory(0));
	EXPECT_EQ(componentId.GetSubCategory(), componentId.GetCategory(1));
	EXPECT_EQ("client", componentId.GetCategory(2));

	auto versions = componentId.GetVersions();

	ASSERT_NE(nullptr, versions);

	EXPECT_EQ(1, versions[0]);
	EXPECT_EQ(2, versions[1]);
	EXPECT_EQ(3, versions[2]);
	EXPECT_EQ(46, versions[3]);
}

TEST(ComponentIdTest, VersionsEqual)
{
	ComponentId left = ComponentId::Parse("[1.2]");
	ComponentId right = ComponentId::Parse("[1.2]");

	EXPECT_TRUE(left.CompareVersion(right) == 0);
}

TEST(ComponentIdTest, VersionsLess)
{
	ComponentId left = ComponentId::Parse("[1.2.2]");
	ComponentId right = ComponentId::Parse("[1.2.3]");

	EXPECT_TRUE(left.CompareVersion(right) < 0);
}

TEST(ComponentIdTest, VersionsGreater)
{
	ComponentId left = ComponentId::Parse("[1.2.3]");
	ComponentId right = ComponentId::Parse("[1.2.2]");

	EXPECT_TRUE(left.CompareVersion(right) > 0);
}

TEST(ComponentIdTest, VersionsSubcomponent)
{
	ComponentId left = ComponentId::Parse("[1.2]");
	ComponentId right = ComponentId::Parse("[1.2.3]");

	EXPECT_TRUE(left.CompareVersion(right) < 0);
}