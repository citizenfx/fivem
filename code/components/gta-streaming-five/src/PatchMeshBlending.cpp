#include <StdInc.h>
#include <Hooking.h>
#include <MinHook.h>
#include <jitasm.h>
#include <CoreConsole.h>

#define RAGE_FORMATS_GAME_FIVE
#define RAGE_FORMATS_IN_GAME
#include <gtaDrawable.h>
#include <grcTexture.h>
#include <Streaming.h>

uint32_t g_textureDimensionOverride = -1;
constexpr uint32_t g_textureDimensionDefault = 512;
constexpr uint32_t g_meshBlendingSlots = 80; // All known game version have 80 scratch textures

struct MeshBlendReplacements
{
	int32_t originalId;
	int32_t replacementId;
};
std::array<MeshBlendReplacements, g_meshBlendingSlots * 3> g_meshBlendReplacements;

uint64_t g_customMeshRenderTarget;
uint64_t g_customMeshRenderTargetTemp;
std::unordered_map<uint32_t, uint64_t> g_customMipRenderTargets;
std::unordered_map<uint32_t, uint64_t> g_customMipRenderTargetTexturesDX10;

static hook::cdecl_stub<rage::five::pgDictionary<rage::grcTexture>*(void*, int)> textureDictionaryCtor([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8B F8 EB 02 33 FF 4C 8D 3D"));
});

static hook::cdecl_stub<const char*(const char*)> gta_strdup([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8D 4E F8"));
});

int32_t CreateSingleTexTxd(const char* name, int32_t width, int32_t height)
{
	if (!name)
		return -1;

	streaming::Manager* streaming = streaming::Manager::GetInstance();
	auto txdStore = streaming->moduleMgr.GetStreamingModule("ytd");

	uint32_t txdSlotID = 0;
	txdStore->FindSlotFromHashKey(&txdSlotID, name);
	if (txdSlotID == -1)
		return -1;

	auto& entry = streaming->Entries[txdStore->baseIdx + txdSlotID];
	if (entry.handle)
		return -1;

	void* memoryStub = rage::GetAllocator()->Allocate(sizeof(rage::five::pgDictionary<rage::grcTexture>), 16, 0);
	auto txd = textureDictionaryCtor(memoryStub, 1);

	streaming::strAssetReference ref;
	ref.asset = txd;

	txdStore->SetResource(txdSlotID, ref);
	entry.flags = (512 << 8) | 1;

	// This is missing in RuntimeAssetNatives but prevents crashes when swapping drawables during gameplay
	//
	txdStore->AddRef(txdSlotID);

	rage::grcManualTextureDef textureDef;
	memset(&textureDef, 0, sizeof(textureDef));
	textureDef.isStaging = 1; //0x0
	textureDef.usage = 1; //0x8
	textureDef.arraySize = 1; //0x24 / 36

	// grcFormat-32 -> dxgiFormat-71 = DXGI_FORMAT_BC1_UNORM
	//
	auto tex = rage::grcTextureFactory::getInstance()->createManualTexture(width, height, 32, nullptr, true, &textureDef);

	// Probably a good idea to have this for debugging fatal crashes in the future
	//
	*(const char**)((uint64_t)tex + 0x28) = gta_strdup(name);

	txd->Add(name, tex);

	return txdSlotID;
}

static int g_blendSubstructIndexOffset;

uint64_t(__fastcall* g_orig_MeshblendingMainStruct__FillMeshBlendingSubStruct3)(void* a1, int a2, int dwdstoreid_head, int txdstoreid_head_diff_000_a_whi, int txdstoreid_feet_diff_000_a_whi, int txdstoreid_uppr_diff_000_a_whi, int txdstoreid_lowr_diff_000_a_whi);
uint64_t __fastcall MeshblendingMainStruct__FillMeshBlendingSubStruct3(char* a1, int a2, int dwdstoreid_head, int txdstoreid_head_diff_000_a_whi, int txdstoreid_feet_diff_000_a_whi, int txdstoreid_uppr_diff_000_a_whi, int txdstoreid_lowr_diff_000_a_whi)
{
	// this only applies if `a2` is `0`, but it always appears to be
	// the count also won't increment beyond the '65' counter even if resetting, so we might set the last one
	// a bit too often
	int slotID = *(int*)(a1 + g_blendSubstructIndexOffset) * 3;

	g_meshBlendReplacements[slotID++] = { txdstoreid_feet_diff_000_a_whi, -1 };
	g_meshBlendReplacements[slotID++] = { txdstoreid_uppr_diff_000_a_whi, -1 };
	g_meshBlendReplacements[slotID++] = { txdstoreid_lowr_diff_000_a_whi, -1 };

	return g_orig_MeshblendingMainStruct__FillMeshBlendingSubStruct3(a1, a2, dwdstoreid_head, txdstoreid_head_diff_000_a_whi, txdstoreid_feet_diff_000_a_whi, txdstoreid_uppr_diff_000_a_whi, txdstoreid_lowr_diff_000_a_whi);
}

uint64_t(__fastcall* g_orig_createRenderTargetDX11)(uint64_t a1, const char* a2, unsigned int a3, unsigned int a4, unsigned int a5, int a6, uint64_t a7, __int64 a8);
uint64_t __fastcall createRenderTargetDX11(uint64_t a1, const char* a2, unsigned int a3, unsigned int a4, unsigned int a5, int a6, uint64_t a7, __int64 a8)
{
	if (a2)
	{
		if (!strcmp(a2, "MeshBlendManager"))
		{
			g_customMeshRenderTarget = g_orig_createRenderTargetDX11(a1, "FivemMeshBlendManager", a3, g_textureDimensionOverride, g_textureDimensionOverride, a6, a7, a8);
		}
		else if (!strcmp(a2, "MeshBlendManagerTemp"))
		{
			g_customMeshRenderTargetTemp = g_orig_createRenderTargetDX11(a1, "FivemMeshBlendManagerTemp", a3, g_textureDimensionOverride, g_textureDimensionOverride, a6, a7, a8);
		}
		else if (!strcmp(a2, "MeshBlendDxt1Target128x128"))
		{
			uint32_t width = g_textureDimensionOverride >> 2;
			uint32_t height = g_textureDimensionOverride >> 2;
			while (true)
			{
				char buffer[64];
				sprintf(buffer, "FivemMeshBlendDxt1Target%dx%d", width, height);
				uint64_t mipTarget = g_orig_createRenderTargetDX11(a1, buffer, a3, width, height, a6, a7, a8);
				if (mipTarget == 0)
				{
					trace("Failed to create a custom DX11 grcRenderTarget for %dx%d\n", width, height);
				}
				else
				{
					g_customMipRenderTargets.insert({ width, mipTarget });
				}

				width >>= 1;
				height >>= 1;
				if (width <= 1)
					break;
			}
		}
	}

	return g_orig_createRenderTargetDX11(a1, a2, a3, a4, a5, a6, a7, a8);
}

uint64_t(__fastcall* g_orig_createRenderTargetDX10)(uint64_t* a1, const char* a2, ID3D11Resource* a3, __int64 a4);
uint64_t __fastcall createRenderTargetDX10(uint64_t* a1, const char* a2, ID3D11Resource* a3, __int64 a4)
{
	if (a2 && !strcmp(a2, "MeshBlendDxt1Target128x128"))
	{
		uint32_t width = g_textureDimensionOverride >> 2;
		uint32_t height = g_textureDimensionOverride >> 2;
		while (true)
		{
			rage::grcManualTextureDef textureDef;
			memset(&textureDef, 0, sizeof(textureDef));
			textureDef.isStaging = 0;
			textureDef.usage = 3;
			*(uint64_t*)&textureDef.isRenderTarget = 1i64;
			*(uint32_t*)&textureDef.pad2[4] = 2;
			textureDef.arraySize = 1;

			auto tex = rage::grcTextureFactory::getInstance()->createManualTexture(width, height, 12, nullptr, true, &textureDef);

			char buffer[64];
			sprintf(buffer, "FivemMeshBlendDxt1Target%dx%d", width, height);
			uint64_t mipTarget = g_orig_createRenderTargetDX10(a1, buffer, tex->texture, a4);
			if (mipTarget == 0)
			{
				trace("Failed to create a custom DX10 grcRenderTarget for %dx%d\n", width, height);
			}
			else
			{
				g_customMipRenderTargets.insert({ width, mipTarget });
				g_customMipRenderTargetTexturesDX10.insert({ width, (uint64_t)tex });
			}

			width >>= 1;
			height >>= 1;
			if (width <= 1)
				break;
		}
	}

	return g_orig_createRenderTargetDX10(a1, a2, a3, a4);
}

uint64_t(__fastcall* g_orig_getMeshblendMipRenderTargetDX11)(uint64_t meshBlendManager, int32_t targetWidth, int32_t targetHeight);
uint64_t __fastcall getMeshblendMipRenderTargetDX11(uint64_t meshBlendManager, int32_t targetWidth, int32_t targetHeight)
{
	uint64_t mipRenderTarget = g_orig_getMeshblendMipRenderTargetDX11(meshBlendManager, targetWidth, targetHeight);
	if (mipRenderTarget == 0 && (targetWidth << 2 == g_textureDimensionOverride))
	{
		auto mipTarget = g_customMipRenderTargets.find(targetWidth);
		if (g_customMipRenderTargets.find(targetWidth) == g_customMipRenderTargets.end())
		{
			trace("Failed to retrieve a custom DX11 grcRenderTarget for %dx%d\n", targetWidth, targetHeight);

			// Returning null will lead to an improperly processed component, but will not cause a fatal crash
			//
			return 0;
		}
		return mipTarget->second;
	}
	return mipRenderTarget;
}

uint64_t(__fastcall* g_orig_getMeshblendMipRenderTargetTexturesDX10)(uint64_t meshBlendManager, int32_t targetWidth, int32_t targetHeight);
uint64_t __fastcall getMeshblendMipRenderTargetTexturesDX10(uint64_t meshBlendManager, int32_t targetWidth, int32_t targetHeight)
{
	uint64_t mipRenderTarget = g_orig_getMeshblendMipRenderTargetTexturesDX10(meshBlendManager, targetWidth, targetHeight);
	if (mipRenderTarget == 0 && (targetWidth << 2 == g_textureDimensionOverride))
	{
		auto mipTarget = g_customMipRenderTargetTexturesDX10.find(targetWidth);
		if (g_customMipRenderTargetTexturesDX10.find(targetWidth) == g_customMipRenderTargetTexturesDX10.end())
		{
			trace("Failed to retrieve a custom DX10 grcRenderTarget for %dx%d. The game will likely crash now\n", targetWidth, targetHeight);
			return 0;
		}
		return mipTarget->second;
	}
	return mipRenderTarget;
}

static HookFunction hookFunction([]
{
	#pragma warning(suppress : 26812)
	MH_Initialize();

	auto str_meshBlendTextureRes = new ConVar<int32_t>(
	"str_meshBlendTextureRes", ConVar_Archive, 2048, [](internal::ConsoleVariableEntry<int32_t> * arg) -> auto
	{
		if (arg->GetRawValue() == -1)
			trace("str_meshBlendTextureRes set to -1. MeshBlendManager improvements will be disabled after a client restart\n");
		else
			trace("str_meshBlendTextureRes set to %dx%d. This change will be effective after a client restart\n", arg->GetRawValue(), arg->GetRawValue());
	});
	str_meshBlendTextureRes->GetHelper()->SetConstraints(-1, 4096);

	// Users with weak machines might want to disable this behaviour entirely
	//
	g_textureDimensionOverride = str_meshBlendTextureRes->GetValue();
	if (g_textureDimensionOverride == -1)
	{
		trace("MeshBlendManager improvements are disabled by user preference. If you want to change this, change str_meshBlendTextureRes\n");
		return;
	}

	//DX11 and DX10.1
	{
		{ // DX11: Used to create our custom main renderTargets and mip renderTargets with the exact same parameters as the game does
			//
			auto location = hook::get_pattern("48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 56 48 83 EC 40 48 8B BC 24");
			MH_CreateHook(location, createRenderTargetDX11, (void**)&g_orig_createRenderTargetDX11);
			MH_EnableHook(location);
		}

		{
			// DX11: While processing a blend request the game code will try to use mip levels
			// We create our own set of mip renderTargets and supply them to the game logic on demand
			//
			auto location = hook::get_call(hook::get_pattern("E8 ? ? ? ? 83 64 24 ? ? 83 64 24 ? ? 48 8B D0 45 8B"));
			MH_CreateHook(location, getMeshblendMipRenderTargetDX11, (void**)&g_orig_getMeshblendMipRenderTargetDX11);
			MH_EnableHook(location);
		}
	}

	//DX10
	{
		{ 
			// DX10: the game code creates the meshblending mip renderTargets for DX10 mode manually by setting up grcTextures, using the backing D3D texture
			// as an argument in the constructor
			//
			auto location = hook::get_pattern("48 83 EC 48 48 83 64 24 ? ? 4C 89");
			MH_CreateHook(location, createRenderTargetDX10, (void**)&g_orig_createRenderTargetDX10);
			MH_EnableHook(location);
		}

		{
			// DX10: due to the special setup of the mip level renderTargets in DX10 mode we need to hook this and return the underlying grcTexture
			//
			auto location = hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8B D8 48 85 C0 74 76 45"));
			MH_CreateHook(location, getMeshblendMipRenderTargetTexturesDX10, (void**)&g_orig_getMeshblendMipRenderTargetTexturesDX10);
			MH_EnableHook(location);
		}
	}

	{
		// This is invoked from CPed::vft::unk_48 on drawable or drawable texture switch for all components that support cloth blending (FEET, UPPR, LOWR)
		//
		static struct : jitasm::Frontend
		{
			uint64_t mutexAddr;
			virtual void InternalMain() override
			{
				// Compensate for hook::call emitted return addr on stack
				//
				if (xbr::IsGameBuildOrGreater<2699>())
				{ //2699-2802?
					mov(eax, dword_ptr[rsp + 0x8 + 0x78 + 0x18]); //componentType
				}
				else
				{ //1604 - 2612
					mov(eax, dword_ptr[rsp + 0x8 + 0x28]); //slotIndex
				}

				push(rbp);
				push(rsi);
				push(rdi);
				push(rbx);
				push(r12);
				push(r13);
				push(r14);
				push(r15);

				if (xbr::IsGameBuildOrGreater<2699>())
				{ //2699-2802?
					//mov(eax, edx); //slotIndex
					mov(r8d, edx); //slotIndex
					mov(edx, eax); //componentType
				}
				else
				{ //1604-2612
					mov(r8d, eax); //slotIndex
				}
				mov(ecx, r9d); //clothingTxdId
				mov(r9, rbx); //meshBlendManager

				sub(rsp, 0x28);
				mov(rax, (uint64_t)stub);
				call(rax);
				add(rsp, 0x28);

				pop(r15);
				pop(r14);
				pop(r13);
				pop(r12);
				pop(rbx);
				pop(rdi);
				pop(rsi);
				pop(rbp);

				mov(rax, mutexAddr);
				lea(rcx, qword_ptr[rax]);

				ret();
			}

			static void stub(uint32_t clothingTxdId, uint64_t componentType, uint64_t slotIndex, uint64_t meshBlendManager)
			{
				if (clothingTxdId == -1 || componentType > 2 || slotIndex >= g_meshBlendingSlots || meshBlendManager == 0)
					return;

				int32_t meshBlendIndex = ((slotIndex * 3) + componentType);

				static uint32_t meshBlendStructOffset = xbr::IsGameBuildOrGreater<2699>() ? 0x14610 : 0x14410;
				int32_t* meshBlendScratchTextures = (int32_t*)(meshBlendManager + meshBlendStructOffset + 0x98 + (slotIndex * 0xB8));

				if (meshBlendIndex >= (sizeof(g_meshBlendReplacements) / sizeof(MeshBlendReplacements)))
					return;

				streaming::Manager* streaming = streaming::Manager::GetInstance();
				if (!streaming)
					return;

				auto txdStore = streaming->moduleMgr.GetStreamingModule("ytd");
				if (!txdStore)
					return;

				rage::five::pgDictionary<rage::grcTexture>* pgDict = (rage::five::pgDictionary<rage::grcTexture>*)txdStore->GetResource(clothingTxdId);
				if (!pgDict || pgDict->GetCount() != 1 || !pgDict->GetAt(0))
					return;

				if (pgDict->GetAt(0)->GetHeight() <= g_textureDimensionDefault && pgDict->GetAt(0)->GetWidth() <= g_textureDimensionDefault)
				{
					int32_t* replacementId = &g_meshBlendReplacements[meshBlendIndex].replacementId;
					if (*replacementId != -1)
					{
						// We don't need the larger scratch texture anymore, free it
						// If drawable is swapped refcount is 1, if the user just swaps texture refcount is > 1
						// B2372: I verified that RemoveRef is enough, the game code removes the remaining refs and texture is actually free'd
						//
						auto refs = txdStore->GetNumRefs(*replacementId);
						txdStore->RemoveRef(*replacementId);

						*replacementId = -1;
						meshBlendScratchTextures[componentType] = g_meshBlendReplacements[meshBlendIndex].originalId;
					}

					return;
				}

				// If this slot already has an upgraded texture and the new clothing item is a highres one again
				// we can re-use it
				//
				if (g_meshBlendReplacements[meshBlendIndex].replacementId != -1)
					return;

				// Create a new scratch texture with upgraded dimensions
				//
				char buffer[64];
				sprintf(buffer, "fivem_diff_%01d_%03d", (uint8_t)componentType, (uint8_t)slotIndex);
				int32_t highresScratchTxd = CreateSingleTexTxd(buffer, g_textureDimensionOverride, g_textureDimensionOverride);

				meshBlendScratchTextures[componentType] = highresScratchTxd;
				g_meshBlendReplacements[meshBlendIndex].replacementId = highresScratchTxd;
			}
		} pedDrawableDataUpdater;

		// under normal circumstances this is definitely a shitty pattern, but it was carefully selected and if it breaks the stub is definitely broken anyways
		auto location = hook::get_pattern("3A 44 1F 34 0F 84 ? ? ? ? 48 8D 0D ? ? ? ? E8", 10);
		pedDrawableDataUpdater.mutexAddr = hook::get_address<uint64_t>(location, 3, 7);
		hook::nop(location, 7);
		hook::call(location, pedDrawableDataUpdater.GetCode());
	}

	{
		// Before doing the skinblending the game code validates that the renderTarget has the correct dímension for the provided scratch texture
		// This obviously fails when we inject larger scratch textures, so we hook the fail case and change codeflow after swapping in a suitable renderTarget
		//
		static struct : jitasm::Frontend
		{
			uint64_t continueDest;

			virtual void InternalMain() override
			{
				// this seems excessive, but the function differs in register usage between game versions
				// We also have two possible cf destinations which expect different registers to be preserved
				push(rbx);
				push(rcx);
				push(rdx);
				push(rsi);
				push(rdi);
				push(rbp);
				push(r8);
				push(r9);
				push(r10);
				push(r11);
				push(r12);
				push(r13);
				push(r14);
				push(r15);

				sub(rsp, 0x28);

				mov(edx, edi); //width
				mov(r8, r14); //SkinStruct2
				mov(r9, r12); //scratchTex
				mov(rax, (uint64_t)stub);
				call(rax);

				add(rsp, 0x28);

				pop(r15);
				pop(r14);
				pop(r13);
				pop(r12);
				pop(r11);
				pop(r10);
				pop(r9);
				pop(r8);
				pop(rbp);
				pop(rdi);
				pop(rsi);
				pop(rdx);
				pop(rcx);
				pop(rbx);

				cmp(rax, 0);
				jz("fail");

				mov(rcx, rax); //customMeshRenderTargetTemp
				mov(rax, continueDest);
				mov(qword_ptr[rsp], rax);
				ret();

				L("fail");
				mov(edi, 1);
				ret();
			}

			static uint64_t stub(rage::grcRenderTarget* meshBlendTargetSmall, uint32_t width, uint64_t skinStruct2, rage::grcTexture* scratchTexture)
			{
				if (!meshBlendTargetSmall || !skinStruct2 || !scratchTexture)
				{
					trace("Invalid MeshBlend target, struct or scratchTexture. 0x%p, 0x%p, 0x%p\n", (void*)meshBlendTargetSmall, (void*)skinStruct2, (void*)scratchTexture);
					return 0;
				}

				if (width < meshBlendTargetSmall->GetWidth())
				{
					// If this case is hit we are dealing with a default game "error case"
					return 0;
				}

				if (width != g_textureDimensionOverride)
				{
					trace("Invalid MeshBlend custom texture size. %d\n", width);
					return 0;
				}

				static uint32_t skinStruct2mipLevelOffset = xbr::IsGameBuildOrGreater<2699>() ? 0x4FA : 0x4F6;

				*(uint64_t*)skinStruct2 = g_customMeshRenderTarget;
				*(uint8_t*)(skinStruct2 + skinStruct2mipLevelOffset) = scratchTexture->GetLevels();
				return (uint64_t)g_customMeshRenderTargetTemp;
			}
		} renderTargetSizeMismatchBranch;

		renderTargetSizeMismatchBranch.continueDest = (uint64_t)hook::get_pattern("49 89 4E 08 4C");

		auto location = hook::get_pattern("BF 01 ? ? ? 41 8A 86");
		hook::call(location, renderTargetSizeMismatchBranch.GetCode());
	}

	{
		//TODO: verify if newer builds use more than 65 slots (maybe finally the full 80?)
		// MeshBlendManager uses 65 "temporary" blending slots. These contain 4 scratch textures (head, upper, lower, feet) and two grc::renderTarget instances
		// The default scratch images are loaded from here: update\x64\dlcpacks\mppatchesng\dlc.rpf\x64\models\cdimages\mppatches.rpf\mp_headtargets and support up to 80 blend slots
		//
		auto location = hook::get_pattern<char>("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 81 EC ? ? ? ? 80 B9 ? ? ? ? ? 41");
		g_blendSubstructIndexOffset = *(int32_t*)(location + 0x3A);
		MH_CreateHook(location, MeshblendingMainStruct__FillMeshBlendingSubStruct3, (void**)&g_orig_MeshblendingMainStruct__FillMeshBlendingSubStruct3);
		MH_EnableHook(location);
	}
});
