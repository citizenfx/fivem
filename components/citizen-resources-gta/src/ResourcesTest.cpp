/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ResourceManager.h"

#include <fiDevice.h>
#include <CachedResourceMounter.h>

fwRefContainer<fx::ResourceManager> g_resourceManager;

void CfxCollection_AddStreamingFile(const std::string& fileName, rage::ResourceFlags flags);

static InitFunction initFunction([] ()
{
	fx::OnAddStreamingResource.Connect([] (const fx::StreamingEntryData& entry)
	{
		CfxCollection_AddStreamingFile("cache:/" + entry.resourceName + "/" + entry.fileName, { entry.rscPagesVirtual, entry.rscPagesPhysical });
	});

	rage::fiDevice::OnInitialMount.Connect([] ()
	{
		//while (true)
		{
			fwRefContainer<fx::ResourceManager> manager = fx::CreateResourceManager();
			Instance<fx::ResourceManager>::Set(manager.GetRef());

			g_resourceManager = manager;
		}
	}, 9000);
});