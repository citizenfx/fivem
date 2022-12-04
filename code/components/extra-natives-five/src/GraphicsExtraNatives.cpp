#include <StdInc.h>

#include <ScriptEngine.h>

#include <GamePrimitives.h>

struct scrVector
{
	float x;
	int _pad;
	float y;
	int _pad2;
	float z;
	int _pad3;
};

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
});
