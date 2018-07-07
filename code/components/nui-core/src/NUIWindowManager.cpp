/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIWindowManager.h"

#include "memdbgon.h"

void NUIWindowManager::AddWindow(NUIWindow* window)
{
	auto lock = std::unique_lock<std::mutex>(m_nuiWindowMutex);
	m_nuiWindows.push_back(window);
}

void NUIWindowManager::ForAllWindows(std::function<void(fwRefContainer<NUIWindow>)> callback)
{
	decltype(m_nuiWindows) windowsCopy;

	{
		auto lock = std::unique_lock<std::mutex>(m_nuiWindowMutex);
		windowsCopy = m_nuiWindows;
	}

	for (auto& window : windowsCopy)
	{
		callback(window);
	}
}

void NUIWindowManager::RemoveWindow(NUIWindow* window)
{
	auto lock = std::unique_lock<std::mutex>(m_nuiWindowMutex);

	for (auto it = m_nuiWindows.begin(); it != m_nuiWindows.end();)
	{
		if ((*it).GetRef() == window)
		{
			it = m_nuiWindows.erase(it);
		}
		else
		{
			it++;
		}
	}
}

static InitFunction initFunction([] ()
{
	Instance<NUIWindowManager>::Set(new NUIWindowManager());
}, -50);
