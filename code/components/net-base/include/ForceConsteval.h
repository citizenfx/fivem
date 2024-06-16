#pragma once

namespace net
{
	// TODO: replace with C++20 consteval
	template <typename T, T value>
	constexpr T force_consteval = value;
}
