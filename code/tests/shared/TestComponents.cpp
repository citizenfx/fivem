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
#include "MumbleMessage.h"
#include "StreamByteReader.h"
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
	sectorComponent.someComponent.someStringView = std::string_view(someString);
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
	net::ByteMaxCounter byteCounter;
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
	REQUIRE(net::SerializableComponent::GetMaxSize<SerializableComponentUnlimitedSizeTest::Component>() == UINT64_MAX);
}

namespace ByteCounterTest
{
class Component: public net::SerializableComponent
{
public:
	net::SerializableProperty<net::Span<uint8_t>, net::storage_type::StreamTail> value1 {};

	template <typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			value1
		);
	}
};

class ComponentConstrained: public net::SerializableComponent
{
public:
	net::SerializableProperty<net::Span<uint8_t>, net::storage_type::ConstrainedStreamTail<128, 2048>> value1 {};

	template <typename T>
	bool Process(T& stream)
	{
		return ProcessPropertiesInOrder<T>(
			stream,
			value1
		);
	}
};
}

TEST_CASE("byte counter test")
{
	REQUIRE(net::SerializableComponent::GetMaxSize<ByteCounterTest::Component>() == UINT64_MAX);
	REQUIRE(net::SerializableComponent::GetMinSize<ByteCounterTest::Component>() == 0);
	ByteCounterTest::Component component;
	const size_t bufferSize = GENERATE(fx::TestUtils::u64Random(UINT16_MAX), 0);
	std::vector<uint8_t> buffer(bufferSize);
	fx::TestUtils::fillVectorU8Random(buffer);
	component.value1.SetValue({buffer.data(), buffer.size()});
	REQUIRE(net::SerializableComponent::GetSize<ByteCounterTest::Component>(component) == bufferSize);

	REQUIRE(net::SerializableComponent::GetMaxSize<ByteCounterTest::ComponentConstrained>() == 2048);
	REQUIRE(net::SerializableComponent::GetMinSize<ByteCounterTest::ComponentConstrained>() == 128);
}

TEST_CASE("read tcp stream")
{
	GIVEN("three mumble message")
	{
		REQUIRE(net::SerializableComponent::GetMaxSize<net::packet::ClientMumbleMessage>() == 2 + 4 + 8192);
		REQUIRE(net::SerializableComponent::GetMinSize<net::packet::ClientMumbleMessage>() == 2 + 4);
		std::vector<uint8_t> receiveBuffer(net::SerializableComponent::GetMaxSize<net::packet::ClientMumbleMessage>() * 10);
		std::vector<uint8_t> clientData1(GENERATE(8192, 0));
		std::vector<uint8_t> clientData2(GENERATE(8192, 0));
		std::vector<uint8_t> clientData3(GENERATE(8192, 0));
		uint8_t clientType1 = fx::TestUtils::u64Random(UINT8_MAX);
		uint8_t clientType2 = fx::TestUtils::u64Random(UINT8_MAX);
		uint8_t clientType3 = fx::TestUtils::u64Random(UINT8_MAX);
		net::ByteWriter writer(receiveBuffer.data(), receiveBuffer.size());
		{
			net::packet::ClientMumbleMessage clientMumbleMessage;
			clientMumbleMessage.type.SetValue(clientType1);
			clientMumbleMessage.data.SetValue({ clientData1.data(), clientData1.size() });
			clientMumbleMessage.Process(writer);
		}

		{
			net::packet::ClientMumbleMessage clientMumbleMessage;
			clientMumbleMessage.type.SetValue(clientType2);
			clientMumbleMessage.data.SetValue({ clientData2.data(), clientData2.size() });
			clientMumbleMessage.Process(writer);
		}

		size_t offsetBefore = writer.GetOffset();

		{
			net::packet::ClientMumbleMessage clientMumbleMessage;
			clientMumbleMessage.type.SetValue(clientType3);
			clientMumbleMessage.data.SetValue({ clientData3.data(), clientData3.size() });
			clientMumbleMessage.Process(writer);
		}

		size_t offsetAfter = writer.GetOffset();
		writer.Seek(offsetBefore + 6);
		THEN("read the received stream")
		{
			std::vector<uint8_t> remainingMessageBuffer(net::SerializableComponent::GetMaxSize<net::packet::ClientMumbleMessage>());
			net::StreamByteReader streamByteReader(remainingMessageBuffer.data(), remainingMessageBuffer.size());
			net::Span receiveBufferSpan{ receiveBuffer.data(), writer.GetOffset() };
			std::vector<net::packet::ClientMumbleMessage> receivedMessages;
			bool res = streamByteReader.Push<net::packet::ClientMumbleMessage>(receiveBufferSpan, [&](net::packet::ClientMumbleMessage& clientMumbleMessage)
			{
				receivedMessages.push_back(clientMumbleMessage);
			});

			REQUIRE(res == true);
			// when the 3th data is empty the + 6 allows reading it fully
			if (!clientData3.empty())
			{
				REQUIRE(receivedMessages.size() == 2);
			}
			else
			{
				REQUIRE(receivedMessages.size() == 3);
			}

			REQUIRE(receivedMessages[0].type.GetValue() == clientType1);
			REQUIRE(receivedMessages[1].type.GetValue() == clientType2);
			REQUIRE(receivedMessages[0].data.GetValue() == net::Span<uint8_t>(clientData1.data(), clientData1.size()));
			REQUIRE(receivedMessages[1].data.GetValue() == net::Span<uint8_t>(clientData2.data(), clientData2.size()));
			receivedMessages.clear();
			// remaining data,if data is not empty
			if (!clientData3.empty())
			{
				REQUIRE(clientData3.empty() == false);
				REQUIRE(offsetAfter - offsetBefore == clientData3.size() + 6);
				REQUIRE(streamByteReader.GetRemainingDataSize() == 6);

				receiveBufferSpan = { receiveBuffer.data() + writer.GetOffset(), offsetAfter - offsetBefore - 6 };
				REQUIRE(receiveBufferSpan.size() == clientData3.size());
				REQUIRE(*reinterpret_cast<const uint16_t*>(streamByteReader.GetData()) == net::ntohs(clientType3));
				REQUIRE(*reinterpret_cast<const uint32_t*>(streamByteReader.GetData() + 2) == net::ntohl(clientData3.size()));
				res = streamByteReader.Push<net::packet::ClientMumbleMessage>(receiveBufferSpan, [&](net::packet::ClientMumbleMessage& clientMumbleMessage)
				{
					receivedMessages.push_back(clientMumbleMessage);
				});

				REQUIRE(res == true);
				REQUIRE(receivedMessages.size() == 1);
				REQUIRE(receivedMessages[0].type.GetValue() == clientType3);
				REQUIRE(receivedMessages[0].data.GetValue() == net::Span{clientData3.data(), clientData3.size()});
				REQUIRE(streamByteReader.GetRemainingDataSize() == 0);
				REQUIRE(receiveBufferSpan.size() == 0);
				// ensure data is within the same buffer
				REQUIRE(*(uint16_t*)(receivedMessages[0].data.GetValue().data() - 6) == net::htons(clientType3));
				REQUIRE(*(uint32_t*)(receivedMessages[0].data.GetValue().data() - 4) == net::htonl(clientData3.size()));
			}
		}
	}
	GIVEN("too large message")
	{
		std::vector<uint8_t> remainingMessageBuffer(net::SerializableComponent::GetMaxSize<net::packet::ClientMumbleMessage>());
		std::vector<uint8_t> stream(12000);
		uint32_t size = net::htonl(8193);
		memcpy(stream.data() + 2, &size, 4);
		net::StreamByteReader streamByteReader(remainingMessageBuffer.data(), remainingMessageBuffer.size());
		net::Span receiveBufferSpan{ stream.data(), stream.size() };
		bool res = streamByteReader.Push<net::packet::ClientMumbleMessage>(receiveBufferSpan, [&](net::packet::ClientMumbleMessage& clientMumbleMessage)
		{
		});
		REQUIRE(res == false);
	}
	GIVEN("large random stream")
	{
		uint32_t amount = 50000;
		std::vector<uint8_t> receiveBuffer (amount * (2 + 4 + 8192));
		// vector can not be resized, otherwise pointers inside the receivedMessages will be invalid
		std::vector<net::packet::ClientMumbleMessage> receivedMessages;
		net::ByteWriter writer {receiveBuffer.data(), receiveBuffer.size()};
		for (uint16_t i = 0; i < amount; i++)
		{
			net::packet::ClientMumbleMessage& clientMumbleMessage = receivedMessages.emplace_back();
			clientMumbleMessage.type.SetValue(fx::TestUtils::u64Random(UINT16_MAX - 1));
			const uint32_t size = fx::TestUtils::u64Random(8192);
			if (size > 0)
			{
				clientMumbleMessage.data.SetValue({ receiveBuffer.data() + 6 + writer.GetOffset(), size });
			}
			else
			{
				clientMumbleMessage.data.SetValue({ nullptr, 0 });
			}

			REQUIRE(clientMumbleMessage.Process(writer) == net::SerializableResult::Success);
		}

		std::vector<uint8_t> remainingMessageBuffer(net::SerializableComponent::GetMaxSize<net::packet::ClientMumbleMessage>());
		net::StreamByteReader streamByteReader(remainingMessageBuffer.data(), remainingMessageBuffer.size());
		size_t offset {0};
		uint32_t counter {0};
		bool res;
		while (writer.GetOffset() > offset)
		{
			size_t readSize = fx::TestUtils::u64Random(2 + 4 + 8192 - 1) + 1;
			if (offset + readSize > writer.GetOffset())
			{
				readSize = writer.GetOffset() - offset;
			}

			net::Span receiveBufferSpan{ receiveBuffer.data() + offset, readSize };
			res = streamByteReader.Push<net::packet::ClientMumbleMessage>(receiveBufferSpan, [&](net::packet::ClientMumbleMessage& clientMumbleMessage)
			{
				if (counter > amount)
				{
					// prevent overflow in case it fails
					REQUIRE(false);
				}

				REQUIRE(receivedMessages[counter].type.GetValue() == clientMumbleMessage.type.GetValue());
				REQUIRE(receivedMessages[counter].data.GetValue().size() == clientMumbleMessage.data.GetValue().size());
				REQUIRE(receivedMessages[counter].data.GetValue() == clientMumbleMessage.data.GetValue());
				++counter;
			});

			REQUIRE(res == true);

			offset += readSize;
		}

		REQUIRE(counter == amount);
	}
}
