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