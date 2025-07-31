// hook for the initial mountpoint of filesystem stuff
#include "StdInc.h"
#include <fiDevice.h>
#include <Hooking.h>

#include <Error.h>
#include <MinHook.h>

#include "Hooking.Stubs.h"
#include "PureModeState.h"

static void(*originalMount)();

static void CallInitialMount()
{
	// do pre-initial mount
	originalMount();

	rage::fiDevice::OnInitialMount();
}

static bool (*g_validationRoutine)(const char* path, const uint8_t* data, size_t size);

DLL_EXPORT void SetPackfileValidationRoutine(bool (*routine)(const char*, const uint8_t*, size_t))
{
	if (!g_validationRoutine)
	{
		g_validationRoutine = routine;
	}
}

#if 0
static std::string currentPack;

static bool OpenArchiveWrap(rage::fiPackfile* packfile, const char* archive, bool a3, int a4, intptr_t a5)
{
	currentPack = archive;

	bool retval = packfile->OpenPackfile(archive, a3, a4, a5);

	currentPack = "";

	if (!retval)
	{
		FatalError("Could not open %s. Please try to verify your GTA V files, see http://rsg.ms/verify for more information.\n\nCurrently, the installation directory %s is being used.", archive, ToNarrow(MakeRelativeGamePath(L"")));
	}

	return retval;
}

static void PackfileEncryptionError()
{
	FatalError("Invalid rage::fiPackfile encryption type%s.\n\nIf you have any modified game files, please remove or verify them. See http://rsg.ms/verify for more information on verifying your game files.\n"
		"If using OpenIV, please make sure you have used the 'mods' folder for placing your modified files.\n\n"
		"Currently, the installation directory %s is being used.",
		(!currentPack.empty()) ? fmt::sprintf(" in packfile %s", currentPack) : " specified",
		ToNarrow(MakeRelativeGamePath(L"")));
}
#endif

static void CheckPackFile(const char* headerData, const char* fileName)
{
	auto entries = headerData + 0x110;
	size_t numEntries = *reinterpret_cast<const uint32_t*>(headerData + 4);

	if(g_validationRoutine)
	{
		if(!g_validationRoutine(fileName, (const uint8_t*)entries, numEntries * 24))
		{
			FatalError("Invalid modified game files (%s)\nThe server you are trying to join has enabled 'pure mode', but you have modified game files. Please verify your RDR installation (see http://rsg.ms/verify) and try again. Alternately, ask the server owner for help.", fileName);
		}
	}
}

static HookFunction hookFunction([] ()
{
	/*static hook::inject_call<void, int> injectCall(0x7B2E27);

	injectCall.inject([] (int)
	{
		injectCall.call();

		rage::fiDevice::OnInitialMount();
	});*/

	// increase non-DLC fiDevice mount limit
	{
		auto location = hook::get_pattern<int>("8B ? ? ? ? ? ? ? ? 00 00 C7 05 ? ? ? ? 64 00 00 00", 17);
		hook::put<int>(location, *location * 15); // '1500' mount limit now, instead of '500'
	}

	hook::set_call(&originalMount, hook::pattern("48 03 C3 44 88 34 38 66 01 1D").count(1).get(0).get<void>(0xE));
	hook::call(hook::pattern("48 03 C3 44 88 34 38 66 01 1D").count(1).get(0).get<void>(0xE), CallInitialMount);

	// don't sort update:/ relative devices before ours
	hook::nop(hook::pattern("C6 80 00 01 00 00 01 E8").count(1).get(0).get<void>(12), 5);

	// disable `settings:/` mount
	hook::nop(hook::get_pattern("4C 8B C3 48 8D 15 ? ? ? ? 48 8D 8D B0 00 00 00 E8 ? ? ? ? 45", 154), 5);

	// fail sanely on missing game packfiles
#if 0
	{
		auto matches = hook::pattern("E8 ? ? ? ? 84 C0 75 0A E8 ? ? ? ? 84 C0").count_hint(7);

		for (int i = 0; i < matches.size(); i++)
		{
			hook::call(matches.get(i).get<void>(0), OpenArchiveWrap);
		}
	}

	// wrap err_gen_invalid failures
	hook::call(hook::get_pattern("B9 EA 0A 0E BE E8", 5), PackfileEncryptionError);
#endif

	if (fx::client::GetPureLevel() == 0)
	{
		return;
	}
	
	{
        auto location = hook::get_pattern<char>("49 8D B1 ? ? ? ? 48 8B CD"); // fiPackfile::ReInit
            
        static struct : jitasm::Frontend
        {
            uintptr_t returnAddress;
            
            virtual void InternalMain() override
            {
                lea(rsi, qword_ptr[r9+0x110]);
                push(rbx);
                push(rsi);
                push(rdi);
                push(r12);
                push(r13);
                push(r14);
                push(r15);

                push(rax);
                push(rcx);
                push(rdx);
                push(r8);
                push(r9);
                push(r10);
                push(r11);

                sub(rsp, 32);

                mov(rcx, r9);      // headerData
                mov(rdx, r15);     // fileName
                mov(rax, reinterpret_cast<uintptr_t>(&CheckPackFile));
                call(rax);

                add(rsp, 32);

                pop(r11);
                pop(r10);
                pop(r9);
                pop(r8);
                pop(rdx);
                pop(rcx);
                pop(rax);

                pop(r15);
                pop(r14);
                pop(r13);
                pop(r12);
                pop(rdi);
                pop(rsi);
                pop(rbx);

                mov(rax, returnAddress);
                jmp(rax);
            }
        } stub;
        stub.returnAddress = (uintptr_t)location + 7;
        hook::nop(location, 7);
        hook::jump(location, stub.GetCode());
    }
});
