/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "UvLoopTimer.h"

namespace fx::nodejs
{
	void UvLoopTimer::Initialize()
	{
		m_runtimes.reserve(32);

		const auto mainLoop = Instance<net::UvLoopManager>::Get()->GetOrCreate(std::string("svMain"))->GetLoop();
		uv_timer_init(mainLoop, &m_timer);
		uv_timer_start(&m_timer, UvPersistentCallback(&m_timer, [this](uv_timer_t*)
		{
			for(const auto rt : m_runtimes)
			{
				rt->TickFast();
			}
		}), 0, 1);

		m_initialized = true;
	}

	void UvLoopTimer::Shutdown()
	{
		if (m_initialized)
		{
			uv_timer_stop(&m_timer);
		}
	}

	void UvLoopTimer::AddRuntime(NodeScriptRuntime* rt)
	{
		m_runtimes.push_back(rt);
	}

	void UvLoopTimer::RemoveRuntime(NodeScriptRuntime* rt)
	{
		m_runtimes.erase(std::find(m_runtimes.begin(), m_runtimes.end(), rt));
	}
}
