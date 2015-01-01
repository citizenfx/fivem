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
	m_nuiWindowMutex.lock();
	m_nuiWindows.push_back(window);
	m_nuiWindowMutex.unlock();
}

void NUIWindowManager::ForAllWindows(std::function<void(fwRefContainer<NUIWindow>)> callback)
{
	m_nuiWindowMutex.lock();

	for (auto& window : m_nuiWindows)
	{
		callback(window);
	}

	m_nuiWindowMutex.unlock();
}

void NUIWindowManager::RemoveWindow(NUIWindow* window)
{
	m_nuiWindowMutex.lock();

	for (auto it = m_nuiWindows.begin(); it != m_nuiWindows.end();)
	{
		if (*it == window)
		{
			it = m_nuiWindows.erase(it);
		}
		else
		{
			it++;
		}
	}

	m_nuiWindowMutex.unlock();
}

static InitFunction initFunction([] ()
{
	Instance<NUIWindowManager>::Set(new NUIWindowManager());
}, -50);