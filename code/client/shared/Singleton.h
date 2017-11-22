/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

template<class T>
class fwSingleton
{
private:
	static T* m_instance;

	fwSingleton(const fwSingleton&);

private:
	inline static void EnsureInstance()
	{
		if (!m_instance)
		{
			m_instance = new(std::nothrow) T;

			assert(m_instance);
		}
	}

protected:
	fwSingleton() {}
	~fwSingleton() {}

public:
	static T* GetInstance()
	{
		EnsureInstance();

		return m_instance;
	}
};

template<class T>
T* fwSingleton<T>::m_instance = nullptr;