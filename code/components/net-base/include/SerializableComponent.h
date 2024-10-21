#pragma once

#include <ByteCounter.h>

#include "SerializableProperty.h"

namespace net
{
	/// <summary>
	/// A SerializeComponent contains SerializeProperty's.
	/// These properties can be serialized and deserialized with the SerializeProperties method of the SerializeComponent.
	/// The method generates the serialization code automatically depending on the property Serialize implementation.
	/// </summary>
	struct SerializableComponent
	{
		static constexpr bool kIsComponent = true;

		template<typename T>
		static size_t GetMaxSize()
		{
			ByteMaxCounter counter;
			T value;
			value.Process(counter);
			return counter.GetOffset();
		}

		template<typename T>
		static size_t GetMinSize()
		{
			ByteMinCounter counter;
			T value;
			value.Process(counter);
			return counter.GetOffset();
		}

		template<typename T>
		static size_t GetSize(T& value)
		{
			ByteCounter counter;
			value.Process(counter);
			return counter.GetOffset();
		}

		template <typename T, typename... Property>
		bool ProcessPropertiesInOrder(T& stream, Property&... property)
		{
			bool result = true;
			([&]()
			{
				if (result && !property.Process(stream))
				{
					result = false;
				}
			}(), ...);

			return result;
		}

		template <typename T, typename... Property>
		SerializableResult ProcessPropertiesResultInOrder(T& stream, Property&... property)
		{
			SerializableResult result = SerializableResult::Success;
			([&]()
			{
				if (result == SerializableResult::Success)
				{
					result = property.Process(stream);
				}
			}(), ...);

			return result;
		}
	};
}
