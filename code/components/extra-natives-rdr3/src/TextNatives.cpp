#include <StdInc.h>
#include <ScriptEngine.h>
#include <Hooking.h>

struct ccolor
{
	uint8_t	r, g, b, a;
};

struct text_info_t
{
	ccolor	color;
	float	text_scale;
	float	text_scale2;
	float	wrap_start;
	float	wrap_end;
	int		font;
	uint8_t pad[0x4];
	WORD	justification;
};

static text_info_t* g_TextInfo;

static HookFunction textNativesFunc([]()
{
	g_TextInfo = hook::get_address<text_info_t*>(hook::get_pattern("48 8D 05 ? ? ? ? 48 89 44 24 ? 8B 05 ? ? ? ? 89 44 24 28 8B 05 ? ? ? ? 89 44 24 20", 3));

	fx::ScriptEngine::RegisterNativeHandler("SET_TEXT_FONT_FOR_CURRENT_COMMAND", [](fx::ScriptContext& scriptContext)
	{
		auto fontId = scriptContext.GetArgument<int>(0);

		g_TextInfo->font = fontId;
	});
	
	fx::ScriptEngine::RegisterNativeHandler("SET_TEXT_JUSTIFICATION", [](fx::ScriptContext& scriptContext)
	{
		/* Text Flags are:
			0 = Centre,
			1 = None/Left,
			2 = Right
		*/
		
		auto justification = scriptContext.GetArgument<int>(0);

		g_TextInfo->justification = justification;
	});
	
	fx::ScriptEngine::RegisterNativeHandler("SET_TEXT_WRAP", [](fx::ScriptContext& scriptContext)
	{
		auto start = scriptContext.GetArgument<float>(0);
		auto end = scriptContext.GetArgument<float>(1);
		
		g_TextInfo->wrap_start = start;
		g_TextInfo->wrap_end = end;
	});
});
