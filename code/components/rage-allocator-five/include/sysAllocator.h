#pragma once

#include "XBRVirtual.h"

#ifdef COMPILING_RAGE_ALLOCATOR_FIVE
#define GAMESPEC_EXPORT __declspec(dllexport)
#else
#define GAMESPEC_EXPORT __declspec(dllimport)
#endif

namespace rage
{
	class sysMemAllocator : XBR_VIRTUAL_BASE_2802(2)
	{
	public:
		XBR_VIRTUAL_DTOR(sysMemAllocator)

		XBR_VIRTUAL_METHOD(void, SetQuitOnFail, (bool arg))
		XBR_VIRTUAL_METHOD(void*, Allocate, (size_t size, size_t align, int subAllocator))

		inline void* allocate(size_t size, size_t align, int subAllocator)
		{
			return Allocate(size, align, subAllocator);
		}

		XBR_VIRTUAL_METHOD(void*, TryAllocate, (size_t size, size_t align, int subAllocator))

		XBR_VIRTUAL_METHOD(void, Free, (void* pointer))

		inline void free(void* pointer)
		{
			return Free(pointer);
		}

		XBR_VIRTUAL_METHOD(void, TryFree, (void* pointer))

		XBR_VIRTUAL_METHOD(void, Resize, (void* pointer, size_t size))

		XBR_VIRTUAL_METHOD(sysMemAllocator*, GetAllocator_Const, (int allocator))
		XBR_VIRTUAL_METHOD(sysMemAllocator*, GetAllocator, (int allocator))

		XBR_VIRTUAL_METHOD(sysMemAllocator*, GetPointerOwner, (void* pointer))


		XBR_VIRTUAL_METHOD(size_t, GetSize, (void* pointer))

		XBR_VIRTUAL_METHOD(size_t, GetMemoryUsed, (int memoryBucket))

		XBR_VIRTUAL_METHOD(size_t, GetMemoryAvailable, ())

		// and a lot of other functions below that aren't needed right now

	public:
		static GAMESPEC_EXPORT uint32_t GetAllocatorTlsOffset();

		static GAMESPEC_EXPORT sysMemAllocator* UpdateAllocatorValue();
	};

	GAMESPEC_EXPORT sysMemAllocator* GetAllocator();

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
