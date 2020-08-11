#pragma once
#include <mutex>
#include <cstdlib>
#include <atomic>
#include <algorithm>
#include <cstring>
#include <memory>

#include "detached_queue.h"
#include "type_helpers.h"

#include <xenium/ramalhete_queue.hpp>
#include <xenium/reclamation/generic_epoch_based.hpp>

#ifdef __clang__
#define _aligned_malloc(x, a) aligned_alloc(a, x)
#endif

namespace fx
{
// Object pools allow for fast singular type allocation based on plf::colony. The implementation here is heavily geared towards multithreading; it is almost completely wait-free.
//
template<typename T,
size_t InitialSize = 1ull * 1024 * 1024,
size_t GrowthCap = 5,
size_t GrowthFactor = 1>
struct object_pool
{
	// Forward declares and type exports.
	//
	using value_type = T;
	struct pool_instance;
	struct bucket_entry;

	static constexpr size_t Align = std::min<size_t>(alignof(T), 16);

	// A single entry in the pool.
	//
	struct object_entry
	{
		// Stores raw data.
		//
		std::aligned_storage_t<sizeof(T), Align> raw_data;

		// Pointer to owning pool.
		//
		pool_instance* pool;

		// Key for free queue.
		//
		detached_queue_key<object_entry> free_queue_key;

		// Decay into object pointer.
		//
		T* decay()
		{
			return (T*)&raw_data;
		}
		const T* decay() const
		{
			return (const T*)make_mutable(this)->decay();
		}

		// Resolve from object pointer.
		//
		static object_entry* resolve(const void* obj)
		{
			return ptr_at<object_entry>((T*)obj, -make_offset(&object_entry::raw_data));
		}
	};

	static_assert(sizeof(object_entry) < InitialSize, "Objects cannot be larger than initial size.");

	// A list of frees not tied to a bucket.
	//
	inline static xenium::ramalhete_queue<object_entry*, xenium::policy::reclaimer<xenium::reclamation::new_epoch_based<>>> detached_frees;

	// Base pool type.
	//
	struct pool_instance
	{
		// Parent bucket.
		//
		bucket_entry* bucket;

		// Number of objects we store and the objects themselves.
		//
		size_t object_count;
		object_entry objects[1];
	};

	// Pool bucket.
	//
	struct bucket_entry
	{
		// Queue of free memory regions.
		//
		detached_mpsc_queue<object_entry> free_queue;

		// Current "growth stage" of the pool.
		//
		std::atomic<size_t> current_growth_stage;

		// Atomic counter responsible of the grouping of allocations to buckets.
		//
		std::atomic<size_t> counter = { 0 };

		// Have we expired?
		//
		bool expired = false;

		// Allocate / deallocate wrappers.
		//
		T* allocate()
		{
			// Pop entry from local free queue, if non null:.
			//
			if (object_entry* entry = free_queue.pop(&object_entry::free_queue_key))
			{
				// Return the entry.
				//
				return entry->decay();
			}

			// If we can find a detached free, use it.
			object_entry* detached;
			if (detached_frees.try_pop(detached))
			{
				return detached->decay();
			}

			// Allocate a new pool.
			//

			size_t growth_stage = std::min<size_t>(current_growth_stage.fetch_add(+1), GrowthCap);

			// Determine new pool's size (raw size is merely an approximation).
			//
			size_t new_pool_size_raw = InitialSize << (growth_stage * GrowthFactor);
			size_t object_count = new_pool_size_raw / sizeof(object_entry);

			// Allocate the pool, keep the first object to ourselves.
			//
			pool_instance* new_pool = (pool_instance*)_aligned_malloc(sizeof(pool_instance) + sizeof(object_entry) * (object_count - 1), Align);
			new_pool->object_count = object_count;
			new_pool->bucket = this;

			object_entry* return_value = &new_pool->objects[0];
			return_value->pool = new_pool;

			// Initialize every other object.
			//
			for (size_t i = 1; i < object_count; i++)
			{
				auto obj = new_pool->objects + i;
				obj->pool = new_pool;
				free_queue.push(&obj->free_queue_key);
			}

			// Return the allocated address.
			//
			return return_value->decay();
		}

		void deallocate(object_entry* pointer)
		{
			if (expired)
				detached_frees.push(pointer);
			else
				free_queue.push(&pointer->free_queue_key);
		}

		// On kill, distribute pool frees.
		//
		void kill()
		{
			expired = true;
			while (auto free_val = free_queue.pop(&object_entry::free_queue_key))
				detached_frees.push(free_val);
		}

		// Revive from pool.
		//
		void revive()
		{
			expired = false;
		}
	};

	// Thread-local proxy.
	//
	struct bucket_proxy
	{
		// Free buckets; this happens when a thread gets destroyed.
		//
		inline static xenium::ramalhete_queue<bucket_entry*, xenium::policy::reclaimer<xenium::reclamation::new_epoch_based<>>> free_buckets;

		// Our bucket.
		//
		bucket_entry* bucket;

		T* allocate()
		{
			return bucket->allocate();
		}

		bucket_proxy()
		{
			if (!free_buckets.try_pop(bucket))
				bucket = new bucket_entry();
			else
				bucket->revive();
		}

		~bucket_proxy()
		{
			bucket->kill();
			free_buckets.push(bucket);
		}
	};

	object_pool() = default;

	// mmmm delet
	//
	object_pool(const object_pool& other) = delete;
	object_pool(object_pool&& other) = delete;

	object_pool& operator=(const object_pool& other) = delete;
	object_pool& operator=(object_pool&& other) = delete;

	T* allocate()
	{
		static thread_local bucket_proxy proxy;
		return proxy.allocate();
	}

	// Construct / deconsturct wrappers.
	//
	template<typename... Tx>
	T* construct(Tx&&... args)
	{
		return new (allocate()) T(std::forward<Tx>(args)...);
	}
	void destruct(T* pointer)
	{
		std::destroy_at<T>(pointer);
		auto ptr = object_entry::resolve(pointer);
		ptr->pool->bucket->deallocate(ptr);
	}
};

template<typename T>
struct generic_object_pool
{
	inline static object_pool<T> pool;

	static T* allocate()
	{
		return pool.allocate();
	}

	// Construct / deconsturct wrappers.
	//
	template<typename... Tx>
	static T* construct(Tx&&... args)
	{
		return pool.construct(std::forward<Tx>(args)...);
	}
	static void destruct(T* pointer)
	{
		pool.destruct(pointer);
	}
};
}
