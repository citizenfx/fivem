#include <StdInc.h>
#include <ScriptEngine.h>
#include <HudPos.h>
#include <scrEngine.h>

#include <Hooking.h>

static HookFunction hookFunc([]()
{
	fx::ScriptEngine::RegisterNativeHandler("SET_LOADING_TEXT", [=](fx::ScriptContext& context)
	{
		mbstowcs((wchar_t*)(*hook::get_pattern<void*>("3D 80 00 00 00 73 1E", 12)), context.CheckArgument<const char*>(0), 64);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_HUD_POSITION", [=](fx::ScriptContext& context)
	{
		auto hudPos = HudPositions::GetPosition(context.CheckArgument<const char*>(0));

		if (!hudPos)
		{
			return 0;
		}

		*context.GetArgument<float*>(1) = hudPos->x;
		*context.GetArgument<float*>(2) = hudPos->y;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_HUD_SIZE", [=](fx::ScriptContext& context)
	{
		auto hudPos = HudPositions::GetPosition(context.CheckArgument<const char*>(0));

		if (!hudPos)
		{
			return 0;
		}

		*context.GetArgument<float*>(1) = hudPos->w;
		*context.GetArgument<float*>(2) = hudPos->h;
	});

	fx::ScriptEngine::RegisterNativeHandler("NETWORK_CHANGE_EXTENDED_GAME_CONFIG_CIT", [=](fx::ScriptContext& context)
	{
		int gameConfig[31];

		gameConfig[1] = 16;
		gameConfig[2] = 0;
		gameConfig[3] = 32;
		gameConfig[4] = 0;
		gameConfig[5] = 0;
		gameConfig[6] = 0;
		gameConfig[7] = 0;
		gameConfig[8] = 0;

		gameConfig[9] = -1;
		gameConfig[10] = -1;
		gameConfig[11] = -1;
		gameConfig[12] = 0;
		gameConfig[13] = 1;
		gameConfig[14] = 7;
		gameConfig[15] = 0;
		gameConfig[16] = -1;

		gameConfig[17] = -1;
		gameConfig[18] = -1;
		gameConfig[19] = 1;
		gameConfig[20] = 1;
		gameConfig[21] = 0;
		gameConfig[22] = 1;
		gameConfig[23] = 2;
		gameConfig[24] = 0;

		gameConfig[25] = 1;
		gameConfig[26] = 0;
		gameConfig[27] = 0;
		gameConfig[28] = 1;
		gameConfig[29] = 0;
		gameConfig[30] = 0;

		NativeInvoke::Invoke<0x4CFE3998, int*>(gameConfig);
	});
});
