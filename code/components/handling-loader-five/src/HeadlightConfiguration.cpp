#include "StdInc.h"
#include "Hooking.h"

#include <ScriptEngine.h>
#include <ICoreGameInit.h>

static hook::cdecl_stub<void(void*, int, int, float, int, void*, void*)> setPartRotation([]()
{
	return hook::get_call(hook::get_pattern("41 83 E0 07 48 89 44 24 30 48 83", 20));
});

static std::map<uint32_t, std::tuple<float, float, bool>> g_headlightConfiguration;

static void CustomHandleHeadlights(char* vehicle)
{
	// run the function
	char* modelData = *(char**)(vehicle + 32);

	char* unkBit = **(char***)(modelData + 176);

	// light index?
	int lightIndex = *(char*)(unkBit + 277); // VERSION-SPECIFIC: 505

	if (lightIndex < 0)
	{
		// no light
		*(float*)(vehicle + 740) = 1.0f; // 505 specific
	}
	else
	{
		// default values
		float rate = 3.0f;
		float rotation = 7.0f;
		bool inverse = true;

		// is this in any configuration? if so, overwrite values
		{
			uint32_t mi = *(uint32_t*)(modelData + 24);

			auto it = g_headlightConfiguration.find(mi);

			if (it != g_headlightConfiguration.end())
			{
				rate = std::get<0>(it->second);
				rotation = std::get<1>(it->second);
				inverse = std::get<bool>(it->second);
			}
			else if (mi == HashString("tropos"))
			{
				rate = 30.0f;
				rotation = 40.0f;
				inverse = false;
			}
		}

		// hardcoded 505
		float deltaTime = *(float*)0x142ACFF94;
		float step = (rate / rotation) * deltaTime;

		// if paused, don't step at all
		if (*(char*)0x142ACFF37 || *(char*)0x142ACFF38 || *(char*)0x142ACFF39 || *(char*)0x142ACFF3B)
		{
			step = 0.0f;
		}

		// should the headlights be up or down?
		bool shouldBeUp = false;

		if (*(uint8_t*)(vehicle + 2114) & 0x10)
		{
			if (*(uint8_t*)(vehicle + 2112) & 0xC0)
			{
				shouldBeUp = true;
			}
			else if ((*(uint8_t*)(vehicle + 2320) & 7) == 2)
			{
				shouldBeUp = true;
			}
		}

		// rotate up/down
		uint32_t stage = *(uint32_t*)(vehicle + 736);

		// new rotation value
		float rotationValue = *(float*)(vehicle + 740);

		switch (stage)
		{
			// stage: down
			case 0:
				if (shouldBeUp)
				{
					// initiate moving upwards
					stage = 2;
				}

				rotationValue = 0.0f;

				break;
			// stage: up
			case 1:
				if (!shouldBeUp)
				{
					// initiate moving downwards
					stage = 3;
				}

				rotationValue = 1.0f;

				break;
			// stage: moving up
			case 2:
				rotationValue += step;

				// are we up already?
				if (rotationValue >= 1.0f)
				{
					stage = 1;
					rotationValue = 1.0f;
				}

				break;
			// stage: moving down
			case 3:
				rotationValue -= step;

				// are we down already?
				if (rotationValue <= 0.0f)
				{
					stage = 0;
					rotationValue = 0.0f;
				}

				break;
		}

		// store new values
		*(uint32_t*)(vehicle + 736) = stage;
		*(float*)(vehicle + 740) = rotationValue;

		// compute the actual rotation
		float newRotation = (rotationValue * rotation * 0.017453292f);

		if (inverse)
		{
			newRotation = -newRotation;
		}

		// set part rotation
		setPartRotation(vehicle, lightIndex, 3, newRotation, 1, nullptr, nullptr);
	}
}

static HookFunction hookFunction([]()
{
	hook::jump(hook::get_pattern("83 FE 03 7C CF 48 8B CF 4C", 0x27), CustomHandleHeadlights);

	Instance<ICoreGameInit>::Get()->OnGameRequestLoad.Connect([]()
	{
		g_headlightConfiguration.clear();
	});
});

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("SET_MODEL_HEADLIGHT_CONFIGURATION", [] (fx::ScriptContext& context)
	{
		uint32_t modelHash = context.GetArgument<uint32_t>(0);
		float step = context.GetArgument<float>(1);
		float rotation = context.GetArgument<float>(2);
		bool inverse = context.GetArgument<bool>(3);

		g_headlightConfiguration[modelHash] = { step, rotation, inverse };

		trace("SET_MODEL_HEADLIGHT_CONFIGURATION: set headlight configuration for %08x to {step: %f deg, rotation: %f deg, inverse: %d}\n", modelHash, step, rotation, (inverse) ? 1 : 0);
	});
});