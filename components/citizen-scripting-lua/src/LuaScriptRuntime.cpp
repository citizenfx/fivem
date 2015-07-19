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

	OMPtr<fxIStream> stream;
	
	if (SUCCEEDED(scriptHost->OpenHostFile("__resource.lua", stream.GetAddressOf())))
	{
		char buffer[8192];
		uint32_t didRead;

		if (SUCCEEDED(stream->Read(buffer, sizeof(buffer), &didRead)))
		{
			buffer[didRead] = '\0';

			trace("read:\n%s\n", buffer);
		}
	}

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
}