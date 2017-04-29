/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "BaseResourceScripting.h"
#include <MissionCleanup.h>
#include <stack>

CRITICAL_SECTION g_scriptCritSec;
BaseScriptEnvironment* g_currentEnvironment;
std::stack<BaseScriptEnvironment*> g_environmentStack;

PushEnvironment::PushEnvironment(BaseScriptEnvironment* environment)
{
	EnterCriticalSection(&g_scriptCritSec);

	m_oldEnvironment = g_currentEnvironment;
	g_currentEnvironment = environment;

	environment->Activate();

	g_environmentStack.push(m_oldEnvironment);
}

PushEnvironment::~PushEnvironment()
{
	g_currentEnvironment = m_oldEnvironment;

	if (m_oldEnvironment)
	{
		m_oldEnvironment->Activate();
	}
	else
	{
		OnDeactivateLastEnvironment();
	}

	g_environmentStack.pop();

	LeaveCriticalSection(&g_scriptCritSec);
}

BaseScriptEnvironment* BaseScriptEnvironment::GetCurrentEnvironment()
{
	return g_currentEnvironment;
}

BaseScriptEnvironment* BaseScriptEnvironment::GetInvokingEnvironment()
{
	return (g_environmentStack.empty()) ? nullptr : g_environmentStack.top();
}

CMissionCleanup* BaseScriptEnvironment::GetMissionCleanup()
{
	return nullptr;
}

void BaseScriptEnvironment::Activate() {}

static InitFunction initFunction([] ()
{
	InitializeCriticalSection(&g_scriptCritSec);
});

BaseScriptEnvironment::~BaseScriptEnvironment() {}

__declspec(dllexport) fwEvent<> PushEnvironment::OnDeactivateLastEnvironment;
__declspec(dllexport) fwEvent<fwString, bool> OnSetWorldAssetConfig;