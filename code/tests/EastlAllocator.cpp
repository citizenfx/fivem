#include <StdInc.h>

// both operators are required to be defined for eastl and the CitiTest module uses eastl in some tests.

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return ::operator new[](size);
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return ::operator new[](size);
}
