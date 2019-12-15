#include <StdInc.h>

#include <CoreConsole.h>
#include <ScriptEngine.h>

#include <fxScripting.h>

#include <Resource.h>
#include <ResourceManager.h>

#include <boost/circular_buffer.hpp>

#include <GameServerComms.h>

#include <sstream>

static std::shared_mutex g_consoleBufferMutex;
static boost::circular_buffer<std::string> g_consoleBuffer(1500);

static std::shared_mutex g_printListMutex;
static std::multimap<std::string, std::string> g_resourcePrintListeners;

static void LogPrintListener(ConsoleChannel channel, const char* msg)
{
	auto msgFmt = fmt::sprintf("%s", msg);

	{
		std::unique_lock<std::shared_mutex> lock(g_consoleBufferMutex);
		g_consoleBuffer.push_back(msgFmt);
	}

	if (!g_resourcePrintListeners.empty())
	{
		gscomms_execute_callback_on_main_thread([channel = std::move(channel), msgFmt = std::move(msgFmt)]()
		{
			std::unique_lock<std::shared_mutex> lock(g_printListMutex);
			
			for (auto& [resourceName, ref] : g_resourcePrintListeners)
			{
				lock.unlock();

				fx::ResourceManager::GetCurrent()->CallReference<void>(ref, channel, msgFmt);

				lock.lock();
			}
		});
	}
}

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("REGISTER_CONSOLE_LISTENER", [](fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			if (resource)
			{
				std::string handlerRef = context.GetArgument<const char*>(0);

				{
					std::unique_lock<std::shared_mutex> lock(g_printListMutex);
					g_resourcePrintListeners.insert({ resource->GetName(), handlerRef });
				}

				resource->OnStop.Connect([resource]()
				{
					std::unique_lock<std::shared_mutex> lock(g_printListMutex);
					g_resourcePrintListeners.erase(resource->GetName());
				});
			}
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_CONSOLE_BUFFER", [](fx::ScriptContext& context)
	{
		std::stringstream ss;

		{
			std::shared_lock<std::shared_mutex> lock(g_consoleBufferMutex);
			for (const auto& message : g_consoleBuffer)
			{
				ss << message;
			}
		}

		static std::string retStr;
		retStr = ss.str();

		context.SetResult(retStr.c_str());
	});

	console::CoreAddPrintListener(LogPrintListener);
}, INT32_MIN);
