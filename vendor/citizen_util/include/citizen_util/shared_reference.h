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
		std::atomic<long> ref_counter;
		std::atomic<long> weak_counter;

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
		long get_ref()
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
		long get_weak()
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

	operator bool() const
	{
		return value != nullptr;
	}

	bool operator==(const shared_reference& other)
	{
		return value == other.value;
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
		reset();

		value = other.value;
		block = other.block;

		other.value = nullptr;

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

	operator bool() const
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
			long old_ref = block->get_ref();
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
		reset();

		value = other.value;
		block = other.block;

		other.value = nullptr;

		return *this;
	}

	bool operator==(const weak_reference& other)
	{
		return value == other.value;
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
