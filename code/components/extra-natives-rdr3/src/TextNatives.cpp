#include <StdInc.h>
#include <ScriptEngine.h>
#include <Hooking.h>

static int* g_textFontForCurrentCommand;

static HookFunction textNativesFunc([]()
{
	g_textFontForCurrentCommand = hook::get_address<int*>(hook::get_pattern("48 69 C1 08 7D 00 00", 17));

	fx::ScriptEngine::RegisterNativeHandler("SET_TEXT_FONT_FOR_CURRENT_COMMAND", [](fx::ScriptContext& scriptContext)
	{
		auto fontId = scriptContext.GetArgument<int>(0);

		*g_textFontForCurrentCommand = fontId;
	});
});
