#include <type_traits>
#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include <ByteReader.h>
#include <ByteWriter.h>

#include <SerializableComponent.h>
#include <SerializableProperty.h>
#include <SerializableOptional.h>
#include <SerializableVector.h>
#include <SerializableStorageType.h>

#include "ByteCounter.h"
#include "TestUtils.h"

struct SomeComponent : net::SerializableComponent
{
	enum class SomeEnum: uint8_t
	{
		First,
		Second,
		Third
	};
	net::SerializableProperty<bool> someBool;
	net::SerializableProperty<uint32_t> someNumber;
	net::SerializableOptional<net::SerializableProperty<uint32_t>> someOptionalNumber;
	// string_view property can only be used when the buffer stays in memory while the struct is read
	net::SerializableProperty<std::string_view, net::storage_type::BytesArray> someStringView;
	net::SerializableProperty<std::string, net::storage_type::BytesArray> someString;
	net::SerializableVector<net::SerializableProperty<uint32_t>, net::storage_type::BytesArray> someNumberVector;
	net::SerializableProperty<std::vector<uint8_t>, net::storage_type::BytesArray> someBuffer;
	net::SerializableProperty<SomeEnum> someEnum;

	template <typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			someBool,
			someNumber,
			someOptionalNumber,
			someStringView,
			someString,
			someNumberVector,
			someBuffer,
			someEnum
		);
	}
};

struct SectorComponent : net::SerializableComponent
{
	net::SerializableProperty<int16_t, net::storage_type::Bits<10>> x;
	net::SerializableProperty<int16_t, net::storage_type::Bits<10>> y;
	net::SerializableProperty<int16_t, net::storage_type::Bits<6>> z;
	SomeComponent someComponent;
	net::SerializableOptional<SomeComponent> someOptionalNumber;

	template <typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			x,
			y,
			z,
			someComponent,
			someOptionalNumber
		);
	}
};

TEST_CASE("Components test")
{
	std::string someString = fx::TestUtils::asciiRandom(1000);
	SectorComponent sectorComponent;
	sectorComponent.x = fx::TestUtils::u64Random(500);
	sectorComponent.y = fx::TestUtils::u64Random(500);
	sectorComponent.z = fx::TestUtils::u64Random(500);
	sectorComponent.someComponent.someBool = fx::TestUtils::u64Random(1) == 1;
	sectorComponent.someComponent.someNumber = fx::TestUtils::u64Random(500);
	sectorComponent.someComponent.someOptionalNumber = fx::TestUtils::u64Random(500);
	sectorComponent.someComponent.someStringView = someString;
	sectorComponent.someComponent.someString = fx::TestUtils::asciiRandom(10);
	sectorComponent.someComponent.someNumberVector.EmplaceBack(1);
	sectorComponent.someComponent.someNumberVector.EmplaceBack(2);
	sectorComponent.someComponent.someBuffer.GetValue().resize(3);
	sectorComponent.someComponent.someBuffer.GetValue()[0] = 1;
	sectorComponent.someComponent.someBuffer.GetValue()[1] = 2;
	sectorComponent.someComponent.someBuffer.GetValue()[2] = 3;
	sectorComponent.someComponent.someEnum = SomeComponent::SomeEnum::Second;
	uint8_t buffer[4 * 1024];
	net::ByteWriter writeStream(buffer, sizeof(buffer));
	if (!sectorComponent.Process(writeStream))
	{
		REQUIRE(false);
	}
	SectorComponent readSectorComponent;
	net::ByteReader readStream(buffer, sizeof(buffer));
	if (!readSectorComponent.Process(readStream))
	{
		REQUIRE(false);
	}
	REQUIRE(sectorComponent.x == readSectorComponent.x);
	REQUIRE(sectorComponent.y == readSectorComponent.y);
	REQUIRE(sectorComponent.z == readSectorComponent.z);
	REQUIRE(sectorComponent.someComponent.someBool == readSectorComponent.someComponent.someBool);
	REQUIRE(sectorComponent.someComponent.someNumber == readSectorComponent.someComponent.someNumber);
	REQUIRE(
		sectorComponent.someComponent.someOptionalNumber == readSectorComponent.someComponent.
		someOptionalNumber);
	REQUIRE(sectorComponent.someComponent.someStringView == readSectorComponent.someComponent.someStringView);
	REQUIRE(sectorComponent.someComponent.someString == readSectorComponent.someComponent.someString);
	REQUIRE(sectorComponent.someComponent.someNumberVector == readSectorComponent.someComponent.someNumberVector);
	REQUIRE(sectorComponent.someComponent.someBuffer == readSectorComponent.someComponent.someBuffer);
	REQUIRE(sectorComponent.someComponent.someEnum == readSectorComponent.someComponent.someEnum);
	net::ByteCounter byteCounter;
	if (!sectorComponent.Process(byteCounter))
	{
		REQUIRE(false);
	}
	uint64_t size = byteCounter.GetCapacity();
	REQUIRE(size == uint64_t(1066));
}
