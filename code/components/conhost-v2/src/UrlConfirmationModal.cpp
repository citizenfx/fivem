#include <StdInc.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <ConsoleHost.h>

#include <CoreConsole.h>

#include <shellapi.h>

#include <UrlConfirmationExport.h>

void DrawUrlConfirmationModal()
{
	if (!nui::g_showUrlConfirmModal.load())
	{
		return;
	}

	ImGui::OpenPopup("##UrlConfirmModal");

	std::lock_guard<std::mutex> lock(nui::g_urlModalMutex);

	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.11f, 0.11f, 0.13f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.20f, 0.20f, 0.22f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(32.0f, 24.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(850, 0), ImGuiCond_Always);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

	if (ImGui::BeginPopupModal("##UrlConfirmModal", nullptr, flags))
	{
		ImVec2 windowSize = ImGui::GetWindowSize();
		ImVec2 windowPadding = ImGui::GetStyle().WindowPadding;

		// Content area
		ImGui::SetCursorPos(ImVec2(windowPadding.x, windowPadding.y + 16.0f));

		// Main heading
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.85f, 0.87f, 1.0f));
		ImGui::Text("This link will take you to the website below");
		ImGui::PopStyleColor();

		ImGui::Dummy(ImVec2(0, 8.0f));

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.75f, 0.25f, 1.0f));
		ImGui::Text("%s", nui::g_pendingUrl.c_str());
		ImGui::PopStyleColor();

		ImGui::Dummy(ImVec2(0, 12.0f));

		// Separator
		ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.25f, 0.25f, 0.28f, 1.0f));
		ImGui::Separator();
		ImGui::PopStyleColor();

		ImGui::Dummy(ImVec2(0, 12.0f));

		// Warning text
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.55f, 0.57f, 1.0f));
		ImGui::TextWrapped("You are now leaving FiveM and visiting an unaffiliated external site.");
		ImGui::TextWrapped("This link may not be secure. Only click on links from trusted senders.");
		ImGui::PopStyleColor();

		ImGui::Dummy(ImVec2(0, 20.0f));

		float buttonWidth = ImGui::GetContentRegionAvail().x;
		float buttonHeight = 44.0f;

		// Visit Site button
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.30f, 0.55f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.38f, 0.65f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.08f, 0.25f, 0.48f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

		if (ImGui::Button("Visit Site", ImVec2(buttonWidth, buttonHeight)))
		{
			ShellExecuteW(nullptr, L"open", ToWide(nui::g_pendingUrl).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
			nui::g_pendingUrl.clear();
			nui::g_showUrlConfirmModal.store(false);
			ImGui::CloseCurrentPopup();
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);

		ImGui::Dummy(ImVec2(0, 8.0f));

		// Cancel button
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.17f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.20f, 0.23f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.12f, 0.14f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.35f, 0.35f, 0.38f, 1.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

		if (ImGui::Button("Cancel", ImVec2(buttonWidth, buttonHeight)))
		{
			nui::g_pendingUrl.clear();
			nui::g_showUrlConfirmModal.store(false);
			ImGui::CloseCurrentPopup();
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(5);

		ImGui::EndPopup();
	}

	ImGui::PopStyleVar(4);
	ImGui::PopStyleColor(2);
}
