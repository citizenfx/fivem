#include "StdInc.h"
#include "Hooking.h"

#include "Streaming.h"

#include <Error.h>

#include <atArray.h>

#define RAGE_FORMATS_IN_GAME

#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE

#include <phBound.h>

#include <MinHook.h>

static int(*g_origInsertModule)(void*, void*);

static thread_local std::string g_currentStreamingName;

std::string GetCurrentStreamingName()
{
	return g_currentStreamingName;
}

class strStreamingModule
{
public:
	virtual ~strStreamingModule() {}

public:
	uint32_t baseIdx;
	uint32_t pad;
	uint32_t unk1;
	atArray<char> name;
};

static void CallBeforeStreamingLoad(strStreamingModule* strModule, uint32_t index, void* data)
{
	uint32_t moduleBase = strModule->baseIdx;

	g_currentStreamingName = streaming::GetStreamingNameForIndex(moduleBase + index);
}

static void CallAfterStreamingLoad(strStreamingModule* strModule, uint32_t index, void* data)
{
	g_currentStreamingName = "";
}

static int InsertStreamingModuleWrap(void* moduleMgr, void* strModule)
{
	void** vt = *(void***)strModule;
	
	struct StreamingOnLoadStub : public jitasm::Frontend
	{
		void* m_origFunc;

		StreamingOnLoadStub(void* origFunc)
			: m_origFunc(origFunc)
		{

		}

		virtual void InternalMain() override
		{
			push(rbx);
			push(rsi);
			push(rbp);
			push(rdi);

			sub(rsp, 0x28);

			mov(rbx, rcx); // streaming module
			mov(rsi, rdx); // index in module
			mov(rbp, r8);  // data pointer
			mov(rdi, r9);  // unknown

			mov(rax, (uintptr_t)CallBeforeStreamingLoad);
			call(rax);

			mov(rcx, rbx);
			mov(rdx, rsi);
			mov(r8, rbp);
			mov(r9, rdi);

			mov(rax, (uintptr_t)m_origFunc);
			call(rax);

			mov(rcx, rbx);
			mov(rdx, rsi);
			mov(r8, rbp);

			mov(rax, (uintptr_t)CallAfterStreamingLoad);
			call(rax);

			add(rsp, 0x28);

			pop(rdi);
			pop(rbp);
			pop(rsi);
			pop(rbx);

			ret();
		}
	};

	auto stub = new StreamingOnLoadStub(vt[6]);
	vt[6] = stub->GetCode();

	return g_origInsertModule(moduleMgr, strModule);
}

static void PolyError(const std::string& str, fmt::ArgList args)
{
	if (g_currentStreamingName.find(".yft") != std::string::npos)
	{
		FatalError("Physics validation failed for asset %s.\nThis asset is **INVALID**. Please remove it, or fix the exporter used to export it.\nDetails: %s",
			g_currentStreamingName, fmt::sprintf(str, args));
	}
	else
	{
		trace("Physics validation failed for asset %s.\nThis asset is **INVALID**. Please remove it, or fix the exporter used to export it.\nDetails: %s",
			g_currentStreamingName, fmt::sprintf(str, args));
	}
}

FMT_VARIADIC(void, PolyError, const std::string&);

static void ValidateGeometry(void* geomPtr)
{
	// only validate #ft files for the time being.
	// #bn/#dr files tend to be exported with GIMS Evo, which inherently exports broken data
	// (edges point to vertices, not polygons; this is *plain wrong*)

	// also, slow PCs don't like validating collisions, that makes people fall through the ground
	if (g_currentStreamingName.find(".yft") == std::string::npos)
	{
		return;
	}

	auto geom = (rage::five::phBoundGeometry*)geomPtr;
	auto polys = geom->GetPolygons();
	auto numPolys = geom->GetNumPolygons();
	auto numVerts = geom->GetNumVertices();

	for (size_t i = 0; i < numPolys; i++)
	{
		auto poly = &polys[i];

		if (poly->type == 0)
		{
#define CHECK_EDGE(e, a, b) \
	if (poly->poly.e != -1) \
	{ \
		if (poly->poly.e >= numPolys) \
		{ \
			PolyError("Poly %d has edge %d which is > %d", i, poly->poly.e, numPolys); \
		} \
		\
		{ \
			auto& p = polys[poly->poly.e]; \
			if (p.type == 0) \
			{ \
				uint16_t v = (p.poly.v1 & 0x7FFF) + (p.poly.v2 & 0x7FFF) + (p.poly.v3 & 0x7FFF) - (poly->poly.a & 0x7FFF) - (poly->poly.b & 0x7FFF); \
				\
				if (v >= numVerts) \
				{ \
					PolyError("Poly %d edge reference is invalid. It leads to vertex %d, when there are only %d vertices.", i, v, numVerts); \
				} \
			} \
		} \
	}

			CHECK_EDGE(e1, v1, v2);
			CHECK_EDGE(e2, v2, v3);
			CHECK_EDGE(e3, v1, v3);

#undef CHECK_EDGE
		}
	}
}

static void(*g_origBVHThing)(void*, void*);

static void DoBVHThing(char* a1, void* a2)
{
	g_origBVHThing(a1, a2);

	ValidateGeometry(a1);
}

static void(*g_origGeometryThing)(void*, void*);

static void DoGeometryThing(char* a1, void* a2)
{
	g_origGeometryThing(a1, a2);

	ValidateGeometry(a1);
}

static HookFunction hookFunction([]()
{
	MH_Initialize();

	{
		auto insertModule = hook::get_pattern("76 16 48 8B 41 18 44 0F B7 41 20 4E", -15);
		MH_CreateHook(insertModule, InsertStreamingModuleWrap, (void**)&g_origInsertModule);
	}

	MH_EnableHook(MH_ALL_HOOKS);

	// phBoundBVH geometry check
	{
		auto location = hook::get_pattern("EB 4E 48 8B D1 48 8B CB E8", 8);
		hook::set_call(&g_origBVHThing, location);

		hook::call(location, DoBVHThing);
	}

	// phBoundGeometry check
	{
		auto location = hook::get_pattern("EB 4E 48 8B D1 48 8B CB E8", 21);
		hook::set_call(&g_origGeometryThing, location);

		hook::call(location, DoGeometryThing);
	}
});
