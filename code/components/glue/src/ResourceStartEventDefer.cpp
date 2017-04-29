/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#ifdef GTA_NY
#include <ResourceManager.h>
#include <NetLibrary.h>

static InitFunction initFunction([] ()
{
	static bool isPlayerActivated = false;

	NetLibrary::OnNetLibraryCreate.Connect([] (NetLibrary* netLibrary)
	{
		netLibrary->OnInitReceived.Connect([] (NetAddress)
		{
			isPlayerActivated = false;
		});
	});

	Resource::OnStartedResource.Connect([] (fwRefContainer<Resource> resource)
	{
		if (isPlayerActivated)
		{
			msgpack::sbuffer nameArgs;
			msgpack::packer<msgpack::sbuffer> packer(nameArgs);

			packer.pack_array(1);
			packer.pack(resource->GetName());

			TheResources.TriggerEvent(fwString("onClientResourceStart"), nameArgs);
		}
	});

	ResourceManager::OnTriggerEvent.Connect([] (const fwString& eventName, const fwString&, int)
	{
		if (eventName == "playerActivated")
		{
			isPlayerActivated = true;

			TheResources.ForAllResources([] (fwRefContainer<Resource> resource)
			{
				msgpack::sbuffer nameArgs;
				msgpack::packer<msgpack::sbuffer> packer(nameArgs);

				packer.pack_array(1);
				packer.pack(resource->GetName());

				TheResources.TriggerEvent(fwString("onClientResourceStart"), nameArgs);
			});
		}
	});
});
#endif