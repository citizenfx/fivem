#pragma once

namespace rage
{
class sysMemAllocator
{
public:
	virtual ~sysMemAllocator() = 0;

	virtual void m_4() = 0;
	virtual void* allocate(size_t size, size_t align, int subAllocator) = 0;
	virtual void free(void* pointer) = 0;

	// and a lot of other functions below that aren't needed right now
};

class GAMESPEC_EXPORT sysUseAllocator
{
public:
	void* operator new(size_t size);

	void operator delete(void* memory);
};
}