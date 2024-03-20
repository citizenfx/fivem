#include <StdInc.h>

#include <ResourceEventComponent.h>
#include <ResourceManager.h>

#ifndef IS_FXSERVER
#include <NetLibrary.h>
#endif
#include <boost/circular_buffer.hpp>

#include <ConsoleHost.h>
#include <imgui.h>

#include <CoreConsole.h>

struct EventLogEntry
{
	std::string eventName;
	size_t payloadSize;
	bool out;
	std::optional<std::string> eventSource;
};

static boost::circular_buffer<EventLogEntry> g_eventLog(
	// We want a bigger buffer on the server because it will send/receive *a lot* more events, and 150 will quickly get filled
#if IS_FXSERVER
	600
#else
	150
#endif
);
static std::mutex g_eventLogMutex;

static void EventLog_Add(const std::string& eventName, size_t payloadSize, bool out, std::optional<std::string> eventSource = std::nullopt)
{
	EventLogEntry eve;
	eve.eventName = eventName;
	eve.payloadSize = payloadSize;
	eve.out = out;
	// this should always be nullopt on client
	eve.eventSource = eventSource;

	std::unique_lock<std::mutex> lock(g_eventLogMutex);
	g_eventLog.push_front(std::move(eve));
}

static InitFunction initFunction([]()
{
	static bool eventLogEnabled;
	static ConVar<bool> eventLogVar("netEventLog", ConVar_Archive | ConVar_UserPref, false, &eventLogEnabled);

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || eventLogEnabled;
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if (!eventLogEnabled)
		{
			return;
		}

		ImGui::SetNextWindowSize(ImVec2(500.0f, 300.0f), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Network Event Log", &eventLogEnabled))
		{
			// If we're on the server we're going to have the "Sender/Receiver" field so we need an extra column
			ImGui::Columns(
#ifdef IS_FXSERVER
				4
#else
				3
#endif
			);

			{
				ImGui::Text("Dir");
				ImGui::NextColumn();
				ImGui::Text("Name");
				ImGui::NextColumn();
#ifdef IS_FXSERVER
				ImGui::Text("Sender/Receiver");
				ImGui::NextColumn();
#endif
				ImGui::Text("Size");
				ImGui::NextColumn();

				std::unique_lock<std::mutex> lock(g_eventLogMutex);

				for (auto& entry : g_eventLog)
				{

					ImGui::Text(
#ifndef IS_FXSERVER
						(entry.out) ? "C->S" : "S->C"
#else
						(entry.out) ? "S->C" : "C->S"
#endif
					);
					ImGui::NextColumn();
					ImGui::Text("%s", entry.eventName.c_str());
					ImGui::NextColumn();
#if IS_FXSERVER
					ImGui::Text("%s", entry.eventSource.value_or("server").c_str());
					ImGui::NextColumn();
#endif
					ImGui::Text("%lld B", entry.payloadSize);
					ImGui::NextColumn();
				}
			}

			ImGui::Columns();
		}

		ImGui::End();
	});

	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* manager)
	{
		auto eventManager = manager->GetComponent<fx::ResourceEventManagerComponent>();

		eventManager->OnQueueEvent.Connect([](const std::string& eventName, const std::string& eventPayload, const std::string& eventSource)
		{
			if (eventSource.find("net:") == 0)
			{
#if IS_FXSERVER
				EventLog_Add(eventName, eventPayload.size(), false, std::optional{ eventSource.substr(4) });
#else
				EventLog_Add(eventName, eventPayload.size(), false);
#endif
				
			}
		});

		eventManager->OnClientEventTriggered.Connect([](const std::string_view& eventName, size_t& eventSize, const std::optional<std::string_view>& eventTarget)
		{
			EventLog_Add(std::string{ eventName }, eventSize, true, std::string{ eventTarget.value_or("-1") });
		});

	}, INT32_MAX);

#ifndef IS_FXSERVER
	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* library)
	{
		library->OnTriggerServerEvent.Connect([](const std::string& eventName, const std::string& eventPayload)
		{
			EventLog_Add(eventName, eventPayload.size(), true);
		});
	});
#endif
});

