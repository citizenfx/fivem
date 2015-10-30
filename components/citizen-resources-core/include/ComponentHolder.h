/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

namespace fx
{
template<typename THolder>
class IAttached
{
public:
	virtual void AttachToObject(THolder* object) = 0;
};

template<typename THolder>
class ComponentHolder
{
public:
	//
	// Gets the object-specific instance registry.
	//
	virtual fwRefContainer<RefInstanceRegistry> GetInstanceRegistry() = 0;
};

template<typename THolder>
class ComponentHolderAccessor
{
public:
	// mark this as a virtual type to allow dynamic_cast
	virtual ~ComponentHolderAccessor()
	{

	}

	//
	// Utility function to get an instance of a particular interface from the instance registry.
	//
	template<typename TInstance>
	fwRefContainer<TInstance> GetComponent()
	{
		auto asHolder = dynamic_cast<ComponentHolder<THolder>*>(this);
		assert(asHolder);

		return Instance<TInstance>::Get(asHolder->GetInstanceRegistry());
	}

	//
	// Utility function to set an instance of a particular interface in the instance registry.
	//
	template<typename TInstance>
	void SetComponent(fwRefContainer<TInstance> inst)
	{
		// attach to this resource if the component supports attaching
		IAttached<THolder>* attached = dynamic_cast<IAttached<THolder>*>(inst.GetRef());

		if (attached)
		{
			attached->AttachToObject(dynamic_cast<THolder*>(this));
		}

		auto asHolder = dynamic_cast<ComponentHolder<THolder>*>(this);
		assert(asHolder);

		Instance<TInstance>::Set(inst, asHolder->GetInstanceRegistry());
	}
};

template<typename TBase>
class ComponentHolderImpl : public ComponentHolder<TBase>
{
private:
	struct RefInstanceRegistryHolder
	{
		fwRefContainer<RefInstanceRegistry> registry;

		RefInstanceRegistryHolder()
		{
			registry = new RefInstanceRegistry();
		}
	};

	RefInstanceRegistryHolder m_instanceRegistry;

public:
	virtual fwRefContainer<RefInstanceRegistry> GetInstanceRegistry() override
	{
		return m_instanceRegistry.registry;
	}
};
}