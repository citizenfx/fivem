#include "StdInc.h"
#include "Hooking.h"

#include "scrEngine.h"

void* CScriptEntityExtension__GetOwner(char* entityExtension)
{
	void* owner = *(void**)(entityExtension + 60);

	if (!owner)
	{
		rage::scrThread* thread = rage::scrEngine::GetActiveThread();

		if (thread)
		{
			GtaThread* gtaThread = static_cast<GtaThread*>(thread);

			owner = gtaThread->GetScriptHandler();
		}
	}

	return owner;
}

static HookFunction hookFunction([]()
{
	// CScriptEntityExtension vtable
	char* location = hook::get_pattern<char>("48 8B DA 48 89 41 08 89 71 18 66 89 71 1C 89 71", -4);
	void** vt = (void**)(location + *(int32_t*)location + 4);

	vt[7] = CScriptEntityExtension__GetOwner;
});