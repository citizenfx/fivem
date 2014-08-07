#include "StdInc.h"
#include "HookFunction.h"

static HookFunction* g_hookFunctions;

void HookFunction::Register()
{
	m_next = g_hookFunctions;
	g_hookFunctions = this;
}

void HookFunction::RunAll()
{
	for (auto func = g_hookFunctions; func; func = func->m_next)
	{
		func->m_function();
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