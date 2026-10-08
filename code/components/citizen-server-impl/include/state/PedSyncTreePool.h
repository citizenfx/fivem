#pragma once

#ifdef _WIN32
#include <Windows.h>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <malloc.h>
#include <memory>
#include <new>
#include <type_traits>

namespace fx::sync::tree_pool
{
inline constexpr size_t SlabBytes = 1024 * 1024;
inline constexpr size_t MaxCachedBytes = 512 * SlabBytes;

struct Counters
{
    std::atomic<uint64_t> cachedBytes{0}, capacity{0}, inUse{0};
    std::atomic<uint64_t> allocations{0}, fallbacks{0}, growthFailures{0}, slotBytes{0};
};
inline Counters counters;

// Only raw storage is pooled. allocate_shared retains responsibility for object
// construction, destruction, exceptions, and the last weak reference.
template<typename T, size_t CacheLimit = MaxCachedBytes>
class StoragePool
{
    struct alignas(MEMORY_ALLOCATION_ALIGNMENT) Slot
    {
        SLIST_ENTRY link;
        bool pooled;
        alignas(T) unsigned char storage[sizeof(T)];
    };
    static_assert(alignof(T) <= MEMORY_ALLOCATION_ALIGNMENT);
    static_assert(std::is_standard_layout_v<Slot>);
    static_assert(sizeof(Slot) <= SlabBytes);
    static_assert(CacheLimit % SlabBytes == 0);

    SLIST_HEADER freeList;
    SRWLOCK growthLock = SRWLOCK_INIT;
    size_t cachedBytes = 0; // Protected by growthLock.

    Slot* Pop() noexcept
    {
        return reinterpret_cast<Slot*>(InterlockedPopEntrySList(&freeList));
    }

    Slot* GrowOrPop() noexcept
    {
        AcquireSRWLockExclusive(&growthLock);
        Slot* result = Pop();
        if (!result && cachedBytes < CacheLimit)
        {
            auto memory = static_cast<unsigned char*>(VirtualAlloc(nullptr, SlabBytes,
                MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
            if (memory)
            {
                constexpr size_t count = SlabBytes / sizeof(Slot);
                // Reserve slot zero for this caller; publish only initialized slots.
                result = ::new (static_cast<void*>(memory)) Slot;
                result->pooled = true;
                for (size_t i = 1; i < count; ++i)
                {
                    auto slot = ::new (static_cast<void*>(memory + i * sizeof(Slot))) Slot;
                    slot->pooled = true;
                    InterlockedPushEntrySList(&freeList, &slot->link);
                }
                cachedBytes += SlabBytes;
                counters.cachedBytes.fetch_add(SlabBytes, std::memory_order_relaxed);
                counters.capacity.fetch_add(count, std::memory_order_relaxed);
                counters.slotBytes.store(sizeof(Slot), std::memory_order_relaxed);
            }
            else
            {
                counters.growthFailures.fetch_add(1, std::memory_order_relaxed);
            }
        }
        ReleaseSRWLockExclusive(&growthLock);
        return result;
    }

public:
    StoragePool() noexcept { InitializeSListHead(&freeList); }
    StoragePool(const StoragePool&) = delete;
    StoragePool& operator=(const StoragePool&) = delete;

    T* Allocate()
    {
        auto slot = Pop();
        if (!slot) slot = GrowOrPop();
        if (!slot)
        {
            // Cache exhaustion is not an entity limit. Overflow storage goes back
            // to the CRT immediately when the last weak reference releases it.
            auto memory = _aligned_malloc(sizeof(Slot), alignof(Slot));
            if (!memory) throw std::bad_alloc();
            slot = ::new (memory) Slot;
            slot->pooled = false;
            counters.fallbacks.fetch_add(1, std::memory_order_relaxed);
        }
        counters.allocations.fetch_add(1, std::memory_order_relaxed);
        counters.inUse.fetch_add(1, std::memory_order_relaxed);
        return reinterpret_cast<T*>(slot->storage);
    }

    void Deallocate(T* value) noexcept
    {
        auto slot = reinterpret_cast<Slot*>(reinterpret_cast<unsigned char*>(value) - offsetof(Slot, storage));
        counters.inUse.fetch_sub(1, std::memory_order_relaxed);
        if (slot->pooled)
            InterlockedPushEntrySList(&freeList, &slot->link);
        else
            _aligned_free(slot);
    }
};

template<typename T, size_t CacheLimit = MaxCachedBytes>
struct Allocator
{
    using value_type = T;
    using is_always_equal = std::true_type;
    template<typename U> struct rebind { using other = Allocator<U, CacheLimit>; };
    Allocator() noexcept = default;
    template<typename U> Allocator(const Allocator<U, CacheLimit>&) noexcept {}

    static StoragePool<T, CacheLimit>& Pool()
    {
        // ponytail: one bounded process-lifetime cache per rebound allocation
        // type. No teardown/free of slabs: late shared/weak releases remain safe.
        static StoragePool<T, CacheLimit> pool;
        static_assert(std::is_trivially_destructible_v<StoragePool<T, CacheLimit>>);
        return pool;
    }

    T* allocate(size_t count)
    {
        if (count != 1) return std::allocator<T>{}.allocate(count);
        return Pool().Allocate();
    }
    void deallocate(T* value, size_t count) noexcept
    {
        if (count != 1) std::allocator<T>{}.deallocate(value, count);
        else Pool().Deallocate(value);
    }
};

template<typename T, typename U, size_t Limit>
bool operator==(const Allocator<T, Limit>&, const Allocator<U, Limit>&) noexcept { return true; }
template<typename T, typename U, size_t Limit>
bool operator!=(const Allocator<T, Limit>&, const Allocator<U, Limit>&) noexcept { return false; }
}
#endif
