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
	rage::g_tlsOffset = *hook::pattern("45 33 C9 B9 ? ? 00 00 48 8B 04 C2 45 8D 41 10 49 8B").count_hint(1).get(0).get<uint32_t>(4);
});
