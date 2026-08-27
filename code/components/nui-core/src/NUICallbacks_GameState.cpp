/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIApp.h"

#include <HostSharedData.h>
#include <NuiGameState.h>

#include "memdbgon.h"

static bool ReadGameState(NuiGameStateData& out)
{
	static HostSharedData<NuiGameState> gameState("CfxNuiGameState");

	for (int i = 0; i < 16; i++)
	{
		auto sequence = gameState->sequence.load(std::memory_order_acquire);

		// odd: a write is in progress
		if (sequence & 1)
		{
			continue;
		}

		out = gameState->data;

		std::atomic_thread_fence(std::memory_order_acquire);

		if (gameState->sequence.load(std::memory_order_relaxed) == sequence)
		{
			// 0 means the game process never wrote a frame
			return sequence != 0;
		}
	}

	return false;
}

static CefRefPtr<CefV8Value> CreateVector(const float (&value)[3])
{
	auto object = CefV8Value::CreateObject(nullptr, nullptr);

	object->SetValue("x", CefV8Value::CreateDouble(value[0]), V8_PROPERTY_ATTRIBUTE_NONE);
	object->SetValue("y", CefV8Value::CreateDouble(value[1]), V8_PROPERTY_ATTRIBUTE_NONE);
	object->SetValue("z", CefV8Value::CreateDouble(value[2]), V8_PROPERTY_ATTRIBUTE_NONE);

	return object;
}

static InitFunction initFunction([]()
{
	auto nuiApp = Instance<NUIApp>::Get();

	nuiApp->AddV8Handler("getGameState", [](const CefV8ValueList& arguments, CefString& exception) -> CefRefPtr<CefV8Value>
	{
		NuiGameStateData data;

		if (!ReadGameState(data))
		{
			return CefV8Value::CreateNull();
		}

		auto ped = CefV8Value::CreateObject(nullptr, nullptr);
		ped->SetValue("position", CreateVector(data.pedPos), V8_PROPERTY_ATTRIBUTE_NONE);

		auto camera = CefV8Value::CreateObject(nullptr, nullptr);
		camera->SetValue("position", CreateVector(data.camPos), V8_PROPERTY_ATTRIBUTE_NONE);
		camera->SetValue("rotation", CreateVector(data.camRot), V8_PROPERTY_ATTRIBUTE_NONE);
		camera->SetValue("forward", CreateVector(data.camForward), V8_PROPERTY_ATTRIBUTE_NONE);
		camera->SetValue("up", CreateVector(data.camUp), V8_PROPERTY_ATTRIBUTE_NONE);
		camera->SetValue("fov", CefV8Value::CreateDouble(data.camFov), V8_PROPERTY_ATTRIBUTE_NONE);

		auto state = CefV8Value::CreateObject(nullptr, nullptr);
		state->SetValue("ped", ped, V8_PROPERTY_ATTRIBUTE_NONE);
		state->SetValue("camera", camera, V8_PROPERTY_ATTRIBUTE_NONE);

		return state;
	});
}, 1);
