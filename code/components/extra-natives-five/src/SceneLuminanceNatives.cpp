#include "StdInc.h"

#include <SceneLuminance.h>
#include <ScriptEngine.h>
#include <ScriptSerialization.h>

#include <algorithm>

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_SCENE_LUMINANCE", [](fx::ScriptContext& context)
	{
		const float x = context.GetArgument<float>(0);
		const float y = context.GetArgument<float>(1);
		const float width = context.GetArgument<float>(2);
		const float height = context.GetArgument<float>(3);

		const float left = std::clamp(x, 0.0f, 1.0f);
		const float top = std::clamp(y, 0.0f, 1.0f);
		const float right = std::clamp(x + width, 0.0f, 1.0f);
		const float bottom = std::clamp(y + height, 0.0f, 1.0f);

		float value = 0;

		if (left < right && top < bottom && GetSceneLuminance(left, top, right, bottom, value))
		{
			context.SetResult(fx::SerializeObject(value));
			return;
		}

		context.SetResult(fx::SerializeObject(msgpack::type::nil_t{}));
	});
});
