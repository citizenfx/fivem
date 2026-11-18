#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>

static void* SafeGetSrvFromResource(void* resource)
{
	__try
	{
		void** vtable = *(void***)resource;
		using GetSrvFn = void* (__fastcall*)(void*);
		return ((GetSrvFn)vtable[22])(resource); // 0xB0 / sizeof(void*)
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		static bool warned = false;
		if (!warned)
		{
			warned = true;
			trace("WARNING: SetTextureResourcesUsingVectorDXAPICall: skipped freed grcProgramResource (vtable AV)\n");
		}
		return nullptr;
	}
}

static HookFunction hookFunction([]
{
	static struct : jitasm::Frontend
	{
		void InternalMain() override
		{
			sub(rsp, 0x28);
			mov(rax, reinterpret_cast<uintptr_t>(&SafeGetSrvFromResource));
			call(rax);
			add(rsp, 0x28);
			ret();
		}
	} safeSrvStub;

	auto location = hook::get_pattern<char>("48 8B 01 FF 90 ? ? ? ? 4C 8D 05 ? ? ? ? EB");
	hook::nop(location, 9);
	hook::call(location, safeSrvStub.GetCode());
});
