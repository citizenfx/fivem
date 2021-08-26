#include "StdInc.h"

#include <ConsoleHost.h>

#include <CL2LaunchMode.h>
#include <json.hpp>

#include <CoreConsole.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <chrono>

#include <CfxLocale.h>

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

// tuple slice from https://stackoverflow.com/a/40836163/10995747
namespace detail
{
template<std::size_t Ofst, class Tuple, std::size_t... I>
constexpr auto slice_impl(Tuple&& t, std::index_sequence<I...>)
{
	return std::forward_as_tuple(
	std::get<I + Ofst>(std::forward<Tuple>(t))...);
}
}

template<std::size_t I1, std::size_t I2, class Cont>
constexpr auto tuple_slice(Cont&& t)
{
	static_assert(I2 >= I1, "invalid slice");
	static_assert(std::tuple_size<std::decay_t<Cont>>::value >= I2,
	"slice index out of bounds");

	return detail::slice_impl<I1>(std::forward<Cont>(t),
	std::make_index_sequence<I2 - I1>{});
}

static std::chrono::microseconds lastHitch;

#ifdef GTA_FIVE
#include <Hooking.h>
#include <InputHook.h>
#include <Error.h>

static decltype(&SetThreadExecutionState) origSetThreadExecutionState;
static decltype(&PeekMessageW) origPeekMessageW;

static std::chrono::microseconds lastPeekMessage;
static std::chrono::microseconds currentTotal;
static bool currentWasFocusEvent = false;

static EXECUTION_STATE WINAPI SetThreadExecutionState_Track(EXECUTION_STATE esFlags)
{
	if (esFlags == 3)
	{
		currentTotal = std::chrono::microseconds{ 0 };
		currentWasFocusEvent = false;
	}

	return origSetThreadExecutionState(esFlags);
}

static BOOL WINAPI PeekMessageW_Track(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	// a focus event may take long, so we ignore it
	if (currentWasFocusEvent)
	{
		return origPeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
	}

	// track just the PeekMessage call
	// this will include internal wndproc invocations as well once other events run out
	auto thisStart = usec();
	auto rv = origPeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
	auto thisEnd = usec();

	currentTotal += (thisEnd - thisStart);
	lastPeekMessage = thisEnd;

	// if we're out of events, and didn't get a focus event generated anyway
	if (!rv && !currentWasFocusEvent)
	{
		if (currentTotal > 30ms)
		{
			lastHitch = thisEnd;
		}
	}

	return rv;
}

#include <Resource.h>
#include <ResourceEventComponent.h>

constexpr int NumThreads = 4;

using TThreadStack = std::deque<std::string>;

static TThreadStack g_threadStack;

static TThreadStack* GetThread()
{
	return &g_threadStack;
}

static HookFunction hookFunctionGameTime([]()
{
	InputHook::DeprecatedOnWndProc.Connect([](HWND, UINT uMsg, WPARAM, LPARAM, bool&, LRESULT&)
	{
		// we want to ignore both focus-in and focus-out events
		if (uMsg == WM_ACTIVATEAPP)
		{
			currentWasFocusEvent = true;
		}
	}, INT32_MIN);

	origSetThreadExecutionState = hook::iat("kernel32.dll", SetThreadExecutionState_Track, "SetThreadExecutionState");
	origPeekMessageW = hook::iat("user32.dll", PeekMessageW_Track, "PeekMessageW");

	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		auto resourceName = resource->GetName();

		auto ev = resource->GetComponent<fx::ResourceEventComponent>();
		ev->OnTriggerEvent.Connect([resourceName](const std::string& eventName, const std::string& eventPayload, const std::string& eventSource, bool* eventCanceled)
		{
			auto th = GetThread();

			if (th)
			{
				th->push_front(fmt::sprintf("%s: event %s", resourceName, eventName));
			}
		},
		-100);

		ev->OnTriggerEvent.Connect([](const std::string& eventName, const std::string& eventPayload, const std::string& eventSource, bool* eventCanceled)
		{
			auto th = GetThread();

			if (th)
			{
				th->pop_front();
			}
		},
		100);

		resource->OnTick.Connect([resourceName]()
		{
			auto th = GetThread();

			if (th)
			{
				th->push_front(fmt::sprintf("%s: tick", resourceName));
			}
		},
		INT32_MIN);

		resource->OnTick.Connect([]()
		{
			auto th = GetThread();

			if (th)
			{
				th->pop_front();
			}
		},
		INT32_MAX);
	}, INT32_MAX);

	if (!CoreIsDebuggerPresent())
	{
		std::thread([]()
		{
			using namespace std::chrono_literals;

			SetThreadName(-1, "Window Watchdog");

			while (true)
			{
				auto now = usec();
				Sleep(1000);

				if (lastPeekMessage.count() > 0 && (now - lastPeekMessage) > 15s && (now - lastPeekMessage) < 60s)
				{
					std::string reasoning;

					if (!g_threadStack.empty())
					{
						std::stringstream s;

						auto& threadStack = g_threadStack;

						for (auto& entry : threadStack)
						{
							s << entry << " <- ";
						}

						s << "root";

						reasoning = fmt::sprintf("\n\nScript stack: %s", s.str());
					}

					FatalError("FiveM has stopped responding\nThe game stopped responding for %.1f seconds and needs to be restarted. Information on this hang is currently being uploaded.%s",
					std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPeekMessage).count() / 1000.0,
					reasoning);
				}
			}
		})
		.detach();
	}
});
#endif

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
		auto displayWarningDialog = [](auto cb)
		{
			ImGui::PushFont(GetConsoleFontTiny());

			const float DISTANCE = 10.0f;
			ImVec2 window_pos = ImVec2(ImGui::GetMainViewport()->Pos.x + ImGui::GetIO().DisplaySize.x - DISTANCE, ImGui::GetMainViewport()->Pos.y + ImGui::GetIO().DisplaySize.y - DISTANCE);
			ImVec2 window_pos_pivot = ImVec2(1.0f, 1.0f);
			ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);

			ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
			if (ImGui::Begin("Time Warning", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
			{
				cb();
			}

			ImGui::End();
			ImGui::PopFont();
		};

		if ((usec() - warningLastShown) < 20s)
		{
			displayWarningDialog([]
			{
				ImGui::Text("/!\\ Resource time warning");
				ImGui::Separator();
				ImGui::Text(resourceTimeWarningText.c_str());
				ImGui::Separator();
				ImGui::Text("Please contact the server owner to resolve this issue.");
			});
		}
		else if ((usec() - lastHitch) < 5s)
		{
			displayWarningDialog([]
			{
				ImGui::Text(va("/!\\ %s", gettext("Slow system performance detected")));
				ImGui::Separator();
				ImGui::Text("%s", gettext("A call into the Windows API took too long recently and led to a game stutter.").c_str());
				ImGui::Separator();
				ImGui::Text("%s", gettext("Please close any software you have running in the background (including Windows apps such as File Explorer or Task Manager).").c_str());
			});
		}
#endif

#ifndef IS_FXSERVER
		if (launch::IsSDKGuest())
		{
			const auto& resourceDatas = resourceMonitor->GetResourceDatas();

			if (resourceDatas.size() >= 2)
			{
				static std::chrono::microseconds lastSendTime;
				static std::chrono::milliseconds maxPause(333);

				// only send data thrice a second
				if (usec() - lastSendTime >= maxPause)
				{
					static constexpr uint32_t SEND_SDK_MESSAGE = HashString("SEND_SDK_MESSAGE");

					std::vector<std::tuple<std::string, double, double, int64_t, int64_t>> resourceDatasClean;
					for (const auto& data : resourceDatas)
					{
						resourceDatasClean.push_back(tuple_slice<0, std::tuple_size_v<decltype(resourceDatasClean)::value_type>>(data));
					}

					nlohmann::json resourceDatasJson;
					resourceDatasJson["type"] = "fxdk:clientResourcesData";
					resourceDatasJson["data"] = resourceDatasClean;

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

							return std::get<0>(left) < std::get<0>(right);
						});
					}
				}

				for (const auto& [resourceName, avgTickMs, avgFrameFraction, memorySize, streamingUsage, recentTicks] : resourceDatas)
				{
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGui::Text("%s", resourceName.c_str());

					ImGui::TableSetColumnIndex(1);

					if (avgTickMs >= 0.0)
					{
						using TTick = decltype(recentTicks.get().tickTimes);
						static constexpr const size_t sampleCount = 2;

						ImGui::TextColored(GetColorForRange(1.0f, 8.0f, avgTickMs), "%.2f ms", avgTickMs);
						ImGui::SameLine();
						ImGui::PlotLines(
						"", [](void* data, int idx) {
							auto dataRef = (const std::chrono::microseconds*)data;
							float curSample = 0.0f;
							for (int i = 0; i < sampleCount; i++)
							{
								curSample += static_cast<float>(
									std::chrono::duration_cast<std::chrono::microseconds>(
										dataRef[
											((size_t(idx) * sampleCount) + i) % std::size(TTick{})
										]
									).count()
								) / 1000.0f;
							}

							return curSample / float(sampleCount);
						},
						(void*)recentTicks.get().tickTimes,
						std::size(TTick{}) / sampleCount,
						recentTicks.get().curTickTime / sampleCount,
						nullptr,
						0.0f,
						2.5f,
						ImVec2(
							100.0f,
							ImGui::GetTextLineHeight()
						));
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
