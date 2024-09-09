/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "scrBind.h"
#include "Hooking.h"

void scrBindAddSafePointer(void* classPtr)
{

}

bool scrBindIsSafePointer(void* classPtr)
{
	return true;
}

fx::TNativeHandler scrBindCreateNativeMethodStub(scrBindNativeMethodStub stub, void* udata)
{
	return [stub, udata](fx::ScriptContext& context)
	{
		stub(context, udata);
	};
}
