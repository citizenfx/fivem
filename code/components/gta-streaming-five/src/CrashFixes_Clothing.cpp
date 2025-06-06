#include <StdInc.h>

#include "Hooking.Patterns.h"

#include <Hooking.h>
#include <jitasm.h>

#include "CrossBuildRuntime.h"
#include "Hooking.Stubs.h"
#include "ICoreGameInit.h"

const static int MAX_STREAMING_DEPENDENCIES = 120;

// Determines the maximum number of streaming dependencies allowed based on the game build version.
static int GetMaxStreamingDependenciesForBuild()
{
	// Rockstar increased the limit to 100 in build 2612 and later.
	if (xbr::IsGameBuildOrGreater<2612>()) return 100;

	return 80; // Default limit for older builds.
}

struct StreamingDependenciesPair
{
	std::string_view pattern;
	// Locations where the arguments are calculated so we can add the difference of the new limit.
	// This can also contain the reserving of the stack space.
	std::initializer_list<int> argsOffsets;
	std::initializer_list<int> limitOffset; // Locations where the 80(for older builds) or 100(for newer builds) limit is set.
};

namespace rage
{
	struct SPedDLCMetaFileQueryData
	{
		int32_t m_storeIndex;
		uint32_t m_dlcNameHash;
	};
}

static int (*g_origRequestObject)(void* self, uint32_t idx, int type, void* extraStackFrame);
static int RequestObjectWrap(void* self, uint32_t idx, int type, void* extraStackFrame)
{
	uint32_t frame[MAX_STREAMING_DEPENDENCIES /* count */ + 32 /* safe grow offset */];
	for (uint32_t& id : frame)
	{
		id = -1;
	}

	return g_origRequestObject(self, idx, type, frame);
}

static void (*g_origCreateDependentsGraph)(void* self, __int64 a2, void* extraStackFrame);
static void CreateDependentsGraphWrap(void* self, __int64 a2, void* extraStackFrame)
{
	uint32_t frame[MAX_STREAMING_DEPENDENCIES /* count */ + 32 /* safe grow offset */];
	for (uint32_t& id : frame)
	{
		id = -1;
	}
	
	g_origCreateDependentsGraph(self, a2, frame);
}

static int (*g_origGetNextFilesOnCdNew)(void* self, unsigned int a2, unsigned __int8 a3, __int64* a4, int a5, unsigned int a6, int a7, void* extraStackFrame);
static int GetNextFilesOnCdNewWrap(void* self, unsigned int a2, unsigned __int8 a3, __int64* a4, int a5, unsigned int a6, int a7, void* extraStackFrame)
{
	uint32_t frame[MAX_STREAMING_DEPENDENCIES /* count */ + 32 /* safe grow offset */];
	for (uint32_t& id : frame)
	{
		id = -1;
	}

	return g_origGetNextFilesOnCdNew(self, a2, a3, a4, a5, a6, a7, frame);
}

static int (*g_origModelInfoStreamingModuleGetDependencies)(void* self, uint32_t localIdx, uint32_t* indices, int count, void* extraStackFrame);
static int ModelInfoStreamingModuleGetDependenciesWrap(void* self, uint32_t localIdx, uint32_t* indices, int count, void* extraStackFrame)
{
	char extra[(MAX_STREAMING_DEPENDENCIES * 8) /* PedDLCMetaFile */ + 4 /* m_count of fixedArray */ + 32 /* safe grow offset */];
	for(char& id : extra)
	{
		id = 99; // Fill with dummy data
	}
	
	return g_origModelInfoStreamingModuleGetDependencies(self, localIdx, indices, count, extra);
}

static void (*g_origSetupExternallyDrivenDOFs)(void* self, void* extraStackFrame);
static void SetupExternallyDrivenDOFsWrap(void* self, void* extraStackFrame)
{
	char extra[(MAX_STREAMING_DEPENDENCIES * 8) /* PedDLCMetaFile */ + 4 /* m_count of fixedArray */ + 32 /* safe grow offset */];
	/*for(char& id : extra)
	{
		id = 99; // Fill with dummy data
	}*/

	g_origSetupExternallyDrivenDOFs(self, extra);
}

static char (*g_origProcessExternallyDrivenDOFs)(void* self, void* a2, void* extraStackFrame);
static char ProcessExternallyDrivenDOFsWrap(void* self, void* a2, void* extraStackFrame)
{
	char extra[(MAX_STREAMING_DEPENDENCIES * 8) /* PedDLCMetaFile */ + 4 /* m_count of fixedArray */ + 32 /* safe grow offset */];
	for(char& id : extra)
	{
		id = 99; // Fill with dummy data
	}

	return g_origProcessExternallyDrivenDOFs(self, a2, extra);
}

/*static char (*g_origClearDLCScenarioInfos)(void* self, __int64 file);
static char ClearDLCScenarioInfos(void* self, __int64 file)
{
	trace("ClearDLCScenarioInfos called with file: %lld\n", file);
	g_origClearDLCScenarioInfos(self, file);
	return true;
}*/

template <class T,int C>
class atFixedArray {
public:
	T elements[C];
	int count;
};

/*static void (*g_origGetCreatureMetaDataIndices)(void* self, void* a2, atFixedArray<rage::SPedDLCMetaFileQueryData, MAX_STREAMING_DEPENDENCIES >& targetArray);
static void GetCreatureMetaDataIndicesWrap(void* self, void* a2, atFixedArray<rage::SPedDLCMetaFileQueryData, MAX_STREAMING_DEPENDENCIES >& targetArray)
{
	g_origGetCreatureMetaDataIndices(self, a2, targetArray);
	trace("GetCreatureMetaDataIndices %d\n", targetArray.count);
}*/

static hook::cdecl_stub<void(void* self, void* modelInfo, void* targetArray)> _getCreatureMetaDataIndices([]()
{
	return hook::get_pattern("40 55 53 56 57 41 54 41 56 41 57 48 8B EC 48 83 EC ? 8B 45 ? 45 33 E4");
});

static hook::cdecl_stub<void(void* self)> _invalidateExternallyDrivenDOFs([]()
{
	return hook::get_call(hook::get_pattern("48 8B C4 53 48 81 EC ? ? ? ? 48 8B 51 ? 48 8B D9", 57));
});

static void* g_extraMetadataManager;

static void UpdatePropExpressions(hook::FlexStruct* self)
{
	if(self)
	{
		void* pedModelInfo = self->At<void*>(0x20);
		if(pedModelInfo)
		{
			// CAUTION! _getCreatureMetaDataIndices need to be patched to support the new size of the fixed array
			atFixedArray<rage::SPedDLCMetaFileQueryData, MAX_STREAMING_DEPENDENCIES> targetArray;
			targetArray.count = 0;
			_getCreatureMetaDataIndices(g_extraMetadataManager, pedModelInfo, &targetArray);
			//GetCreatureMetaDataIndicesWrap(g_extraMetadataManager, pedModelInfo, targetArray);
			if(targetArray.count > 0)
			{
				_invalidateExternallyDrivenDOFs(self);
			}
		}
	}
}

static HookFunction hookFunctionStack2([]()
{
	//This patch is very long, and I'm currently working on b1604, if we get it working on this game build we can transform this patch to b3407, or at least try :)
	if(!xbr::IsGameBuild<1604>())
	{
		trace("Game build is not 1604, skipping streaming dependencies patch.\n");
		return;
	}
	const int originalMaxStreamingDependencies = GetMaxStreamingDependenciesForBuild();
	
	// The current implementation supports a maximum of 255 dependencies due to instruction limitations,
	// as some instructions can only store values up to 1 byte.
	// This limitation could be bypassed with modifications, but for now, 255 should be more than sufficient.
	assert(MAX_STREAMING_DEPENDENCIES <= 255 && "MaxStreamingDependencies should be less or equal to 255 due to instruction limitations.");

	const int maxStreamingDependenciesDiff = (MAX_STREAMING_DEPENDENCIES - originalMaxStreamingDependencies) * 4;

	std::initializer_list<StreamingDependenciesPair> streamingDependenciesLocations = {
		{
			"48 89 5C 24 ? 48 89 6C 24 ? 89 54 24 ? 56 57 41 56 48 81 EC ? ? ? ? 4C 8B F1", // strStreamingInfoManager::AddDependentCounts
			{ 21, 43, 107, 191 },
			{ 49 }
		},
		{
			"48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 89 50 ? 57 48 81 EC ? ? ? ? 48 8B D9", // strStreamingInfoManager::RemoveDependentCountsAndUnrequest
			{ 22, 44, 234, 260 },
			{ 50 }
		},
		{
			"48 89 5C 24 ? 44 89 44 24 ? 89 4C 24 ? 57", // rage::FindDependents
			{ 18, 67, 103, 122, 148, 175, 185, 192 },
			{ 46, 81 }
		},
		{
			"48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 78 ? 41 56 48 81 EC ? ? ? ? 48 8B F1 4D 8B F0", // strStreamingInfoManager::GetObjectAndDependenciesSizes
			{ 24, 55, 87, 126, 152, 197 },
			{ 36 }
		},
		{
			"48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 89 50 ? 57 48 81 EC ? ? ? ? 33 C0", // strStreamingModule::GetObjectAndDependencies
			{ 22, 34, 149, 236, 273 },
			{ 158 }
		},
		{
			"48 89 5C 24 ? 89 4C 24 ? 56 57 41 56 48 81 EC", // rage::AreDependenciesMet
			{ 16, 63, 162, 234, 241 },
			{ 77 }
		},
		{
			"48 89 5C 24 ? 48 89 54 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 4C 8B F1", // rage::strStreamingLoader::RequestStreamFiles
			{ 24, 57, 164, 479, 492, 518, 525 },
			{ 358 }
		},
		{
			"40 57 48 81 EC ? ? ? ? 41 B8", // CSceneStreamerMgr::AllDepsSatisfied
			{ 5, 109 },
			{ 11 }
		},
	};

	for(const auto& entry : streamingDependenciesLocations)
	{
		auto location = hook::get_pattern<char>(entry.pattern);

		for(const auto& argsOffsets : entry.argsOffsets)
		{
			auto value = *(int32_t*)(location + argsOffsets);
			hook::put<int32_t>(location + argsOffsets, value + (value < 0 ? -maxStreamingDependenciesDiff : maxStreamingDependenciesDiff));
		}

		for(const auto& limitOffset : entry.limitOffset)
		{
			hook::put<char>(location + limitOffset, MAX_STREAMING_DEPENDENCIES);
		}
	}

	// This is to prevent an unknown crash that I think is fixed in latest game builds, I was wrong :(
	//g_origClearDLCScenarioInfos = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 89 54 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 4C 8B 31"), ClearDLCScenarioInfos);
	
	constexpr const int kCountArrayOffset = (MAX_STREAMING_DEPENDENCIES * 8);

	// CExtraMetadataMgr::GetCreatureMetaDataIndices
	// Here we need to change the offset of m_count because GetCreatureMetaDataIndices except r8(third arg) to be a fixed array of (80 for old builds, 100 for new builds) elements.
	// So we need to calculate the new offset to m_count based in the new MAX_STREAMING_DEPENDENCIES value.
	// Also, we need to make sure all functions that call this are also patched to use the new m_count offset.
	{
		auto location = hook::get_pattern<char>("40 55 53 56 57 41 54 41 56 41 57 48 8B EC 48 83 EC ? 8B 45 ? 45 33 E4");
		//g_origGetCreatureMetaDataIndices = hook::trampoline(location, GetCreatureMetaDataIndicesWrap);

		// Location of the m_count offsets 
		std::initializer_list<int> getCreatureMetaDataIndicesOffsets = {
			85,  /* mov     [r8+280h], r12d */
			98,  /* mov     dword ptr [r8+280h], 1 */
			256, /* movsxd  rax, dword ptr [rdi+280h] */
			273  /* mov     [rdi+280h], eax */
		};

		for(const auto& offset : getCreatureMetaDataIndicesOffsets)
		{
			hook::put<int32_t>(location + offset, kCountArrayOffset);
		}
	}
	
	// CPed::ProcessExternallyDrivenDOFs
	{
		constexpr const int kStackFrameSize = 8 + 0x3F0 + 64;
		
		auto location = hook::get_pattern<char>("48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 83 A1");

		g_origProcessExternallyDrivenDOFs = hook::trampoline(location, ProcessExternallyDrivenDOFsWrap);

		auto location2 = hook::get_pattern<char>("4C 8D 45 ? 49 8B D7 44 89 AD");

		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				//lea(r8, qword_ptr[rbp-0x50]); // Original instruction
				lea(r8, qword_ptr[rsp + kStackFrameSize]);
				mov(rdx, r15);
				//mov(dword_ptr[rbp+0x230], r13d); // Original instruction
				mov(dword_ptr[rsp + kStackFrameSize + kCountArrayOffset], r13d);
				ret();
			}
		} stub1;
		hook::nop(location2, 14);
		hook::call_reg<2>(location2, stub1.GetCode()); // rdx

		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				//cmp(dword_ptr[rbp + 0x230], r13d); // Original instruction
				cmp(dword_ptr[rsp + kStackFrameSize + kCountArrayOffset], r13d);
				ret();
			}
		} stub2;
		hook::nop(location2 + 33, 7);
		hook::call_rcx(location2 + 33, stub2.GetCode());

		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				//lea(r14, qword_ptr[rbp - 0x50]); // Original instruction
				lea(r14, qword_ptr[rsp + kStackFrameSize]);
				mov(qword_ptr[rsp + 0x20 + 8 /* Return addr for call */], r14); // Original instruction
				ret();
			}
		} stub3;
		hook::nop(location2 + 46, 9);
		hook::call_rcx(location2 + 46, stub3.GetCode());

		auto location3 = hook::get_pattern("3B B5 ? ? ? ? 0F 82 ? ? ? ? 41 83 CC");
		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				//cmp(esi, dword_ptr[rbp + 0x230]); // Original instruction
				cmp(esi, dword_ptr[rsp + kStackFrameSize + kCountArrayOffset]);
				ret();
			}
		} stub4;
		hook::nop(location3, 6);
		hook::call_rcx(location3, stub4.GetCode());
	}
	
	// CPed::SetupExternallyDrivenDOFs
    {
        constexpr const int kStackFrameSize = 8 + 0x420 + 64;
        
        auto location = hook::get_pattern<char>("48 8B C4 48 89 48 ? 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 33 F6");
        
        g_origSetupExternallyDrivenDOFs = hook::trampoline(location, SetupExternallyDrivenDOFsWrap);

        static struct : jitasm::Frontend
        {
            virtual void InternalMain() override
            {
                lea(r8, qword_ptr[rbp + kStackFrameSize]); // Original instruction AQUI CARGA EL ARRAY
                mov(rdx, rdi); // Original instruction
                mov(qword_ptr[rbp - 0x40], rax); // Original instruction
                mov(dword_ptr[rbp + kStackFrameSize + kCountArrayOffset], esi); // Original instruction
                ret();
            }
        } stub1;
        hook::nop(location + 150, 17);
        hook::call_reg<2>(location + 150, stub1.GetCode()); // rdx

        static struct : jitasm::Frontend
        {
            virtual void InternalMain() override
            {
                cmp(dword_ptr[rbp + kStackFrameSize + kCountArrayOffset], esi); // Original instruction
                ret();
            }
        } stub2;
        hook::nop(location + 184, 6);
        hook::call_reg<2>(location + 184, stub2.GetCode()); // rdx

        static struct : jitasm::Frontend
        {
            virtual void InternalMain() override
            {
                movsxd(rbx, dword_ptr[rbp + rcx * 8 + kStackFrameSize]); // Original instruction
                ret();
            }
        } stub3;
        hook::nop(location + 196, 5);
        hook::call_reg<3>(location + 196, stub3.GetCode()); // rbx

        auto location2 = hook::get_pattern("3B 85 ? ? ? ? 0F 82 ? ? ? ? 0F B7 5C 24");
        static struct : jitasm::Frontend
        {
            virtual void InternalMain() override
            {
                cmp(eax, dword_ptr[rbp + kStackFrameSize + kCountArrayOffset]); // Original instruction
                ret();
            }
        } stub4;
        hook::nop(location2, 6);
        hook::call_reg<2>(location2, stub4.GetCode()); // rdx

        auto location4 = hook::get_pattern<char>("8B FE 89 B5 ? ? ? ? 39 B5", 8);
        static struct : jitasm::Frontend
        {
            virtual void InternalMain() override
            {
                cmp(dword_ptr[rbp + kStackFrameSize + kCountArrayOffset], esi); // Original instruction
                ret();
            }
        } stub5;
        hook::nop(location4, 6);
        hook::call_rcx(location4, stub5.GetCode());

        static struct : jitasm::Frontend
        {
            virtual void InternalMain() override
            {
                lea(rsi, qword_ptr[rbp + kStackFrameSize]); // Original instruction
                mov(qword_ptr[rbp+0x300], rsi); // Original instruction
                ret();
            }
        } stub6;
        hook::nop(location4 + 12, 11);
        hook::call_reg<2>(location4 + 12, stub6.GetCode());

        auto location5 = hook::get_pattern("3B BD ? ? ? ? 0F 82 ? ? ? ? 33 F6");
        static struct : jitasm::Frontend
        {
            virtual void InternalMain() override
            {
                cmp(edi, dword_ptr[rbp + kStackFrameSize + kCountArrayOffset]); // Original instruction
                ret();
            }
        } stub7;
        hook::nop(location5, 6);
        hook::call_rcx(location5, stub7.GetCode());
    }

	// CPedPropsMgr::UpdatePropExpressions
	// This is one of the functions that call GetCreatureMetaDataIndices, so we have patched the m_count and increase the reserve of the stack(sub rsp, add rsp)
	{
		auto location = hook::get_pattern<char>("48 85 C9 74 ? 48 8B C4");

		g_extraMetadataManager = hook::get_address<void*>(location + 31);

		hook::trampoline(location, UpdatePropExpressions);
	}
	
	// CModelInfoStreamingModule::GetDependencies
	// This function has been trampolined to add a new arg(5th arg) to the function with a new stack frame that contains the fixed array of MAX_STREAMING_DEPENDENCIES elements.
	// This is one of the functions that call GetCreatureMetaDataIndices, so we have patched the m_count here
	{
		constexpr const int kStackFrameSize = 8 + 0x5D0;

		auto location = hook::get_pattern<char>("48 8B C4 48 89 58 ? 44 89 48 ? 89 50 ? 48 89 48");

		g_origModelInfoStreamingModuleGetDependencies = hook::trampoline(location, ModelInfoStreamingModuleGetDependenciesWrap);

		auto location2 = hook::get_pattern<char>("4C 8D 85 ? ? ? ? 49 8B D7 44 89 B5");

		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				//lea(r8, qword_ptr[rbp + 0xA0]); // Original instruction
				lea(r8, qword_ptr[rsp + kStackFrameSize]); // Option 1
				ret();
			}
		} stub1;
		hook::nop(location2, 7);
		hook::call(location2, stub1.GetCode());

		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				//mov(r8d, dword_ptr[rbp+0x320]); // Original instruction
				mov(r8d, dword_ptr[rsp + kStackFrameSize + kCountArrayOffset]); // del array
				ret();
			}
		} stub4;
		hook::nop(location2 + 22, 7);
		hook::call(location2 + 22, stub4.GetCode());
		
		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				//lea(rdx, qword_ptr[rbp + 0xA0]); // Original instruction
				lea(rdx, qword_ptr[rsp + (kStackFrameSize)]); // Option 1
				ret();
			}
		} stub2;
		
		hook::nop(location2 + 37, 7);
		hook::call(location2 + 37, stub2.GetCode());
		
		auto location3 = hook::get_pattern("44 8B 85 ? ? ? ? 89 74 24");
		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				//mov(r8d, dword_ptr[rbp+0x320]); // Original instruction
				mov(r8d, dword_ptr[rsp + kStackFrameSize + kCountArrayOffset]);
				ret();
			}
		} stub3;
		hook::nop(location3, 7);
		hook::call(location3, stub3.GetCode());
		
		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				//mov(dword_ptr[rbp + 0x320], r14d); // Original instruction
				mov(dword_ptr[rsp + kStackFrameSize + kCountArrayOffset], r14d);
				ret();
			}
		} stub5;
		hook::nop(location2 + 10, 7);
		hook::call(location2 + 10, stub5.GetCode());
	}
	
	// rage::strStreamingInfoManager::GetNextFilesOnCdNew
	{
		constexpr const int kStackFrameSize = 8 + 0x228 + 64;

		auto location = hook::get_pattern<char>("48 8B C4 4C 89 48 ? 44 88 40 ? 89 50 ? 53");

		g_origGetNextFilesOnCdNew = hook::trampoline(location, GetNextFilesOnCdNewWrap);

		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				lea(rdi, qword_ptr[rsp + kStackFrameSize]);
				ret();
			}
		} stub1;
		
		hook::nop(location + 68, 7);
		hook::call_rcx(location + 68, stub1.GetCode());

		hook::put<uint32_t>(location + 224, kStackFrameSize);
		hook::put<uint32_t>(location + 236, MAX_STREAMING_DEPENDENCIES);

		hook::put<uint32_t>(location + 284, kStackFrameSize);

		auto location2 = hook::get_pattern<char>("4C 8D 84 24 ? ? ? ? 41 B9 ? ? ? ? 48 8B 08");

		hook::put<uint32_t>(location2 + 4, kStackFrameSize);
		hook::put<uint32_t>(location2 + 10, MAX_STREAMING_DEPENDENCIES);

		hook::put<uint32_t>(location2 + 53, kStackFrameSize);

		auto location3 = hook::get_pattern<char>("8B 94 9C");

		hook::put<uint32_t>(location3 - 165, kStackFrameSize);
		hook::put<uint32_t>(location3 - 159, MAX_STREAMING_DEPENDENCIES);

		hook::put<uint32_t>(location3 + 3, kStackFrameSize);

		hook::put<uint32_t>(location3 + 75, kStackFrameSize);
	}

	// rage::strStreamingInfoManager::CreateDependentsGraph
	{
		constexpr const int kStackFrameSize = 8 /* stack frame return */ + 0x1F0 /* stack start */ + 0x40 /* redzone and some other stuff */;

		auto location = hook::get_pattern("48 89 4C 24 ? 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 45 33 C0");

		g_origCreateDependentsGraph = hook::trampoline(location, CreateDependentsGraphWrap);

		auto location1 = hook::get_pattern<char>("48 8D 7C 24 ? 41 8B CC 8B D3");
		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				lea(rdi, qword_ptr[rsp + kStackFrameSize]);
				ret();
			}
		} stub1;
		hook::call(location1, stub1.GetCode());

		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				lea(r8, qword_ptr[rsp + kStackFrameSize]);
				mov(r12d, MAX_STREAMING_DEPENDENCIES);
				ret();
			}
		} stub2;
		hook::call_rcx(location1 + 30, stub2.GetCode());

		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				mov(eax, dword_ptr[rsp + (r14*4) + kStackFrameSize]);
				ret();
			}
		} stub3;
		hook::call_rcx(location1 + 116, stub3.GetCode());
	}

	// rage::strStreamingInfoManager::RequestObject
	{
		constexpr const int kStackFrameSize = 8 + 0x230 + 64;
		
		auto location = hook::get_pattern<char>("41 8B F8 48 8B F1 75 07 32 C0", -0x1C);
		
		g_origRequestObject = hook::trampoline(location, RequestObjectWrap);

		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				lea(r8, qword_ptr[rsp + kStackFrameSize]);
				ret();
			}
		} stub1;

		hook::call_rcx(location + 0x1A3, stub1.GetCode());
		hook::put<uint32_t>(location + 0x1AD, MAX_STREAMING_DEPENDENCIES);

		static struct : jitasm::Frontend
		{
			virtual void InternalMain() override
			{
				mov(eax, dword_ptr[rsp + (rcx * 4) + (kStackFrameSize)]);

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
