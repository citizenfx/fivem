#pragma once

namespace fx
{
	// TODO: replace with C++20 consteval
	template <typename T, T value>
	T force_consteval = value;
}
