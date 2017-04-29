/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <om/core.h>
#include <om/IBase.h>

class OMComponent
{
public:
	virtual result_t CreateObjectInstance(const guid_t& guid, const guid_t& iid, void** objectRef) = 0;

	virtual std::vector<guid_t> GetImplementedClasses(const guid_t& iid) = 0;
};

struct OMFactoryDefinition
{
	guid_t guid;
	fxIBase*(*factory)();

	OMFactoryDefinition* next;

	inline OMFactoryDefinition(const guid_t& guid, fxIBase*(*factory)());
};

struct OMImplements
{
	guid_t iid;
	guid_t clsid;

	OMImplements* next;

	inline OMImplements(const guid_t& clsid, const guid_t& iid);
};

class OMComponentBaseImpl
{
private:
	static OMComponentBaseImpl* ms_instance;

	OMFactoryDefinition* m_factoryList;

	OMImplements* m_implList;

public:
	inline OMComponentBaseImpl()
	{
		m_factoryList = nullptr;
		m_implList = nullptr;
	}

	static inline OMComponentBaseImpl* Get()
	{
		if (!ms_instance)
		{
			ms_instance = new OMComponentBaseImpl();
		}

		return ms_instance;
	}

	inline OMFactoryDefinition*& GetCurrentFactory()
	{
		return m_factoryList;
	}

	inline OMImplements*& GetCurrentImplements()
	{
		return m_implList;
	}
};

OMFactoryDefinition::OMFactoryDefinition(const guid_t& guid, fxIBase*(*factory)())
	: guid(guid), factory(factory), next(nullptr)
{
	auto& entry = OMComponentBaseImpl::Get()->GetCurrentFactory();

	if (entry)
	{
		next = entry->next;
		entry->next = this;
	}
	else
	{
		entry = this;
	}
}

OMImplements::OMImplements(const guid_t& clsid, const guid_t& iid)
	: clsid(clsid), iid(iid), next(nullptr)
{
	auto& entry = OMComponentBaseImpl::Get()->GetCurrentImplements();

	if (entry)
	{
		next = entry->next;
		entry->next = this;
	}
	else
	{
		entry = this;
	}
}

template<typename TBaseComponent>
class OMComponentBase : public TBaseComponent, public OMComponent
{
private:
	OMComponentBaseImpl* m_impl;

public:
	OMComponentBase()
		: TBaseComponent()
	{
		m_impl = OMComponentBaseImpl::Get();
	}

	virtual result_t CreateObjectInstance(const guid_t& guid, const guid_t& iid, void** objectRef) override
	{
		// if the GUID is the null GUID, we assume matching by interface
		const guid_t& matchingGuid = (fx::IsNullGuid(guid)) ? iid : guid;

		for (const OMFactoryDefinition* entry = m_impl->GetCurrentFactory(); entry; entry = entry->next)
		{
			if (entry->guid == matchingGuid)
			{
				// create an instance of the class and try to cast to the interface
				fxIBase* base = entry->factory();
				result_t result = base->QueryInterface(iid, objectRef);

				// release the base class reference
				base->Release();

				// if the interface is defined, return the result
				if (result != FX_E_NOINTERFACE)
				{
					return result;
				}
			}
		}

		// return an error
		return FX_E_NOINTERFACE;
	}

	virtual std::vector<guid_t> GetImplementedClasses(const guid_t& iid) override
	{
		std::vector<guid_t> retval;

		for (const OMImplements* entry = m_impl->GetCurrentImplements(); entry; entry = entry->next)
		{
			if (entry->iid == iid)
			{
				retval.push_back(entry->clsid);
			}
		}

		return retval;
	}
};