/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include "Hooking.h"

#include <Error.h>

#include <typeinfo>

struct VirtualBase
{
	virtual ~VirtualBase() {}
};

struct VirtualDerivative : public VirtualBase
{
	virtual ~VirtualDerivative() override {}
};

static void PurecallHandler(VirtualBase* self)
{
	// get the type name of the object
	const char* typeName = va("unknown (vtable %p)", *(void**)self);

	try
	{
		typeName = typeid(*self).name();
	}
	catch (std::__non_rtti_object&)
	{
		
	}

	// get the module base
	const char* moduleBaseString = "";
	HMODULE module;

	if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)_ReturnAddress(), &module))
	{
		char filename[MAX_PATH];
		GetModuleFileNameA(module, filename, _countof(filename));

		moduleBaseString = va(" - %s+%X", strrchr(filename, '\\') + 1, (char*)_ReturnAddress() - (char*)module);
	}

	// format a fatal error message with the right data
	FatalError("Pure virtual function call in GTA function (type %s, called from %p%s)", typeName, _ReturnAddress(), moduleBaseString);
}

static HookFunction hookFunction([] ()
{
	hook::jump(hook::pattern("48 83 EC 28 B9 79 38 F4 43 E8").count(1).get(0).get<void>(0), PurecallHandler);
});