/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "HookFunction.h"

static HookFunctionBase* g_hookFunctions;

void HookFunctionBase::Register()
{
	m_next = g_hookFunctions;
	g_hookFunctions = this;
}

void HookFunctionBase::RunAll()
{
	for (auto func = g_hookFunctions; func; func = func->m_next)
	{
		func->Run();
	}
}

static RuntimeHookFunction* g_runtimeHookFunctions;

void RuntimeHookFunction::Register()
{
	m_next = g_runtimeHookFunctions;
	g_runtimeHookFunctions = this;
}

void RuntimeHookFunction::Run(const char* key)
{
	for (auto func = g_runtimeHookFunctions; func; func = func->m_next)
	{
		if (func->m_key == key)
		{
			func->m_function();
		}
	}
}