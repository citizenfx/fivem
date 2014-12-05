#pragma once

#include <atArray.h>

template<typename T>
class ScriptObjectRegistry
{
private:
	static ScriptObjectRegistry ms_instance;

	atArray<T> m_values;

public:
	inline static ScriptObjectRegistry* GetInstance()
	{
		return &ms_instance;
	}

	inline T& GetValue(uint16_t index)
	{
		return m_values.Get(index);
	}

	inline uint16_t AddValue(const T& value)
	{
		m_values.Set(m_values.GetCount(), value);

		return m_values.GetCount() - 1;
	}

	inline void RemoveValue(uint16_t index)
	{
		m_values.Set(index, T());
	}
};

template<typename T>
inline uint16_t AddValueToRegistry(const T& value)
{
	return ScriptObjectRegistry<T>::GetInstance()->AddValue(value);
}

template<typename T>
inline ScriptObjectRegistry<T>* GetScriptRegistry()
{
	return ScriptObjectRegistry<T>::GetInstance();
}