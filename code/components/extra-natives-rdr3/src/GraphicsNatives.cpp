#include <StdInc.h>
#include <ScriptEngine.h>
#include <Hooking.h>

#include "CfxRGBA.h"
#include "Hooking.Stubs.h"
#include "GamePrimitives.h"

enum ScriptImDrawType : uint32_t
{
	SCRIPT_IM_LINE = 0,
	SCRIPT_IM_POLY,
	SCRIPT_IM_BOX,
	SCRIPT_IM_BACKFACE_CULLING_OFF,
	SCRIPT_IM_BACKFACE_CULLING_ON,
};

struct ScriptImRequest
{
	rage::Vec3V m_first;
	rage::Vec3V m_second;
	rage::Vec3V m_third;
	uint32_t m_color;
	ScriptImDrawType m_type;
	char m_pad[8];
};

static constexpr int kMaxScriptImRequests = 1024;
static std::vector<ScriptImRequest> g_scriptImRequests{};

static ScriptImRequest** g_scriptImBuffer;
static uint16_t* g_scriptImEntriesCount;

static hook::cdecl_stub<void()> initScriptImBuffer([]()
{
	return hook::get_pattern("48 89 5C 24 ? 57 48 83 EC 20 0F B7 05 ? ? ? ? 33 FF");
});

static void(*g_origRenderScriptIm)();
static void RenderScriptIm()
{
	// The game never uses this buffer itself, but it *may* be changed in the future updates.
	// This code isn't ready for such a scenario, so might potentially require tweaks.
	// Like if something allocate script im buffer before us, we would need to resize it
	// and not just rewrite everything with out custom stuff.

	if (g_scriptImRequests.empty())
	{
		return;
	}

	*g_scriptImEntriesCount = g_scriptImRequests.size();

	// If the buffer isn't allocated, let's do it.
	if (!*g_scriptImBuffer)
	{
		initScriptImBuffer();
	}

	// Failed to allocate? Bail out.
	if (!*g_scriptImBuffer)
	{
		*g_scriptImEntriesCount = 0;
		return;
	}

	// Move stuff from our temp vector to the in-game buffer.
	std::move(g_scriptImRequests.begin(), g_scriptImRequests.end(), *g_scriptImBuffer);

	g_origRenderScriptIm();

	g_scriptImRequests.clear();
	*g_scriptImEntriesCount = 0;
}

struct DrawOriginData
{
	rage::Vec3V m_pos;
	bool m_is2d; // +16
	char m_pad[15];
}; // 32

struct DrawOriginStore
{
	DrawOriginData m_items[32];
	uint32_t m_count;
	char pad_404[12];
};

static bool(*isGamePaused)();

static int* g_frameDrawIndex1;
static int* g_frameDrawIndex2;
static DrawOriginStore* g_drawOriginStore;
static uint32_t* g_scriptDrawOriginIndex;

struct WorldhorizonManager
{
	char m_unknown;
	char m_disableRendering;

	// etc...
};

static WorldhorizonManager* g_worldhorizonMgr;

static HookFunction hookFunction([]()
{
	static_assert(sizeof(ScriptImRequest) == 64);
	static_assert(sizeof(DrawOriginData) == 32);
	static_assert(sizeof(DrawOriginStore) == 1040);

	{
		g_worldhorizonMgr = hook::get_address<WorldhorizonManager*>(hook::get_pattern("89 44 24 40 48 8D 0D ? ? ? ? 8B 84 24 90 00", 7));
	}

	{
		auto location = hook::get_pattern<char>("48 69 D0 10 04 00 00 48 8D 05 ? ? ? ? 48 03");

		g_frameDrawIndex1 = hook::get_address<int*>(location - 20);
		g_frameDrawIndex2 = hook::get_address<int*>(location - 7);
		g_drawOriginStore = hook::get_address<DrawOriginStore*>(location + 10);

		hook::set_call(&isGamePaused, location - 27);
	}

	{
		auto location = hook::get_pattern<char>("89 41 34 8B 05 ? ? ? ? 89 41 30 8A 05");
		g_scriptDrawOriginIndex = hook::get_address<uint32_t*>(location + 5);
	}

	{
		auto location = hook::get_pattern<char>("48 89 05 ? ? ? ? E8 ? ? ? ? 66 89 3D");

		g_scriptImBuffer = hook::get_address<ScriptImRequest**>(location + 3);
		g_scriptImEntriesCount = hook::get_address<uint16_t*>(location + 15);
	}

	{
		auto location = hook::get_pattern("33 C9 E8 ? ? ? ? E8 ? ? ? ? 33 C9 E8 ? ? ? ? 0F", -0x28);
		g_origRenderScriptIm = hook::trampoline(location, RenderScriptIm);
	}

	fx::ScriptEngine::RegisterNativeHandler("SET_DRAW_ORIGIN", [](fx::ScriptContext& context)
	{
		auto drawIndex = *(isGamePaused() ? g_frameDrawIndex2 : g_frameDrawIndex1);

		auto& store = g_drawOriginStore[drawIndex];
		const auto index = store.m_count;

		if (index >= 32)
		{
			return;
		}

		DrawOriginData data;
		data.m_pos.x = context.GetArgument<float>(0);
		data.m_pos.y = context.GetArgument<float>(1);
		data.m_pos.z = context.GetArgument<float>(2);
		data.m_is2d = context.GetArgument<bool>(3);

		store.m_items[index] = data;
		store.m_count += 1;

		*g_scriptDrawOriginIndex = store.m_count;
	});

	fx::ScriptEngine::RegisterNativeHandler("CLEAR_DRAW_ORIGIN", [](fx::ScriptContext& context)
	{
		*g_scriptDrawOriginIndex = -1;
	});

	fx::ScriptEngine::RegisterNativeHandler("DRAW_LINE", [](fx::ScriptContext& context)
	{
		if (g_scriptImRequests.size() >= kMaxScriptImRequests)
		{
			return;
		}

		auto fromX = context.GetArgument<float>(0);
		auto fromY = context.GetArgument<float>(1);
		auto fromZ = context.GetArgument<float>(2);

		auto toX = context.GetArgument<float>(3);
		auto toY = context.GetArgument<float>(4);
		auto toZ = context.GetArgument<float>(5);

		auto red = context.GetArgument<uint8_t>(6);
		auto green = context.GetArgument<uint8_t>(7);
		auto blue = context.GetArgument<uint8_t>(8);
		auto alpha = context.GetArgument<uint8_t>(9);

		ScriptImRequest req;
		req.m_type = SCRIPT_IM_LINE;
		req.m_first = { fromX, fromY, fromZ };
		req.m_second = { toX, toY, toZ };
		req.m_color = CRGBA(red, green, blue, alpha).AsABGR();

		g_scriptImRequests.push_back(req);
	});

	fx::ScriptEngine::RegisterNativeHandler("DRAW_POLY", [](fx::ScriptContext& context)
	{
		if (g_scriptImRequests.size() >= kMaxScriptImRequests)
		{
			return;
		}

		auto firstX = context.GetArgument<float>(0);
		auto firstY = context.GetArgument<float>(1);
		auto firstZ = context.GetArgument<float>(2);

		auto secondX = context.GetArgument<float>(3);
		auto secondY = context.GetArgument<float>(4);
		auto secondZ = context.GetArgument<float>(5);

		auto thirdX = context.GetArgument<float>(6);
		auto thirdY = context.GetArgument<float>(7);
		auto thirdZ = context.GetArgument<float>(8);

		auto red = context.GetArgument<uint8_t>(9);
		auto green = context.GetArgument<uint8_t>(10);
		auto blue = context.GetArgument<uint8_t>(11);
		auto alpha = context.GetArgument<uint8_t>(12);

		ScriptImRequest req;
		req.m_type = SCRIPT_IM_POLY;
		req.m_first = { firstX, firstY, firstZ };
		req.m_second = { secondX, secondY, secondZ };
		req.m_third = { thirdX, thirdY, thirdZ };
		req.m_color = CRGBA(red, green, blue, alpha).AsABGR();

		g_scriptImRequests.push_back(req);
	});

	fx::ScriptEngine::RegisterNativeHandler("DRAW_BOX", [](fx::ScriptContext& context)
	{
		if (g_scriptImRequests.size() >= kMaxScriptImRequests)
		{
			return;
		}

		auto fromX = context.GetArgument<float>(0);
		auto fromY = context.GetArgument<float>(1);
		auto fromZ = context.GetArgument<float>(2);

		auto toX = context.GetArgument<float>(3);
		auto toY = context.GetArgument<float>(4);
		auto toZ = context.GetArgument<float>(5);

		auto red = context.GetArgument<uint8_t>(6);
		auto green = context.GetArgument<uint8_t>(7);
		auto blue = context.GetArgument<uint8_t>(8);
		auto alpha = context.GetArgument<uint8_t>(9);

		ScriptImRequest req;
		req.m_type = SCRIPT_IM_BOX;
		req.m_first = { fromX, fromY, fromZ };
		req.m_second = { toX, toY, toZ };
		req.m_color = CRGBA(red, green, blue, alpha).AsABGR();

		g_scriptImRequests.push_back(req);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_BACKFACECULLING", [](fx::ScriptContext& context)
	{
		if (g_scriptImRequests.size() >= kMaxScriptImRequests)
		{
			return;
		}

		auto flag = context.GetArgument<bool>(0);

		ScriptImRequest req;
		req.m_type = flag ? SCRIPT_IM_BACKFACE_CULLING_ON : SCRIPT_IM_BACKFACE_CULLING_OFF;

		g_scriptImRequests.push_back(req);
	});

	fx::ScriptEngine::RegisterNativeHandler("DISABLE_WORLDHORIZON_RENDERING", [](fx::ScriptContext& context)
	{
		auto flag = context.GetArgument<bool>(0);

		if (g_worldhorizonMgr)
		{
			g_worldhorizonMgr->m_disableRendering = flag;
		}
	});
});
