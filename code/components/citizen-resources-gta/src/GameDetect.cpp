#include <StdInc.h>
#include <ScriptEngine.h>

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_CURRENT_GAME_NAME", [](fx::ScriptContext& context)
	{
#if defined(GTA_NY)
		context.SetResult<const char*>("gta4");
#elif defined(GTA_FIVE)
		context.SetResult<const char*>("gta5");
#elif defined(IS_RDR3)
		context.SetResult<const char*>("rdr3");
#else
		context.SetResult<const char*>("unk");
#endif
	});
});
