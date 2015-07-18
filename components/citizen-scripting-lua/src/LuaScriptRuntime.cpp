/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "fxScripting.h"

#include <om/OMComponent.h>

namespace fx
{
class LuaScriptRuntime : public OMClass<LuaScriptRuntime, IScriptRuntime, IScriptFileHandlingRuntime>
{
public:
	NS_DECL_ISCRIPTRUNTIME;

	NS_DECL_ISCRIPTFILEHANDLINGRUNTIME;
};

result_t LuaScriptRuntime::Create(IScriptHost *scriptHost)
{
	scriptHost->InvokeNative(fxNativeContext{});

	return FX_S_OK;
}

result_t LuaScriptRuntime::Destroy()
{
	return FX_S_OK;
}

int32_t LuaScriptRuntime::GetInstanceId()
{
	// TODO: handle properly
	return 435;
}

int32_t LuaScriptRuntime::HandlesFile(char* fileName)
{
	return strstr(fileName, ".lua") != 0;
}

// {A7242855-0350-4CB5-A0FE-61021E7EAFAA}
FX_DEFINE_GUID(CLSID_LuaScriptRuntime,
			0xa7242855, 0x350, 0x4cb5, 0xa0, 0xfe, 0x61, 0x2, 0x1e, 0x7e, 0xaf, 0xaa);

FX_NEW_FACTORY(LuaScriptRuntime);

FX_IMPLEMENTS(CLSID_LuaScriptRuntime, IScriptRuntime);
FX_IMPLEMENTS(CLSID_LuaScriptRuntime, IScriptFileHandlingRuntime);

class TestScriptHost : public OMClass<TestScriptHost, IScriptHost>
{
public:
	NS_DECL_ISCRIPTHOST;
};

result_t TestScriptHost::InvokeNative(fxNativeContext & context)
{
	return FX_S_OK;
}

// {441CA62C-7A70-4349-8A97-2BCBF7EAA61F}
FX_DEFINE_GUID(CLSID_TestScriptHost,
			0x441ca62c, 0x7a70, 0x4349, 0x8a, 0x97, 0x2b, 0xcb, 0xf7, 0xea, 0xa6, 0x1f);

FX_NEW_FACTORY(TestScriptHost);

FX_IMPLEMENTS(CLSID_TestScriptHost, IScriptHost);
}