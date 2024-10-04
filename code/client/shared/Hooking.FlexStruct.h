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
			return *(const T*)((uintptr_t)this + offset);
		}

		template<typename T>
		void Set(int32_t offset, const T& value)
		{
			*(T*)((uintptr_t)this + offset) = value;
		}

		template<typename T>
		T& At(int32_t offset)
		{
			return *(T*)((uintptr_t)this + offset);
		}

		template<typename Ret, typename... Args>
		Ret CallVirtual(int32_t offset, Args... args)
		{
			// Calculate the address of the virtual method
			auto vtable = *(uintptr_t**)this; // Dereference to get the vtable
			auto method = (Ret(*)(void*, Args...))(*(uintptr_t*)((uintptr_t)vtable + offset));

			// Call the virtual method
			return method(this, std::forward<Args>(args)...);
		}
	};
}
