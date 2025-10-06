#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <MinHook.h>
#include <NetLibrary.h>

#include <GameInit.h>
#include <DirectXMath.h>

#include <CoreNetworking.h>
#include <CrossBuildRuntime.h>
#include <Error.h>
#include <CloneManager.h>
#include <xmmintrin.h>
#include <netPlayerManager.h>
#include <netObjectMgr.h>
#include <netObject.h>
#include <CoreConsole.h>

/*
* Patch out inlined instances of playerLists that are intentionally capped at 32 players
* This patch also provides backwards-compatability for non-onesync ensuring behaviour remains the same
*/

extern ICoreGameInit* icgi;

extern CNetGamePlayer* g_players[256];
extern CNetGamePlayer* g_playerListRemote[256];
extern int g_playerListCountRemote;

static CNetGamePlayer** GetPlayers()
{
	if (icgi->OneSyncEnabled)
	{
		return g_players;
	}

	//@TODO: non-onesync support
	return nullptr;
}

static CNetGamePlayer** GetPlayersRemote()
{
	if (icgi->OneSyncEnabled)
	{
		return g_playerListRemote;
	}

	//@TODO: non-onesync support
	return nullptr;
}

static CNetGamePlayer* GetPlayerByIndex(uint8_t index)
{
	if (!icgi->OneSyncEnabled)
	{
		//@TODO: non-onesync support
		return nullptr;
	}

	if (index < 0 || index >= 256)
	{
		return nullptr;
	}

	return g_players[index];
}

static int GetPlayerCount()
{
	if (icgi->OneSyncEnabled)
	{
		return g_playerListCountRemote;
	}

	//@TODO: non-onesync support
	return 0;
}

static void* (*g_getRelevancePlayers)(void*, void*, void*, bool);
static void* _getRelevancePlayers(void* a1, void* a2, void* a3, bool a4)
{
	trace("Return address %p\n", (void*)hook::get_unadjusted(_ReturnAddress()));
	return g_getRelevancePlayers(a1, a2, a3, a4);
}


static HookFunction hookFunction([]
{
	// Fix netObject::_isObjectSyncedWithPlayers
	{
		auto location = hook::get_call(hook::get_pattern("E8 ? ? ? ? 3A C3 74 ? B3 ? 8A C3 48 83 C4 ? 5B C3 40 53 48 83 EC ? 48 8B 01"));

		/*
		.text:0000000142C0C1F5 48 8B 3D 54 26 F0 02                                         mov     rdi, cs:rage__netInterface__m_PlayerMgr
		.text:0000000142C0C1FC 84 C0                                                        test    al, al
		.text:0000000142C0C1FE 74 08                                                        jz      short loc_142C0C208
		.text:0000000142C0C200 8B AF 9C 02 00 00                                            mov     ebp, [rdi+29Ch]
		.text:0000000142C0C206 EB 02                                                        jmp     short loc_142C0C20A
		*/
		static struct : jitasm::Frontend
		{
			intptr_t retnFail = 0;
			intptr_t retnSuccess = 0;

			void Init(const intptr_t success, const intptr_t fail)
			{
				this->retnSuccess = success;
				this->retnFail = fail;
			}

			virtual void InternalMain() override
			{
				push(rax);
				mov(r11, reinterpret_cast<uintptr_t>(&GetPlayersRemote));
				call(r11);
				mov(rdi, rax);
				pop(rax);

				test(al, al);
				jz("Fail");

				push(rax);
				mov(r11, reinterpret_cast<uintptr_t>(&GetPlayerCount));
				call(r11);
				mov(ebp, eax);
				pop(rax);

				mov(r11, retnSuccess);
				jmp(r11);

				L("Fail");
				mov(r11, retnFail);
				jmp(r11);
			};
		} patchStub;

		const uintptr_t patch = (uintptr_t)location + 45;

		const uintptr_t retnFail = patch + 19;
		const uintptr_t retnSuccess = retnFail + 2;

		patchStub.Init(retnSuccess, retnFail);

		hook::nop(patch, 19);
		hook::nop(retnSuccess + 4, 7);
		hook::jump_reg<5>(patch, patchStub.GetCode());
	}

	// Fix StartSynchronising by using our own playerList
	{
		auto location = hook::get_pattern("8A 15 ? ? ? ? 44 8A F8", 9);
		/*
		.text:0000000142C1D48D 48 8B 1D BC 13 EF 02                                            mov     rbx, cs:rage__netInterface__m_PlayerMgr
		.text:0000000142C1D494 84 D2                                                           test    dl, dl
		.text:0000000142C1D496 74 08                                                           jz      short loc_142C1D4A0
		.text:0000000142C1D498 8B 8B 9C 02 00 00                                               mov     ecx, [rbx+29Ch]
		.text:0000000142C1D49E EB 02                                                           jmp     short loc_142C1D4A2
		.text:0000000142C1D4A0 33 C9                                                           xor     ecx, ecx
		.text:0000000142C1D4A2 84 D2                                                           test    dl, dl
		.text:0000000142C1D4A4 74 09                                                           jz      short loc_142C1D4AF
		.text:0000000142C1D4A6 48 81 C3 98 07 00 00                                            add     rbx, 798h
		*/
		static struct : jitasm::Frontend
		{
			intptr_t retnFail = 0;
			intptr_t retnSuccess = 0;

			void Init(const intptr_t success, const intptr_t fail)
			{
				this->retnSuccess = success;
				this->retnFail = fail;
			}

			virtual void InternalMain() override
			{
				push(rax);
				mov(r11, reinterpret_cast<uintptr_t>(&GetPlayersRemote));
				call(r11);
				mov(rbx, rax);
				pop(rax);

				test(dl, dl);
				jz("Fail");

				push(rax);
				mov(r11, reinterpret_cast<uintptr_t>(&GetPlayerCount));
				call(r11);
				mov(ecx, eax);
				pop(rax);

				mov(r11, retnSuccess);
				jmp(r11);

				L("Fail");
				mov(r11, retnFail);
				jmp(r11);
			};
		} patchStub;

		const uintptr_t retnFail = (uintptr_t)location + 19;
		const uintptr_t retnSuccess = retnFail + 2;

		patchStub.Init(retnSuccess, retnFail);

		hook::nop(location, 19);
		hook::jump_reg<5>(location, patchStub.GetCode());

		// Remove +add
		hook::nop((uintptr_t)location + 0x19, 7);
	}

	// Vehicle scene related
	{
		auto location = hook::get_pattern("40 0F B6 C6 48 8B B4 C5");

		static struct : jitasm::Frontend
		{
			uintptr_t retnAddr;

			void Init(uintptr_t retn)
			{
				this->retnAddr = retn;
			}

			virtual void InternalMain() override
			{
				// Original code
				movzx(eax, si);

				// rsi should be the pointer to the player
				mov(r14, rax);
				push(rcx);
				push(rax);

				sub(rsp, 0x20);
				mov(rcx, r14);
				mov(r11, reinterpret_cast<uintptr_t>(&GetPlayerByIndex));
				call(r11);
				add(rsp, 0x20);
				mov(rsi, rax);

				pop(rcx);
				pop(rax);

				mov(r11, retnAddr);
				jmp(r11);
			}
		} patchStub;

		hook::nop(location, 12);
		patchStub.Init((uintptr_t)location + 12);
		hook::jump_reg<5>(location, patchStub.GetCode());
	}

	// Patch netObject::DependencyThreadUpdate
	{
		auto location = hook::get_pattern("0F B6 C3 48 8B B4 C6");

		static struct : jitasm::Frontend
		{
			uintptr_t retnFail;
			uintptr_t retnSuccess;

			void Init(uintptr_t success, uintptr_t fail)
			{
				retnFail = fail;
				retnSuccess = success;
			}

			virtual void InternalMain() override
			{
				// Original code
				movzx(eax, bl);
				push(rcx);

				mov(rcx, rax);
				mov(r11, reinterpret_cast<uintptr_t>(&GetPlayerByIndex));
				call(r11);

				mov(rsi, rax);

				pop(rcx);

				test(rsi, rsi);
				jz("fail");

				mov(r11, retnSuccess);
				jmp(r11);

				L("fail");
				mov(r11, retnFail);
				jmp(r11);
			}
		} patchStub2;

		const uintptr_t retnSuccess = (uintptr_t)location + 16;
		const uintptr_t retnFail = retnSuccess + 50;

		hook::nop(location, 16);
		patchStub2.Init(retnSuccess, retnFail);
		hook::jump_reg<5>(location, patchStub2.GetCode());
	}

	g_getRelevancePlayers = hook::trampoline(hook::get_pattern("48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 0F 29 70 ? 41 8A D9"), _getRelevancePlayers);
});
