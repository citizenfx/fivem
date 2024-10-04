#include "StdInc.h"
#include "Hooking.h"
#include "ScriptEngine.h"

static char* g_drawRects;
static int g_drawRectsSize;
static int* g_mainThreadFrameIndex;

static hook::cdecl_stub<char*(void*)> _allocateDrawRect([]()
{
	return hook::get_call(hook::get_pattern("F3 44 0F 59 0D ? ? ? ? E8 ? ? ? ? 33 FF", 9));
});

static hook::cdecl_stub<void(void*, float a2, float a3, float a4, float a5)> _setDrawRectCoords([]()
{
	return hook::get_call(hook::get_pattern("F3 44 0F 59 0D ? ? ? ? E8 ? ? ? ? 33 FF", 0x45));
});

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("DRAW_RECT_ROTATED", [](fx::ScriptContext& context)
	{
		if (auto rect = _allocateDrawRect(&g_drawRects[g_drawRectsSize * *g_mainThreadFrameIndex]))
		{
			float x = context.GetArgument<float>(0);
			float y = context.GetArgument<float>(1);
			float w = context.GetArgument<float>(2);
			float h = context.GetArgument<float>(3);

			uint32_t color = (
				(std::clamp(context.GetArgument<int>(8), 0, 255) << 24) | 
				(std::clamp(context.GetArgument<int>(5), 0, 255) << 16) | 
				(std::clamp(context.GetArgument<int>(6), 0, 255) << 8) | 
				(std::clamp(context.GetArgument<int>(7), 0, 255) << 0)
			);

			_setDrawRectCoords(rect, x - (w / 2), y - (h / 2), x + (w / 2), y + (h / 2));
			*(void**)(rect + 0) = nullptr; // texture
			*(uint32_t*)(rect + 52) &= 0xF4;
			*(uint32_t*)(rect + 52) |= 0x84;
			*(float*)(rect + 28) = context.GetArgument<float>(4) * 0.017453292; // deg2rad
			*(uint32_t*)(rect + 40) = color;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("DRAW_LINE_2D", [](fx::ScriptContext& context)
	{
		if (auto rect = _allocateDrawRect(&g_drawRects[g_drawRectsSize * *g_mainThreadFrameIndex]))
		{
			float x1 = context.GetArgument<float>(0);
			float y1 = context.GetArgument<float>(1);
			float x2 = context.GetArgument<float>(2);
			float y2 = context.GetArgument<float>(3);

			uint32_t color = (
				(std::clamp(context.GetArgument<int>(8), 0, 255) << 24) | 
				(std::clamp(context.GetArgument<int>(5), 0, 255) << 16) | 
				(std::clamp(context.GetArgument<int>(6), 0, 255) << 8) | 
				(std::clamp(context.GetArgument<int>(7), 0, 255) << 0)
			);

			_setDrawRectCoords(rect, x1, y1, x2, y2);
			*(uint32_t*)(rect + 52) &= 0xFA;
			*(uint32_t*)(rect + 52) |= 0x8A;
			*(float*)(rect + 28) = context.GetArgument<float>(4);
			*(uint32_t*)(rect + 40) = color;
		}
	});
});

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern<char>("F3 44 0F 59 0D ? ? ? ? E8 ? ? ? ? 33 FF", -0x4D);
		g_drawRects = hook::get_address<char*>(location + 0x30, 3, 7);
		g_drawRectsSize = *(int*)(location + 0x2C);
		g_mainThreadFrameIndex = hook::get_address<int*>(location + 12, 3, 7);
	}
});
