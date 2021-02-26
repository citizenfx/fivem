#include <StdInc.h>
#include <Hooking.h>
#include <MinHook.h>

#include <CrossBuildRuntime.h>
#include <ICoreGameInit.h>

#include <Streaming.h>

// solve 80-dependency limit being hit for clothing by *chaining* dependent .#mt files
static std::map<uint32_t, uint32_t> g_extraMetadataInheritance;

static int _metadataGetDependencies(streaming::strStreamingModule* self, uint32_t localIdx, uint32_t* indices, int count)
{
	if (auto it = g_extraMetadataInheritance.find(self->baseIdx + localIdx); it != g_extraMetadataInheritance.end())
	{
		indices[0] = it->second;
		return 1;
	}

	return 0;
}

static int (*g_origArchetypeGetDependencies)(void* self, uint32_t localIdx, uint32_t* indices, int count);

static int _archetypeGetDependencies(void* self, uint32_t localIdx, uint32_t* indices, int count)
{
	static auto strMgr = streaming::Manager::GetInstance();
	static auto mtStore = strMgr->moduleMgr.GetStreamingModule("ymt");

	// get the original, and copy out any metadata dependencies ourselves (except a root entry)
	uint32_t dependencies[512];
	int depCount = g_origArchetypeGetDependencies(self, localIdx, dependencies, std::size(dependencies));

	int outCount = 0;
	auto addOut = [&outCount, indices, count](uint32_t idx)
	{
		if (outCount < count)
		{
			indices[outCount++] = idx;
		}
	};

	uint32_t rootMetadataIndex = 0;

	for (size_t i = 0; i < depCount; i++)
	{
		auto module = strMgr->moduleMgr.GetStreamingModule(dependencies[i]);

		// if this *isn't* metadata, let's just continue
		if (module != mtStore)
		{
			addOut(dependencies[i]);
			continue;
		}

		// if it is, and we don't have a root index yet, set the root index
		if (!rootMetadataIndex)
		{
			rootMetadataIndex = dependencies[i];
			addOut(dependencies[i]);
			continue;
		}

		// and if we do, add a recursive dependency to the (last) root, so we end up with a linked list of dependencies
		g_extraMetadataInheritance[rootMetadataIndex] = dependencies[i];
		rootMetadataIndex = dependencies[i];
	}

	return outCount;
}

static hook::cdecl_stub<bool(void*, uint32_t)> _IsObjectInImage([]()
{
	return hook::get_pattern("74 20 8B C2 48 8B", -7);
});

static void strStreamingModule__GetObjectAndDependencies(streaming::strStreamingModule* self, uint32_t globalIdx, atArray<uint32_t>& outArray, uint32_t* ignoreList, int ignoreCount)
{
	static auto str = streaming::Manager::GetInstance();

	// should we be ignored?
	for (int i = 0; i < ignoreCount; i++)
	{
		if (ignoreList[i] == globalIdx)
		{
			return;
		}
	}

	if (!_IsObjectInImage(str, globalIdx))
	{
		return;
	}

	// start relocating the output array
	if (outArray.GetSize() == 80 && outArray.GetCount() == 0)
	{
		static thread_local uint32_t strIdxList[512];

		outArray.m_offset = strIdxList;
		outArray.m_size = std::size(strIdxList);
	}

	// are we already in the list?
	for (uint32_t entry : outArray)
	{
		if (entry == globalIdx)
		{
			return;
		}
	}

	// add ourselves
	outArray[outArray.m_count++] = globalIdx;

	// add our dependencies
	uint32_t dependencies[80];
	int numDeps = self->GetDependencies(globalIdx - self->baseIdx, dependencies, std::size(dependencies));

	for (int i = 0; i < numDeps; i++)
	{
		auto module = str->moduleMgr.GetStreamingModule(dependencies[i]);
		strStreamingModule__GetObjectAndDependencies(module, dependencies[i], outArray, ignoreList, ignoreCount);
	}
}

static HookFunction hookFunctionMetadataDep([]
{
	// CModelInfoStreamingModule::GetDependencies
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("8B 7F 08 8B 9D 78 03 00 00", -0x4B), _archetypeGetDependencies, (void**)&g_origArchetypeGetDependencies);
	MH_EnableHook(MH_ALL_HOOKS);

	// rage::fwMetaDataStore vtable -> GetDependencies
	{
		auto location = hook::get_pattern("45 8D 41 0C 48 8B D9 C7 40 D8 49 02 00 00 E8", 0x16);
		auto vtbl = hook::get_address<void**>(location);
		hook::put(&vtbl[21], _metadataGetDependencies);
	}

	// reset
	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		g_extraMetadataInheritance.clear();
	});

	// strStreamingModule::GetObjectAndDependencies rewrite-hack to properly cap-resize
	hook::jump(hook::get_pattern("49 8B D8 8D 48 01 48 85 D2 7E 12", -0x2C), strStreamingModule__GetObjectAndDependencies);
});

// increase stack frame size for implied atArray in some clothing metadata loader
static HookFunction hookFunctionStack([]
{
	if (xbr::IsGameBuild<1604>())
	{
		auto matches = hook::pattern("B8 00 04 00 00 48 2B E0 48 8D 4C 24 50 8B 01").count(2);

		for (size_t i = 0; i < matches.size(); i++)
		{
			auto match = matches.get(i);
			hook::put<uint32_t>(match.get<void>(1), 0xC00);
			hook::put<uint16_t>(match.get<void>((i == 0) ? 0x22 : 0x28), 0x180);
		}
	}
});

// stack-increasing attempt at exceeding the 80 dependency limit
// not sufficiently viable: needed 4 more places
#if 0
static int (*g_origRequestObject)(void* self, uint32_t idx, int type, void* extraStackFrame);

static int RequestObjectWrap(void* self, uint32_t idx, int type)
{
	uint32_t frame[512 /* count */ + 32 /* safe grow offset */];
	for (uint32_t& idx : frame)
	{
		idx = -1;
	}

	return g_origRequestObject(self, idx, type, frame);
}

constexpr const int kStackFrameSize = 8 /* stack frame return */ + 0x1B0 /* stack start */ + 64 /* redzone and some other stuff */;

static HookFunction hookFunction([]
{
	// request object
	{
		auto location = hook::get_pattern<char>("41 8B F8 48 8B F1 75 07 32 C0", -0x1C);
		assert(location[0x1AD] == 0x50);

		MH_Initialize();
		MH_CreateHook(location, RequestObjectWrap, (void**)&g_origRequestObject);
		MH_EnableHook(MH_ALL_HOOKS);

		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				lea(r8, qword_ptr[rsp + kStackFrameSize]);
				ret();
			}
		} stub1;

		hook::call_rcx(location + 0x1A3, stub1.GetCode());
		hook::put<uint32_t>(location + 0x1AD, 512);

		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				mov(eax, dword_ptr[rsp + (rcx * 4) + kStackFrameSize]);

				// NOTE: flags straddling ret!
				cmp(dword_ptr[rdx + rax * 8], 0);
				ret();
			}
		} stub2;

		hook::nop(location + 0x1D2, 8);
		hook::call(location + 0x1D2, stub2.GetCode());

		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				mov(rcx, rsi);
				mov(edx, dword_ptr[rsp + (rax * 4) + kStackFrameSize]);
				ret();
			}
		} stub3;

		hook::nop(location + 0x2FA, 7);
		hook::call_rcx(location + 0x2FA, stub3.GetCode());

		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				mov(ecx, dword_ptr[rsp + (r8 * 4) + kStackFrameSize]);
				ret();
			}
		} stub4;

		hook::call_rcx(location + 0x31C, stub4.GetCode());
	}
});
#endif
