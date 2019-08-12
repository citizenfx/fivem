#include "StdInc.h"
#include <sfFontStuff.h>

#include <atPool.h>

#include <MinHook.h>

#include <Hooking.h>

#include <Streaming.h>
#include <nutsnbolts.h>

#include <queue>

static std::vector<std::string> g_fontIds = {
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
	"$Font2",
	"$Font2",
	"$Font2",
	"$Font2",
};

static std::set<std::string> g_fontLoadQueue;

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
		g_fontLoadQueue.insert(swfName);
	}
}

static void(*gfxStringAssign)(void*, const char*);

static void GetFontById(void* gfxString, int id)
{
	if (id < 0 || id >= g_fontIds.size())
	{
		id = 0;
	}

	gfxStringAssign(gfxString, g_fontIds[id].c_str());
}

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

struct GFxMovieDef
{

};

struct GRectF
{
	float left, top, right, bottom;
};

struct GFxMovieRoot
{
	char pad[200];
	GRectF VisibleFrameRect;
};

static GFxMovieDef* g_md;
static GFxMovieRoot* g_movie;

template<typename TFn>
static void HandleEarlyLoading(TFn&& cb)
{
	static uint32_t idx = 0;

	if (idx == 0)
	{
		uint32_t fileId = 0;
		streaming::RegisterRawStreamingFile(&fileId, "citizen:/font_lib_cfx.gfx", true, "font_lib_cfx.gfx", false);

		idx = fileId;

		auto gfxStore = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("gfx");
		auto pool = (atPoolBase*)((char*)gfxStore + 56);
		auto refBase = pool->GetAt<char>(idx - gfxStore->baseIdx);
		refBase[50] = true; // 'create movie once loaded' flag

		// '7' flag so the game won't try unloading it
		streaming::Manager::GetInstance()->RequestObject(idx, 7);

		// we should load *now*
		streaming::LoadObjectsNow(false);
	}

	if (idx != 0)
	{
		auto gfxStore = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("gfx");

		auto pool = (atPoolBase*)((char*)gfxStore + 56);
		auto refBase = pool->GetAt<char*>(idx - gfxStore->baseIdx);

		auto ref = *refBase;

		if (ref)
		{
			auto movieDefRef = *(void***)(ref + 16);
			auto movieRef = *(void**)(*(char**)(ref + 24) + 16);

			if (movieDefRef)
			{
				g_md = (GFxMovieDef*)* movieDefRef;
				g_movie = (GFxMovieRoot*)movieRef;

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

static void AddRef(void* ptr)
{
	InterlockedIncrement((unsigned long*)((char*)ptr + 8));
}

static void ReleaseRef(void* ptr)
{
	struct VBase
	{
	public:
		virtual ~VBase() = 0;
	};

	if (!InterlockedDecrement((unsigned long*)((char*)ptr + 8)))
	{
		delete (VBase*)ptr;
	}
}

static hook::cdecl_stub<void*(GFxMovieRoot*, int)> _gfxMovieRoot_getLevelMovie([]()
{
	return hook::get_pattern("48 8B 49 40 44 8B C0 4D 03 C0 42 39 14 C1 74 0D", -0xB);
});

static void* Alloc_Align(void* self, size_t size, size_t align)
{
	return _aligned_malloc(size, align);
}

static void* Alloc(void* self, size_t size)
{
	return _aligned_malloc(size, 16);
}

static void* Realloc(void* self, void* ptr, size_t size)
{
	return _aligned_realloc(ptr, size, 16);
}

static void Free(void* self, void* ptr)
{
	_aligned_free(ptr);
}

static void* AllocAuto_Align(void* self, void* h, size_t size, size_t align)
{
	return _aligned_malloc(size, align);
}

static void* AllocAuto(void* self, void* h, size_t size)
{
	return _aligned_malloc(size, 16);
}

class GFxMemoryHeap
{
public:
	virtual void m_00() = 0;
	virtual void m_08() = 0;
	virtual void m_10() = 0;
	virtual void m_18() = 0;
	virtual void m_20() = 0;
	virtual void m_28() = 0;
	virtual void m_30() = 0;
	virtual void m_38() = 0;
	virtual void m_40() = 0;
	virtual void m_48() = 0;
	virtual void* Alloc(uint32_t size) = 0;
	virtual void m_58() = 0;
	virtual void Free(void* memory) = 0;
};

extern GFxMemoryHeap** g_gfxMemoryHeap;

static void* GetHeap(void* self)
{
	return self;
}

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

struct Matrix2D
{
	float matrix[2][3];

	inline Matrix2D()
	{
		memset(matrix, 0, sizeof(matrix));
		matrix[0][0] = 1.0f;
		matrix[1][1] = 1.0f;
	}

	inline void AppendScaling(float sx, float sy)
	{
		matrix[0][0] *= sx;
		matrix[0][1] *= sx;
		matrix[0][2] *= sx;
		matrix[1][0] *= sy;
		matrix[1][1] *= sy;
		matrix[1][2] *= sy;
	}

	inline void AppendTranslation(float dx, float dy)
	{
		matrix[0][2] += dx;
		matrix[1][2] += dy;
	}

	inline void Prepend(const Matrix2D& m)
	{
		Matrix2D t = *this;
		matrix[0][0] = t.matrix[0][0] * m.matrix[0][0] + t.matrix[0][1] * m.matrix[1][0];
		matrix[1][0] = t.matrix[1][0] * m.matrix[0][0] + t.matrix[1][1] * m.matrix[1][0];
		matrix[0][1] = t.matrix[0][0] * m.matrix[0][1] + t.matrix[0][1] * m.matrix[1][1];
		matrix[1][1] = t.matrix[1][0] * m.matrix[0][1] + t.matrix[1][1] * m.matrix[1][1];
		matrix[0][2] = t.matrix[0][0] * m.matrix[0][2] + t.matrix[0][1] * m.matrix[1][2] + t.matrix[0][2];
		matrix[1][2] = t.matrix[1][0] * m.matrix[0][2] + t.matrix[1][1] * m.matrix[1][2] + t.matrix[1][2];
	}
};

struct CXform
{
	float matrix[4][2];

	CXform()
	{
		matrix[0][0] = 1.0f;
		matrix[1][0] = 1.0f;
		matrix[2][0] = 1.0f;
		matrix[3][0] = 1.0f;
		matrix[0][1] = 0.0f;
		matrix[1][1] = 0.0f;
		matrix[2][1] = 0.0f;
		matrix[3][1] = 0.0f;
	}
};

struct GFxTextImageDesc
{
	void* vtbl;
	int refcount;
	void* imageShape;
	void* spriteShape;
	int baseLineX;
	int baseLineY;
	uint32_t screenWidth;
	uint32_t screenHeight;
	Matrix2D matrix;
};

struct HTMLImageTagInfo
{
	GFxTextImageDesc* textImageDesc;
	char pad[32];
	uint32_t Width, Height;
};

struct GFxResource
{
	virtual void m_0() = 0;
	virtual void m_1() = 0;
	virtual int GetType() = 0;

	inline bool IsSprite()
	{
		return ((GetType() >> 8) & 0xff) == 0x84;
	}
};

struct GFxSprite
{
	virtual void m0() = 0;
	virtual void m1() = 0;
	virtual void m2() = 0;
	virtual void m3() = 0;
	virtual void m4() = 0;
	virtual void m5() = 0;
	virtual void m6() = 0;
	virtual void m7() = 0;
	virtual void m8() = 0;
	virtual void m9() = 0;
	virtual void m10() = 0;
	virtual void m11() = 0;
	virtual void m12() = 0;
	virtual void m13() = 0;
	virtual void m14() = 0;
	virtual void m15() = 0;
	virtual GRectF GetRectBounds(const Matrix2D& matrix) = 0;
	virtual void m17() = 0;
	virtual void m18() = 0;
	virtual void m19() = 0;
	virtual void m20() = 0;
	virtual void m21() = 0;
	virtual void m22() = 0;
	virtual void m23() = 0;
	virtual void m24() = 0;
	virtual void m25() = 0;
	virtual void m26() = 0;
	virtual void m27() = 0;
	virtual void m28() = 0;
	virtual void Display(void* context) = 0;
	virtual void Restart() = 0;
};

struct GFxSpriteDef : public GFxResource
{
	virtual void m_s0() = 0;
	virtual void m_s1() = 0;
	virtual void m_s2() = 0;
	virtual void m_s3() = 0;
	virtual void m_s4() = 0;
	virtual void m_s5() = 0;
	virtual void* CreateCharacterInstance(void* parent, uint32_t* id,
		void* pbindingImpl) = 0;
};

struct DisplayContext
{
	CXform* parentCxform;
	Matrix2D* parentMatrix;
};

static void HandleSprite(GFxResource* resource, HTMLImageTagInfo* imgTagInfo, uint64_t document, void* md, void* character)
{
	auto sprite = (GFxSpriteDef*)resource;

	static uint32_t id = 0x1234;
	auto ci = (GFxSprite*)sprite->CreateCharacterInstance(character ? character : _gfxMovieRoot_getLevelMovie(g_movie, 0), &id, g_md ? g_md : md);

	AddRef(ci);

	if (imgTagInfo->textImageDesc->spriteShape)
	{
		ReleaseRef(imgTagInfo->textImageDesc->spriteShape);
	}

	imgTagInfo->textImageDesc->spriteShape = ci;

	float screenWidth = imgTagInfo->Width;
	float screenHeight = imgTagInfo->Height;

	ci->Restart();

	auto rect = ci->GetRectBounds(Matrix2D{});
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

	ReleaseRef(ci);
}

static HookFunction hookFunction([]()
{
	// get font by id, hook
	{
		auto location = hook::get_pattern<char>("85 D2 74 68 FF CA 74 5B FF CA 74 4E");
		hook::jump(location, GetFontById);
		hook::set_call(&gfxStringAssign, location + 115);
	}

	// init font things
	{
		auto location = hook::get_pattern<char>("BA 13 00 00 00 48 8B 48 20 48 8B 01 FF 50 10 44");
		hook::set_call(&assignFontLib, location + 32);
		hook::call(location + 32, AssignFontLibWrap);
	}

	OnMainGameFrame.Connect([]()
	{
		UpdateFontLoading();
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

		static void HandleNewImageTag(HTMLImageTagInfo* imgTagInfo, GFxResource* resource, void* self, void* md)
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

		static int HandleNewImageTag(HTMLImageTagInfo* imgTagInfo, GFxResource* resource, uint64_t document)
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

		static void DrawSprite(GFxTextImageDesc* image, DisplayContext* drawContext, const Matrix2D* matrix)
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

		static void DrawSpriteInternal(GFxTextImageDesc* image, DisplayContext* drawContext, const Matrix2D* matrix)
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
			CXform identityCxform;

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

	// formerly debuggability for GFx heap, but as it seems now this is required to not get memory corruption
	// (memory locking, maybe? GFx allows disabling thread safety of its heap)
	// TODO: figure this out
	auto memoryHeapPt = hook::get_address<void**>(hook::get_call(hook::get_pattern<char>("41 F6 06 04 75 03 83 CD 20", 12)) + 0x19);
	memoryHeapPt[9] = Alloc_Align;
	memoryHeapPt[10] = Alloc;
	memoryHeapPt[11] = Realloc;
	memoryHeapPt[12] = Free;
	memoryHeapPt[13] = AllocAuto_Align;
	memoryHeapPt[14] = AllocAuto;
	memoryHeapPt[15] = GetHeap;

	// always enable locking for GFx heap
	// doesn't fix it, oddly - probably singlethreaded non-atomic reference counts?
	//hook::put<uint8_t>(hook::get_pattern("24 01 41 88 87 C0 00 00 00 41 8B"), 0xB0);
});
