#pragma once

#ifdef COMPILING_RAGE_ALLOCATOR_FIVE
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

		virtual void m_4() = 0;
		virtual void* allocate(size_t size, size_t align, int subAllocator) = 0;
		virtual void* m_C(size_t size, size_t align, int subAllocator) = 0;
		virtual void free(void* pointer) = 0;

		// and a lot of other functions below that aren't needed right now

	public:
		static GAMESPEC_EXPORT uint32_t GetAllocatorTlsOffset();
	};

	inline sysMemAllocator* GetAllocator()
	{
		sysMemAllocator* allocator = *(sysMemAllocator**)(*(uintptr_t*)(__readgsqword(88)) + sysMemAllocator::GetAllocatorTlsOffset());

		return allocator;
	}

	class GAMESPEC_EXPORT sysUseAllocator
	{
	public:
		void* operator new(size_t size);

		void operator delete(void* memory);
	};
}