#include "StdInc.h"

#include <netPlayerManager.h>
#include "Hooking.Patterns.h"
#include "netObjectMgr.h"

static bool (*CNetObjGame__CanClone)(hook::FlexStruct*, const void*, uint32_t*);

static bool CNetObjPropSet__CanClone(hook::FlexStruct* self, void* player, uint32_t* reason)
{
	if(!CNetObjGame__CanClone(self, player, reason))
	{
		return false;
	}
	
	auto parentId = self->At<int16_t>(0x43C);
	if(parentId != 0)
	{
		if(auto parent = rage::netObjectMgr::GetInstance()->GetNetworkObject(parentId, true))
		{
			return parent->CanClone(rage::GetLocalPlayer(), reason);
		}
	}
	
	return true;
}

static HookFunction hookFunction([]()
{
	auto vtable = hook::get_address<uintptr_t*>(hook::get_pattern("48 8D 05 ? ? ? ? 48 89 03 0F 57 C0 48 8D 05 ? ? ? ? 48 8B CB", 0x3));
	auto canCloneOffset = 0x190 / 8;
	CNetObjGame__CanClone = (decltype(CNetObjGame__CanClone))vtable[canCloneOffset];
	hook::put<uintptr_t>(&vtable[canCloneOffset], (uintptr_t)CNetObjPropSet__CanClone);
});