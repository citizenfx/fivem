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
	net::SerializableProperty<net::Span<uint16_t>, net::storage_type::ConstrainedSmallBytesArray<0, 3>> someSpan;

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
			someEnum,
			someSpan
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
	std::vector<uint16_t> spanBuffer;
	spanBuffer.resize(3);
	spanBuffer[0] = fx::TestUtils::u64Random(500);
	spanBuffer[1] = fx::TestUtils::u64Random(500);
	spanBuffer[2] = fx::TestUtils::u64Random(500);
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
	sectorComponent.someComponent.someSpan = {spanBuffer.data(), sectorComponent.someComponent.someBuffer.GetValue().size()};

	uint8_t spanIterations = 0;
	for (const uint16_t value : sectorComponent.someComponent.someSpan.GetValue())
	{
		REQUIRE(value == spanBuffer[spanIterations]);
		spanIterations++;
	}
	REQUIRE(spanIterations == spanBuffer.size());

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
	REQUIRE(sectorComponent.someComponent.someSpan == readSectorComponent.someComponent.someSpan);
	net::ByteCounter byteCounter;
	if (!sectorComponent.Process(byteCounter))
	{
		REQUIRE(false);
	}
	uint64_t size = byteCounter.GetCapacity();
	REQUIRE(size == uint64_t(917549));
}

namespace SerializableComponentPropertyTest {
class Component: public net::SerializableComponent
{
public:
	net::SerializableProperty<uint32_t> value {};

	template <typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			value
		);
	}
};

class ComponentHoldingClass : public net::SerializableComponent
{
public:
	net::SerializableProperty<Component> component {};
		
	template <typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			component
		);
	}
};
}

TEST_CASE("Serializable component property")
{
	uint32_t random = fx::TestUtils::u64Random(UINT32_MAX);

	SerializableComponentPropertyTest::ComponentHoldingClass instance;
	instance.component.GetValue().value.SetValue(random);

	uint8_t buffer[4];
	net::ByteWriter writer(buffer, sizeof(buffer));
	REQUIRE(instance.Process(writer));

	REQUIRE(*reinterpret_cast<uint32_t*>(buffer) == random);

	SerializableComponentPropertyTest::ComponentHoldingClass instanceRead;

	net::ByteReader reader(buffer, sizeof(buffer));
	REQUIRE(instanceRead.Process(reader));

	REQUIRE(instanceRead.component.GetValue().value.GetValue() == random);
}

namespace SerializableComponentUnlimitedSizeTest
{
class Component: public net::SerializableComponent
{
public:
	net::SerializableProperty<uint8_t> value1 {};
	net::SerializableProperty<net::Span<uint8_t>, net::storage_type::StreamTail> value2 {};

	template <typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			value1,
			value2
		);
	}
};
}

TEST_CASE("Serializable component size of unlimited size")
{
	REQUIRE(net::SerializableComponent::GetSize<SerializableComponentUnlimitedSizeTest::Component>() == UINT64_MAX);
}
