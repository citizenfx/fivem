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

// set while console listeners are being called, so output printed by a listener isn't passed to listeners again
static thread_local bool g_inPrintListener;

static bool IsPrintListenerRegistered(const std::string& resourceName, const std::string& ref)
{
	std::shared_lock<std::shared_mutex> lock(g_printListMutex);

	auto [begin, end] = g_resourcePrintListeners.equal_range(resourceName);

	return std::any_of(begin, end, [&ref](const auto& entry)
	{
		return entry.second == ref;
	});
}

static void LogPrintListener(ConsoleChannel channel, const char* msg)
{
	auto msgFmt = fmt::sprintf("%s", msg);

	{
		std::unique_lock<std::shared_mutex> lock(g_consoleBufferMutex);
		g_consoleBuffer.push_back(msgFmt);
	}

	if (g_inPrintListener)
	{
		return;
	}

	{
		std::shared_lock<std::shared_mutex> lock(g_printListMutex);

		if (g_resourcePrintListeners.empty())
		{
			return;
		}
	}

	// always queue this, even if we're already on the main thread: calling listeners inline would run script code from
	// wherever the print happened, e.g. while a stopping resource is tearing down its script runtimes
	gscomms_execute_callback_on_main_thread([channel = std::move(channel), msgFmt = std::move(msgFmt)]()
	{
		// copy the listener list, as a listener may (indirectly) stop a resource and modify it
		std::vector<std::pair<std::string, std::string>> listeners;

		{
			std::shared_lock<std::shared_mutex> lock(g_printListMutex);
			listeners.assign(g_resourcePrintListeners.begin(), g_resourcePrintListeners.end());
		}

		g_inPrintListener = true;

		for (const auto& [resourceName, ref] : listeners)
		{
			// skip listeners that got removed by an earlier listener
			if (!IsPrintListenerRegistered(resourceName, ref))
			{
				continue;
			}

			fx::ResourceManager::GetCurrent()->CallReference<void>(ref, channel, msgFmt);
		}

		g_inPrintListener = false;
	},
	true);
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
