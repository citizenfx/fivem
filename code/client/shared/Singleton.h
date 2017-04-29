/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
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