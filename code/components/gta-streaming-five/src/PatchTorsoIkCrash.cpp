#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>
#include <CrossBuildRuntime.h>

static hook::cdecl_stub<int(void*, uint16_t)> getBoneIndexFromTag([]()
{
    return hook::get_call(hook::get_pattern("E8 ? ? ? ? 66 89 46 02"));
});

static constexpr uint16_t kRequiredBoneTags[5] {
    0xE0FD, // SKEL_Spine_Root
    0x5C01, // SKEL_Spine0
    0x60F0, // SKEL_Spine1
    0x60F1, // SKEL_Spine2
    0x60F2, // SKEL_Spine3
};

static bool IsPedSkeletonApplicable(char* ped, char* skeleton)
{
    if (skeleton == nullptr)
    {
        return false;
    }

    for (auto boneTag : kRequiredBoneTags)
    {
        if (getBoneIndexFromTag(ped, boneTag) == -1)
        {
            return false;
        }
    }

    return true;
}

static HookFunction hookFunction([]()
{
    static struct : jitasm::Frontend
    {
        intptr_t retSuccess;
        intptr_t retFail;

        void Init(intptr_t success, intptr_t fail)
        {
            this->retSuccess = success;
            this->retFail = fail;
        }

        virtual void InternalMain() override
        {
			if (xbr::IsGameBuildOrGreater<3258>())
			{
				mov(rcx, qword_ptr[rbx + 0x30]); // CPed*
			}
			else
			{
				mov(rcx, qword_ptr[rdi + 0x30]); // CPed*
			}
            mov(rdx, r15); // rage::crSkeleton*

            mov(rax, (uintptr_t)IsPedSkeletonApplicable);
            call(rax);

            test(al, al);
            jz("fail");

            mov(rax, retSuccess);
            jmp(rax);

            L("fail");

            mov(rax, retFail);
            jmp(rax);
        }
    } patchStub;

    {
		void* start = nullptr;
		if (xbr::IsGameBuildOrGreater<3258>())
		{
			start = hook::get_pattern("4D 85 FF 0F 84 ? ? ? ? 48 8B 4B", 0);
		}
		else
		{
			start = hook::get_pattern("4D 85 FF 0F 84 83 09 00 00", 0);
		}

        auto finish = hook::get_pattern("8B 95 A0 01 00 00 41 B0 01 49 8B CF", 17);

        patchStub.Init(reinterpret_cast<intptr_t>(start) + 9, reinterpret_cast<intptr_t>(finish));

        hook::nop(start, 9);
        hook::jump(start, patchStub.GetCode());
    }
});
