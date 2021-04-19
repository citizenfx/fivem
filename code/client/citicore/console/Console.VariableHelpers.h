#pragma once

#include "Console.Variables.h"
#include "Console.h"

template <typename TVariable>
class ConVar
{
private:
	int m_token;
	ConsoleVariableManager* m_manager;

	std::shared_ptr<internal::ConsoleVariableEntry<TVariable>> m_helper;

public:
	ConVar(const std::string& name, int flags, const TVariable& defaultValue)
	    : ConVar(ConsoleVariableManager::GetDefaultInstance(), name, flags, defaultValue, nullptr, nullptr)
	{
	}

	ConVar(console::Context* context, const std::string& name, int flags, const TVariable& defaultValue)
	    : ConVar(context->GetVariableManager(), name, flags, defaultValue)
	{
	}

	ConVar(const std::string& name, int flags, const TVariable& defaultValue, TVariable* trackingVar)
	    : ConVar(ConsoleVariableManager::GetDefaultInstance(), name, flags, defaultValue, trackingVar)
	{
	}

	ConVar(console::Context* context, const std::string& name, int flags, const TVariable& defaultValue, TVariable* trackingVar)
	    : ConVar(context->GetVariableManager(), name, flags, defaultValue, trackingVar)
	{
	}

	ConVar(const std::string& name, int flags, const TVariable& defaultValue, void (*changeCallback)(internal::ConsoleVariableEntry<TVariable>*))
		: ConVar(ConsoleVariableManager::GetDefaultInstance(), name, flags, defaultValue, nullptr, changeCallback)
	{
	}

	ConVar(console::Context* context, const std::string& name, int flags, const TVariable& defaultValue, void (*changeCallback)(internal::ConsoleVariableEntry<TVariable>*))
		: ConVar(context->GetVariableManager(), name, flags, defaultValue, nullptr, changeCallback)
	{
	}

	ConVar(ConsoleVariableManager* manager, const std::string& name, int flags, const TVariable& defaultValue, TVariable* trackingVar = nullptr, void (*changeCallback)(internal::ConsoleVariableEntry<TVariable>*) = nullptr)
	    : m_manager(manager)
	{
		m_helper = CreateVariableEntry<TVariable>(manager, name, defaultValue);
		m_token  = m_manager->Register(name, flags, m_helper);

		if (trackingVar)
		{
			m_helper->SetTrackingVar(trackingVar);
		}
		if (changeCallback)
		{
			m_helper->SetChangeCallback(changeCallback);
		}
	}

	~ConVar()
	{
		if (m_token != -1)
		{
			m_manager->Unregister(m_token);
			m_token = -1;
		}
	}

	inline const TVariable& GetValue()
	{
		return m_helper->GetRawValue();
	}

	inline std::shared_ptr<internal::ConsoleVariableEntry<TVariable>> GetHelper()
	{
		return m_helper;
	}
};

