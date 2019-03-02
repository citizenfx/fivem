/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <ResourceManager.h>
#include <scrEngine.h>

#include <GameInit.h>

#include <Brofiler.h>

extern fwRefContainer<fx::ResourceManager> g_resourceManager;

class TestScriptThread : public GtaThread
{
	virtual void DoRun() override
	{
		PROFILE;

		static bool initedGame = false;

		if (!initedGame)
		{
			initedGame = Instance<ICoreGameInit>::Get()->HasVariable("networkInited");

			return;
		}

		if (Instance<ICoreGameInit>::Get()->HasVariable("networkTimedOut"))
		{
			return;
		}

		static std::once_flag of;

		std::call_once(of, []()
		{
			OnKillNetwork.Connect([](const char*)
			{
				initedGame = false;
			});
		});

		g_resourceManager->Tick();
	}
};

TestScriptThread thread;
extern GtaThread* g_resourceThread;

#ifdef USE_PROFILER
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
			m_desc = Profiler::EventDescription::Create(va("Resource::Tick %s", resource->GetName()), __FILE__, __LINE__, Profiler::Color::GreenYellow);
		}

		m_event = std::make_unique<Profiler::Event>(*m_desc);
	}

	void EndTick()
	{
		m_event = nullptr;
	}

private:
	Profiler::EventDescription* m_desc;

	std::unique_ptr<Profiler::Event> m_event;
};

DECLARE_INSTANCE_TYPE(ProfilerEventHolder);
#endif

static InitFunction initFunction([] ()
{
	rage::scrEngine::OnScriptInit.Connect([] ()
	{
		rage::scrEngine::CreateThread(&thread);
		g_resourceThread = &thread;
	});

#ifdef USE_PROFILER
	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		resource->SetComponent(new ProfilerEventHolder(resource));
	});
#endif
});
