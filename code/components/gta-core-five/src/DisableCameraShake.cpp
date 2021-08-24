/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <GameInit.h>
#include <Hooking.h>
#include <CoreConsole.h>
#include <nutsnbolts.h>

static std::shared_ptr<ConVar<bool>> g_cameraShakeConvar;

static void CameraShakeOverride()
{
	if (g_cameraShakeConvar->GetValue())
	{
		//Vehicle high speed camera shake
		//48 8B D9                  mov     rbx, rcx
		//48 81 C7 88 04 00 00      add     rdi, 488h     <--------------
		//8B 6F 08                  mov     ebp, [rdi+8]

		hook::nop(hook::get_pattern("48 8B D9 48 81 C7 ? ? ? ? 8B ? ? 85 ?", 3), 7);

		//Ped running camera shake
		//0F 29 70 E8				movaps xmmword ptr[rax - 18h], xmm6
		//0F 29 78 D8				movaps  xmmword ptr [rax-28h], xmm7
		//48 81 C7 08 08 00 00		add rdi, 808h		 <--------------
		//48 8B F1					mov rsi, rcx

		hook::nop(hook::get_pattern("57 48 81 EC ? ? ? ? 48 8B B9 ? ? ? ? 0F 29 70 E8 0F 29 78 D8 48 81 C7 ? ? ? ?", 23), 7);
	}
}

static InitFunction initFunction([]()
{
	g_cameraShakeConvar = std::make_shared<ConVar<bool>>("cam_disableCameraShake", ConVar_Archive, false);

	OnFirstLoadCompleted.Connect([]()
	{
		CameraShakeOverride();
	});
});
