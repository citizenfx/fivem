#pragma once

#include <memory>
#include <cstdint>

#include <SerializableStorageType.h>
#include <Span.h>

namespace net
{
	enum class SerializableResult: uint8_t
	{
		Error,
		Success,
		Incomplete
	};

	/// <summary>
	/// A SerializableProperty contains data that can be serialized and deserialized.
	/// The property can be declared with a specified data type.
	/// It is also possible to specify a optional size, but this is only need when serializing types that don't have a implicit size.
	/// The Process generates the serialization code automatically for default types.
	/// </summary>
	template <typename Type, typename SizeOption = void, bool StreamResult = false, bool BigEndian = false>
	struct SerializableProperty
	{
	private:
		using ReturnType = std::conditional_t<StreamResult, SerializableResult, bool>;
		static constexpr ReturnType Error = [] {
			if constexpr (StreamResult) {
				return SerializableResult::Error;
			} else {
				return false;
			}
		}();

		static constexpr ReturnType Success = [] {
			if constexpr (StreamResult) {
				return SerializableResult::Success;
			} else {
				return true;
			}
		}();

		static constexpr ReturnType Incomplete = [] {
			if constexpr (StreamResult) {
				return SerializableResult::Incomplete;
			} else {
				return false;
			}
		}();

		template <typename T>
		struct HasIsComponent
		{
			template <typename U>
			static auto TestIsComponent(int) -> decltype(U::kIsComponent, std::true_type());

			template <typename>
			static auto TestIsComponent(...) -> std::false_type;

			static constexpr bool kValue = decltype(TestIsComponent<T>(0))::value;
		};

		template <typename C>
		struct IsTypeVector : std::false_type
		{
		};

		template <typename T, typename A>
		struct IsTypeVector<std::vector<T, A>> : std::true_type
		{
		};

		template <typename C>
		struct IsTypeSpan : std::false_type
		{
		};

		template <typename T>
		struct IsTypeSpan<net::Span<T>> : std::true_type
		{
		};

		static constexpr bool IsVector = IsTypeVector<Type>::value;

		static constexpr bool IsSpan = IsTypeSpan<Type>::value;

		template <typename... TypesList>
		static constexpr bool IsOneOf = std::disjunction_v<std::is_same<Type, TypesList>...>;

		static constexpr bool IsComponent = HasIsComponent<Type>::kValue;

	public:
		using ValueType = Type;

		Type m_value;

		template <typename T>
		SerializableProperty(T&& v) : m_value(std::forward<T>(v))
		{
		}

		SerializableProperty() : m_value()
		{
		}

		operator Type() const
		{
			return m_value;
		}

		SerializableProperty& operator=(const Type& value)
		{
			m_value = value;
			return *this;
		}

		bool operator ==(const Type& value) const
		{
			return m_value == value;
		}

		Type& GetValue()
		{
			return m_value;
		}

		Type GetValue() const
		{
			return m_value;
		}

		void SetValue(const Type& value)
		{
			m_value = value;
		}

		template <typename T>
		ReturnType Process(T& stream)
		{
			if constexpr (IsOneOf<bool, uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t>)
			{
				if constexpr (!std::is_same<SizeOption, void>())
				{
					if constexpr (SizeOption::kType != storage_type::SerializableSizeOption::Type::Value)
					{
						static_assert(
								!std::is_same_v<Type,Type>,
								"serializable of a primitive type requires a SerializableSizeOptionValue when SizeOption is specified.")
							;
					}

					// TODO: add SizeOption::ValueBitSize for future BitReader, BitWriter
					if (!stream.Field(m_value))
					{
						return Incomplete;
					}
				}
				else
				{
					if constexpr (BigEndian)
					{
						Type value;

						if constexpr (T::kType == DataStream::Type::Writer)
						{
							value = net::hton(m_value);	
						}

						if (!stream.Field(value))
						{
							return Incomplete;
						}

						if constexpr (T::kType == DataStream::Type::Reader)
						{
							m_value = net::ntoh(value);	
						}
					}
					else
					{
						if (!stream.Field(m_value))
						{
							return Incomplete;
						}
					}
				}
			}
			else if constexpr (IsOneOf<std::string, std::string_view>)
			{
				if constexpr (std::is_same<SizeOption, void>() || SizeOption::kType !=
					storage_type::SerializableSizeOption::Type::Area)
				{
					static_assert(
						!std::is_same_v<Type,Type>,
						"serializable of a std::string or std::string_view requires a SerializableSizeOptionArea.");
				}

				bool validSize;
				auto size = SizeOption::template Process<T, decltype(m_value.size()), BigEndian>(stream, m_value.size(), validSize);
				if (!validSize)
				{
					return Error;
				}

				if (!stream.Field(m_value, size))
				{
					return Incomplete;
				}
			}
			else if constexpr (IsComponent)
			{
				ReturnType result = m_value.Process(stream);
				if (result != Success)
				{
					return result;
				}
			}
			else if constexpr (IsVector)
			{
				if constexpr (std::is_same<SizeOption, void>() || SizeOption::kType !=
					storage_type::SerializableSizeOption::Type::Area)
				{
					static_assert(!std::is_same_v<Type,Type>, "serializable of a buffer requires a SerializableSizeOptionArea.");
				}

				bool validSize;
				auto size = SizeOption::template Process<T, decltype(m_value.size()), BigEndian>(stream, m_value.size(), validSize);
				if (!validSize)
				{
					return Error;
				}

				if (size > 0)
				{
					m_value.resize(size);
					if (!stream.Field(reinterpret_cast<Type&>(*m_value.data()), sizeof(typename Type::value_type) * size))
					{
						return Incomplete;
					}
				}

				return Success;
			}
			else if constexpr (IsSpan)
			{
				if constexpr (std::is_same<SizeOption, void>() || SizeOption::kType !=
					storage_type::SerializableSizeOption::Type::Area)
				{
					static_assert(!std::is_same_v<Type,Type>, "serializable of a buffer requires a SerializableSizeOptionArea.");
				}

				bool validSize;
				auto size = SizeOption::template Process<T, decltype(m_value.size()), BigEndian>(stream, m_value.size(), validSize);
				if (!validSize)
				{
					return Error;
				}

				if (size > 0)
				{
					if (!stream.Field(m_value, size))
					{
						if constexpr (StreamResult)
						{
							// setting the size while noting that the data is incomplete
							// to make it possible for the reader to know the amount of missing data
							m_value = net::Span<uint8_t>(nullptr, size);
						}

						return Incomplete;
					}
				}

				return Success;
			}
			else if constexpr (std::is_enum_v<Type>)
			{
				if (!stream.Field(reinterpret_cast<std::underlying_type<Type>&>(m_value), sizeof(Type)))
				{
					return Incomplete;
				}

				return Success;
			}
			else
			{
				static_assert(!std::is_same_v<Type,Type>, "Unsupported property type");
			}

			return Success;
		}
	};
}
