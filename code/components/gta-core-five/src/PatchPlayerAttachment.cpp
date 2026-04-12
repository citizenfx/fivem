#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <GameInit.h>
#include <CoreConsole.h>

enum class SanitizePlayerAttachmentMode : int8_t
{
	Disabled = 0,
	Vehicles = 1,
	VehiclesAndObjects = 2,
};

static int8_t g_SanitizePlayerAttachment = false;

class CPedFactory
{
public:
	virtual ~CPedFactory() = default;
	void* m_LocalPlayer;
};
static_assert(sizeof(CPedFactory) == 0x10, "CPedFactory has wrong size!");

static CPedFactory** g_PedFactory;

static bool (*g_CNetObjVehicle_AttemptPendingAttachment)(hook::FlexStruct*, hook::FlexStruct*, uint32_t*);

static bool CNetObjVehicle_AttemptPendingAttachment(hook::FlexStruct* thisPtr, hook::FlexStruct* entityAttachTo, uint32_t* failReason)
{
	const bool blockLocalPlayerAttachment = g_SanitizePlayerAttachment >= static_cast<int8_t>(SanitizePlayerAttachmentMode::Vehicles) 
		&& (*g_PedFactory)->m_LocalPlayer == entityAttachTo;

	if (blockLocalPlayerAttachment)
	{
		return false;
	}

	return g_CNetObjVehicle_AttemptPendingAttachment(thisPtr, entityAttachTo, failReason);
}

static bool (*g_CNetObjObject_AttemptPendingAttachment)(hook::FlexStruct*, hook::FlexStruct*, uint32_t*);

static bool CNetObjObject_AttemptPendingAttachment(hook::FlexStruct* thisPtr, hook::FlexStruct* entityAttachTo, uint32_t* failReason)
{
	const bool blockLocalPlayerAttachment = g_SanitizePlayerAttachment == static_cast<int8_t>(SanitizePlayerAttachmentMode::VehiclesAndObjects) 
		&& (*g_PedFactory)->m_LocalPlayer == entityAttachTo;

	if (blockLocalPlayerAttachment)
	{
		return false;
	}

	return g_CNetObjObject_AttemptPendingAttachment(thisPtr, entityAttachTo, failReason);
}

static HookFunction hookFunction([]
{
	static ConVar<int8_t> sanitizeRagdollEvents("game_sanitizePlayerAttachment", ConVar_Replicated, (int8_t)SanitizePlayerAttachmentMode::Vehicles, &g_SanitizePlayerAttachment);

	g_PedFactory = hook::get_address<CPedFactory**>(hook::get_pattern("E8 ? ? ? ? 48 8B 05 ? ? ? ? 48 8B 58 08 48 8B CB E8", 8));

	g_CNetObjVehicle_AttemptPendingAttachment = hook::trampoline(hook::get_pattern("48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 57 41 56 41 57 48 83 EC ? 0F 29 70 D8 0F 29 78 C8 4C 8B F1"), CNetObjVehicle_AttemptPendingAttachment);
	g_CNetObjObject_AttemptPendingAttachment = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 40 8A F8 84 C0 74 ? ? ? ? 48 8B CB FF 92 ? ? ? ? 0F BA A8"), CNetObjObject_AttemptPendingAttachment);
});
