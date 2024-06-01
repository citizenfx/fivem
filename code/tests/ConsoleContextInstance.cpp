#include "ConsoleContextInstance.h"

#include <StdInc.h>

#include "EventCore.h"
#include "console/Console.h"
#include "console/Console.Variables.h"

namespace fx
{
	fwRefContainer<console::Context> CreateConsoleContext()
	{
		fwRefContainer<console::Context> consoleContainer;
		console::CreateContext(console::GetDefaultContext(), &consoleContainer);
		consoleContainer->AddRef();
		return consoleContainer;
	}
}

console::Context* ConsoleContextInstance::Get()
{
	auto instance = fx::CreateConsoleContext();
	return instance.GetRef();
}
