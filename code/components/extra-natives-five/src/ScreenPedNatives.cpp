#include <StdInc.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <string>

#include <d3d11.h>
#include <wrl.h>

#include <DrawCommands.h>
#include <EntitySystem.h>
#include <GameInit.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <RageParser.h>
#include <Resource.h>
#include <ScriptEngine.h>
#include <fxScripting.h>
#include <grcTexture.h>
#include <nutsnbolts.h>
#include <scrEngine.h>

#include <CefOverlay.h>

namespace WRL = Microsoft::WRL;

namespace
{
constexpr int kMaxSlots = 5;

// joaat("PAUSE_SINGLE_MIDDLE")
constexpr uint32_t kTemplatePresetHash = 0xAD197067;

// arbitrary name hash for our own (unregistered) preset
constexpr uint32_t kScreenPedPresetHash = 0x43465853;

constexpr uint16_t kVisPhaseMaskOther = 1;
constexpr float kFrameBottom = -1.2f;
constexpr float kFrameTop = 1.0f;
constexpr float kFrameWidth = 0.8f;

// native hashes
constexpr uint64_t CLONE_PED = 0xEF29A16337FACADB;
constexpr uint64_t DOES_ENTITY_EXIST = 0x7239B21A38F536BA;
constexpr uint64_t DELETE_ENTITY = 0xAE3CBE5BF394C9C9;
constexpr uint64_t FREEZE_ENTITY_POSITION = 0x428CA6DBD1094446;
constexpr uint64_t SET_ENTITY_COLLISION = 0x1A9205C1B9EE827F;
constexpr uint64_t SET_ENTITY_INVINCIBLE = 0x3882114BDE571AD4;
constexpr uint64_t SET_BLOCKING_OF_NON_TEMPORARY_EVENTS = 0x9F8AA94D6D97DBF4;
constexpr uint64_t SET_PED_CAN_RAGDOLL = 0xB128377056A54E2A;
constexpr uint64_t STOP_PED_SPEAKING = 0x9D64D7405520E3D3;
constexpr uint64_t DISABLE_PED_PAIN_AUDIO = 0xA9A41C1E940FB0E8;

struct alignas(16) Vec4
{
	float x;
	float y;
	float z;
	float w;
};

struct UISceneObjectData
{
	void* entityRef; // +0x00
	char pad_08[8];
	Vec4 position; // +0x10
	Vec4 position43; // +0x20
	Vec4 rotationXYZ; // +0x30
	Vec4 bgRectXYWH; // +0x40
	Vec4 bgRectXYWH43; // +0x50
	uint32_t bgRectColor; // +0x60
	uint32_t blendColor; // +0x64
	bool enabled; // +0x68
	char pad_69[3];
	float perspectiveShear[2]; // +0x6C
	char postfxData[0xC]; // +0x74, UIPostFXFilterData
};

struct UI3DCamera
{
	Vec4 position; // +0x00, m_vCamPos
	Vec4 anglesDeg; // +0x10, m_vCamAnglesDeg
	float fieldOfView; // +0x20, m_CamFieldOfView, vertical, degrees
};

struct UIScenePreset
{
	uint32_t name; // +0x00, atHashValue
	char pad_04[0xC];
	UISceneObjectData elements[kMaxSlots]; // +0x10, atFixedArray<UISceneObjectData, 5>
	uint32_t elementCount; // +0x290
	char pad_294[0xC];
};

struct UISceneObjectPatchedData
{
	void* ped; // +0x00, fwRegdRef<CPed>
	char pad_08[8];
	Vec4 offset; // +0x10
	Vec4 rotationXYZ; // +0x20
	char pad_30[0x24];
	float alpha; // +0x54
	float globalLightIntensity; // +0x58
	char pad_5C[4];
};

struct ScreenPed
{
	bool used = false;
	int handle = 0;
	std::string owner;

	// user rectangle
	float x = 0.0f;
	float y = 0.0f;
	float w = 0.0f;
	float h = 0.0f;

	Vec4 offset = { 0.0f, 0.0f, 0.0f, 0.0f };
	Vec4 rotation = { 0.0f, 0.0f, 0.0f, 0.0f };
};


bool g_initialized;

char** g_ui3dInstance;
size_t g_currentPresetOffset;
size_t g_patchedDataOffset;
size_t g_depthRTOffset;
size_t g_camPosOffset;

uint32_t g_pedAiLodByteOffset;
uint32_t g_pedAiLodFlagsOffset;
uint8_t g_pedPhaseVisibilityOffset;

void (*g_origPreRender)(char* self);
void (*g_origAddToDrawList)(char* self);
void (*g_origCompositeToBackbuffer)(rage::grcRenderTarget* src, const float* rect, float alpha);

void (*g_changeVisibilityMask)(void* entity, uint16_t flags, bool value, bool processChildAttachments);

bool (*g_assignPedToSlot)(char* self, uint32_t slot, void* ped, const Vec4* offset, const Vec4* euler, float alpha);
UIScenePreset* (*g_getPreset)(char* self, const uint32_t* hash);
bool (*g_isPauseMenuActive)(int flags);
float (*g_getAspectRatio)(bool physical);

// game thread state
std::array<ScreenPed, kMaxSlots> g_screenPeds;

alignas(16) UIScenePreset g_preset;
UISceneObjectData g_templateElement;
bool g_haveTemplate;
bool g_claimed;
uint32_t g_assignedSlots;

// render thread state
struct CompositeCapture
{
	rage::grcRenderTarget* source;
	Vec4 rect;
	float alpha;
};

std::array<CompositeCapture, kMaxSlots> g_captures;
std::array<WRL::ComPtr<ID3D11Texture2D>, kMaxSlots> g_colorCopies;
std::array<WRL::ComPtr<ID3D11Texture2D>, kMaxSlots> g_depthCopies;
std::array<bool, kMaxSlots> g_captureHasDepth;
int g_captureCount;
bool g_capturing;
}

static char* GetManager()
{
	if (!g_initialized || !g_ui3dInstance)
	{
		return nullptr;
	}

	return *g_ui3dInstance;
}

static UIScenePreset*& CurrentPreset(char* manager)
{
	return *reinterpret_cast<UIScenePreset**>(manager + g_currentPresetOffset);
}

static UISceneObjectPatchedData* PatchedData(char* manager)
{
	return reinterpret_cast<UISceneObjectPatchedData*>(manager + g_patchedDataOffset);
}

static bool ValidateLayout()
{
	auto structure = rage::GetStructureDefinition("UISceneObjectData");

	if (!structure)
	{
		trace("Screen peds: UISceneObjectData parser structure not found, disabling.\n");
		return false;
	}

	struct
	{
		const char* name;
		size_t offset;
	} expected[] = {
		{ "position", offsetof(UISceneObjectData, position) },
		{ "position_43", offsetof(UISceneObjectData, position43) },
		{ "bgRectXYWH", offsetof(UISceneObjectData, bgRectXYWH) },
		{ "bgRectXYWH_43", offsetof(UISceneObjectData, bgRectXYWH43) },
		{ "bgRectColor", offsetof(UISceneObjectData, bgRectColor) },
		{ "enabled", offsetof(UISceneObjectData, enabled) },
		{ "perspectiveShear", offsetof(UISceneObjectData, perspectiveShear) },
	};

	for (const auto& entry : expected)
	{
		bool found = false;

		for (auto& member : structure->m_members)
		{
			if (member->m_definition->hash == HashRageString(entry.name))
			{
				found = (member->m_definition->offset == entry.offset);
				break;
			}
		}

		if (!found)
		{
			trace("Screen peds: unexpected UISceneObjectData layout (%s), disabling.\n", entry.name);
			return false;
		}
	}

	return true;
}

static bool EnsureTemplate(char* manager)
{
	if (g_haveTemplate)
	{
		return true;
	}

	static bool layoutChecked;
	static bool layoutValid;

	if (!layoutChecked)
	{
		layoutChecked = true;
		layoutValid = ValidateLayout();
	}

	if (!layoutValid)
	{
		return false;
	}

	uint32_t hash = kTemplatePresetHash;
	UIScenePreset* templatePreset = g_getPreset(manager, &hash);

	if (!templatePreset)
	{
		static bool warned;

		if (!warned)
		{
			warned = true;
			trace("Screen peds: template UI scene preset not found, screen peds can't be drawn yet.\n");
		}

		return false;
	}

	uint32_t count = templatePreset->elementCount;
	UISceneObjectData* element = &templatePreset->elements[0];

	for (uint32_t i = 0; i < count && i < kMaxSlots; i++)
	{
		UISceneObjectData* candidate = &templatePreset->elements[i];

		if (candidate->enabled)
		{
			element = candidate;
			break;
		}
	}

	memcpy(&g_templateElement, element, sizeof(g_templateElement));

	g_templateElement.entityRef = nullptr;
	memset(&g_preset, 0, sizeof(g_preset));

	g_preset.name = kScreenPedPresetHash;
	g_preset.elementCount = kMaxSlots;

	g_haveTemplate = true;
	return true;
}

static void BuildElement(int slot, const ScreenPed& entry, const UI3DCamera& camera, float aspect)
{
	UISceneObjectData& element = g_preset.elements[slot];
	memcpy(&element, &g_templateElement, sizeof(element));

	if (!(aspect > 0.0f))
	{
		aspect = 16.0f / 9.0f;
	}

	// the game squeezes rects into a 16:9 area (x' = x * k - (k - 1) / 2, w' = w * k) - invert that so the callers rect maps 1:1 to the screen
	float k = (16.0f / 9.0f) / aspect;

	Vec4 rect;
	rect.x = (entry.x + (k - 1.0f) * 0.5f) / k;
	rect.y = entry.y;
	rect.z = entry.w / k;
	rect.w = entry.h;

	float fov = camera.fieldOfView;

	if (!(fov > 1.0f && fov < 179.0f))
	{
		fov = 40.0f;
	}

	float tanHalfFov = tanf(fov * 0.5f * 0.017453292f);
	float shownHeight = std::min(entry.h, aspect * 0.5f);
	float shownWidth = std::min(entry.w * aspect, 0.5f);

	float distance = std::max((kFrameTop - kFrameBottom) / (4.0f * tanHalfFov * shownHeight), kFrameWidth / (4.0f * tanHalfFov * shownWidth));

	float dirX = g_templateElement.position.x - camera.position.x;
	float dirY = g_templateElement.position.y - camera.position.y;
	float dirLength = sqrtf(dirX * dirX + dirY * dirY);

	if (dirLength > 0.001f)
	{
		dirX /= dirLength;
		dirY /= dirLength;
	}
	else
	{
		dirX = 0.0f;
		dirY = -1.0f;
	}

	Vec4 position;
	position.x = camera.position.x + dirX * distance;
	position.y = camera.position.y + dirY * distance;
	position.z = camera.position.z - (kFrameTop + kFrameBottom) * 0.5f;
	position.w = g_templateElement.position.w;

	element.position = position;
	element.position43 = position;
	element.perspectiveShear[0] = 0.0f;
	element.perspectiveShear[1] = 0.0f;
	element.bgRectXYWH = rect;
	element.bgRectXYWH43 = rect;
	element.bgRectColor = 0;
	element.enabled = true;
}

static void HideFromWorld(char* ped)
{
	*reinterpret_cast<uint16_t*>(ped + g_pedPhaseVisibilityOffset) |= kVisPhaseMaskOther;

	g_changeVisibilityMask(ped, static_cast<uint16_t>(~kVisPhaseMaskOther), false, false);
}

static void ReleaseSlots(char* manager)
{
	static const Vec4 zero = { 0.0f, 0.0f, 0.0f, 0.0f };

	for (int slot = 0; slot < kMaxSlots; slot++)
	{
		if (g_assignedSlots & (1 << slot))
		{
			g_assignPedToSlot(manager, slot, nullptr, &zero, &zero, 0.0f);
		}
	}

	g_assignedSlots = 0;
}

static void UnclaimIfOurs(char* manager)
{
	if (CurrentPreset(manager) == &g_preset)
	{
		CurrentPreset(manager) = nullptr;
	}

	if (g_assignedSlots)
	{
		ReleaseSlots(manager);
	}

	g_claimed = false;
}

static bool TryClaim(char* manager)
{
	if (g_claimed)
	{
		return CurrentPreset(manager) == &g_preset;
	}

	bool any = false;

	for (auto& entry : g_screenPeds)
	{
		if (entry.used)
		{
			any = true;
			break;
		}
	}

	if (!any)
	{
		return false;
	}

	if (CurrentPreset(manager) != nullptr || g_isPauseMenuActive(0))
	{
		return false;
	}

	if (!EnsureTemplate(manager))
	{
		return false;
	}

	const UI3DCamera& camera = *reinterpret_cast<const UI3DCamera*>(manager + g_camPosOffset);
	float aspect = g_getAspectRatio(true);

	g_assignedSlots = 0;

	for (int slot = 0; slot < kMaxSlots; slot++)
	{
		auto& entry = g_screenPeds[slot];
		UISceneObjectData& element = g_preset.elements[slot];

		element.enabled = false;

		if (!entry.used)
		{
			continue;
		}

		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(entry.handle);

		if (!entity || !entity->IsOfType<CPed>())
		{
			entry = {};
			continue;
		}

		auto ped = reinterpret_cast<char*>(entity);

		*reinterpret_cast<uint8_t*>(ped + g_pedAiLodByteOffset) |= 0x40;
		*reinterpret_cast<uint32_t*>(ped + g_pedAiLodFlagsOffset) |= 0xC0;

		BuildElement(slot, entry, camera, aspect);

		if (g_assignPedToSlot(manager, slot, ped, &entry.offset, &entry.rotation, 1.0f))
		{
			PatchedData(manager)[slot].globalLightIntensity = 1.0f;
			g_assignedSlots |= (1 << slot);
		}
		else
		{
			element.enabled = false;
		}
	}

	if (!g_assignedSlots)
	{
		return false;
	}

	CurrentPreset(manager) = &g_preset;
	g_claimed = true;

	return true;
}

static void PreRenderHook(char* self)
{
	if (self == GetManager())
	{
		TryClaim(self);
	}

	g_origPreRender(self);
}

static void SetCapturing(uintptr_t enable, uintptr_t)
{
	g_capturing = (enable != 0);

	if (g_capturing)
	{
		g_captureCount = 0;
	}
}

static void AddToDrawListHook(char* self)
{
	if (self != GetManager())
	{
		g_origAddToDrawList(self);
		return;
	}

	bool ours = TryClaim(self);

	if (ours)
	{
		uintptr_t enable = 1;
		uintptr_t unused = 0;
		EnqueueGenericDrawCommand(SetCapturing, &enable, &unused);
	}

	g_origAddToDrawList(self);

	if (ours)
	{
		uintptr_t enable = 0;
		uintptr_t unused = 0;
		EnqueueGenericDrawCommand(SetCapturing, &enable, &unused);
	}

	UnclaimIfOurs(self);
}

static bool EnsureCopyTexture(WRL::ComPtr<ID3D11Texture2D>& copy, ID3D11Resource* source)
{
	if (!source)
	{
		return false;
	}

	WRL::ComPtr<ID3D11Texture2D> sourceTexture;

	if (FAILED(source->QueryInterface(IID_PPV_ARGS(&sourceTexture))))
	{
		return false;
	}

	D3D11_TEXTURE2D_DESC desc;
	sourceTexture->GetDesc(&desc);

	if (desc.SampleDesc.Count != 1)
	{
		return false;
	}

	if (copy)
	{
		D3D11_TEXTURE2D_DESC current;
		copy->GetDesc(&current);

		if (current.Width == desc.Width && current.Height == desc.Height && current.Format == desc.Format && current.MipLevels == desc.MipLevels && current.ArraySize == desc.ArraySize)
		{
			return true;
		}

		copy.Reset();
	}

	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	return SUCCEEDED(GetD3D11Device()->CreateTexture2D(&desc, nullptr, copy.ReleaseAndGetAddressOf()));
}

static void CompositeToBackbufferHook(rage::grcRenderTarget* source, const float* rect, float alpha)
{
	if (!g_capturing || g_captureCount >= kMaxSlots || !source || !source->texture)
	{
		return g_origCompositeToBackbuffer(source, rect, alpha);
	}

	int index = g_captureCount;

	if (!EnsureCopyTexture(g_colorCopies[index], source->texture))
	{
		return g_origCompositeToBackbuffer(source, rect, alpha);
	}

	auto context = GetD3D11DeviceContext();
	context->CopyResource(g_colorCopies[index].Get(), source->texture);

	g_captureHasDepth[index] = false;

	if (char* manager = GetManager())
	{
		auto depth = *reinterpret_cast<rage::grcRenderTarget**>(manager + g_depthRTOffset);

		if (depth && depth->texture && EnsureCopyTexture(g_depthCopies[index], depth->texture))
		{
			context->CopyResource(g_depthCopies[index].Get(), depth->texture);
			g_captureHasDepth[index] = true;
		}
	}

	auto& capture = g_captures[index];
	capture.source = source;
	capture.rect = { rect[0], rect[1], rect[2], rect[3] };
	capture.alpha = alpha;

	g_captureCount++;
}

static void ReplayCaptures()
{
	if (g_captureCount == 0)
	{
		return;
	}

	int count = g_captureCount;
	g_captureCount = 0;

	char* manager = GetManager();

	if (!manager)
	{
		return;
	}

	auto compositeRT = *reinterpret_cast<rage::grcRenderTarget**>(manager);
	auto depthRT = *reinterpret_cast<rage::grcRenderTarget**>(manager + g_depthRTOffset);

	auto context = GetD3D11DeviceContext();

	auto oldRasterizerState = GetRasterizerState();
	auto oldBlendState = GetBlendState();
	auto oldDepthStencilState = GetDepthStencilState();

	for (int i = 0; i < count; i++)
	{
		auto& capture = g_captures[i];

		if (capture.source != compositeRT || !compositeRT || !compositeRT->texture || !g_colorCopies[i])
		{
			continue;
		}

		context->CopyResource(compositeRT->texture, g_colorCopies[i].Get());

		if (g_captureHasDepth[i] && depthRT && depthRT->texture && g_depthCopies[i])
		{
			context->CopyResource(depthRT->texture, g_depthCopies[i].Get());
		}

		g_origCompositeToBackbuffer(compositeRT, &capture.rect.x, capture.alpha);
	}

	SetRasterizerState(oldRasterizerState);
	SetBlendState(oldBlendState);
	SetDepthStencilState(oldDepthStencilState);
}

static int FindSlotByHandle(int handle)
{
	if (handle == 0)
	{
		return -1;
	}

	for (int slot = 0; slot < kMaxSlots; slot++)
	{
		if (g_screenPeds[slot].used && g_screenPeds[slot].handle == handle)
		{
			return slot;
		}
	}

	return -1;
}

static std::string GetCurrentResourceName()
{
	fx::OMPtr<IScriptRuntime> runtime;

	if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
	{
		auto resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

		if (resource)
		{
			return resource->GetName();
		}
	}

	return {};
}

static void DeleteClone(int handle)
{
	if (NativeInvoke::Invoke<DOES_ENTITY_EXIST, bool>(handle))
	{
		NativeInvoke::Invoke<DELETE_ENTITY, void>(&handle);
	}
}

static void ResetAll()
{
	if (char* manager = GetManager())
	{
		UnclaimIfOurs(manager);
	}

	for (auto& entry : g_screenPeds)
	{
		entry = {};
	}

	g_claimed = false;
	g_assignedSlots = 0;
}

static bool ReadRect(fx::ScriptContext& context, int firstArg, float& x, float& y, float& w, float& h)
{
	x = context.GetArgument<float>(firstArg);
	y = context.GetArgument<float>(firstArg + 1);
	w = context.GetArgument<float>(firstArg + 2);
	h = context.GetArgument<float>(firstArg + 3);

	return std::isfinite(x) && std::isfinite(y) && std::isfinite(w) && std::isfinite(h) && w > 0.0f && h > 0.0f;
}

static HookFunction hookFunction([]()
{
	auto preRenderCall = hook::get_pattern<char>("48 8B 0D ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 48 8B 15 ? ? ? ? 48 8D 0D");
	g_ui3dInstance = hook::get_address<char**>(preRenderCall + 3);

	auto addToDrawList = hook::get_pattern<char>("48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D A8 C8 FA FF FF 48 81 EC F8 05 00 00 4C 8B B1");
	g_currentPresetOffset = *reinterpret_cast<uint32_t*>(addToDrawList + 0x20);

	auto assign = hook::get_pattern<char>("83 FA 05 0F 83 ? ? ? ? 44 8B F2 4B 8D 3C 76 48 C1 E7 05 48 8D 34 0F 48 8B 8E");
	g_patchedDataOffset = *reinterpret_cast<uint32_t*>(assign + 0x1B);
	g_assignPedToSlot = reinterpret_cast<decltype(g_assignPedToSlot)>(assign - 0x21);

	g_depthRTOffset = *hook::get_pattern<uint8_t>("48 8B 52 ? E8 ? ? ? ? 41 83 C9 FF 48 8D 8D ? ? ? ? 41 8D 51 1E", 3);

	g_camPosOffset = *hook::get_pattern<uint32_t>("E8 ? ? ? ? 0F 28 93 ? ? ? ? 48 8D 4B 70 45 33 C0 B2 01", 8);

	auto aiLod = hook::get_pattern<char>("80 88 ? ? ? ? 40 81 88 ? ? ? ? C0 00 00 00");
	g_pedAiLodByteOffset = *reinterpret_cast<uint32_t*>(aiLod + 2);
	g_pedAiLodFlagsOffset = *reinterpret_cast<uint32_t*>(aiLod + 9);

	g_pedPhaseVisibilityOffset = *hook::get_pattern<uint8_t>("66 44 89 77 ? 66 C7 87 C4 00 00 00 FF FF", 4);

	g_changeVisibilityMask = reinterpret_cast<decltype(g_changeVisibilityMask)>(hook::get_pattern("41 8A F1 41 8A E8 0F B7 FA 48 8B D9 45 84 C0 74 ? 66 09 51 ? EB ? 0F B7 C7 66 F7 D0 66 21 41", -0x14));

	g_getPreset = reinterpret_cast<decltype(g_getPreset)>(hook::get_call(hook::get_pattern("8B 02 48 8D 54 24 38 48 8B D9 89 44 24 38 E8 ? ? ? ? 48 8B C8 33 C0 48 85 C9 74 1A 38 83", 14)));

	g_isPauseMenuActive = reinterpret_cast<decltype(g_isPauseMenuActive)>(hook::get_pattern("80 3D ? ? ? ? 00 8B F9 75 0D 80 3D ? ? ? ? 00 75 04 32 DB", -0xA));

	g_getAspectRatio = reinterpret_cast<decltype(g_getAspectRatio)>(hook::get_call(hook::get_pattern("B1 01 F3 44 0F 10 76 04 F3 0F 11 74 24 ? E8", 14)));

	auto compositeToBackbuffer = hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? 33 D2 48 89 44 24 20 E8 ? ? ? ? 48 8D 4D ? E8", 3));

	g_origPreRender = hook::trampoline(hook::get_call(preRenderCall + 7), PreRenderHook);
	g_origAddToDrawList = hook::trampoline(addToDrawList, AddToDrawListHook);
	g_origCompositeToBackbuffer = hook::trampoline(compositeToBackbuffer, CompositeToBackbufferHook);

	g_initialized = true;
});

static InitFunction initFunction([]()
{
	// replay the deferred ped composites above all NUI windows, but below the NUI cursor
	nui::OnPostRenderWindows.Connect([]()
	{
		ReplayCaptures();
	});

	// fallback in case NUI didnt render this frame, still draw the peds after NUI would have
	OnPostFrontendRender.Connect([]()
	{
		if (IsOnRenderThread())
		{
			ReplayCaptures();
		}
		else
		{
			uintptr_t a = 0;
			uintptr_t b = 0;
			EnqueueGenericDrawCommand([](uintptr_t, uintptr_t)
			{
				ReplayCaptures();
			}, &a, &b);
		}
	}, 1000);

	OnMainGameFrame.Connect([]()
	{
		if (char* manager = GetManager())
		{
			UnclaimIfOurs(manager);
		}

		if (!g_initialized)
		{
			return;
		}

		for (auto& entry : g_screenPeds)
		{
			if (!entry.used)
			{
				continue;
			}

			fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(entry.handle);

			if (!entity || !entity->IsOfType<CPed>())
			{
				continue;
			}

			auto ped = reinterpret_cast<char*>(entity);

			if (*reinterpret_cast<uint16_t*>(ped + g_pedPhaseVisibilityOffset) != kVisPhaseMaskOther)
			{
				HideFromWorld(ped);
			}
		}
	});

	OnKillNetworkDone.Connect([]()
	{
		ResetAll();
	});

	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		resource->OnStop.Connect([resource]()
		{
			const std::string& name = resource->GetName();

			for (auto& entry : g_screenPeds)
			{
				if (entry.used && entry.owner == name)
				{
					int handle = entry.handle;
					entry = {};

					DeleteClone(handle);
				}
			}
		});
	});

	fx::ScriptEngine::RegisterNativeHandler("CLONE_PED_TO_SCREEN", [](fx::ScriptContext& context)
	{
		context.SetResult<int>(0);

		if (!g_initialized)
		{
			return;
		}

		int ped = context.GetArgument<int>(0);
		float x, y, w, h;

		if (!ReadRect(context, 1, x, y, w, h))
		{
			return;
		}

		int slot = -1;

		for (int i = 0; i < kMaxSlots; i++)
		{
			if (!g_screenPeds[i].used)
			{
				slot = i;
				break;
			}
		}

		if (slot < 0)
		{
			return;
		}

		int clone = NativeInvoke::Invoke<CLONE_PED, int>(ped, false, false, true);

		if (clone == 0)
		{
			return;
		}

		NativeInvoke::Invoke<FREEZE_ENTITY_POSITION, void>(clone, true);
		NativeInvoke::Invoke<SET_ENTITY_COLLISION, void>(clone, false, false);
		NativeInvoke::Invoke<SET_ENTITY_INVINCIBLE, void>(clone, true);
		NativeInvoke::Invoke<SET_BLOCKING_OF_NON_TEMPORARY_EVENTS, void>(clone, true);
		NativeInvoke::Invoke<SET_PED_CAN_RAGDOLL, void>(clone, false);
		NativeInvoke::Invoke<STOP_PED_SPEAKING, void>(clone, true);
		NativeInvoke::Invoke<DISABLE_PED_PAIN_AUDIO, void>(clone, true);

		fwEntity* cloneEntity = rage::fwScriptGuid::GetBaseFromGuid(clone);

		if (!cloneEntity || !cloneEntity->IsOfType<CPed>())
		{
			return;
		}

		HideFromWorld(reinterpret_cast<char*>(cloneEntity));

		auto& entry = g_screenPeds[slot];
		entry = {};
		entry.used = true;
		entry.handle = clone;
		entry.owner = GetCurrentResourceName();
		entry.x = x;
		entry.y = y;
		entry.w = w;
		entry.h = h;

		context.SetResult<int>(clone);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_SCREEN_PED_RECT", [](fx::ScriptContext& context)
	{
		context.SetResult<bool>(false);

		int slot = FindSlotByHandle(context.GetArgument<int>(0));
		float x, y, w, h;

		if (slot < 0 || !ReadRect(context, 1, x, y, w, h))
		{
			return;
		}

		auto& entry = g_screenPeds[slot];
		entry.x = x;
		entry.y = y;
		entry.w = w;
		entry.h = h;

		context.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_SCREEN_PED_TRANSFORM", [](fx::ScriptContext& context)
	{
		context.SetResult<bool>(false);

		int slot = FindSlotByHandle(context.GetArgument<int>(0));

		if (slot < 0)
		{
			return;
		}

		float values[6];

		for (int i = 0; i < 6; i++)
		{
			values[i] = context.GetArgument<float>(1 + i);

			if (!std::isfinite(values[i]))
			{
				return;
			}
		}

		auto& entry = g_screenPeds[slot];
		entry.offset = { values[0], values[1], values[2], 0.0f };
		entry.rotation = { values[3], values[4], values[5], 0.0f };

		context.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("DELETE_SCREEN_PED", [](fx::ScriptContext& context)
	{
		int slot = FindSlotByHandle(context.GetArgument<int>(0));

		if (slot < 0)
		{
			return;
		}

		int handle = g_screenPeds[slot].handle;
		g_screenPeds[slot] = {};

		DeleteClone(handle);
	});
});
