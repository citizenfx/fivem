#pragma once

#include <memory>

namespace net
{
// todo: remove when cpp20 is used
template<typename T>
class Span
{
	using value_type = T;

	value_type* m_data;
	size_t m_size;
public:
	Span(): m_data(nullptr), m_size(0)
	{
	}

	Span(T* data, const size_t size): m_data(data), m_size(size)
	{
	}

	bool operator==(const Span<T>& b) const
	{
		if (m_size != b.m_size)
		{
			return false;
		}

		return !memcmp(m_data, b.m_data, m_size * sizeof(T));
	}

	const T& operator[](const size_t index) const noexcept
	{
		return m_data[index];
	}

	T& operator[](const size_t index) noexcept
	{
		return m_data[index];
	}

	size_t size() const
	{
		return m_size;
	}
	
	size_t size_bytes() const
	{
		return m_size * sizeof(T);
	}

	T* data() const
	{
		return m_data;
	}

	T* begin() const
	{
		return m_data;
	}

	T* end() const
	{
		return m_data + m_size;
	}

	bool empty() const
	{
		return m_size == 0;
	}
};
}
