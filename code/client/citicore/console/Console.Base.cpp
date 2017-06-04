#include <StdInc.h>
#include "Console.Base.h"

#include "Console.VariableHelpers.h"

#include <stdarg.h>

namespace console
{
static std::vector<void (*)(ConsoleChannel, const char*)> g_printListeners;
static int g_useDeveloper;

void Printf(ConsoleChannel channel, const char* format, const fmt::ArgList& argList)
{
	auto buffer = fmt::sprintf(format, argList);

	// print to all interested listeners
	for (auto& listener : g_printListeners)
	{
		listener(channel, buffer.data());
	}
}

void DPrintf(ConsoleChannel channel, const char* format, const fmt::ArgList& argList)
{
	if (g_useDeveloper > 0)
	{
		Printf(channel, format, argList);
	}
}

void PrintWarning(ConsoleChannel channel, const char* format, const fmt::ArgList& argList)
{
	// print the string directly
	Printf(channel, "^3Warning: %s^7", fmt::sprintf(format, argList));
}

void PrintError(ConsoleChannel channel, const char* format, const fmt::ArgList& argList)
{
	// print the string directly
	Printf(channel, "^1Error: %s^7", fmt::sprintf(format, argList));
}

static ConVar<int> developerVariable(GetDefaultContext(), "developer", ConVar_Archive, 0, &g_useDeveloper);
}

extern "C" DLL_EXPORT void CoreAddPrintListener(void(*function)(ConsoleChannel, const char*))
{
	console::g_printListeners.push_back(function);
}
