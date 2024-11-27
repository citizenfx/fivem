#include <StdInc.h>
#include <Hooking.h>
#include <Hooking.FlexStruct.h>
#include <Hooking.Stubs.h>

static void* (*g_orig_CDecalCallbacks__GetLod)(void* self, hook::FlexStruct* entity, hook::FlexStruct* drawable);

static void* CDecalCallbacks__GetLod(void* self, hook::FlexStruct* entity, hook::FlexStruct* drawable)
{
	void* foundLod = g_orig_CDecalCallbacks__GetLod(self, entity, drawable);
	if (foundLod)
	{
		return foundLod;
	}

	// Check entity is ENTITY_TYPE_VEHICLE as only these require the patch
	if (entity && entity->Get<uint8_t>(0x28) == 3)
	{
		// Find any suitable drawable from LOD_HIGH to LOD_VLOW
		auto lodGroup = drawable->At<void*[]>(0x50);
		for (int lodIndex = 0; lodIndex < 4; lodIndex++)
		{
			void* lod = lodGroup[lodIndex];
			if (lod)
			{
				return lod;
			}
		}
	}

	return nullptr;
}

static HookFunction hookFunction([]
{
	// Some calls to CDecalCallbacks::GetLod assume that a valid pointer is returned.
	// 
	// For vehicles, this function returns LOD_HIGH if a _hi (HD) file exists;
	// However if this file does not exist, it will return the LOD_MED instead.
	// 
	// Due to a large percentage of content creators only defining a LOD_HIGH, this will crash if no _hi (HD) file is present;
	// As a workaround many people duplicate the #ft and append _hi to force the LOD_HIGH to be used.
	// 
	// This adjusts the function to return any suitable lod if the original function didn't return one.
	g_orig_CDecalCallbacks__GetLod = hook::trampoline(hook::get_pattern("48 85 D2 74 5B 8A 42 28"), CDecalCallbacks__GetLod);
});
