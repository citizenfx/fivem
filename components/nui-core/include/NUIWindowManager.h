/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIWindow.h"

#include <mutex>

class NUIWindowManager
{
private:
	std::vector<NUIWindow*> m_nuiWindows;

	std::mutex m_nuiWindowMutex;

	fwRefContainer<NUIWindow> m_rootWindow;

public:
	void AddWindow(NUIWindow* window);

	void ForAllWindows(std::function<void(fwRefContainer<NUIWindow>)> callback);

	void RemoveWindow(NUIWindow* window);

public:
	inline fwRefContainer<NUIWindow> GetRootWindow() { return m_rootWindow; }

	inline void SetRootWindow(fwRefContainer<NUIWindow> window) { m_rootWindow = window; }
};

DECLARE_INSTANCE_TYPE(NUIWindowManager);