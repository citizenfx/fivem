#include "StdInc.h"
#include <sfFontStuff.h>
#include <sfDefinitions.h>

#include <atPool.h>

#include <MinHook.h>

#include <jitasm.h>
#include <Hooking.h>

#include <Streaming.h>
#include <nutsnbolts.h>

#include <GameInit.h>

#include <queue>

#include "Hooking.Stubs.h"

static std::vector<std::string> g_fontIds = {
#ifdef GTA_FIVE
	"$Font2",
	"$Font5",
	"$RockstarTAG",
	"$GTAVLeaderboard",
	"$Font2_cond",
	"$FixedWidthNumbers",
	"$Font2_cond_NOT_GAMERNAME",
	"$gtaCash",
	// padding for bad scripts
	"$Font2",
	"$Taxi_font", // added in a newer release after a $Font2 for padding
	"$Font2",
	"$Font2",
	"$Font2",
#endif
};

#if IS_RDR3
constexpr int GAME_FONT_COUNT = 31; // common:/data/ui/fontmap.xml
#endif

static std::set<std::string> g_fontLoadQueue;
static std::set<std::string> g_loadedFonts;

namespace sf
{
	int RegisterFontIndex(const std::string& fontName)
	{
		// find if the same font is already registered
		auto it = std::find(g_fontIds.begin(), g_fontIds.end(), fontName);

		if (it != g_fontIds.end())
		{
			return it - g_fontIds.begin();
		}

		// if not, don't
		g_fontIds.push_back(fontName);
		return g_fontIds.size() - 1;
	}

	void RegisterFontLib(const std::string& swfName)
	{
		if (g_loadedFonts.find(swfName) != g_loadedFonts.end())
		{
			return;
		}

		g_fontLoadQueue.insert(swfName);
		g_loadedFonts.insert(swfName);
	}
}

#ifdef GTA_FIVE
static void(*gfxStringAssign)(void*, const char*);

static void GetFontById(void* gfxString, int id)
{
	if (id < 0 || id >= g_fontIds.size())
	{
		id = 0;
	}

	gfxStringAssign(gfxString, g_fontIds[id].c_str());
}
#else
static char* (*g_GetFontNameById)(int id);
static char* GetFontById(int id)
{
	if (id < 0 || id >= (GAME_FONT_COUNT + g_fontIds.size()))
	{
		trace("Font ID out of range: %d\n", id);
		id = 0;
	}
	
	if(id >= GAME_FONT_COUNT)
	{
		return const_cast<char*>(g_fontIds[id - GAME_FONT_COUNT].c_str());
	}
	
	char* fontName = g_GetFontNameById(id);
	return fontName;
}
#endif

static void(*assignFontLib)(void*, void*, int);

struct strStreamingModule
{
	uintptr_t vtable;
	int baseIdx;
};

strStreamingModule* g_scaleformStore;
void* g_fontLib;

static void AssignFontLibWrap(strStreamingModule* store, void* lib, int idx)
{
	assignFontLib(store, lib, idx);

	if (store)
	{
		g_scaleformStore = store;
		g_fontLib = lib;
	}
}

static void UpdateFontLoading()
{
	std::set<std::string> toRemove;

	auto cstreaming = streaming::Manager::GetInstance();

	for (const auto& font : g_fontLoadQueue)
	{
		// get the streaming entry
		auto strIdx = streaming::GetStreamingIndexForName(font + ".gfx");

		if (strIdx != 0)
		{
			// check if the font file is loaded
			if ((cstreaming->Entries[strIdx].flags & 3) == 1)
			{
				// it has. add to the font library
				trace("font file %s loaded - adding to GFxFontLib\n", font);

				assignFontLib(g_scaleformStore, g_fontLib, strIdx - g_scaleformStore->baseIdx);

				toRemove.insert(font);
			}
			else
			{
				cstreaming->RequestObject(strIdx, 0);
			}
		}
		else
		{
			trace("unknown font file %s\n", font);

			toRemove.insert(font);
		}
	}

	for (const auto& font : toRemove)
	{
		g_fontLoadQueue.erase(font);
	}
}

static bool(*g_origFindExportedResource)(void* movieRoot, void* localDef, void* bindData, void* symbol);

static GFxMovieDef* g_md;
static GFxMovieRoot* g_movie;

static uint32_t g_idx = 0;
static void** g_scaleformMgr;

static void InitEarlyLoading()
{
	if (g_idx != 0)
	{
		return;
	}

	if (!*g_scaleformMgr)
	{
		return;
	}

	uint32_t fileId = 0;
	streaming::RegisterRawStreamingFile(&fileId, "citizen:/font_lib_cfx.gfx", true, "font_lib_cfx.gfx", false);

	g_idx = fileId;

	auto gfxStore = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("gfx");
	auto pool = (atPoolBase*)((char*)gfxStore + 56);
	auto refBase = pool->GetAt<char>(g_idx - gfxStore->baseIdx);
	refBase[50] = true; // 'create movie once loaded' flag

	// '7' flag so the game won't try unloading it
	streaming::Manager::GetInstance()->RequestObject(g_idx, 7);

	// we should load *now*
	streaming::LoadObjectsNow(false);
}

template<typename TFn>
static void HandleEarlyLoading(TFn&& cb)
{
	if (g_idx != 0)
	{
		auto gfxStore = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("gfx");

		auto pool = (atPoolBase*)((char*)gfxStore + 56);
		auto refBase = pool->GetAt<char*>(g_idx - gfxStore->baseIdx);

		auto ref = *refBase;

		if (ref)
		{
			auto movieDefRef = *(void***)(ref + 16);
			auto movieRef = *(void**)(*(char**)(ref + 24) + 16);

			if (movieDefRef)
			{
				g_md = (GFxMovieDef*)* movieDefRef;
				g_movie = (GFxMovieRoot*)movieRef;

				// GH-2157: HandleSprite can be called from both MainThread and
				// RenderThread simultaneously: ensure the heap is flagged to be
				// thread-safe.
				GMemoryHeap* heap = g_movie->pHeap;
				if (!heap->useLocks)
				{
					heap->useLocks = true;
				}
				cb();
			}
		}
	}
}

static bool FindExportedResource(void* movieRoot, void* localDef, void* bindData, void* symbol)
{
	bool rv = g_origFindExportedResource(movieRoot, localDef, bindData, symbol);

	HandleEarlyLoading([&rv, movieRoot, bindData, symbol]()
	{
		if (!rv)
		{
			rv = g_origFindExportedResource(movieRoot, g_md, bindData, symbol);
		}
	});

	return rv;
}

#if GTA_FIVE

static bool(*g_origGetExportedResource)(void* movieDef, void* bindData, void* symbol, void* ignoreDef);

static bool GetExportedResource(void* movieDef, void* bindData, void* symbol, void* ignoreDef)
{
	bool rv = g_origGetExportedResource(movieDef, bindData, symbol, ignoreDef);

	HandleEarlyLoading([&rv, bindData, symbol]()
	{
		if (!rv)
		{
			rv = g_origGetExportedResource(g_md, bindData, symbol, NULL);
		}
	});

	return rv;
}

static hook::cdecl_stub<void*(GFxMovieRoot*, int)> _gfxMovieRoot_getLevelMovie([]()
{
	return hook::get_pattern("48 8B 49 40 44 8B C0 4D 03 C0 42 39 14 C1 74 0D", -0xB);
});

static LONG SafetyFilter(PEXCEPTION_POINTERS pointers)
{
	static bool excepted = false;

	if (!excepted)
	{
		trace("Exception in unsafe stubbed GFx hooks: %08x at %016llx.\n", pointers->ExceptionRecord->ExceptionCode, (uint64_t)pointers->ExceptionRecord->ExceptionAddress);

		excepted = true;
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

// GH-2157: Other methods used path into GASEnvironment/GlobalEnvironment bits
// that are not thread-safe.
static std::mutex g_handleSprite;
static void HandleSprite(GFxResource* resource, GFxStyledText::HTMLImageTagInfo* imgTagInfo, uint64_t document, void* md, void* character)
{
	std::unique_lock _(g_handleSprite);
	auto sprite = (GFxSpriteDef*)resource;

	static uint32_t id = 0x1234;
	auto ci = (GFxSprite*)sprite->CreateCharacterInstance(character ? character : _gfxMovieRoot_getLevelMovie(g_movie, 0), &id, g_md ? g_md : md);

	ci->AddRef();

	if (imgTagInfo->textImageDesc->spriteShape)
	{
		imgTagInfo->textImageDesc->spriteShape->Release();
	}

	imgTagInfo->textImageDesc->spriteShape = ci;

	float screenWidth = imgTagInfo->Width;
	float screenHeight = imgTagInfo->Height;

	ci->Restart();

	auto rect = ci->GetRectBounds(GMatrix2D{});
	float origWidth = abs(rect.right - rect.left);
	float origHeight = abs(rect.bottom - rect.top);

	imgTagInfo->textImageDesc->screenWidth = screenWidth;

	float origAspect = (origWidth / origHeight);
	screenWidth = screenHeight * origAspect;

	imgTagInfo->textImageDesc->matrix.AppendTranslation(0.0f, -origHeight);
	imgTagInfo->textImageDesc->matrix.AppendScaling(screenWidth / origWidth, screenHeight / origHeight);

	imgTagInfo->textImageDesc->screenHeight = screenHeight;

	// SetCompleteReformatReq
	*(uint8_t*)(document + 456i64) |= 2u;

	ci->Release();
}
#endif

static HookFunction hookFunction([]()
{
	// get font by id, hook
	{
#ifdef GTA_FIVE
		auto location = hook::get_pattern<char>("85 D2 74 68 FF CA 74 5B FF CA 74 4E");
		hook::jump(location, GetFontById);
		hook::set_call(&gfxStringAssign, location + 115);
#else
		auto location = hook::get_call(hook::get_pattern<char>("E8 ? ? ? ? 33 C9 48 89 45 ? 48 85 FF"));
		g_GetFontNameById = hook::trampoline(location, GetFontById);
#endif
	}

	// init font things
	{
#ifdef GTA_FIVE
		auto location = hook::get_pattern<char>("BA 13 00 00 00 48 8B 48 20 48 8B 01 FF 50 10 44", 32);
#else
		auto location = hook::get_pattern<char>("E8 ? ? ? ? 48 63 53 ? 48 8B 05");
#endif
		hook::set_call(&assignFontLib, location);
		hook::call(location, AssignFontLibWrap);
	}

	{
#ifdef GTA_FIVE
		auto location = hook::get_pattern<char>("74 1A 80 3D ? ? ? ? 00 74 11 E8 ? ? ? ? 48 8B");
#else
		auto location = hook::get_pattern<char>("E8 ? ? ? ? 48 63 53 ? 48 8B 05");
#endif
		g_scaleformMgr = hook::get_address<void**>(location + 19);
	}
	
	OnMainGameFrame.Connect([]()
	{
		UpdateFontLoading();
	});

#ifdef GTA_FIVE

	OnLookAliveFrame.Connect([]()
	{
		InitEarlyLoading();
	});
	
	// find resource parent movie defs
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("B0 01 EB 47 48 8B 5F 70 48 83 C7 68", -0x39), FindExportedResource, (void**)&g_origFindExportedResource);
	MH_CreateHook(hook::get_pattern("48 8B F9 33 DB 49 8B 4A 40 48 8B F2 83", -0x2F), GetExportedResource, (void**)&g_origGetExportedResource);
	MH_EnableHook(MH_ALL_HOOKS);

	static struct : jitasm::Frontend
	{
		virtual void InternalMain() override
		{
			mov(r9, r14);						// moviedef
			mov(r8, r12);						// self
			mov(rdx, qword_ptr[rbp - 0x30]);	// resource
			mov(rcx, rsi);						// img tag info, base
			add(rcx, r15);						// img tag info, offset

			sub(rsp, 0x28);

			mov(rax, (uint64_t)&HandleNewImageTag);
			call(rax);

			add(rsp, 0x28);

			ret();
		}

		static void HandleNewImageTag(GFxStyledText::HTMLImageTagInfo* imgTagInfo, GFxResource* resource, void* self, void* md)
		{
			if (resource->IsSprite())
			{
				__try
				{
					HandleSprite(resource, imgTagInfo, *(uint64_t*)((char*)self + 272), md, self);
				}
				__except (SafetyFilter(GetExceptionInformation()))
				{

				}
			}
		}
	} textFieldStub;

	{
		auto location = hook::get_pattern("3D 00 01 00 00 74 0F 48 8B 4D D0 48 8B 01", 7);
		hook::nop(location, 10);
		hook::call(location, textFieldStub.GetCode());
	}

	static void* returnAddress;

	static struct : jitasm::Frontend
	{
		virtual void InternalMain() override
		{
			mov(r8, qword_ptr[rbp + 0x67]); // document
			mov(rdx, rcx);					// resource
			mov(rcx, qword_ptr[rbp + 0xF]); // image tag info, base
			add(rcx, r15);					// image tag info, itself

			sub(rsp, 0x28);

			mov(rax, (uint64_t)&HandleNewImageTag);
			call(rax);

			add(rsp, 0x28);

			test(rax, rax);
			jz("jumpOut");

			and(eax, 0xFF00);				// original code

			ret();

			L("jumpOut");					// jump to end of block
			mov(rax, (uint64_t)returnAddress);
			mov(qword_ptr[rsp], rax);
			ret();
		}

		static int HandleNewImageTag(GFxStyledText::HTMLImageTagInfo* imgTagInfo, GFxResource* resource, uint64_t document)
		{
			__try
			{
				if (resource->IsSprite() && g_md)
				{
					HandleSprite(resource, imgTagInfo, document, g_md, nullptr);

					return 0;
				}
			}
			__except (SafetyFilter(GetExceptionInformation()))
			{

			}

			return resource->GetType();
		}
	} drawTextStub;

	{
		auto location = hook::get_pattern<char>("48 8B 4D C7 48 8B 01 FF 50 10 25 00 FF 00", 7);

		returnAddress = location + 76;

		hook::nop(location, 8);
		hook::call(location, drawTextStub.GetCode());
	}

	static struct : jitasm::Frontend
	{
		virtual void InternalMain() override
		{
			cmp(qword_ptr[r14 + 0x18], 0);	// if (image->spriteShape != 0)
			jz("onReturn");					// {

			lea(r8, qword_ptr[rbp - 0x48]); // matrix
			mov(rdx, rsi);					// context
			mov(rcx, r14);					// pImage

			sub(rsp, 0x28);
			mov(rax, (uint64_t)&DrawSprite);// DrawSprite(pImage, context, matrix)
			call(rax);
			add(rsp, 0x28);

			L("onReturn");					// }

			cmp(qword_ptr[r14 + 0x10], 0);	// original code
			ret();
		}

		static void DrawSprite(GFxTextImageDesc* image, GFxDisplayContext* drawContext, const GMatrix2D* matrix)
		{
			// add a SEH frame here so we won't try unwinding over the jitasm stub
			__try
			{
				DrawSpriteInternal(image, drawContext, matrix);
			}
			__except (SafetyFilter(GetExceptionInformation()))
			{

			}
		}

		static void DrawSpriteInternal(GFxTextImageDesc* image, GFxDisplayContext* drawContext, const GMatrix2D* matrix)
		{
			// expand the visible rectangle of the placeholder movie
			// otherwise, only ~600x400 on-screen gets sprites drawn due to culling tests
			if (g_movie)
			{
				g_movie->VisibleFrameRect.left = 0.0f;
				g_movie->VisibleFrameRect.top = 0.0f;
				g_movie->VisibleFrameRect.bottom = 10000.0f * 20.0f;
				g_movie->VisibleFrameRect.right = 10000.0f * 20.0f;
			}

			// +88 -> local matrix
			GFxSprite* sprite = (GFxSprite*)image->spriteShape;

			// copy the input (character) matrix and prepend the local matrix
			auto m2 = *matrix;
			m2.Prepend(image->matrix);

			// set the matrix/color transform in the context
			GRenderer::CXform identityCxform;

			auto oldParentCxform = drawContext->parentCxform;
			auto oldParentMatrix = drawContext->parentMatrix;

			drawContext->parentCxform = &identityCxform;
			drawContext->parentMatrix = &m2;

			// draw the sprite
			sprite->Display(drawContext);

			// restore the old color transform and matrix
			drawContext->parentCxform = oldParentCxform;
			drawContext->parentMatrix = oldParentMatrix;

		}
	} lineBufferStub;

	hook::call(hook::get_pattern("F3 0F 58 55 34 F3 0F 11 4D C0 F3 0F 11 55 CC", 24), lineBufferStub.GetCode());

	// fix bug when img tag is last in a text formatting tag (https://github.com/citizenfx/fivem/issues/1112)
	static void (*origPopBack)(void* vec, size_t len);

	static struct : jitasm::Frontend
	{
		void InternalMain() override
		{
			lea(r8, qword_ptr[rbp + 0x300]);
			mov(rax, (uint64_t)ParseHtmlReset);
			jmp(rax);
		}

		static void ParseHtmlReset(GFxElemDesc** vec, size_t len, GFxTextFormat* format)
		{
			if (len > 0)
			{
				(*vec)[len - 1].fmt.presentMask |= (*vec)[len].fmt.presentMask;
			}
			else
			{
				format->presentMask |= (*vec)[len].fmt.presentMask;
			}

			origPopBack(vec, len);
		}
	} parseHtmlStub;

	{
		auto location = hook::get_pattern<char>("49 8D 54 24 FF 48 8D 4D 80 E8", 9);
		hook::set_call(&origPopBack, location);
		hook::call(location, parseHtmlStub.GetCode());
		hook::call(location + 0x62, parseHtmlStub.GetCode());
	}
#endif

	// undo fonts if reloading
	OnKillNetworkDone.Connect([]()
	{
		g_loadedFonts.clear();
	});
});
