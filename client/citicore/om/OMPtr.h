/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

namespace fx
{
template<class T>
class OMPtr
{
private:
	T* m_ref;

public:
	OMPtr()
		: m_ref(nullptr)
	{

	}

	~OMPtr()
	{
		if (m_ref)
		{
			if (m_ref->Release())
			{
				m_ref = nullptr;
			}
		}
	}

	OMPtr(T* ref)
	{
		m_ref = ref;

		if (m_ref)
		{
			m_ref->AddRef();
		}
	}

	OMPtr(const OMPtr& rc)
	{
		m_ref = rc.m_ref;

		if (m_ref)
		{
			m_ref->AddRef();
		}
	}

	uint32_t GetRefCount() const
	{
		return m_ref->GetRefCount();
	}

	T* GetRef() const
	{
		return m_ref;
	}

	T* operator->() const
	{
		return m_ref;
	}

	OMPtr& operator=(const OMPtr& other)
	{
		if (m_ref)
		{
			m_ref->Release();
		}

		m_ref = other.GetRef();

		if (m_ref)
		{
			m_ref->AddRef();
		}

		return *this;
	}

	inline T** GetAddressOf()
	{
		return &m_ref;
	}

	inline T** ReleaseAndGetAddressOf()
	{
		if (m_ref)
		{
			if (m_ref->Release())
			{
				m_ref = nullptr;
			}
		}

		return GetAddressOf();
	}

	template<class TOther>
	result_t As(OMPtr<TOther>* p)
	{
		result_t res = FX_E_INVALIDARG;

		if (m_ref)
		{
			res = m_ref->QueryInterface(TOther::GetIID(), (void**)p->GetAddressOf());
		}

		return res;
	}
};

template<typename T>
bool operator<(const OMPtr<T>& left, const OMPtr<T>& right)
{
	return (left.GetRef() < right.GetRef());
}

template<typename TInterface>
result_t MakeInterface(OMPtr<TInterface>* ptr)
{
	return fxCreateObjectInstance(GetNullGuid(), TInterface::GetIID(), (void**)ptr->ReleaseAndGetAddressOf());
}

template<typename TInterface>
result_t MakeInterface(OMPtr<TInterface>* ptr, const guid_t& clsid)
{
	return fxCreateObjectInstance(clsid, TInterface::GetIID(), (void**)ptr->ReleaseAndGetAddressOf());
}
}