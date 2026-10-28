#pragma once

#include <boost/pool/pool_alloc.hpp>
#include <tbb/spin_mutex.h>

namespace fx::sync::tree_pool
{
// Pool shared-allocation storage; allocate_shared owns object and weak lifetime.
// Grow in blocks of 64 slots so growth does not initialize a large free list
// while holding the pool mutex. Total retained storage follows peak demand.
template<typename T>
using Allocator = boost::fast_pool_allocator<T,
	boost::default_user_allocator_new_delete,
	tbb::spin_mutex, 64, 64>;
}
