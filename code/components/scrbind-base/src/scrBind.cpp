/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "scrBind.h"
#include "Hooking.h"
#include <jitasm.h>

void scrBindAddSafePointer(void* classPtr)
{

}

bool scrBindIsSafePointer(void* classPtr)
{
	return true;
}

struct scrMethodStub : public jitasm::function_cdecl<void, scrMethodStub, rage::scrNativeCallContext*>
{
private:
	void* m_udata;
	void* m_target;

public:
	scrMethodStub(void* target, void* udata)
		: m_target(target), m_udata(udata)
	{

	}

	void main(jitasm::Addr context)
	{
#if defined(_M_IX86)
		push((intptr_t)m_udata);

		mov(eax, dword_ptr[context]);
		push(eax);

		mov(eax, (intptr_t)m_target);
		call(eax);

		add(esp, 8);
#else
//#error Unknown machine type for scrBind!
#endif
	}
};

rage::scrEngine::NativeHandler scrBindCreateNativeMethodStub(scrBindNativeMethodStub stub, void* udata)
{
	scrMethodStub methodStub(stub, udata);
		
	hook::FunctionAssembly* assembled = new hook::FunctionAssembly(methodStub);

	return (rage::scrEngine::NativeHandler)assembled->GetCode();
}