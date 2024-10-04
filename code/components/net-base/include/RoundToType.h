#pragma once

namespace net
{
	template <typename Target, typename T>
	constexpr T roundToType(T val)
	{
		constexpr auto multiple = sizeof(Target);

		return ((val + multiple - 1) / multiple) * multiple;
	}
}
