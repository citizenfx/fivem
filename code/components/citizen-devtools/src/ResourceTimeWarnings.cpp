#include "StdInc.h"

#include <ConsoleHost.h>

#include <CL2LaunchMode.h>
#include <json.hpp>

#include <CoreConsole.h>

#include <imgui.h>

#include <chrono>

#include "ResourceMonitor.h"

DLL_IMPORT ImFont* GetConsoleFontTiny();

using namespace std::chrono_literals;

inline std::chrono::microseconds usec()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
}

static ImVec4 GetColorForRange(float min, float max, float num)
{
	float avg = (min + max) / 2.0f;
	float scale = 1.0f / (avg - min);

	if (num < min)
	{
		return ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	}
	else if (num >= max)
	{
		return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (num >= avg)
	{
		return ImVec4(1.0f, 1.0f - ((num - avg) * scale), 0.0f, 1.0f);
	}
	else
	{
		return ImVec4(((num - min) * scale), 1.0f, 0.0f, 1.0f);
	}
}

static InitFunction initFunction([]()
{
	static auto* resourceMonitor = fx::ResourceMonitor::GetCurrent();

	static bool resourceTimeWarningShown;
	static std::chrono::microseconds warningLastShown;
	static std::string resourceTimeWarningText;
	static std::mutex mutex;

	static bool taskMgrEnabled;

	static ConVar<bool> taskMgrVar("resmon", ConVar_Archive, false, &taskMgrEnabled);

	fx::ResourceMonitor::OnWarning.Connect([](const std::string& warningText)
	{
		std::unique_lock<std::mutex> lock(mutex);

		if (!resourceTimeWarningShown)
		{
			warningLastShown = usec();
		}

		// warn again 10 minutes later
		if ((usec() - warningLastShown) > 600s)
		{
			warningLastShown = usec();
		}

		resourceTimeWarningShown = true;
		resourceTimeWarningText = warningText;
	});

	fx::ResourceMonitor::OnWarningGone.Connect([]()
	{
		if (resourceTimeWarningShown && (usec() - warningLastShown) < 17s)
		{
			warningLastShown = usec() - 17s;
		}

		resourceTimeWarningShown = false;
	});

	ConHost::OnShouldDrawGui.Connect([](bool* should)
	{
		*should = *should || ((usec() - warningLastShown) < 20s) || taskMgrEnabled;
	});

	ConHost::OnDrawGui.Connect([]()
	{
#ifndef IS_FXSERVER
		if ((usec() - warningLastShown) < 20s)
		{
			ImGui::PushFont(GetConsoleFontTiny());

			const float DISTANCE = 10.0f;
			ImVec2 window_pos = ImVec2(ImGui::GetMainViewport()->Pos.x + ImGui::GetIO().DisplaySize.x - DISTANCE, ImGui::GetMainViewport()->Pos.y + ImGui::GetIO().DisplaySize.y - DISTANCE);
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
			}
			ImGui::End();
			ImGui::PopFont();
		}
#endif

#ifndef IS_FXSERVER
		if (launch::IsSDKGuest())
		{
			auto& resourceDatas = resourceMonitor->GetResourceDatas();

			if (resourceDatas.size() >= 2)
			{
				static std::chrono::microseconds lastSendTime;
				static std::chrono::milliseconds maxPause(333);

				// only send data thrice a second
				if (usec() - lastSendTime >= maxPause)
				{
					static constexpr uint32_t SEND_SDK_MESSAGE = HashString("SEND_SDK_MESSAGE");

					nlohmann::json resourceDatasJson;
					resourceDatasJson["type"] = "fxdk:clientResourcesData";
					resourceDatasJson["data"] = resourceDatas;

					NativeInvoke::Invoke<SEND_SDK_MESSAGE, const char*>(resourceDatasJson.dump().c_str());

					lastSendTime = usec();
				}
			}
		}
#endif

		if (taskMgrEnabled)
		{
			if (ImGui::Begin("Resource Monitor", &taskMgrEnabled) && ImGui::BeginTable("##resmon", 5, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable))
			{
				ImGui::TableSetupColumn("Resource");
				ImGui::TableSetupColumn("CPU msec", ImGuiTableColumnFlags_PreferSortDescending);
				ImGui::TableSetupColumn("Time %", ImGuiTableColumnFlags_PreferSortDescending);
				ImGui::TableSetupColumn("Memory", ImGuiTableColumnFlags_PreferSortDescending);
				ImGui::TableSetupColumn("Streaming", ImGuiTableColumnFlags_PreferSortDescending);
				ImGui::TableHeadersRow();

				auto& resourceDatas = resourceMonitor->GetResourceDatas();

				if (resourceDatas.size() < 2)
				{
					ImGui::EndTable();
					ImGui::End();
					return;
				}

				double avgFrameMs = resourceMonitor->GetAvgFrameMs();
				double avgScriptMs = resourceMonitor->GetAvgScriptMs();

				ImGui::TableNextRow();
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.65f)));

				ImGui::TableSetColumnIndex(0);
				ImGui::Text("[Total CPU]");

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.2f", avgScriptMs);

				ImGui::TableSetColumnIndex(2);

				if (avgFrameMs == 0.0f)
				{
					ImGui::Text("-");
				}
				else
				{
					ImGui::Text("%.1f%%", (avgScriptMs / avgFrameMs) * 100.0);
				}

				{
					auto sortSpecs = ImGui::TableGetSortSpecs();

					if (sortSpecs && sortSpecs->SpecsCount > 0)
					{
						std::sort(resourceDatas.begin(), resourceDatas.end(), [sortSpecs](const auto& left, const auto& right)
						{
							auto compare = [](const auto& left, const auto& right)
							{
								if (left < right)
									return -1;

								if (left > right)
									return 1;

								return 0;
							};

							for (int n = 0; n < sortSpecs->SpecsCount; n++)
							{
								const ImGuiTableSortSpecsColumn* sortSpec = &sortSpecs->Specs[n];
								int delta = 0;
								switch (sortSpec->ColumnIndex)
								{
									case 0:
										delta = compare(std::get<0>(left), std::get<0>(right));
										break;
									case 1:
										delta = compare(std::get<1>(left), std::get<1>(right));
										break;
									case 2:
										delta = compare(std::get<2>(left), std::get<2>(right));
										break;
									case 3:
										delta = compare(std::get<3>(left), std::get<3>(right));
										break;
									case 4:
										delta = compare(std::get<4>(left), std::get<4>(right));
										break;
								}
								if (delta > 0)
									return (sortSpec->SortDirection == ImGuiSortDirection_Ascending) ? false : true;
								if (delta < 0)
									return (sortSpec->SortDirection == ImGuiSortDirection_Ascending) ? true : false;
							}

							return (left < right);
						});
					}
				}

				for (const auto& [resourceName, avgTickMs, avgFrameFraction, memorySize, streamingUsage] : resourceDatas)
				{
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGui::Text("%s", resourceName.c_str());

					ImGui::TableSetColumnIndex(1);

					if (avgTickMs >= 0.0)
					{
						ImGui::TextColored(GetColorForRange(1.0f, 8.0f, avgTickMs), "%.2f ms", avgTickMs);
					}
					else
					{
						ImGui::Text("-");
					}

					ImGui::TableSetColumnIndex(2);

					if (avgFrameFraction < 0.0f)
					{
						ImGui::Text("-");
					}
					else
					{
						ImGui::Text("%.2f%%", avgFrameFraction * 100.0);
					}

					ImGui::TableSetColumnIndex(3);

					int64_t totalBytes = memorySize;

					if (totalBytes == 0 || totalBytes == -1)
					{
						ImGui::Text("?");
					}
					else
					{
						std::string humanSize = fmt::sprintf("%d B", totalBytes);

						if (totalBytes > (1024 * 1024 * 1024))
						{
							humanSize = fmt::sprintf("%.2f GiB", totalBytes / 1024.0 / 1024.0 / 1024.0);
						}
						else if (totalBytes > (1024 * 1024))
						{
							humanSize = fmt::sprintf("%.2f MiB", totalBytes / 1024.0 / 1024.0);
						}
						else if (totalBytes > 1024)
						{
							humanSize = fmt::sprintf("%.2f KiB", totalBytes / 1024.0);
						}

						ImGui::Text("%s+", humanSize.c_str());
					}

					ImGui::TableSetColumnIndex(4);

					if (streamingUsage > 0)
					{
						std::string humanSize = fmt::sprintf("%d B", streamingUsage);

						if (streamingUsage > (1024 * 1024 * 1024))
						{
							humanSize = fmt::sprintf("%.2f GiB", streamingUsage / 1024.0 / 1024.0 / 1024.0);
						}
						else if (streamingUsage > (1024 * 1024))
						{
							humanSize = fmt::sprintf("%.2f MiB", streamingUsage / 1024.0 / 1024.0);
						}
						else if (streamingUsage > 1024)
						{
							humanSize = fmt::sprintf("%.2f KiB", streamingUsage / 1024.0);
						}

						ImGui::Text("%s", humanSize.c_str());
					}
					else
					{
						ImGui::Text("-");
					}
				}

				ImGui::EndTable();
			}

			ImGui::End();
		}
	});
});
