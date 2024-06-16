#include <StdInc.h>
#include <array>
#include <ScriptEngine.h>
#include <GamePrimitives.h>
#include <CfxRGBA.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>

struct scrVector
{
	float x;
	int _pad;
	float y;
	int _pad2;
	float z;
	int _pad3;
};

struct WorldhorizonManager
{
	char m_unknown;
	char m_disableRendering;

	// etc...
};

struct SphereGlowCommand // Actually inherited from DrawCommand
{
	uint32_t m_dataOffset : 12;
	uint32_t m_commandIdx : 8;
	uint32_t m_unkHeader : 12;
	char pad1[12];
	rage::Vec3V m_position;
	float m_radius;
	float m_intensity;
	uint32_t m_color;
	char pad2[4];
	bool m_invert; // Not sure, it seems invert render
	bool m_marker; // Is a "marker" or "overlay"
	char pad3[14];

	SphereGlowCommand()
		: m_position(0.0f, 0.0f, 0.0f), m_radius(0.0f), m_intensity(0.0f), m_color(0), m_invert(false), m_marker(false)
	{
		m_dataOffset = offsetof(SphereGlowCommand, m_position); // This might be correct for any other draw command
		m_commandIdx = 0xEA; // Draw command idx taken from "DRAW_MARKER_SPHERE" native
		m_unkHeader = 0;

		memset(pad1, 0, sizeof(pad1));
		memset(pad2, 0, sizeof(pad2));
		memset(pad3, 0, sizeof(pad3));
	}
};

static WorldhorizonManager* g_worldhorizonMgr;

static constexpr size_t kMaxSphereDrawRequests = 256;
static std::array<SphereGlowCommand, kMaxSphereDrawRequests> g_sphereDrawRequests{};
static uint16_t g_sphereDrawRequestCount = 0;

static void (*g_origRenderGameGlows)();
static void RenderGameGlows()
{
	g_origRenderGameGlows();

	if (g_sphereDrawRequestCount == 0)
	{
		return;
	}

	const auto drawCommandBuffer = rage::dlDrawCommandBuffer::GetInstance();
	if (!drawCommandBuffer)
	{
		return;
	}

	for (int i = 0; i < g_sphereDrawRequestCount; i++)
	{
		drawCommandBuffer->AlignBuffer(16);

		auto command = rage::dlDrawCommandBuffer::AllocateDrawCommand(sizeof(SphereGlowCommand));

		if (command)
		{
			memcpy(command, &g_sphereDrawRequests[i], sizeof(SphereGlowCommand));
		}
	}

	memset(&g_sphereDrawRequests, 0, sizeof(SphereGlowCommand) * g_sphereDrawRequestCount);
	g_sphereDrawRequestCount = 0;
}

static HookFunction hookFunction([]()
{
	static_assert(sizeof(SphereGlowCommand) == 64);

	g_worldhorizonMgr = hook::get_address<WorldhorizonManager*>(hook::get_pattern("83 C8 FF 48 8D 0D ? ? ? ? 89 44 24 38", 6));

	g_origRenderGameGlows = hook::trampoline(hook::get_call(hook::get_pattern("E8 ? ? ? ? F6 C3 04 74 05")), RenderGameGlows);
});

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_WORLD_COORD_FROM_SCREEN_COORD", [](fx::ScriptContext& context)
	{
		float screenX = context.GetArgument<float>(0);
		float screenY = context.GetArgument<float>(1);

		using namespace DirectX;
		rage::Vec3V start = Unproject((*g_viewportGame)->viewport, rage::Vec3V{ screenX, screenY, 0.0f });
		rage::Vec3V end = Unproject((*g_viewportGame)->viewport, rage::Vec3V{ screenX, screenY, 1.0f });

		auto startVector = XMLoadFloat3((XMFLOAT3*)&start);
		auto endVector = XMLoadFloat3((XMFLOAT3*)&end);
		auto normalVector = XMVector3Normalize(XMVectorSubtract(endVector, startVector));

		scrVector* worldOut = context.GetArgument<scrVector*>(2);
		scrVector* normalOut = context.GetArgument<scrVector*>(3);

		worldOut->x = start.x;
		worldOut->y = start.y;
		worldOut->z = start.z;

		normalOut->x = XMVectorGetX(normalVector);
		normalOut->y = XMVectorGetY(normalVector);
		normalOut->z = XMVectorGetZ(normalVector);
	});
	
	fx::ScriptEngine::RegisterNativeHandler("DISABLE_WORLDHORIZON_RENDERING", [](fx::ScriptContext& context)
	{
		auto flag = context.GetArgument<bool>(0);

		if (g_worldhorizonMgr)
		{
			g_worldhorizonMgr->m_disableRendering = flag;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("DRAW_GLOW_SPHERE", [](fx::ScriptContext& context)
	{
		if (g_sphereDrawRequestCount >= kMaxSphereDrawRequests)
		{
			return;
		}

		auto posX = context.GetArgument<float>(0);
		auto posY = context.GetArgument<float>(1);
		auto posZ = context.GetArgument<float>(2);
		auto radius = context.GetArgument<float>(3);
		auto colorR = context.GetArgument<uint8_t>(4);
		auto colorG = context.GetArgument<uint8_t>(5);
		auto colorB = context.GetArgument<uint8_t>(6);
		auto intensity = context.GetArgument<float>(7);
		auto invert = context.GetArgument<bool>(8);
		auto marker = context.GetArgument<bool>(9);

		SphereGlowCommand command;
		command.m_position = rage::Vec3V{ posX, posY, posZ };
		command.m_radius = radius;
		command.m_color = CRGBA(colorR, colorG, colorB, 255).AsARGB();
		command.m_intensity = intensity;
		command.m_invert = invert;
		command.m_marker = marker;

		g_sphereDrawRequests[g_sphereDrawRequestCount++] = command;
	});
});
