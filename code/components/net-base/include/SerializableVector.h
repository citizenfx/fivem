#pragma once

#include <memory>
#include <cstdint>

namespace net
{
	/// <summary>
	/// A SerializableVector contains a vector of Properties.
	/// The SizeOption are used to specify the size of the buffer.
	/// Property needs to be a SerializableProperty and nothing else.
	/// </summary>
	template <typename Property, typename SizeOption, bool BigEndian = false>
	struct SerializableVector
	{
	private:
		template <typename T>
		struct UseValueType
		{
			template <typename U>
			static auto HasValueType(int) -> typename U::ValueType;

			template <typename>
			static auto HasValueType(...) -> T;

			using Value = decltype(HasValueType<T>(0));
		};

	public:
		std::vector<typename UseValueType<Property>::Value> m_properties;
	
		SerializableVector() : m_properties()
		{
		}

		operator std::vector<typename UseValueType<Property>::Value>&()
		{
			return m_properties;
		}

		SerializableVector& operator=(const std::vector<typename UseValueType<Property>::Value>& _value)
		{
			m_properties = _value;
			return *this;
		}

		bool operator ==(const std::vector<typename UseValueType<Property>::Value>& _value)
		{
			return m_properties == _value;
		}

		size_t Size() const
		{
			return m_properties.size();
		}

		void Clear()
		{
			m_properties.clear();
		}

		template <class... Args>
		void EmplaceBack(Args&&... args)
		{
			m_properties.emplace_back(std::forward<Args>(args)...);
		}

		std::vector<typename UseValueType<Property>::Value>& GetValue()
		{
			return m_properties;
		}

		template <typename T>
		bool Process(T& stream)
		{
			if constexpr (std::is_same<SizeOption, void>() || SizeOption::kType != storage_type::SerializableSizeOption::Type::Area)
			{
				static_assert(true, "serializable of a vector requires a SerializableSizeOptionArea.");
			}

			bool validSize;
			auto size = SizeOption::template Process<T, decltype(m_properties.size()), BigEndian>(stream, m_properties.size(), validSize);
			if (!validSize)
			{
				return false;
			}
			
			m_properties.resize(size);
			for (uint64_t i = 0; i < size; ++i)
			{
				if (!reinterpret_cast<Property*>(&m_properties[i])->Process(stream))
				{
					return false;
				}
			}

			return true;
		}
	};
}
