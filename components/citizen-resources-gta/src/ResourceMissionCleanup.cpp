/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <Resource.h>

#ifdef GTA_FIVE
#include <ScriptHandlerMgr.h>
#include <scrThread.h>
#include <scrEngine.h>

struct MissionCleanupData
{
	rage::scriptHandler* scriptHandler;
	rage::scriptHandler* lastScriptHandler;

	MissionCleanupData()
		: scriptHandler(nullptr)
	{

	}
};

GtaThread* g_resourceThread;

static InitFunction initFunction([] ()
{
	fx::Resource::OnInitializeInstance.Connect([] (fx::Resource* resource)
	{
		auto data = std::make_shared<MissionCleanupData>();

		resource->OnStart.Connect([=] ()
		{
			data->scriptHandler = nullptr;
		}, -10000);

		resource->OnActivate.Connect([=] ()
		{
			// create the script handler if needed
			if (!data->scriptHandler)
			{
				data->scriptHandler = new CGameScriptHandlerNetwork(g_resourceThread);
			}

			// set the current script handler
			GtaThread* gtaThread = g_resourceThread;

			data->lastScriptHandler = gtaThread->GetScriptHandler();
			gtaThread->SetScriptHandler(data->scriptHandler);
		}, -10000);

		resource->OnDeactivate.Connect([=] ()
		{
			// only run if we have an active thread
			if (!rage::scrEngine::GetActiveThread())
			{
				return;
			}

			// put back the last script handler
			GtaThread* gtaThread = static_cast<GtaThread*>(g_resourceThread);

			gtaThread->SetScriptHandler(data->lastScriptHandler);
		}, 10000);

		resource->OnStop.Connect([=] ()
		{
			if (data->scriptHandler)
			{
				trace("deletin'\n");

				data->scriptHandler->CleanupObjectList();
				delete data->scriptHandler;

				trace("deletin'd\n");

				data->scriptHandler = nullptr;
			}
		}, 10000);
	});
});
#endif