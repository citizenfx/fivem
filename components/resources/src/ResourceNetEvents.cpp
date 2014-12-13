/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NetLibrary.h"
#include "ResourceManager.h"
#include "DownloadMgr.h"

static InitFunction initFunction([] ()
{
	NetLibrary::OnNetLibraryCreate.Connect([] (NetLibrary* netLibrary)
	{
		netLibrary->AddReliableHandler("msgResStop", [] (const char* buf, size_t len)
		{
			fwString resourceName(buf, len);

			auto resource = TheResources.GetResource(resourceName);

			if (resource.GetRef() == nullptr)
			{
				trace("Server requested resource %s to be stopped, but we don't know that resource\n", resourceName.c_str());
				return;
			}

			if (resource->GetState() != ResourceStateRunning)
			{
				trace("Server requested resource %s to be stopped, but it's not running\n", resourceName.c_str());
				return;
			}

			resource->Stop();
			resource->Tick(); // to clean up immediately and change to 'stopped'
		});

		netLibrary->AddReliableHandler("msgResStart", [] (const char* buf, size_t len)
		{
			fwString resourceName(buf, len);

			auto resource = TheResources.GetResource(resourceName);

			if (resource.GetRef() != nullptr)
			{
				if (resource->GetState() != ResourceStateStopped)
				{
					trace("Server requested resource %s to be started, but it's not stopped\n", resourceName.c_str());
					return;
				}
			}

			ResourceManager::OnQueueResourceStart(resourceName);
			//TheDownloads.QueueResourceUpdate(resourceName);
		});
	});
});

fwEvent<fwString> ResourceManager::OnQueueResourceStart;