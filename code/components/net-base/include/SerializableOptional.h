#pragma once

#include <memory>
#include <cstdint>

#include "ForceConsteval.h"

namespace net
{
	/// <summary>
	/// A SerializeOptional contains a Property, but only serializes it when the property got set.
	/// To reset a set property call Reset(). Read the SerializeProperty documentation for more infos.
	/// </summary>
	template <typename Property>
	struct SerializableOptional
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

		bool m_set = false;

	public:
		Property m_property;

		SerializableOptional(typename UseValueType<Property>::Value& v) : m_property(v)
		{
		}

		SerializableOptional() : m_property()
		{
		}

		operator typename UseValueType<Property>::Value()
		{
			return m_property.GetValue();
		}

		SerializableOptional& operator=(const typename UseValueType<Property>::Value& value)
		{
			m_set = true;
			m_property.SetValue(value);
			return *this;
		}

		bool operator ==(const typename UseValueType<Property>::Value& value)
		{
			return m_property.GetValue() == value;
		}

		bool IsEmpty() const
		{
			return !m_set;
		}

		void Reset()
		{
			m_set = false;
			m_property.GetValue() = {};
		}

		Property& GetProperty()
		{
			return m_property;
		}

		typename UseValueType<Property>::Value& GetValue()
		{
			return m_property.GetValue();
		}

		template <typename T>
		bool Process(T& stream)
		{
			if (!stream.Field(m_set))
			{
				return false;
			}

			if (m_set || force_consteval<bool, T::kType == DataStream::Type::MaxCounter>)
			{
				return m_property.Process(stream);
			}

			// process always worked when there was no process
			return true;
		}
	};
}
