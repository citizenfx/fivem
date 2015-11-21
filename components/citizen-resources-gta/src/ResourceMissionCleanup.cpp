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

struct DummyThread : public GtaThread
{
	DummyThread(fx::Resource* resource)
	{
		rage::scrThreadContext* context = GetContext();
		context->ScriptHash = HashString(resource->GetName().c_str());
		context->ThreadId = (HashString(resource->GetName().c_str()) * 31) + rand();

		SetScriptName(resource->GetName().c_str());
	}

	virtual void DoRun() override
	{

	}
};

struct MissionCleanupData
{
	rage::scriptHandler* scriptHandler;
	rage::scriptHandler* lastScriptHandler;

	DummyThread* dummyThread;
	rage::scrThread* lastThread;

	MissionCleanupData()
		: scriptHandler(nullptr)
	{

	}
};

GtaThread* g_resourceThread;

static void DeleteDummyThread(DummyThread** dummyThread)
{
	__try
	{
		delete *dummyThread;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{

	}

	*dummyThread = nullptr;
}

static InitFunction initFunction([] ()
{
	fx::Resource::OnInitializeInstance.Connect([] (fx::Resource* resource)
	{
		auto data = std::make_shared<MissionCleanupData>();

		resource->OnStart.Connect([=] ()
		{
			data->scriptHandler = nullptr;
			data->dummyThread = nullptr;
		}, -10000);

		resource->OnActivate.Connect([=] ()
		{
			// create the script handler if needed
			if (!data->scriptHandler)
			{
				data->dummyThread = new DummyThread(resource);
				data->scriptHandler = new CGameScriptHandlerNetwork(data->dummyThread);

				data->dummyThread->SetScriptHandler(data->scriptHandler);
				CGameScriptHandlerMgr::GetInstance()->AttachScript(data->dummyThread);
			}

			// set the current script handler
			GtaThread* gtaThread = data->dummyThread;

			data->lastThread = rage::scrEngine::GetActiveThread();
			rage::scrEngine::SetActiveThread(gtaThread);

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
			GtaThread* gtaThread = data->dummyThread;

			gtaThread->SetScriptHandler(data->lastScriptHandler);

			// restore the last thread
			rage::scrEngine::SetActiveThread(data->lastThread);
		}, 10000);

		resource->OnStop.Connect([=] ()
		{
			if (data->scriptHandler)
			{
				trace("deletin'\n");

				data->scriptHandler->CleanupObjectList();
				CGameScriptHandlerMgr::GetInstance()->DetachScript(data->dummyThread);
				//data->scriptHandler->CleanupObjectList();
				//delete data->scriptHandler;

				trace("deletin'd\n");

				data->scriptHandler = nullptr;
			}

			// having the function content inlined causes a compiler ICE - so we do it separately
			DeleteDummyThread(&data->dummyThread);
		}, 10000);
	});
});
#endif