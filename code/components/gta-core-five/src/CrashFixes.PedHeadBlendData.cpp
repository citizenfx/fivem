#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <Hooking.FlexStruct.h>

static constexpr uint32_t MP_M_FREEMODE_HASH = 1885233650; // "mp_m_freemode_01"
static constexpr uint32_t MP_F_FREEMODE_HASH = 2627665880; // "mp_f_freemode_01"

static std::once_flag traceOnceFlag;

static int32_t offset_NewModelHash;
static int32_t offset_HasHeadBlendData;

static void (*orig_CPlayerAppearanceDataNode_Serialise)(hook::FlexStruct*, void*);

static void CPlayerAppearanceDataNode_Serialise(hook::FlexStruct* self, void* serialiser)
{
	orig_CPlayerAppearanceDataNode_Serialise(self, serialiser);

	const uint32_t modelHash = self->Get<uint32_t>(offset_NewModelHash);
	bool& hasHeadBlendData = self->At<bool>(offset_HasHeadBlendData);

	// Only freemode peds can have head blend data.
	if (hasHeadBlendData && modelHash != MP_M_FREEMODE_HASH && modelHash != MP_F_FREEMODE_HASH)
	{
		// Intercept the head blend data serialisation and set it to false.
		hasHeadBlendData = false;
		std::call_once(traceOnceFlag, []
		{
			trace("Ped head blend data cleared: not a freemode model\n");
		});
	}
}

static HookFunction hookFunction([]
{
	orig_CPlayerAppearanceDataNode_Serialise = hook::trampoline(hook::get_pattern("40 55 53 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC ? 48 8B FA 48 8D 91 ? ? ? ? 48 8B F1 48 8B CF"), CPlayerAppearanceDataNode_Serialise);
	offset_NewModelHash = *hook::get_pattern<int32_t>("48 8D 91 ? ? ? ? 48 8B F1 48 8B CF 45 33 C0 E8 ? ? ? ? 48 8B 07", 3);
	offset_HasHeadBlendData = *hook::get_pattern<int32_t>("48 8D 9E ? ? ? ? 48 8B D3 45 33 C0 48 8B CF FF 50 ? 44 38 2B 75 ? 48 8B 07 48 8B CF FF 90 ? ? ? ? 84 C0 74", 3);
});
