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