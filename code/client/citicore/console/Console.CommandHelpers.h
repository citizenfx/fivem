#pragma once

#include "Console.Commands.h"
#include "Console.h"

class ConsoleCommand
{
private:
	int m_token;
	ConsoleCommandManager* m_manager;

public:
	template <typename TFunction>
	ConsoleCommand(const std::string& name, TFunction function)
	    : ConsoleCommand(ConsoleCommandManager::GetDefaultInstance(), name, function)
	{
	}

	template <typename TFunction>
	ConsoleCommand(console::Context* context, const std::string& name, TFunction function)
	    : ConsoleCommand(context->GetCommandManager(), name, function)
	{
	}

	template <typename TFunction>
	ConsoleCommand(ConsoleCommandManager* manager, const std::string& name, TFunction function)
	    : m_manager(manager)
	{
		auto functionRef = detail::make_function(function);

		using ConsoleCommandFunction = internal::ConsoleCommandFunction<decltype(functionRef)>;

		m_token = m_manager->Register(name, [=](ConsoleExecutionContext& context) {
			return ConsoleCommandFunction::Call(functionRef, context);
		}, ConsoleCommandFunction::kNumArguments);
	}

	~ConsoleCommand()
	{
		if (m_token != -1)
		{
			m_manager->Unregister(m_token);
			m_token = -1;
		}
	}
};
