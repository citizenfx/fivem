#include "StdInc.h"
#include "ScriptWarnings.h"

#include "fxScripting.h"

#include "CoreConsole.h"
#include "Resource.h"

namespace fx::scripting
{
void Warningfv(std::string_view channel, std::string_view format, fmt::printf_args argumentList)
{
	std::string message;

	try
	{
		message = fmt::vsprintf(format, argumentList);
	}
	catch (std::exception& e)
	{
		return;
	}

	fx::OMPtr<IScriptRuntime> runtime;
	std::string resourceName = "#unknown";

	if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
	{
		fx::OMPtr<IScriptWarningRuntime> warningRuntime;

		if (FX_SUCCEEDED(runtime.As(&warningRuntime)))
		{
			if (FX_SUCCEEDED(warningRuntime->EmitWarning(const_cast<char*>(std::string{ channel }.c_str()), const_cast<char*>(message.c_str()))))
			{
				return;
			}
		}

		if (auto resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject()))
		{
			resourceName = resource->GetName();
		}
	}

	console::PrintWarning(fmt::sprintf("script:%s:warning:%s", resourceName, channel), "%s", message);
}
}
