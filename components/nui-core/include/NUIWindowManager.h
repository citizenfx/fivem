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

public:
	void AddWindow(NUIWindow* window);

	void ForAllWindows(std::function<void(fwRefContainer<NUIWindow>)> callback);

	void RemoveWindow(NUIWindow* window);
};

DECLARE_INSTANCE_TYPE(NUIWindowManager);