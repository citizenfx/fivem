#include <StdInc.h>

#include "atArray.h"
#include "Hooking.h"
#include "Hooking.Stubs.h"

struct CTrainToken
{
	void* vtable;
	char pad_8[0xC1];
	unsigned char m_trainConfigIndex;
	char pad_CA[6];
};

struct CTrainConfigs
{
	atArray<void*> m_trainConfigs;
	atArray<void*> m_trainGroups;
};

static uint32_t GetTrainTokenMethodOffset; // CTrain

static CTrainToken* GetTrainToken(void* train)
{
	return (*(CTrainToken*(__fastcall**)(void*))(*(uint64_t*)train + GetTrainTokenMethodOffset))(train);
}

static CTrainConfigs* g_trainConfigs;

static bool (*g_origRequestTrainCarriageAssets)(void*);

static bool RequestTrainCarriageAssets(void* train)
{
	const auto trainToken = (train) ? GetTrainToken(train) : nullptr;

	// Ensure that our train has a train token
	if (!trainToken)
	{
		return false;
	}

	// Bail out if token's train config index is invalid (usually 0xFF)
	if (trainToken->m_trainConfigIndex >= g_trainConfigs->m_trainConfigs.GetCount())
	{
		return false;
	}

	return g_origRequestTrainCarriageAssets(train);
}

static HookFunction hookFunction([]()
{
	static_assert(sizeof(CTrainToken) == 0xD0);
	static_assert(sizeof(CTrainConfigs) == 0x20);

	auto location = hook::get_pattern<char>("44 0F B7 74 DA 28 45 85 F6 0F", -0x4A);

	g_origRequestTrainCarriageAssets = hook::trampoline(location, RequestTrainCarriageAssets);	

	g_trainConfigs = hook::get_address<CTrainConfigs*>(location + 0x3B);

	GetTrainTokenMethodOffset = *(uint32_t*)(location + 0x21);
});
