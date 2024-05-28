#pragma once
#include "EventCore.h"
#include "console/Console.h"

namespace fx
{
	class ConsoleContextInstance
	{
		static fwRefContainer<console::Context> Create()
		{
			fwRefContainer<console::Context> newContext{};
			console::CreateContext(console::GetDefaultContext(), &newContext);
			// increase ref to prevent free of the context
			newContext->AddRef();
			return newContext;
		}

	public:
		static fwRefContainer<console::Context> Get()
		{
			static fwRefContainer<console::Context> consoleContext = Create();
			return consoleContext;
		}
	};
}
