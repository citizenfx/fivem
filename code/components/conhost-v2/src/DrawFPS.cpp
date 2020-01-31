#include "StdInc.h"
#include <ConsoleHost.h>

#include <imgui.h>

#include <CoreConsole.h>

#include <mmsystem.h>

static InitFunction initFunction([]()
{
	static bool drawFpsEnabled;
	static bool streamingListEnabled;
	static bool streamingMemoryEnabled;

	static ConVar<bool> drawFps("cl_drawFPS", ConVar_Archive, false, &drawFpsEnabled);

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || drawFpsEnabled;
	});

	ConHost::OnDrawGui.Connect([]()
	{
		if (!drawFpsEnabled)
		{
			return;
		}

		auto& io = ImGui::GetIO();

		static std::chrono::high_resolution_clock::duration previous;
		static uint32_t index;
		static std::chrono::microseconds previousTimes[6];

		auto t = std::chrono::high_resolution_clock::now().time_since_epoch();
		auto frameTime = std::chrono::duration_cast<std::chrono::microseconds>(t - previous);
		previous = t;

		previousTimes[index % std::size(previousTimes)] = frameTime;
		index++;

		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::SetNextWindowPos(ImVec2(10, 10), 0, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		if (ImGui::Begin("DrawFps", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (index > 6)
			{
				std::chrono::microseconds total{ 0 };

				for (int i = 0; i < std::size(previousTimes); i++)
				{
					total += previousTimes[i];
				}

				if (total.count() == 0)
				{
					total = { 1 };
				}

				uint64_t fps = ((uint64_t)1000000 * 1000) * std::size(previousTimes) / total.count();
				fps = (fps + 500) / 1000;

				ImGui::Text("%llufps", fps);
			}
		}

		ImGui::PopStyleVar();
		ImGui::End();
	});
});
