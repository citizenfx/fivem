/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ComponentLoader.h"
#include <om/OMComponent.h>

#ifndef IS_FXSERVER
#include <ResumeComponent.h>
#endif

int g_argc;
char** g_argv;

class ComponentInstance :
#ifndef IS_FXSERVER
	public LifeCyclePreInitComponentBase<OMComponentBase<Component>>
#else
	public OMComponentBase<Component>
#endif
{
public:
	virtual bool Initialize();

	virtual bool DoGameLoad(void* module);

	virtual bool Shutdown();

	virtual void SetCommandLine(int argc, char* argv[]) override
	{
		g_argc = argc;
		g_argv = argv;
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
