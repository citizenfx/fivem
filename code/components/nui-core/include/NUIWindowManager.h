/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIWindow.h"

#include <d3d11.h>

#include <mutex>

class NUIWindowManager
{
private:
	std::vector<fwRefContainer<NUIWindow>> m_nuiWindows;

	std::mutex m_nuiWindowMutex;

	std::shared_mutex m_nuiRootWindowMutex;

	fwRefContainer<NUIWindow> m_rootWindow;

public:
	void AddWindow(NUIWindow* window);

	void ForAllWindows(std::function<void(fwRefContainer<NUIWindow>)> callback);

	void RemoveWindow(NUIWindow* window);

public:
	inline fwRefContainer<NUIWindow> GetRootWindow()
	{
		std::shared_lock _(m_nuiRootWindowMutex);
		return m_rootWindow;
	}

	inline void SetRootWindow(fwRefContainer<NUIWindow> window)
	{
		fwRefContainer<NUIWindow> old;

		{
			std::shared_lock _(m_nuiRootWindowMutex);
			old = m_rootWindow;
		}

		{
			std::unique_lock _(m_nuiRootWindowMutex);
			m_rootWindow = window;
		}
	}
};

DECLARE_INSTANCE_TYPE(NUIWindowManager);
