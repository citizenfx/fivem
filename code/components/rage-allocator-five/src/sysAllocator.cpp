#include "StdInc.h"
#include "sysAllocator.h"
#include "Hooking.Patterns.h"

namespace rage
{
	void* sysUseAllocator::operator new(size_t size)
	{
		return GetAllocator()->allocate(size, 16, 0);
	}

	void sysUseAllocator::operator delete(void* memory)
	{
		GetAllocator()->free(memory);
	}

	static uint32_t g_tlsOffset;

	uint32_t sysMemAllocator::GetAllocatorTlsOffset()
	{
		return g_tlsOffset;
	}
}

static HookFunction hookFunction([] ()
{
	rage::g_tlsOffset = *hook::pattern("B9 ? ? ? ? 48 8B 0C 01 45 33 C9 49 8B D2 48").get(0).get<uint32_t>(1);
});