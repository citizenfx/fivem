#include <StdInc.h>

#include <GameInit.h>
#include <CoreConsole.h>

#include <Hooking.h>

static bool g_stableOverlaySort;

class PedDecorationCollection
{
public:
	constexpr static size_t kSize = 0xA0; // Class definition has never changed.

	inline uint32_t GetName() const
	{
		return *reinterpret_cast<uint32_t*>((char*)this + 0x10);
	}
};

class PedDecorationManager
{
public:
	inline static ptrdiff_t kCollectionsOffset;
	inline static PedDecorationManager** ms_instance;

public:
	inline PedDecorationCollection* GetCollection(size_t index) const
	{
		auto array = *reinterpret_cast<char**>((char*)this + kCollectionsOffset);
		return reinterpret_cast<PedDecorationCollection*>(array + index * PedDecorationCollection::kSize);
	}

	static PedDecorationManager* GetInstance()
	{
		return *ms_instance;
	}

	static int Comparator(const void* a, const void* b)
	{
		auto instance = GetInstance();
		uint32_t a_hash = instance->GetCollection(*static_cast<const uint32_t*>(a))->GetName();
		uint32_t b_hash = instance->GetCollection(*static_cast<const uint32_t*>(b))->GetName();
		return (a_hash > b_hash) - (a_hash < b_hash);
	}
};

static void (*g_origTattooSort)(char*, size_t, size_t, int (*)(const void*, const void*));
static void TattooSort(char* ptr, size_t count, size_t size, int (*comp)(const void*, const void*))
{
	g_origTattooSort(ptr, count, size, g_stableOverlaySort ? PedDecorationManager::Comparator : comp);
}

static HookFunction hookFunction([]()
{
	// GH-2010: Ensure tattoo collections are sorted using a stable comparator.
	// The game currently subtracts the two hashes which would only work if they
	// were signed types.
	static ConVar<bool> stableTattooSort("game_enableStableOverlaySort", ConVar_Replicated, false, &g_stableOverlaySort);

	{
		auto location = hook::get_pattern<char>("41 0F B7 DE 4C 8D 0D ? ? ? ? 41 B8");

		auto compare = hook::get_address<char*>(location + 0x7);
		PedDecorationManager::ms_instance = hook::get_address<PedDecorationManager**>(compare + 0x3);
		PedDecorationManager::kCollectionsOffset = *reinterpret_cast<int32_t*>(compare + 0x7 + 0x3);

		hook::set_call(&g_origTattooSort, location + 0x1E);
		hook::call(location + 0x1E, TattooSort);
	}
});
