#include "StdInc.h"
#include <ConsoleHost.h>

#include <imgui.h>

#include <CoreConsole.h>

#include "FpsTracker.h"

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

		static FpsTracker fpsTracker;
		fpsTracker.Tick();

		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::SetNextWindowPos(ImVec2(ImGui::GetMainViewport()->Pos.x + 10, ImGui::GetMainViewport()->Pos.y + 10), 0, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		if (ImGui::Begin("DrawFps", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (fpsTracker.CanGet())
			{
				ImGui::Text("%llufps", fpsTracker.Get());
			}
		}

		ImGui::PopStyleVar();
		ImGui::End();
	});
});
