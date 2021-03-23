/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <ResourceManager.h>
#include <scrEngine.h>

#include <ICoreGameInit.h>

#if __has_include(<GameInit.h>)
#include <GameInit.h>
#endif

#include <optick.h>

extern fwRefContainer<fx::ResourceManager> g_resourceManager;

bool DLL_IMPORT UpdateScriptInitialization();

class TestScriptThread : public GtaThread
{
	virtual void DoRun() override
	{
		OPTICK_EVENT();

		static bool initedGame = false;
		static int tickCount = 0;

		if (!initedGame)
		{
			initedGame = Instance<ICoreGameInit>::Get()->HasVariable("networkInited");

			return;
		}

		if (Instance<ICoreGameInit>::Get()->HasVariable("networkTimedOut"))
		{
			return;
		}

		if (!UpdateScriptInitialization())
		{
			return;
		}

		static std::once_flag of;

		std::call_once(of, []()
		{
#if __has_include(<GameInit.h>)
			OnKillNetwork.Connect([](const char*)
			{
				Instance<ICoreGameInit>::Get()->ClearVariable("gameSettled");

				tickCount = 0;
				initedGame = false;
			});
#endif
		});

		tickCount++;

		if (tickCount == 10)
		{
			Instance<ICoreGameInit>::Get()->SetVariable("gameSettled");
		}

		g_resourceManager->Tick();
	}
};

TestScriptThread thread;
extern GtaThread* g_resourceThread;

#if USE_OPTICK
class ProfilerEventHolder : public fwRefCountable
{
public:
	ProfilerEventHolder(fx::Resource* resource)
		: m_desc(nullptr)
	{
		resource->OnTick.Connect([=]() { StartTick(resource); }, -10000000);
		resource->OnTick.Connect([=]() { EndTick(); }, 10000000);
	}

private:
	void StartTick(fx::Resource* resource)
	{
		if (!m_desc)
		{
			m_desc = Optick::EventDescription::Create(va("Resource::Tick %s", resource->GetName()), __FILE__, __LINE__, Optick::Color::GreenYellow);
		}

		m_event = std::make_unique<Optick::Event>(*m_desc);
	}

	void EndTick()
	{
		m_event = nullptr;
	}

private:
	Optick::EventDescription* m_desc;

	std::unique_ptr<Optick::Event> m_event;
};

DECLARE_INSTANCE_TYPE(ProfilerEventHolder);
#endif

#include <stack>

static InitFunction initFunction([] ()
{
	rage::scrEngine::OnScriptInit.Connect([] ()
	{
		rage::scrEngine::CreateThread(&thread);
		g_resourceThread = &thread;
	});

#ifdef IS_RDR3
	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		static thread_local std::stack<rage::scrThread*> lastActiveThread;

		resource->OnActivate.Connect([]()
		{
			lastActiveThread.push(rage::scrEngine::GetActiveThread());
			rage::scrEngine::SetActiveThread(g_resourceThread);
		}, -999);

		resource->OnDeactivate.Connect([]()
		{
			rage::scrEngine::SetActiveThread(lastActiveThread.top());
			lastActiveThread.pop();
		}, 999);
	});
#endif

#if USE_OPTICK
	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		resource->SetComponent(new ProfilerEventHolder(resource));
	});
#endif
});
