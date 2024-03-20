#include <StdInc.h>
#include <Hooking.h>

#include <fiAssetManager.h>

namespace rage
{
static fiAssetManager* g_assetManager = nullptr;
fiAssetManager* fiAssetManager::GetInstance()
{
	return g_assetManager;
}

static hook::thiscall_stub<void(fiAssetManager*, const char*)> _pushFolder([]()
{
	return hook::get_call(hook::get_pattern("75 62 48 8D 0D ? ? ? ? 48 8B D6 E8", 12));
});

static hook::thiscall_stub<void(fiAssetManager*)> _popFolder([]()
{
	return hook::get_call(hook::get_pattern("EB 0A C7 83 ? 00 00 00 05 00 00 00", 0x13));
});

void fiAssetManager::PushFolder(const char* folder)
{
	return _pushFolder(this, folder);
}

void fiAssetManager::PopFolder()
{
	return _popFolder(this);
}
}

static HookFunction hookFunction([]() 
{
	rage::g_assetManager = hook::get_address<rage::fiAssetManager*>(hook::get_pattern("75 62 48 8D 0D ? ? ? ? 48 8B D6 E8", 5));
});
