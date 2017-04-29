/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <shared_mutex>

class ICoreGameInit
{
public:
	virtual bool GetGameLoaded() = 0;

	virtual void KillNetwork(const wchar_t* errorString) = 0;

	virtual bool TryDisconnect() = 0;

	virtual void SetPreventSavePointer(bool* preventSaveValue) = 0;

	virtual void LoadGameFirstLaunch(bool(*callBeforeLoad)()) = 0;

	virtual void ReloadGame() = 0;

	virtual bool TriggerError(const char* errorString) = 0;

	bool ShAllowed = false;

	bool EnhancedHostSupport = false;

private:
	std::set<std::string, std::less<>> VariableList;

	std::shared_mutex DataMutex;

	std::map<std::string, std::string, std::less<>> DataList;

public:
	template<typename T>
	inline bool HasVariable(const T& variable)
	{
		return (VariableList.find(variable) != VariableList.end());
	}

	inline void SetVariable(const std::string& variable)
	{
		VariableList.insert(variable);
	}

	inline void ClearVariable(const std::string& variable)
	{
		VariableList.erase(variable);
	}

	inline bool GetData(const std::string& key, std::string* value)
	{
		std::shared_lock<std::shared_mutex> lock(DataMutex);

		auto it = DataList.find(key);

		if (it != DataList.end())
		{
			*value = it->second;
			return true;
		}

		return false;
	}

	inline void SetData(const std::string& key, const std::string& value)
	{
		std::unique_lock<std::shared_mutex> lock(DataMutex);

		DataList[key] = value;
	}

public:
	// a1: error message
	fwEvent<const std::string&> OnTriggerError;

	fwEvent<> OnGameFinalizeLoad;

	fwEvent<> OnGameRequestLoad;
};

DECLARE_INSTANCE_TYPE(ICoreGameInit);