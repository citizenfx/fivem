/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ComponentLoader.h"

class ComponentInstance : public RunnableComponent
{
public:
	virtual bool Initialize() override;

	virtual bool DoGameLoad(void* module) override;

	virtual bool Shutdown() override;

	virtual void Run() override;
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

namespace fx
{
extern void SdkMain();
}

void ComponentInstance::Run()
{
	fx::SdkMain();
}

extern "C" DLL_EXPORT Component* CreateComponent()
{
	return new ComponentInstance();
}
