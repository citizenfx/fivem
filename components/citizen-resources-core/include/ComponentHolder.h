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
private:
	template<bool value>
	struct AttachToImpl
	{
		template<typename TInstance>
		static void Run(TInstance* inst, THolder* holder)
		{
			
		}
	};

	template<>
	struct AttachToImpl<true>
	{
		template<typename TInstance>
		static void Run(TInstance* inst, THolder* holder)
		{
			static_cast<IAttached<THolder>*>(inst)->AttachToObject(holder);
		}
	};

public:
	//
	// Gets the object-specific instance registry.
	//
	virtual fwRefContainer<RefInstanceRegistry> GetInstanceRegistry() = 0;

	//
	// Utility function to get an instance of a particular interface from the instance registry.
	//
	template<typename TInstance>
	fwRefContainer<TInstance> GetComponent()
	{
		return Instance<TInstance>::Get(this->GetInstanceRegistry());
	}

	//
	// Utility function to set an instance of a particular interface in the instance registry.
	//
	template<typename TInstance>
	void SetComponent(fwRefContainer<TInstance> inst)
	{
		// attach to this resource if the component supports attaching
		AttachToImpl<std::is_base_of_v<IAttached<THolder>, TInstance>>::Run(inst.GetRef(), static_cast<THolder*>(this));

		Instance<TInstance>::Set(inst, this->GetInstanceRegistry());
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