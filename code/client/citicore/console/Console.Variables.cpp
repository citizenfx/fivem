#include <StdInc.h>
#include "Console.Variables.h"
#include "Console.h"

ConsoleVariableManager::ConsoleVariableManager(console::Context* parentContext)
    : m_parentContext(parentContext), m_curToken(0)
{
	auto setCommand = [=](ConsoleVariableFlags addFlags, const std::string& variable, const std::string& value) {
		// weird order is to prevent recursive locking
		{
			Entry* entry = nullptr;

			{
				std::shared_lock<std::shared_mutex> lock(m_mutex);

				auto oldVariable = m_entries.find(variable);

				if (oldVariable != m_entries.end())
				{
					entry = &oldVariable->second;
				}
			}

			if (entry)
			{
				entry->variable->SetValue(value);

				entry->flags |= addFlags;

				return;
			}
		}

		int flags = ConVar_Modified;
		flags |= addFlags;

		auto entry = CreateVariableEntry<std::string>(this, variable, "");
		entry->SetValue(value);

		Register(variable, flags, entry);
	};

	m_setCommand = std::make_unique<ConsoleCommand>(m_parentContext, "set", [=](const std::string& variable, const std::string& value) {
		setCommand(ConVar_None, variable, value);
	});

	// set archived
	m_setaCommand = std::make_unique<ConsoleCommand>(m_parentContext, "seta", [=](const std::string& variable, const std::string& value) {
		setCommand(ConVar_Archive, variable, value);
	});

#if IS_FXSERVER
	// set server
	m_setsCommand = std::make_unique<ConsoleCommand>(m_parentContext, "sets", [=](const std::string& variable, const std::string& value) {
		setCommand(ConVar_ServerInfo, variable, value);
	});
#endif

	// set replicated
	m_setrCommand = std::make_unique<ConsoleCommand>(m_parentContext, "setr", [=](const std::string& variable, const std::string& value) {
		setCommand(ConVar_Replicated, variable, value);
	});

	auto toggleCommand = [=](const std::string& name, const std::string& value1, const std::string& value2)
	{
		auto var = FindEntryRaw(name);

		if (var)
		{
			auto v = var->GetValue();

			// hackaround for bool vars
			if (v == "true")
			{
				v = "1";
			}
			else if (v == "false")
			{
				v = "0";
			}

			if (v == value1)
			{
				var->SetValue(value2);
			}
			else
			{
				var->SetValue(value1);
			}
		}
	};

	m_toggleCommand = std::make_unique<ConsoleCommand>(m_parentContext, "toggle", [=](const std::string& name)
	{
		toggleCommand(name, "1", "0");
	});

	m_toggleCommand2 = std::make_unique<ConsoleCommand>(m_parentContext, "toggle", [=](const std::string& name, const std::string& value1, const std::string& value2)
	{
		toggleCommand(name, value1, value2);
	});

	m_vstrCommand = std::make_unique<ConsoleCommand>(m_parentContext, "vstr", [=](const std::string& name)
	{
		auto var = FindEntryRaw(name);

		if (var)
		{
			m_parentContext->AddToBuffer(var->GetValue() + "\n");
		}
	});

	m_vstrHoldCommand = std::make_unique<ConsoleCommand>(m_parentContext, "+vstr", [=](const std::string& hold, const std::string& release)
	{
		auto var = FindEntryRaw(hold);

		if (var)
		{
			m_parentContext->AddToBuffer(var->GetValue() + "\n");
		}
	});

	m_vstrReleaseCommand = std::make_unique<ConsoleCommand>(m_parentContext, "-vstr", [=](const std::string& hold, const std::string& release)
	{
		auto var = FindEntryRaw(release);

		if (var)
		{
			m_parentContext->AddToBuffer(var->GetValue() + "\n");
		}
	});
}

ConsoleVariableManager::~ConsoleVariableManager()
{
}

ConsoleVariableManager::THandlerPtr ConsoleVariableManager::FindEntryRaw(const std::string& name)
{
	auto variable = m_entries.find(name);

	if (variable != m_entries.end())
	{
		return variable->second.variable;
	}

	return nullptr;
}

int ConsoleVariableManager::Register(const std::string& name, int flags, const THandlerPtr& variable)
{
	std::unique_lock<std::shared_mutex> lock(m_mutex);

	int token = m_curToken.fetch_add(1);
	m_entries.erase(name); // remove any existing entry

	m_entries.insert({name, Entry{name, flags, variable, token}});

	return token;
}

void ConsoleVariableManager::Unregister(int token)
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

void ConsoleVariableManager::AddEntryFlags(const std::string& name, int flags)
{
	std::unique_lock<std::shared_mutex> lock(m_mutex);

	auto it = m_entries.find(name);

	if (it != m_entries.end())
	{
		it->second.flags |= flags;

		m_parentContext->SetVariableModifiedFlags(it->second.flags);
	}
}

void ConsoleVariableManager::RemoveEntryFlags(const std::string& name, int flags)
{
	std::unique_lock<std::shared_mutex> lock(m_mutex);

	auto it = m_entries.find(name);

	if (it != m_entries.end())
	{
		it->second.flags &= ~(flags);
	}
}

int ConsoleVariableManager::GetEntryFlags(const std::string& name)
{
	std::unique_lock<std::shared_mutex> lock(m_mutex);

	auto it = m_entries.find(name);

	if (it != m_entries.end())
	{
		return it->second.flags;
	}

	return 0;
}

void ConsoleVariableManager::ForAllVariables(const TVariableCB& callback, int flagMask)
{
	// store first so we don't have to deal with recursive locks
	std::vector<std::tuple<std::string, int, THandlerPtr>> iterationList;

	{
		std::unique_lock<std::shared_mutex> lock(m_mutex);

		for (auto& entry : m_entries)
		{
			// if flags match the mask
			if ((entry.second.flags & flagMask) != 0)
			{
				iterationList.push_back(std::make_tuple(entry.second.name, entry.second.flags, entry.second.variable));
			}
		}
	}

	// and call the iterator with each tuple
	for (const auto& entry : iterationList)
	{
		apply(callback, entry);
	}
}

void ConsoleVariableManager::RemoveVariablesWithFlag(int flagMask)
{
	std::vector<int> iterationList;

	{
		std::unique_lock<std::shared_mutex> lock(m_mutex);

		for (auto& entry : m_entries)
		{
			// if flags match the mask
			if ((entry.second.flags & flagMask) != 0)
			{
				iterationList.push_back(entry.second.token);
			}
		}
	}

	for (auto token : iterationList)
	{
		Unregister(token);
	}
}

void ConsoleVariableManager::SaveConfiguration(const TWriteLineCB& writeLineFunction)
{
	ForAllVariables([&](const std::string& name, int flags, const THandlerPtr& variable) {
		writeLineFunction("seta \"" + name + "\" \"" + variable->GetValue() + "\"");
	},
	    ConVar_Archive);
}

bool ConsoleVariableManager::Process(const std::string& commandName, const ProgramArguments& arguments)
{
	// we currently don't process any variables specifically
	return false;
}
