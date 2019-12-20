#pragma once

#ifdef COMPILING_RAGE_ALLOCATOR_RDR3
#define GAMESPEC_EXPORT __declspec(dllexport)
#else
#define GAMESPEC_EXPORT __declspec(dllimport)
#endif

namespace rage
{
	class sysMemAllocator
	{
	public:
		virtual ~sysMemAllocator() = 0;

		virtual void SetQuitOnFail(bool) = 0;

		virtual void get_unknown() = 0;

		virtual void* Allocate(size_t size, size_t align, int subAllocator) = 0;

		inline void* allocate(size_t size, size_t align, int subAllocator)
		{
			return Allocate(size, align, subAllocator);
		}

		virtual void* TryAllocate(size_t size, size_t align, int subAllocator) = 0;

		virtual void Free(void* pointer) = 0;

		inline void free(void* pointer)
		{
			return Free(pointer);
		}

		virtual void TryFree(void* pointer) = 0;

		virtual void Resize(void* pointer, size_t size) = 0;

		virtual sysMemAllocator* GetAllocator(int allocator) const = 0;

		virtual sysMemAllocator* GetAllocator(int allocator) = 0;

		virtual sysMemAllocator* GetPointerOwner(void* pointer) = 0;

		virtual size_t GetSize(void* pointer) const = 0;

		//virtual size_t GetMemoryUsed(int memoryBucket) = 0;

		//virtual size_t GetMemoryAvailable() = 0;

		// and a lot of other functions below that aren't needed right now

	public:
		static GAMESPEC_EXPORT uint32_t GetAllocatorTlsOffset();

		static GAMESPEC_EXPORT sysMemAllocator* UpdateAllocatorValue();
	};

	inline sysMemAllocator* GetAllocator()
	{
		sysMemAllocator* allocator = *(sysMemAllocator**)(*(uintptr_t*)(__readgsqword(88)) + sysMemAllocator::GetAllocatorTlsOffset());

		if (!allocator)
		{
			return sysMemAllocator::UpdateAllocatorValue();
		}

		return allocator;
	}

	class GAMESPEC_EXPORT sysUseAllocator
	{
	public:
		void* operator new(size_t size);

		inline void* operator new[](size_t size)
		{
			return sysUseAllocator::operator new(size);
		}

		void operator delete(void* memory);

		inline void operator delete[](void* memory)
		{
			return sysUseAllocator::operator delete(memory);
		}
	};
}
