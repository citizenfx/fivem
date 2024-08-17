#pragma once

#include <ByteCounter.h>

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
		static size_t GetSize()
		{
			ByteCounter counter;
			T value;
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
	};
}
