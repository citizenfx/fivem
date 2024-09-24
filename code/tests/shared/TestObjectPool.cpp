#include <StdInc.h>
#include <random>

#include <catch_amalgamated.hpp>
#include <shared_mutex>
#include <citizen_util/object_pool.h>

#include "TestUtils.h"

namespace
{
constexpr uint16_t kObjectSize = 1024;
constexpr uint16_t kPoolElementOverhead = 8/*pool ptr*/ + 8/* atomic next ptr */;
constexpr uint16_t kObjectPoolElementSize = kObjectSize + kPoolElementOverhead;

class Object
{
	std::array<uint8_t, kObjectSize> data{};
};
}

TEST_CASE("pool")
{
	// each object needs a size of 1024, but the pool object_entry is 1040
	// the initial size of the pool is 1041, because the pool needs to be larger then the object size
	static fx::object_pool<Object, kObjectPoolElementSize + 1> objectPool;

	Object* internalPtr;
	{
		// construct object from pool
		Object* objectInstance = objectPool.allocate();
		// destructs object instance, because out of scope
		internalPtr = objectInstance;
		objectPool.destruct(objectInstance);
	}
	{
		// construct object from pool
		Object* objectInstance = objectPool.allocate();
		// the ptr is the same, because the object from earlier got recycled
		REQUIRE(internalPtr == objectInstance);

		// construct object from pool
		Object* objectInstance2 = objectPool.allocate();
		// this object is not coming from the same memory region as objectInstance
		// because the pool memory region is full and so a new one is allocated for it
		REQUIRE(reinterpret_cast<uint64_t>(objectInstance) - reinterpret_cast<uint64_t>(objectInstance2) != kObjectPoolElementSize);

		objectPool.destruct(objectInstance);
		objectPool.destruct(objectInstance2);
	}
}

TEST_CASE("larger pool")
{
	// each object needs a size of 1024, but the pool object_entry is 1040
	// the initial size of the pool is 2081, because the test uses 2 objects as a sample
	static fx::object_pool<Object, 2 * kObjectPoolElementSize + 1> objectPool;

	Object* internalPtr;
	{
		// construct object from pool
		Object* objectInstance = objectPool.allocate();
		// destructs object instance, because out of scope
		internalPtr = objectInstance;
		objectPool.destruct(objectInstance);
	}
	{
		// construct object from pool
		Object* objectInstance = objectPool.allocate();
		// the ptr has a offset of 1040 by the first allocated element, even when it was freed
		// because the pool still has free memory for a second element
		REQUIRE(reinterpret_cast<uint64_t>(objectInstance) - reinterpret_cast<uint64_t>(internalPtr) == kObjectPoolElementSize);
		objectPool.destruct(objectInstance);
	}
	{
		// construct object from pool
		Object* objectInstance = objectPool.allocate();
		// the ptr is the same, because the object from earlier got recycled and the pool has no more free size for new ptrs
		REQUIRE(internalPtr == objectInstance);
		objectPool.destruct(objectInstance);
	}
}

TEST_CASE("dynamic pool allocations")
{
	for (uint16_t i = 0; i < 1024; ++i)
	{
		fx::object_pool<Object, 2 * kObjectPoolElementSize + 1> objectPool;

		Object* internalPtr;
		{
			// construct object from pool
			Object* objectInstance = objectPool.allocate();
			// destructs object instance, because out of scope
			internalPtr = objectInstance;
			objectPool.destruct(objectInstance);
		}
		{
			// construct object from pool to require next allocation to reuse memory
			// because the pool fits 2 elements in the initial size
			Object* objectInstance = objectPool.allocate();
			objectPool.destruct(objectInstance);
		}
		{
			// construct object from pool
			Object* objectInstance = objectPool.allocate();
			// the ptr is the same, because the object from earlier got recycled and the pool has no more free size for new ptrs
			REQUIRE(internalPtr == objectInstance);
			objectPool.destruct(objectInstance);
		}
	}
}

namespace
{
class QueueObject
{
	std::array<uint8_t, kObjectSize> data{};

public:
	fx::detached_queue_key<QueueObject> free_queue_key;
};
}

TEST_CASE("detached_mpsc_queue")
{
	fx::detached_mpsc_queue<QueueObject> queue;
	uint32_t processorCount = std::thread::hardware_concurrency();

	std::shared_mutex signalStartMutex;
	signalStartMutex.lock();

	// multi provider, single consumer queue gets multiple producer threads spawned
	// and a single consumer
	std::vector<std::thread> threads{};

	// each producer thread will produce 1024 objects
	std::atomic<uint32_t> remainingPops{1024 * processorCount};

	// spawn producer threads
	while (processorCount--)
	{
		threads.emplace_back([&signalStartMutex, &queue]()
		{
			std::shared_lock lck{signalStartMutex};
			uint16_t amountOfRuns = 1024;
			while (amountOfRuns--)
			{
				QueueObject* objectPtr = new QueueObject();
				// pushes the object ptr to the queue using the ptr of the queue key
				// this offset is later subtracted to get back to the original pointer
				queue.push(&objectPtr->free_queue_key);
			}
		});
	}

	// consumer thread
	threads.emplace_back([&signalStartMutex, &queue, &remainingPops]()
	{
		std::shared_lock lck{signalStartMutex};

		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		while (remainingPops)
		{
			// the pop requires the member reference of the free queue key
			// this is used to get the pointer to the QueueObject* from the queue key ptr with the offset
			if (const QueueObject* objectPtrToPop = queue.pop(&QueueObject::free_queue_key))
			{
				delete objectPtrToPop;
				--remainingPops;
			}

			// the timer cancels the while loop after 5 seconds in case the queue has a bug and no longer works
			// to prevent freeze of the unit test
			std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
			if (std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() > 5)
			{
				break;
			}
		}
	});

	// unlock the mutex to let all threads start at the same time
	signalStartMutex.unlock();

	// we await the test case until all threads are finished with producing and consuming
	for (auto& thread : threads)
	{
		thread.join();
	}

	// when all threads are done, the remaining pops should be 0
	REQUIRE(remainingPops == 0);
}
