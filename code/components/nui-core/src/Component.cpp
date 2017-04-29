#include "StdInc.h"
#include "ComponentLoader.h"
#include "ResumeComponent.h"

class ComponentInstance : public LifeCyclePreInitComponentBase<Component>
{
public:
	virtual bool Initialize();

	virtual bool Shutdown();
};

bool ComponentInstance::Initialize()
{
	//InitFunctionBase::RunAll();

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