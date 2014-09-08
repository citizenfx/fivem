#include "StdInc.h"
#include "ComponentLoader.h"

class ComponentInstance : public Component
{
public:
	virtual bool Initialize();

	virtual bool DoGameLoad(HANDLE module);

	virtual bool Shutdown();
};

bool ComponentInstance::Initialize()
{
	InitFunctionBase::RunAll();

	return true;
}

void InitInputHook();

bool ComponentInstance::DoGameLoad(HANDLE module)
{
	InitInputHook();

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