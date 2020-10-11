#include <StdInc.h>

#include <ResourceEventComponent.h>
#include <ResourceManager.h>

#include <NetLibrary.h>

#include <boost/circular_buffer.hpp>

#include <ConsoleHost.h>
#include <imgui.h>

#include <CoreConsole.h>

struct EventLogEntry
{
	std::string eventName;
	size_t payloadSize;
	bool out;
};

static boost::circular_buffer<EventLogEntry> g_eventLog(150);
static std::mutex g_eventLogMutex;

static void EventLog_Add(const std::string& eventName, size_t payloadSize, bool out)
{
	EventLogEntry eve;
	eve.eventName = eventName;
	eve.payloadSize = payloadSize;
	eve.out = out;

	std::unique_lock<std::mutex> lock(g_eventLogMutex);
	g_eventLog.push_front(std::move(eve));
}

static InitFunction initFunction([]()
{
	static bool eventLogEnabled;
	static ConVar<bool> eventLogVar("netEventLog", ConVar_Archive, false, &eventLogEnabled);

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

		static bool eventLogOpen;

		ImGui::SetNextWindowSize(ImVec2(500.0f, 300.0f), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Network Event Log", &eventLogOpen))
		{
			ImGui::Columns(3);

			{
				ImGui::Text("Dir");
				ImGui::NextColumn();
				ImGui::Text("Name");
				ImGui::NextColumn();
				ImGui::Text("Size");
				ImGui::NextColumn();

				std::unique_lock<std::mutex> lock(g_eventLogMutex);

				for (auto& entry : g_eventLog)
				{
					ImGui::Text((entry.out) ? "C->S" : "S->C");
					ImGui::NextColumn();
					ImGui::Text("%s", entry.eventName.c_str());
					ImGui::NextColumn();
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
				EventLog_Add(eventName, eventPayload.size(), false);
			}
		});
	}, INT32_MAX);

	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* library)
	{
		library->OnTriggerServerEvent.Connect([](const std::string& eventName, const std::string& eventPayload)
		{
			EventLog_Add(eventName, eventPayload.size(), true);
		});
	});
});
