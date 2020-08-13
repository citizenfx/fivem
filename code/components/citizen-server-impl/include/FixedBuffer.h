#pragma once

#include <type_traits>

namespace fx
{
	template<typename Base, size_t Size, size_t Align = 8>
	struct FixedBuffer
	{
		alignas(Align) uint8_t m_data[Size] {};
		bool b_isAllocated = false;

		Base* GetBase()
		{
			if (!b_isAllocated)
				return nullptr;
			return reinterpret_cast<Base*>(m_data);
		}

		template<typename T, typename... Args>
		T* Construct(Args&&... args)
		{
			static_assert(sizeof(T) <= Size, "T is too big.");
			static_assert(std::is_base_of_v<Base, T>, "T must have base class Base.");

			assert(!b_isAllocated);

			b_isAllocated = true;
			return new (m_data) T(std::forward<Args>(args)...);
		}

		~FixedBuffer()
		{
			if (Base* base = GetBase())
				base->~Base();
		}

		FixedBuffer() = default;

		// please don't construct me like this!
		FixedBuffer(const FixedBuffer& other) = delete;
		FixedBuffer(FixedBuffer&& other) = delete;

		// ... or assign me!!!
		FixedBuffer& operator=(const FixedBuffer& other) = delete;
		FixedBuffer& operator=(FixedBuffer&& other) = delete;
	};
}
