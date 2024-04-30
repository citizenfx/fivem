#include <StdInc.h>
#include <Error.h>

#include <Hooking.h>

#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE

#define RAGE_FORMATS_IN_GAME
#include <gtaDrawable.h>
#include <fragType.h>

//
// Only draw a vehicle mod fragment if the shader effect exists. Current theory
// for this crash is some weirdness with vehicle mods being shared between
// custom assets (changing parent id?).
//
// Legacy hashes:
//	b2802: east-ohio-uncle
//	b2699: kansas-stream-montana
//

static void ReportCrash(const char* name)
{
	static std::once_flag hasCrashedBefore;
	std::call_once(hasCrashedBefore, [=]()
	{
		auto debugName = name == nullptr ? "NULL" : name;
		trace("WARNING: mod fragment crash triggered (debug name: %s)\n", debugName);

		AddCrashometry("mod_fragment_crash", "true");
	});
}

using /*rage::*/ fragType = rage::five::fragType;
class CVehicleStreamRenderGfx;

static hook::cdecl_stub<fragType*(CVehicleStreamRenderGfx*, uint8_t)> CVehicleStreamRenderGfx_GetShaderEffect([]()
{
	return hook::get_pattern("4C 8B C1 80 FA 33 72 03 33 C0 C3");
});

static fragType* (*g_origGetFragment)(CVehicleStreamRenderGfx*, uint8_t);
static fragType* CVehicleStreamRenderGfx_GetFragment(CVehicleStreamRenderGfx* gfx, uint8_t modSlot)
{
	if (auto fragStore = g_origGetFragment(gfx, modSlot))
	{
		if (CVehicleStreamRenderGfx_GetShaderEffect(gfx, modSlot) != nullptr)
		{
			return fragStore;
		}

		ReportCrash(fragStore->GetName());
	}
	return nullptr;
}

static HookFunction hookFunction([]
{
	auto location = hook::get_pattern("41 8A D6 48 8B CB E8 ? ? ? ? 4C 8B F8", 6);
	hook::set_call(&g_origGetFragment, location);
	hook::call(location, CVehicleStreamRenderGfx_GetFragment);
});
