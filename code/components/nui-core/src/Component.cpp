#include "StdInc.h"
#include "ComponentLoader.h"
#include "ResumeComponent.h"

class ComponentInstance : public LifeCyclePreInitComponentBase<Component>
{
public:
	virtual bool Initialize();

	virtual bool Shutdown();
};

bool g_inited = false;

bool ComponentInstance::Initialize()
{
	if (!g_inited)
	{
		InitFunctionBase::RunAll();
	}

	return true;
}

bool ComponentInstance::Shutdown()
{
	return true;
}

extern "C" __declspec(dllexport) Component* CreateComponent()
{
	return new ComponentInstance();
}
