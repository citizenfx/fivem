/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#pragma once

#include <cstdint>

namespace hook
{
	struct FlexStruct
	{
		template<typename T>
		const T& Get(int32_t offset)
		{
			return *(T*)((uintptr_t)this + offset);
		}

		template<typename T>
		void Set(int32_t offset, const T& value)
		{
			*(T*)((uintptr_t)this + offset) = value;
		}
	};
}
