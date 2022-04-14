#pragma once

#include <type_traits>

namespace math
{
namespace interpolation
{
	//
	// linear interpolation
	//
	template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
	T Linear(T a, T b, T t)
	{
		return a + (b - a) * t;
	}

	//
	// linear interpolation
	//
	template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
	T Lerp(T a, T b, T t)
	{
		return Linear<T>(a, b, t);
	}

	//
	// 1-dimensional quadratic Bezier interpolation
	//
	template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
	T QuadraticBezier(T a, T b, T c, T t)
	{
		return Linear(Linear(a, b, t), Linear(b, c, t), t);
	}

	//
	// n-dimensional quadratic Bezier interpolation
	//
	template<typename T, int size, typename = std::enable_if_t<std::is_floating_point_v<T> && (size > 0)>>
	std::array<T, size> QuadraticBezier(const T (&a)[size], const T (&b)[size], const T (&c)[size], T t)
	{
		std::array<T, size> v;

		for (size_t i = 0; i < size; ++i)
			v[i] = QuadraticBezier<T>(a[i], b[i], c[i], i);

		return v;
	}

	//
	// 1-dimensional cubic Bezier interpolation
	//
	template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
	T CubicBezier(T a, T b, T c, T d, T t)
	{
		T m = Linear(b, c, t);
		return Linear(Linear(Linear(a, b, t), m, t), Linear(m, Linear(c, d, t), t), t);
	}

	//
	// n-dimensional cubic Bezier interpolation
	//
	template<typename T, int size, typename = std::enable_if_t<std::is_floating_point_v<T> && (size > 0)>>
	std::array<T, size> CubicBezier(const T (&a)[size], const T (&b)[size], const T (&c)[size], const T (&d)[size], T t)
	{
		std::array<T, size> v;

		for (size_t i = 0; i < size; ++i)
			v[i] = CubicBezier<T>(a[i], b[i], c[i], d[i], t);

		return v;
	}
}
}
