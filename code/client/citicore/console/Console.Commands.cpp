#include <StdInc.h>
#include "Console.Commands.h"
#include "Console.h"

#include <se/Security.h>

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

const std::string& ConsoleCommandManager::GetRawCommand()
{
	return m_rawCommand;
}

void ConsoleCommandManager::Invoke(const std::string& commandString, const std::string& executionContext)
{
	ProgramArguments arguments = console::Tokenize(commandString);

	if (arguments.Count() == 0)
	{
		return;
	}

	std::string command        = arguments.Shift();

	m_rawCommand = commandString;

	InvokeDirect(command, arguments, executionContext);

	m_rawCommand = "";
}

void ConsoleCommandManager::InvokeDirect(const std::string& commandName, const ProgramArguments& arguments, const std::string& executionContext)
{
	std::vector<THandler> functionAttempts;

	{
		// lock the mutex
		std::shared_lock<std::shared_mutex> lock(m_mutex);

		// acquire a list of command entries
		auto entryPair = m_entries.equal_range(commandName);

		if (entryPair.first == entryPair.second)
		{
			// unlock the shared_mutex
			lock.unlock();

			// try the fallback command handler
			if (!FallbackEvent(commandName, arguments, executionContext))
			{
				return;
			}

			// try in the fallback context, then
			console::Context* fallbackContext = m_parentContext->GetFallbackContext();

			if (fallbackContext)
			{
				fallbackContext->GetCommandManager()->m_rawCommand = m_rawCommand;
				return fallbackContext->GetCommandManager()->InvokeDirect(commandName, arguments, executionContext);
			}

			console::Printf("cmd", "No such command %s.\n", commandName.c_str());
			return;
		}

		// NOTE: to prevent recursive locking, we store the functions in a list first!
		for (std::pair<const std::string, Entry>& entry : fx::GetIteratorView(entryPair))
		{
			functionAttempts.push_back(entry.second.function);
		}
	}

	// check privilege
	if (!seCheckPrivilege(fmt::sprintf("command.%s", commandName)))
	{
		console::Printf("cmd", "Access denied for command %s.\n", commandName);
		return;
	}

	// try executing overloads until finding one that accepts our arguments - if none is found, print the error buffer
	ConsoleExecutionContext context(std::move(arguments), executionContext);
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

bool ConsoleCommandManager::HasCommand(const std::string& name)
{
	std::shared_lock<std::shared_mutex> lock(m_mutex);

	return (m_entries.find(name) != m_entries.end());
}
