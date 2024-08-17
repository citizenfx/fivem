#include <StdInc.h>
#include <Hooking.h>

#include <netSyncTree.h>

static hook::cdecl_stub<rage::netSyncTree* (void*, int)> getSyncTreeForType([]()
{
	return hook::get_pattern("0F B7 CA 83 F9 0F 0F 87");
});

static hook::cdecl_stub<bool(rage::netSyncTree* self, int flags, int flags2, rage::datBitBuffer* buffer, void* netLogStub)> netSyncTree_ReadFromBuffer([]()
{
	return hook::get_pattern("48 83 EC 40 48 83 B9 ? ? ? ? 00 49 8B F1", -13);
});

static hook::cdecl_stub<bool(rage::netSyncTree* self, rage::netObject* obj)> netSyncTree_CanApplyToObject([]()
{
	return hook::get_pattern("48 8B CE BB 01 00 00 00 E8 ? ? ? ? 49 8B 07", -0x2E);
});

static hook::cdecl_stub<bool(rage::netSyncTree* self, rage::netObject* obj)> netSyncTree_PrepareObject([]()
{
	return hook::get_pattern("48 8B F1 48 8B FA 48 8B 0D ? ? ? ? 48 8B 01 FF", -0xF);
});

namespace rage
{
bool netSyncTree::CanApplyToObject(netObject* object)
{
	return netSyncTree_CanApplyToObject(this, object);
}

bool netSyncTree::ReadFromBuffer(int flags, int flags2, rage::datBitBuffer* buffer, void* netLogStub)
{
	return netSyncTree_ReadFromBuffer(this, flags, flags2, buffer, netLogStub);
}

netSyncTree* netSyncTree::GetForType(NetObjEntityType type)
{
	return getSyncTreeForType(nullptr, (int)type);
}
}
