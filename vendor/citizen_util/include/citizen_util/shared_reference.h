// Copyright (c) 2020 Can Boluk and contributors of the VTIL Project
// Modified by DefCon42.
// All rights reserved.   
//    
// Redistribution and use in source and binary forms, with or without   
// modification, are permitted provided that the following conditions are met: 
//    
// 1. Redistributions of source code must retain the above copyright notice,   
//    this list of conditions and the following disclaimer.   
// 2. Redistributions in binary form must reproduce the above copyright   
//    notice, this list of conditions and the following disclaimer in the   
//    documentation and/or other materials provided with the distribution.   
// 3. Neither the name of VTIL Project nor the names of its contributors
//    may be used to endorse or promote products derived from this software 
//    without specific prior written permission.   
//    
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE   
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR   
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF   
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS   
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN   
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)   
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  
// POSSIBILITY OF SUCH DAMAGE.        
//

#pragma once
#include <memory>
#include <functional>
#include <type_traits>
#include <atomic>

#include "object_pool.h"
#include "type_helpers.h"

namespace fx
{
namespace impl
{
	struct control_block
	{
		std::atomic<int32_t> ref_counter;
		std::atomic<int32_t> weak_counter;

		// Wrap atomic operations on reference counters.
		//
		bool inc_ref()
		{
			return ++ref_counter > 1;
		}
		bool dec_ref()
		{
			return --ref_counter == 0;
		}
		auto get_ref()
		{
			return ref_counter.load();
		}

		void inc_weak()
		{
			weak_counter++;
		}
		bool dec_weak()
		{
			return --weak_counter == 0;
		}
		auto get_weak()
		{
			return weak_counter.load();
		}
	};
}

using control_block_pool = object_pool<impl::control_block>;

// Pool of control blocks.
//
inline control_block_pool control_blocks;

template<typename T>
struct weak_reference;

// Used to implement shared and weak references with the control block *not* tied to the original value.
//
template<typename T, auto Pool>
struct shared_reference
{
	friend struct weak_reference<shared_reference>;

	using Type = T;
	constexpr static auto MPool = Pool;

	void reset()
	{
		if (value != nullptr)
		{
			if (block->dec_ref())
			{
				Pool->destruct(value);

				if (block->dec_weak())
				{
					control_blocks.destruct(block);
				}
			}
		}

		value = nullptr;
	}

	explicit operator bool() const
	{
		return value != nullptr;
	}

	bool operator==(const shared_reference& other) const
	{
		return value == other.value;
	}

	bool operator!=(const shared_reference& other) const
	{
		return value != other.value;
	}

	bool operator<(const shared_reference& other) const
	{
		return value < other.value;
	}

	T& operator*() const
	{
		return *value;
	}

	T* operator->() const
	{
		return value;
	}

	T* get() const
	{
		return value;
	}

	shared_reference(const shared_reference& other)
	{
		if (other.value != nullptr)
			other.block->inc_ref();

		value = other.value;
		block = other.block;
	}

	shared_reference(shared_reference&& other) noexcept
	{
		value = other.value;
		block = other.block;

		other.value = nullptr;
	}

	shared_reference& operator=(const shared_reference& other)
	{
		if (other.value != nullptr)
			other.block->inc_ref();

		reset();

		value = other.value;
		block = other.block;

		return *this;
	}

	shared_reference& operator=(shared_reference&& other) noexcept
	{
		std::swap(value, other.value);
		std::swap(block, other.block);
		return *this;
	}

	shared_reference()
		: value(nullptr), block(nullptr)
	{
	}
	~shared_reference()
	{
		reset();
	}

	template<typename... A>
	static shared_reference Construct(A&&... args)
	{
		shared_reference return_val{};

		return_val.block = control_blocks.construct();
		return_val.block->inc_ref();
		return_val.block->inc_weak();
		return_val.value = Pool->construct(std::forward<A>(args)...);

		return return_val;
	}

private:
	T* value;
	impl::control_block* block;
};

template<typename SharedT>
struct weak_reference
{
	using Type = typename SharedT::Type;
	constexpr static auto MPool = SharedT::MPool;

	explicit operator bool() const
	{
		return value != nullptr && block->get_ref() > 0;
	}

	SharedT lock() const
	{
		SharedT return_val;

		if (value != nullptr)
		{
			// this is here to keep NRVO
			auto shared_value = value;

			// unfortunately the best way i know of to do this is a CAS loop :(
			// no wait-free locking for you!
			auto old_ref = block->get_ref();
			do
			{
				if (old_ref <= 0)
				{
					shared_value = nullptr;
					break;
				}
			} while (!block->ref_counter.compare_exchange_weak(old_ref, old_ref + 1));

			return_val.block = block;
			return_val.value = shared_value;
		}

		return return_val;
	}

	void reset()
	{
		if (value != nullptr && block->dec_weak())
			control_blocks.destruct(block);

		value = nullptr;
	}

	weak_reference()
		: value(nullptr), block(nullptr)
	{
	}

	weak_reference(const SharedT& shared)
	{
		if (shared.value != nullptr)
			shared.block->inc_weak();

		value = shared.value;
		block = shared.block;
	}

	weak_reference(const weak_reference& other)
	{
		if (other.value != nullptr)
			other.block->inc_weak();

		value = other.value;
		block = other.block;
	}

	weak_reference& operator=(const weak_reference& other)
	{
		if (other.value != nullptr)
			other.block->inc_weak();

		reset();

		value = other.value;
		block = other.block;

		return *this;
	}

	weak_reference& operator=(weak_reference&& other)
	{
		std::swap(value, other.value);
		std::swap(block, other.block);
		return *this;
	}

	bool operator==(const SharedT& other) const
	{
		return value == other.value;
	}

	bool operator==(const weak_reference& other) const
	{
		return value == other.value;
	}

	bool operator!=(const SharedT& other) const
	{
		return value != other.value;
	}

	bool operator!=(const weak_reference& other) const
	{
		return value != other.value;
	}

	bool operator<(const SharedT& other) const
	{
		return value < other.value;
	}

	bool operator<(const weak_reference& other) const
	{
		return value < other.value;
	}

	~weak_reference()
	{
		reset();
	}

private:
	Type* value;
	impl::control_block* block;
};
}
