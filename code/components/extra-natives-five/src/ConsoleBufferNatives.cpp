#include "StdInc.h"

#include <ScriptEngine.h>
#include <ConsoleHost.h>

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_CONSOLE_BUFFER", [](fx::ScriptContext& context)
	{
		const int requested = context.GetArgument<int>(0);
		const size_t maxLines = (requested <= 0) ? 0 : static_cast<size_t>(requested);

		static thread_local std::string buffer;
		buffer = ConHost::GetBufferText(maxLines);
		context.SetResult<const char*>(buffer.c_str());
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_CONSOLE_OPEN", [](fx::ScriptContext& context)
	{
		context.SetResult<bool>(ConHost::IsConsoleOpen());
	});
});
