#include "StdInc.h"

#include <Resource.h>
#include <ResourceManager.h>
#include <ConsoleHost.h>

#include <imgui.h>

#include <chrono>

using namespace std::chrono_literals;

inline std::chrono::milliseconds msec()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
}

struct ResourceMetrics
{
	std::chrono::milliseconds tickStart;

	int curTickTime = 0;
	std::chrono::milliseconds tickTimes[64];
};

static InitFunction initFunction([]()
{
	static bool resourceTimeWarningShown;
	static std::chrono::milliseconds warningLastShown;
	static std::string resourceTimeWarningText;
	static std::mutex mutex;

	static std::unordered_map<std::string, ResourceMetrics> metrics;

	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		resource->OnStart.Connect([resource]()
		{
			for (auto& tt : metrics[resource->GetName()].tickTimes)
			{
				tt = { 0 };
			}
		});

		resource->OnTick.Connect([resource]()
		{
			metrics[resource->GetName()].tickStart = msec();
		}, -99999999);

		resource->OnTick.Connect([resource]()
		{
			auto& metric = metrics[resource->GetName()];
			metric.tickTimes[metric.curTickTime++] = msec() - metric.tickStart;

			if (metric.curTickTime >= _countof(metric.tickTimes))
			{
				metric.curTickTime = 0;
			}			
		}, 99999999);

		resource->OnStop.Connect([resource]()
		{
			metrics.erase(resource->GetName());
		});
	});

	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* manager)
	{
		manager->OnTick.Connect([]()
		{
			bool showWarning = false;
			std::string warningText;

			for (const auto& metricPair : metrics)
			{
				std::chrono::milliseconds avgTickTime(0);

				for (auto tickTime : metricPair.second.tickTimes)
				{
					avgTickTime += tickTime;
				}

				avgTickTime /= _countof(metricPair.second.tickTimes);

				if (avgTickTime > 5ms)
				{
					float fpsCount = (60 - (1000.f / (16.67f + avgTickTime.count())));

					showWarning = true;
					warningText += fmt::sprintf("%s is taking %d ms (or -%.1f FPS @ 60 Hz)\n", metricPair.first, avgTickTime.count(), fpsCount);
				}
			}
			
			if (showWarning)
			{
				std::unique_lock<std::mutex> lock(mutex);
				
				resourceTimeWarningShown = true;
				warningLastShown = msec();
				resourceTimeWarningText = warningText;
			}
			else
			{
				resourceTimeWarningShown = false;
			}
		});
	});

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || ((msec() - warningLastShown) < 1s);
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if ((msec() - warningLastShown) >= 1s)
		{
			return;
		}

		const float DISTANCE = 10.0f;
		ImVec2 window_pos = ImVec2(ImGui::GetIO().DisplaySize.x - DISTANCE, ImGui::GetIO().DisplaySize.y - DISTANCE);
		ImVec2 window_pos_pivot = ImVec2(1.0f, 1.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);

		ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
		if (ImGui::Begin("Time Warning", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
		{
			ImGui::Text("/!\\ Resource time warning");
			ImGui::Separator();
			ImGui::Text(resourceTimeWarningText.c_str());
			ImGui::Separator();
			ImGui::Text("Please contact the server owner to resolve this issue.");
			ImGui::End();
		}

	});
});