#include <StdInc.h>

#include <CoreConsole.h>
#include <ScriptEngine.h>

#include <fxScripting.h>

#include <Resource.h>
#include <ResourceManager.h>

#include <boost/circular_buffer.hpp>

#include <GameServerComms.h>

#include <sstream>

#include <regex>

static std::shared_mutex g_consoleBufferMutex;
static boost::circular_buffer<std::string> g_consoleBuffer(1500);

struct ConsoleListenerEntry
{
	std::string handlerRef;
	std::optional<std::regex> channelFilter;
	std::optional<std::regex> messageFilter;
};

static std::shared_mutex g_printListMutex;
static std::unordered_multimap<std::string, ConsoleListenerEntry> g_resourcePrintListeners;

static void LogPrintListener(ConsoleChannel channel, const char* msg)
{
	const std::string msgFmt = fmt::sprintf("%s", msg);

	{
		std::unique_lock<std::shared_mutex> lock(g_consoleBufferMutex);
		g_consoleBuffer.push_back(msgFmt);
	}

	if (g_resourcePrintListeners.empty())
		return;
		
	gscomms_execute_callback_on_main_thread([channel = std::move(channel), msgFmt = std::move(msgFmt)]()
	{
		std::unique_lock<std::shared_mutex> lock(g_printListMutex);
		
		for (auto& [resourceName, entry] : g_resourcePrintListeners)
		{
			const bool channelMatch = !entry.channelFilter || std::regex_search(channel, *entry.channelFilter);
			const bool messageMatch = !entry.messageFilter || std::regex_search(msgFmt, *entry.messageFilter);

			if (channelMatch && messageMatch) {
				lock.unlock();

				fx::ResourceManager::GetCurrent()->CallReference<void>(entry.handlerRef, channel, msgFmt);

				lock.lock();
			}
		}
	});
}

static void AddConsoleListener(fx::Resource* resource, ConsoleListenerEntry entry)
{
    {
        std::unique_lock<std::shared_mutex> lock(g_printListMutex);
        g_resourcePrintListeners.emplace(resource->GetName(), std::move(entry));
    }

    resource->OnStop.Connect([resourceName = resource->GetName()]()
    {
        std::unique_lock<std::shared_mutex> lock(g_printListMutex);
        g_resourcePrintListeners.erase(resourceName);
    });
}

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("REGISTER_CONSOLE_LISTENER", [](fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (!FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime))) 
			return;

		auto* resource = static_cast<fx::Resource*>(runtime->GetParentObject());
		if (!resource) 
			return;

		ConsoleListenerEntry entry;
		entry.handlerRef = context.CheckArgument<const char*>(0);

		AddConsoleListener(resource, std::move(entry));
	});

	fx::ScriptEngine::RegisterNativeHandler("REGISTER_CONSOLE_LISTENER_WITH_FILTERS", [](fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (!FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime))) 
			return;

		auto* resource = static_cast<fx::Resource*>(runtime->GetParentObject());
		if (!resource) 
			return;

		std::string handlerRef = context.CheckArgument<const char*>(0);
		auto channelFilterRaw = context.GetArgument<const char*>(1);
		auto messageFilterRaw = context.GetArgument<const char*>(2);

		ConsoleListenerEntry entry;
		entry.handlerRef = handlerRef;

		auto tryEmplaceRegex = [](std::optional<std::regex>& target, const char* pattern, const char* name) {
			if (pattern && *pattern)
			{
				try
				{
					target.emplace(pattern);
				}
				catch (const std::regex_error& e)
				{
					throw std::runtime_error(va("Invalid %s regex '%s': %s", name, pattern, e.what()));
				}
			}
		};

		tryEmplaceRegex(entry.channelFilter, channelFilterRaw, "channel");
		tryEmplaceRegex(entry.messageFilter, messageFilterRaw, "message");

		AddConsoleListener(resource, std::move(entry));
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
