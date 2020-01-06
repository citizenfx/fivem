#pragma once

#include "Console.CommandHelpers.h"
#include "Console.Commands.h"

#include <se/Security.h>

namespace internal
{
class ConsoleVariableEntryBase
{
public:
	virtual std::string GetValue() = 0;

	virtual bool SetValue(const std::string& value) = 0;
};

template <typename T, typename TConstraint = void>
struct Constraints
{
	inline static bool Compare(const T& value, const T& minValue, const T& maxValue)
	{
		return true;
	}
};

template <typename T>
struct Constraints<T, std::enable_if_t<std::is_arithmetic<T>::value>>
{
	inline static bool Compare(const T& value, const T& minValue, const T& maxValue)
	{
		if (typename ConsoleArgumentTraits<T>::Greater()(value, maxValue))
		{
			console::Printf("cmd", "Value out of range (%s) - should be at most %s\n", UnparseArgument(value).c_str(), UnparseArgument(maxValue).c_str());
			return false;
		}

		if (typename ConsoleArgumentTraits<T>::Less()(value, minValue))
		{
			console::Printf("cmd", "Value out of range (%s) - should be at least %s\n", UnparseArgument(value).c_str(), UnparseArgument(minValue).c_str());
			return false;
		}

		return true;
	}
};
}

enum ConsoleVariableFlags
{
	ConVar_None       = 0,
	ConVar_Archive    = 0x1,
	ConVar_Modified   = 0x2,
	ConVar_ServerInfo = 0x4,
	ConVar_Replicated = 0x8,
};

class ConsoleVariableManager
{
public:
	using THandlerPtr = std::shared_ptr<internal::ConsoleVariableEntryBase>;

	using TVariableCB = std::function<void(const std::string& name, int flags, const THandlerPtr& variable)>;

	using TWriteLineCB = std::function<void(const std::string& line)>;

public:
	ConsoleVariableManager(console::Context* context);

	~ConsoleVariableManager();

	virtual int Register(const std::string& name, int flags, const THandlerPtr& variable);

	virtual void Unregister(int token);

	virtual bool Process(const std::string& commandName, const ProgramArguments& arguments);

	virtual THandlerPtr FindEntryRaw(const std::string& name);

	virtual void AddEntryFlags(const std::string& name, int flags);

	virtual void RemoveEntryFlags(const std::string& name, int flags);

	virtual int GetEntryFlags(const std::string& name);

	virtual void ForAllVariables(const TVariableCB& callback, int flagMask = 0xFFFFFFFF);

	virtual void RemoveVariablesWithFlag(int flags);

	virtual void SaveConfiguration(const TWriteLineCB& writeLineFunction);

	inline console::Context* GetParentContext()
	{
		return m_parentContext;
	}

private:
	struct Entry
	{
		std::string name;

		int flags;

		THandlerPtr variable;

		int token;

		inline Entry(const std::string& name, int flags, const THandlerPtr& variable, int token)
		    : name(name), flags(flags), variable(variable), token(token)
		{
		}
	};

private:
	console::Context* m_parentContext;

	std::map<std::string, Entry, console::IgnoreCaseLess> m_entries;

	std::shared_mutex m_mutex;

	std::atomic<int> m_curToken;

	std::unique_ptr<ConsoleCommand> m_setCommand;
	std::unique_ptr<ConsoleCommand> m_setaCommand;
	std::unique_ptr<ConsoleCommand> m_setsCommand;
	std::unique_ptr<ConsoleCommand> m_setrCommand;

	std::unique_ptr<ConsoleCommand> m_toggleCommand;
	std::unique_ptr<ConsoleCommand> m_toggleCommand2;
	std::unique_ptr<ConsoleCommand> m_vstrCommand;
	std::unique_ptr<ConsoleCommand> m_vstrHoldCommand;
	std::unique_ptr<ConsoleCommand> m_vstrReleaseCommand;

public:
	inline static ConsoleVariableManager* GetDefaultInstance()
	{
		return Instance<ConsoleVariableManager>::Get();
	}

	fwEvent<const std::string&> OnConvarModified;
};

namespace internal
{
inline void MarkConsoleVarModified(ConsoleVariableManager* manager, const std::string& name)
{
	manager->AddEntryFlags(name, ConVar_Modified);
	manager->OnConvarModified(name);
}

template <typename T>
class ConsoleVariableEntry : public ConsoleVariableEntryBase
{
public:
	ConsoleVariableEntry(ConsoleVariableManager* manager, const std::string& name, const T& defaultValue)
		: m_manager(manager), m_name(name), m_trackingVar(nullptr), m_defaultValue(defaultValue), m_curValue(defaultValue), m_hasConstraints(false)
	{
		m_getCommand = std::make_unique<ConsoleCommand>(manager->GetParentContext(), name, [=] ()
		{
			console::Printf("cmd", " \"%s\" is \"%s\"\n default: \"%s\"\n type: %s\n", name.c_str(), GetValue().c_str(), UnparseArgument(m_defaultValue).c_str(), ConsoleArgumentName<T>::Get());
		});

		m_setCommand = std::make_unique<ConsoleCommand>(manager->GetParentContext(), name, [=] (const T& newValue)
		{
			SetRawValue(newValue);
		});
	}

	inline void SetConstraints(const T& minValue, const T& maxValue)
	{
		m_minValue = minValue;
		m_maxValue = maxValue;

		m_hasConstraints = true;
	}

	inline void SetTrackingVar(T* variable)
	{
		m_trackingVar = variable;

		if (variable)
		{
			*variable = m_curValue;
		}
	}

	virtual std::string GetValue() override
	{
		// update from a tracking variable
		if (m_trackingVar)
		{
			if (!(typename ConsoleArgumentTraits<T>::Equal()(*m_trackingVar, m_curValue)))
			{
				m_curValue = *m_trackingVar;
			}
		}

		return UnparseArgument(m_curValue);
	}

	virtual bool SetValue(const std::string& value) override
	{
		T newValue;

		if (ParseArgument(value, &newValue))
		{
			return SetRawValue(newValue);
		}

		return false;
	}

	inline const T& GetRawValue()
	{
		return m_curValue;
	}

	inline bool SetRawValue(const T& newValue)
	{
		if (m_hasConstraints && !Constraints<T>::Compare(newValue, m_minValue, m_maxValue))
		{
			return false;
		}

#ifndef IS_FXSERVER
		if (m_manager->GetEntryFlags(m_name) & ConVar_Replicated)
		{
			if (!seCheckPrivilege("builtin.setReplicated"))
			{
				console::Printf("cmd", "Cannot set server-replicated ConVar %s from client console.\n", m_name);
				return false;
			}
		}
#endif

		// keep the old value for comparison
		auto oldValue = m_curValue;

		// set the new value
		m_curValue = newValue;

		if (m_trackingVar)
		{
			*m_trackingVar = m_curValue;
		}

		// update modified flags and trigger change events
		if (!typename ConsoleArgumentTraits<T>::Equal()(oldValue, m_curValue))
		{
			// indirection as manager isn't declared by now
			MarkConsoleVarModified(m_manager, m_name);
		}

		return true;
	}

private:
	std::string m_name;

	T m_curValue;

	T m_minValue;
	T m_maxValue;

	T m_defaultValue;

	T* m_trackingVar;

	bool m_hasConstraints;

	std::unique_ptr<ConsoleCommand> m_getCommand;
	std::unique_ptr<ConsoleCommand> m_setCommand;

	ConsoleVariableManager* m_manager;
};
}

template <typename TValue>
static std::shared_ptr<internal::ConsoleVariableEntry<TValue>> CreateVariableEntry(ConsoleVariableManager* manager, const std::string& name, const TValue& defaultValue)
{
	ConsoleVariableManager::THandlerPtr oldEntry = manager->FindEntryRaw(name);

	// try to return/cast an old entry
	if (oldEntry)
	{
		// if this is already an entry of the right type, return said entry
		auto oldType = std::dynamic_pointer_cast<internal::ConsoleVariableEntry<TValue>>(oldEntry);

		if (oldType)
		{
			return oldType;
		}

		// not the same type, create a new entry we'll hopefully treat better
		std::string oldValue = oldEntry->GetValue();

		auto newEntry = std::make_shared<internal::ConsoleVariableEntry<TValue>>(manager, name, defaultValue);
		newEntry->SetValue(oldValue);

		return newEntry;
	}
	else
	{
		// no old entry exists, just create a default
		auto newEntry = std::make_shared<internal::ConsoleVariableEntry<TValue>>(manager, name, defaultValue);

		return newEntry;
	}
}

DECLARE_INSTANCE_TYPE(ConsoleVariableManager);
