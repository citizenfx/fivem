#include "StdInc.h"
#include "grcTexture.h"
#include "CrossLibraryInterfaces.h"

namespace rage
{
grcTextureFactory* grcTextureFactory::getInstance()
{
	return *(grcTextureFactory**)0x18A8630;
}
}

void RegisterD3DPostResetCallback(void(*function)())
{
	static bool hookInitialized;
	static std::vector<void(*)()> callbacks;

	if (!hookInitialized)
	{
		g_hooksDLL->SetHookCallback(StringHash("d3dPostReset"), [] (void*)
		{
			for (auto& cb : callbacks)
			{
				cb();
			}
		});

		hookInitialized = true;
	}

	callbacks.push_back(function);
}

void WRAPPER ClearRenderTarget(bool a1, int value1, bool a2, float value2, bool a3, int value3) { EAXJMP(0x620470); }