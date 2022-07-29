#pragma once

#ifndef IS_FXSERVER
namespace hook
{
extern void trampoline_raw(void* address, const void* target, void** origTrampoline);

template<typename TFunc, typename TAddr>
TFunc* trampoline(TAddr address, TFunc* target)
{
	TFunc* orig = nullptr;
	trampoline_raw(address, target, (void**)&orig);

	return orig;
}
}
#endif
