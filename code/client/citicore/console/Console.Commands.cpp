#include <StdInc.h>
#include "Console.Commands.h"
#include "Console.h"

#include <IteratorView.h>

ConsoleCommandManager::ConsoleCommandManager(console::Context* parentContext)
    : m_parentContext(parentContext), m_curToken(0)
{
}

ConsoleCommandManager::~ConsoleCommandManager()
{
}

int ConsoleCommandManager::Register(const std::string& name, const THandler& handler)
{
	std::unique_lock<std::shared_mutex> lock(m_mutex);

	int token = m_curToken.fetch_add(1);
	m_entries.insert({name, Entry{name, handler, token}});

	return token;
}

void ConsoleCommandManager::Unregister(int token)
{
	std::unique_lock<std::shared_mutex> lock(m_mutex);

	// look through the list for a matching token
	for (auto it = m_entries.begin(); it != m_entries.end(); it++)
	{
		if (it->second.token == token)
		{
			// erase and return immediately (so we won't increment a bad iterator)
			m_entries.erase(it);
			return;
		}
	}
}

void ConsoleCommandManager::Invoke(const std::string& commandString)
{
	ProgramArguments arguments = console::Tokenize(commandString);
	std::string command        = arguments.Shift();

	return Invoke(command, arguments);
}

void ConsoleCommandManager::Invoke(const std::string& commandName, const ProgramArguments& arguments)
{
	std::vector<THandler> functionAttempts;

	{
		// lock the mutex
		std::shared_lock<std::shared_mutex> lock(m_mutex);

		// acquire a list of command entries
		auto entryPair = m_entries.equal_range(commandName);

		if (entryPair.first == entryPair.second)
		{
			// try in the fallback context first
			console::Context* fallbackContext = m_parentContext->GetFallbackContext();

			if (fallbackContext)
			{
				return fallbackContext->GetCommandManager()->Invoke(commandName, arguments);
			}

			// TODO: replace with console stream output
			console::Printf("cmd", "No such command %s.\n", commandName.c_str());
			return;
		}

		// NOTE: to prevent recursive locking, we store the functions in a list first!
		for (std::pair<const std::string, Entry>& entry : fx::GetIteratorView(entryPair))
		{
			functionAttempts.push_back(entry.second.function);
		}
	}

	// try executing overloads until finding one that accepts our arguments - if none is found, print the error buffer
	ConsoleExecutionContext context(std::move(arguments));
	bool result = false;

	// clear the error buffer
	for (auto& function : functionAttempts)
	{
		context.errorBuffer.str("");

		result = function(context);

		if (result)
		{
			break;
		}
	}

	if (!result)
	{
		console::Printf("cmd", "%s", context.errorBuffer.str().c_str());
	}
}

void ConsoleCommandManager::ForAllCommands(const std::function<void(const std::string&)>& callback)
{
	{
		// lock the mutex
		std::shared_lock<std::shared_mutex> lock(m_mutex);

		// loop through the commands
		for (auto& command : m_entries)
		{
			callback(command.first);
		}
	}
}
