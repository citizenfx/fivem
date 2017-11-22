/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"
#include "ComponentLoader.h"
#include <om/OMComponent.h>

result_t MonoCreateObjectInstance(const guid_t& guid, const guid_t& iid, void** objectRef);

std::vector<guid_t> MonoGetImplementedClasses(const guid_t& iid);

class ComponentInstance : public OMComponentBase<Component>
{
public:
	virtual bool Initialize();

	virtual bool DoGameLoad(void* module);

	virtual bool Shutdown();

	virtual result_t CreateObjectInstance(const guid_t& guid, const guid_t& iid, void** objectRef) override
	{
		return MonoCreateObjectInstance(guid, iid, objectRef);
	}

	virtual std::vector<guid_t> GetImplementedClasses(const guid_t& iid) override
	{
		return MonoGetImplementedClasses(iid);
	}
};

bool ComponentInstance::Initialize()
{
	InitFunctionBase::RunAll();

	return true;
}

bool ComponentInstance::DoGameLoad(void* module)
{
	HookFunction::RunAll();

	return true;
}

bool ComponentInstance::Shutdown()
{
	return true;
}

extern "C" DLL_EXPORT Component* CreateComponent()
{
	return new ComponentInstance();
}

OMComponentBaseImpl* OMComponentBaseImpl::ms_instance;
