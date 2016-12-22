#include <StdInc.h>
#include <Pool.h>

#include <Hooking.h>

static std::map<uint32_t, atPoolBase*> g_pools;

GTA_CORE_EXPORT atPoolBase* rage::GetPoolBase(uint32_t hash)
{
	auto it = g_pools.find(hash);

	if (it == g_pools.end())
	{
		return nullptr;
	}

	return it->second;
}

static atPoolBase* SetPoolFn(atPoolBase* pool, uint32_t hash)
{
	g_pools.insert({ hash, pool });

	return pool;
}

static HookFunction hookFunction([] ()
{
	auto registerPools = [] (hook::pattern& patternMatch, int callOffset, int hashOffset)
	{
		for (size_t i = 0; i < patternMatch.size(); i++)
		{
			auto match = patternMatch.get(i);
			auto hash = *match.get<uint32_t>(hashOffset);

			struct : jitasm::Frontend
			{
				uint32_t hash;
				uint64_t origFn;

				void InternalMain() override
				{
					sub(rsp, 0x38);

					mov(rax, qword_ptr[rsp + 0x38 + 0x28]);
					mov(qword_ptr[rsp + 0x20], rax);

					mov(rax, qword_ptr[rsp + 0x38 + 0x30]);
					mov(qword_ptr[rsp + 0x28], rax);

					mov(rax, origFn);
					call(rax);

					mov(rcx, rax);
					mov(edx, hash);

					mov(rax, (uint64_t)&SetPoolFn);
					call(rax);

					add(rsp, 0x38);

					ret();
				}
			}* stub = new std::remove_pointer_t<decltype(stub)>();

			stub->hash = hash;

			auto call = match.get<void>(callOffset);
			hook::set_call(&stub->origFn, call);
			hook::call(call, stub->GetCode());
		}
	};

	// find initial pools
	registerPools(hook::pattern("BA ? ? ? ? 41 B8 ? ? ? 00 E8 ? ? ? ? 4C 8D 05"), 0x2C, 1);
	registerPools(hook::pattern("C6 BA ? ? ? ? E8 ? ? ? ? 4C 8D 05"), 0x27, 2);
});