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
	rage::g_tlsOffset = *hook::pattern("8B 14 81 80 BA ? ? ? ? 00 74 0D").get(0).get<uint32_t>(5);
});