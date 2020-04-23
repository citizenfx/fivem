#include <StdInc.h>
#include <ScriptEngine.h>
#include <Hooking.h>
#include <MinHook.h>

struct DrawOrigin
{
	float posX; // +0
	float posY; // +4
	float posZ; // +8
	float posPad;  // +12
	int unkIndex; // +16
	char pad[12]; // +20
}; // 32

enum DrawType
{
	Line = 0,
	Poly,
};

struct Vector3
{
	float x, y, z;
};

struct DrawRequest
{
	DrawType type;
	uint32_t color;
	Vector3 first;
	Vector3 second;
	Vector3 third;
};

constexpr int NUM_MAX_DRAW_REQUESTS = 1024;

static DrawRequest drawRequests[NUM_MAX_DRAW_REQUESTS];

static int drawRequestsCount = 0;

static uint32_t ConvertRGBAToHex(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
	return ((alpha & 0xFF) << 24) + ((blue & 0xFF) << 16) + ((green & 0xFF) << 8) + (red & 0xFF);
}

static hook::cdecl_stub<void*(uint64_t points, int pointsAmount, void* a3, bool filling, uint32_t drawColor)> g_drawPoints([]()
{
	return hook::pattern("0F 57 F6 44 0F 29 40 B8 44 0F 29 48 A8 45 0F 57 C9").count(1).get(0).get<void>(-0x37);
});

static void(*g_origScriptImRenderPhase)();
static void ScriptImRenderPhase()
{
	if (drawRequestsCount > 0)
	{
		for (int i = 0; i < drawRequestsCount; i++)
		{
			DrawRequest& data = drawRequests[i];

			switch (data.type)
			{
			case Line:
			{
				float points[8] = {
					data.first.x, data.first.y, data.first.z, 0.0f,
					data.second.x, data.second.y, data.second.z, 0.0f,
				};

				g_drawPoints((uint64_t)&points, 2, 0, false, data.color);
				break;
			}
			case Poly:
			{
				float points[12] = {
					data.first.x, data.first.y, data.first.z, 0.0f,
					data.second.x, data.second.y, data.second.z, 0.0f,
					data.third.x, data.third.y, data.third.z, 0.0f,
				};

				g_drawPoints((uint64_t)&points, 3, 0, true, data.color);
				break;
			}
			}
		}

		drawRequestsCount = 0;
	}

	g_origScriptImRenderPhase();
}

static bool(*isGamePaused)();

static int* g_frameDrawIndex1;
static int* g_frameDrawIndex2;
static void* g_drawOriginStore;

static uint32_t* g_scriptDrawOrigin;

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern<char>("48 8B D9 F3 0F 10 79 0C 44 0F 29 40 A8");

		g_frameDrawIndex1 = hook::get_address<int*>(location + 80);
		g_frameDrawIndex2 = hook::get_address<int*>(location + 93);
		g_drawOriginStore = hook::get_address<void*>(location + 110);

		hook::set_call(&isGamePaused, location + 73);
	}

	{
		auto location = hook::get_pattern<char>("89 41 34 8B 05 ? ? ? ? 89 41 30 8A 05");
		g_scriptDrawOrigin = hook::get_address<uint32_t*>(location + 5);
	}

	fx::ScriptEngine::RegisterNativeHandler("SET_DRAW_ORIGIN", [](fx::ScriptContext& context)
	{
		auto drawIndex = *((isGamePaused()) ? g_frameDrawIndex2 : g_frameDrawIndex1);
		auto store = ((uint64_t)g_drawOriginStore + 1040 * drawIndex);
		auto usedIndexes = *(uint32_t*)(store + 1024);

		if (usedIndexes >= 32)
		{
			return;
		}

		*(uint32_t*)(store + 1024) = usedIndexes + 1;

		DrawOrigin data;
		data.posX = context.GetArgument<float>(0);
		data.posY = context.GetArgument<float>(1);
		data.posZ = context.GetArgument<float>(2);
		data.unkIndex = context.GetArgument<int>(3);

		*(DrawOrigin*)(store + 32 * usedIndexes) = data;
		*g_scriptDrawOrigin = usedIndexes;
	});

	fx::ScriptEngine::RegisterNativeHandler("CLEAR_DRAW_ORIGIN", [](fx::ScriptContext& context)
	{
		*g_scriptDrawOrigin = -1;
	});

	fx::ScriptEngine::RegisterNativeHandler("DRAW_LINE", [](fx::ScriptContext& context)
	{
		if (drawRequestsCount >= NUM_MAX_DRAW_REQUESTS)
		{
			return;
		}

		auto fromX = context.GetArgument<float>(0);
		auto fromY = context.GetArgument<float>(1);
		auto fromZ = context.GetArgument<float>(2);

		auto toX = context.GetArgument<float>(3);
		auto toY = context.GetArgument<float>(4);
		auto toZ = context.GetArgument<float>(5);

		auto red = context.GetArgument<int>(6);
		auto green = context.GetArgument<int>(7);
		auto blue = context.GetArgument<int>(8);
		auto alpha = context.GetArgument<int>(9);

		DrawRequest req;
		req.type = Line;
		req.first = { fromX, fromY, fromZ };
		req.second = { toX, toY, toZ };
		req.color = ConvertRGBAToHex(red, green, blue, alpha);

		drawRequests[drawRequestsCount++] = req;
	});

	fx::ScriptEngine::RegisterNativeHandler("DRAW_POLY", [](fx::ScriptContext& context)
	{
		if (drawRequestsCount >= NUM_MAX_DRAW_REQUESTS)
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

		auto red = context.GetArgument<int>(9);
		auto green = context.GetArgument<int>(10);
		auto blue = context.GetArgument<int>(11);
		auto alpha = context.GetArgument<int>(12);

		DrawRequest req;
		req.type = Poly;
		req.first = { firstX, firstY, firstZ };
		req.second = { secondX, secondY, secondZ };
		req.third = { thirdX, thirdY, thirdZ };
		req.color = ConvertRGBAToHex(red, green, blue, alpha);

		drawRequests[drawRequestsCount++] = req;
	});

	MH_Initialize();
	MH_CreateHook(hook::get_pattern("33 C9 E8 ? ? ? ? E8 ? ? ? ? 33 C9 E8 ? ? ? ? 0F", -0x28), ScriptImRenderPhase, (void**)&g_origScriptImRenderPhase);
	MH_EnableHook(MH_ALL_HOOKS);
});
