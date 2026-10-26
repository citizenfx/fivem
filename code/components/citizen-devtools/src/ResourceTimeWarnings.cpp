#include "StdInc.h"

#include <ConsoleHost.h>

#include <CL2LaunchMode.h>
#include <json.hpp>

#include <CoreConsole.h>

#ifndef IS_FXSERVER
#include <Pool.h>
#include <Streaming.h>
#endif

#include <iomanip>
#include <sstream>
#include <ctime>
#include <chrono>

#include <imgui.h>
#include <imgui_internal.h>

#include <chrono>

#include <CfxLocale.h>

#include <ResourceManager.h>
#include "ResourceMonitor.h"

#include <ScriptLineProfiler.h>

DLL_IMPORT ImFont* GetConsoleFontTiny();

using namespace std::chrono_literals;

void BytesToHumanReadable(std::string& humanSize, int totalBytes)
{
	humanSize = fmt::sprintf("%d B", totalBytes);

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
}

#ifndef IS_FXSERVER
std::string PrettyFormatFileTime(uint64_t filetime)
{
	// Convert uint64_t to FILETIME
	FILETIME ft;
	ft.dwLowDateTime = static_cast<DWORD>(filetime & 0xFFFFFFFF);
	ft.dwHighDateTime = static_cast<DWORD>(filetime >> 32);

	// Convert FILETIME to SYSTEMTIME
	SYSTEMTIME st;
	if (!FileTimeToSystemTime(&ft, &st))
	{
		return "Invalid FILETIME";
	}

	// Create a string stream to format the date and time
	std::stringstream ss;
	ss << std::setfill('0') << std::setw(2) << st.wDay << "."
	   << std::setfill('0') << std::setw(2) << st.wMonth << "."
	   << std::setw(4) << st.wYear << " "
	   << std::setfill('0') << std::setw(2) << st.wHour << ":"
	   << std::setfill('0') << std::setw(2) << st.wMinute << ":"
	   << std::setfill('0') << std::setw(2) << st.wSecond;

	return ss.str();
}
#endif

inline std::chrono::microseconds usec()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
}

template<typename T>
static void SendSdkMessage(const std::string& type, const T& payload)
{
	static constexpr uint32_t SEND_SDK_MESSAGE = HashString("SEND_SDK_MESSAGE");

	nlohmann::json resourceDatasJson;
	resourceDatasJson["type"] = type;
	resourceDatasJson["data"] = payload;

	NativeInvoke::Invoke<SEND_SDK_MESSAGE, const char*>(resourceDatasJson.dump().c_str());
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

struct SqlStats
{
	double msPerSec = 0.0; // query time (send until result) per second of recording
	double queriesPerSec = 0.0;
};

// per-resource MySQL query time from the line profiler
static std::unordered_map<std::string, SqlStats> GetSqlStats()
{
	std::unordered_map<std::string, SqlStats> stats;

	double seconds = fx::lineprof::GetRecordedNs() / 1e9;

	if (seconds <= 0.0)
	{
		return stats;
	}

	for (auto& profile : fx::lineprof::GetResourceProfiles())
	{
		uint64_t waitNs, waitCount;
		profile->GetWaitTotals(waitNs, waitCount);

		if (waitCount > 0)
		{
			stats[profile->GetName()] = { waitNs / 1e6 / seconds, waitCount / seconds };
		}
	}

	return stats;
}

// per-resource bytes allocated by json.encode/json.decode, per second of recording
static std::unordered_map<std::string, double> GetJsonAllocRates()
{
	std::unordered_map<std::string, double> rates;

	double seconds = fx::lineprof::GetRecordedNs() / 1e9;

	if (seconds <= 0.0)
	{
		return rates;
	}

	for (auto& profile : fx::lineprof::GetResourceProfiles())
	{
		uint64_t alloc[size_t(fx::lineprof::SampleKind::Count)];
		profile->GetKindAlloc(alloc);

		if (auto json = alloc[size_t(fx::lineprof::SampleKind::Json)]; json > 0)
		{
			rates[profile->GetName()] = json / seconds;
		}
	}

	return rates;
}

enum class TooltipMode
{
	Cpu, // lines by CPU time
	Sql, // queries by time until the result came back
	Json, // json.encode/decode calls by bytes allocated
	Memory, // lines by bytes allocated
};

static std::string FormatBytes(double bytes)
{
	std::string out;
	BytesToHumanReadable(out, int(std::min(bytes, double(INT_MAX))));

	return out;
}

// tooltip listing what costs the most in a resource, according to `mode`
static void ShowTopLinesTooltip(const std::string& resourceName, TooltipMode mode)
{
	namespace lp = fx::lineprof;

	static std::string cachedKey;
	static std::chrono::steady_clock::time_point cachedAt;
	static std::vector<lp::SampleEntry> cachedTop;
	static double cachedTotal;

	auto metric = [mode](const lp::SampleStats& stats) -> double
	{
		switch (mode)
		{
			case TooltipMode::Sql:
				return double(stats.waitNs);
			case TooltipMode::Json:
			case TooltipMode::Memory:
				return double(stats.allocBytes);
			default:
				return double(stats.selfNs);
		}
	};

	auto key = fmt::sprintf("%s:%d", resourceName, int(mode));
	auto now = std::chrono::steady_clock::now();

	if (key != cachedKey || now - cachedAt > 500ms)
	{
		std::vector<lp::SampleEntry> entries;

		for (auto& profile : lp::GetResourceProfiles())
		{
			if (profile->GetName() == resourceName)
			{
				profile->Snapshot(entries);
				break;
			}
		}

		entries.erase(std::remove_if(entries.begin(), entries.end(), [mode, &metric](const lp::SampleEntry& entry)
		{
			if (mode == TooltipMode::Sql && entry.info.kind != lp::SampleKind::MySQL)
			{
				return true;
			}

			if (mode == TooltipMode::Json && entry.info.kind != lp::SampleKind::Json)
			{
				return true;
			}

			return metric(entry.stats) <= 0.0;
		}),
		entries.end());

		cachedTotal = 0.0;

		for (const auto& entry : entries)
		{
			cachedTotal += metric(entry.stats);
		}

		auto count = std::min<size_t>(entries.size(), 12);

		std::partial_sort(entries.begin(), entries.begin() + count, entries.end(), [&metric](const lp::SampleEntry& a, const lp::SampleEntry& b)
		{
			return metric(a.stats) > metric(b.stats);
		});

		entries.resize(count);

		cachedTop = std::move(entries);
		cachedKey = key;
		cachedAt = now;
	}

	double seconds = lp::GetRecordedNs() / 1e9;

	// the CPU graph adds its own sample value to the same tooltip
	ImGui::BeginTooltip();
	ImGui::Separator();

	if (mode == TooltipMode::Memory)
	{
		ImGui::TextDisabled("Lua memory allocated per line (high and steady = check for leaks)");
	}

	if (cachedTop.empty())
	{
		const char* text = "Nothing recorded for this resource yet.";

		if (!lp::g_enabled && lp::GetRecordedNs() == 0)
		{
			text = "Press 'Start Profiler Record' to see which lines are responsible.";
		}
		else if (mode == TooltipMode::Sql)
		{
			text = "No query results received yet.";
		}
		else if (mode == TooltipMode::Json)
		{
			text = "No json.encode/json.decode calls recorded yet.";
		}

		ImGui::TextUnformatted(text);
	}
	else
	{
		for (const auto& [info, stats] : cachedTop)
		{
			ImVec4 color = ImVec4(0.4f, 0.8f, 0.4f, 1.0f);

			if (info.kind == lp::SampleKind::MySQL)
			{
				color = ImVec4(1.0f, 0.6f, 0.2f, 1.0f);
			}
			else if (info.kind == lp::SampleKind::Json)
			{
				color = ImVec4(1.0f, 0.85f, 0.3f, 1.0f);
			}
			else if (info.kind != lp::SampleKind::Lua)
			{
				color = ImVec4(0.35f, 0.6f, 1.0f, 1.0f);
			}

			double value = metric(stats);

			ImGui::TextColored(color, "%5.1f%%  %-7s", cachedTotal > 0.0 ? value * 100.0 / cachedTotal : 0.0, lp::GetKindName(info.kind));
			ImGui::SameLine();

			if (info.line > 0)
			{
				ImGui::Text("%s:%d  %s [%d-%d]", info.source.c_str(), info.line, info.function.c_str(), info.lineDefined, info.lastLineDefined);
			}
			else
			{
				ImGui::Text("%s  %s", info.source.c_str(), info.function.c_str());
			}

			if (!info.detail.empty() && mode != TooltipMode::Json)
			{
				ImGui::SameLine();
				ImGui::TextDisabled("-> %.120s", info.detail.c_str());
			}

			ImGui::SameLine();

			switch (mode)
			{
				case TooltipMode::Sql:
					ImGui::TextDisabled("(%llu queries, avg %.1f ms, max %.1f ms)", stats.waitCount, stats.waitCount ? stats.waitNs / 1e6 / stats.waitCount : 0.0, stats.maxWaitNs / 1e6);
					break;
				case TooltipMode::Json:
					ImGui::TextDisabled("%s  (%llu calls, %s/s, avg %s per call)", info.detail.c_str(), stats.count, FormatBytes(seconds > 0.0 ? value / seconds : 0.0).c_str(), FormatBytes(stats.count ? value / stats.count : 0.0).c_str());
					break;
				case TooltipMode::Memory:
					ImGui::TextDisabled("(%s/s)", FormatBytes(seconds > 0.0 ? value / seconds : 0.0).c_str());
					break;
				default:
					ImGui::NewLine();
					break;
			}
		}
	}

	ImGui::EndTooltip();
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

#ifdef GTA_FIVE
#include <Hooking.h>
#include <InputHook.h>
#include <Error.h>

static decltype(&SetThreadExecutionState) origSetThreadExecutionState;
static decltype(&PeekMessageW) origPeekMessageW;

static std::chrono::microseconds lastPeekMessage;
static bool currentWasFocusEvent = false;

static EXECUTION_STATE WINAPI SetThreadExecutionState_Track(EXECUTION_STATE esFlags)
{
	if (esFlags == 3)
	{
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
	auto rv = origPeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

	lastPeekMessage = usec();

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

#pragma comment(lib, "version.lib")

#define STOPPED_RESPONDING_MESSAGE(x) \
	"FiveM has stopped responding " x "\nThe game stopped responding for too long and needs to be restarted. When asking for help, please click 'Save information' and upload the file that is saved when you click the button.%s"

static void __declspec(noinline) StoppedRespondingNVIDIA(const std::string& reasoning)
{
	FatalErrorNoReport(STOPPED_RESPONDING_MESSAGE("(NVIDIA drivers)"), reasoning);
}

static void __declspec(noinline) StoppedRespondingScripts(const std::string& reasoning)
{
	FatalErrorNoReport(STOPPED_RESPONDING_MESSAGE("(script deadloop)"), reasoning);
}

static void __declspec(noinline) StoppedRespondingRenderQuery(const std::string& reasoning)
{
	FatalErrorNoReport(STOPPED_RESPONDING_MESSAGE("(DirectX query)"), reasoning);
}

static void __declspec(noinline) StoppedRespondingGeneric(const std::string& reasoning)
{
	FatalErrorNoReport(STOPPED_RESPONDING_MESSAGE(""), reasoning);
}

#ifdef GTA_FIVE
extern DLL_IMPORT bool IsInRenderQuery();
#endif

static HookFunction hookFunctionGameTime([]()
{
#ifdef GTA_FIVE
	// No, we are not a handheld PC.
	if (xbr::IsGameBuildOrGreater<xbr::Build::Winter_2025>())
	{
		auto* addr = hook::get_pattern("FF 15 ? ? ? ? 48 85 C0 74 ? 48 8D 15");
		hook::nop(addr, 6);
		hook::put<uint32_t>(addr, 0x90C03148); // xor rax, rax; nop
	}
#endif

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

		resource->OnActivate.Connect([resourceName]()
		{
			auto th = GetThread();

			if (th)
			{
				th->push_front(resourceName + ": activated");
			}
		},
		INT32_MIN);

		resource->OnDeactivate.Connect([]()
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

				if (lastPeekMessage.count() > 0 && (now - lastPeekMessage) > 45s && (now - lastPeekMessage) < 90s)
				{
					std::string reasoning;
					bool scripts = false;
					bool renderQuery = false;

					if (!g_threadStack.empty())
					{
						std::stringstream s;

						auto& threadStack = g_threadStack;

						for (auto& entry : threadStack)
						{
							s << entry << " <- ";
						}

						s << "root";

						reasoning = fmt::sprintf("\n\nThis is likely caused by a resource/script, the script stack is as follows: %s", s.str());
						scripts = true;
					}

#ifdef GTA_FIVE
					if (IsInRenderQuery())
					{
						reasoning += fmt::sprintf("\n\nGame code was waiting for the GPU to complete the last frame, but this timed out (the GPU got stuck?) - this could be caused by bad assets or graphics mods.");
						renderQuery = true;
					}
#endif

					bool nvidia = false;

					if (auto nvDLL = GetModuleHandleW(L"nvwgf2umx.dll"); nvDLL && reasoning.empty())
					{
						wchar_t nvDllPath[MAX_PATH];
						GetModuleFileNameW(nvDLL, nvDllPath, std::size(nvDllPath));

						DWORD versionInfoSize = GetFileVersionInfoSize(nvDllPath, nullptr);

						if (versionInfoSize)
						{
							std::vector<uint8_t> versionInfo(versionInfoSize);

							if (GetFileVersionInfo(nvDllPath, 0, versionInfo.size(), &versionInfo[0]))
							{
								void* fixedInfoBuffer;
								UINT fixedInfoSize;

								VerQueryValue(&versionInfo[0], L"\\", &fixedInfoBuffer, &fixedInfoSize);

								VS_FIXEDFILEINFO* fixedInfo = reinterpret_cast<VS_FIXEDFILEINFO*>(fixedInfoBuffer);

								// convert versions like 30.0.14.9649 to a decimal like 49649
								uint32_t nvidiaVersion = (fixedInfo->dwFileVersionLS & 0xFFFF) + (((fixedInfo->dwFileVersionLS >> 16) % 10) * 10000);

								if (nvidiaVersion >= 47200 && nvidiaVersion <= 49710)
								{
									nvidia = true;

									reasoning += fmt::sprintf("\n\nThis may be related to a known issue in your NVIDIA GeForce drivers (version %03d.%02d). Downgrade to version 471.68 or below (https://www.nvidia.com/en-us/geforce/drivers/), or disable any overlays/graphics mods (such as Steam or ENBSeries) to fix this.",
										nvidiaVersion / 100,
										nvidiaVersion % 100);
								}
							}
						}
					}

					auto unresponsiveFor = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPeekMessage).count() / 1000.0;

					std::string errorMessage = fmt::sprintf(
						"The game stopped responding for %.1f seconds and will be restarted soon.%s",
						unresponsiveFor,
						reasoning
					);

					SendSdkMessage("fxdk:gameFatalError", errorMessage);

					if (!launch::IsSDKGuest())
					{
						if (scripts)
						{
							StoppedRespondingScripts(reasoning);
						}
						else if (renderQuery)
						{
							StoppedRespondingRenderQuery(reasoning);
						}
						else if (nvidia)
						{
							StoppedRespondingNVIDIA(reasoning);
						}

						StoppedRespondingGeneric(reasoning);
					}
				}
			}
		})
		.detach();
	}
});
#endif

static InitFunction initFunction([]()
{
	static std::shared_mutex gResourceMonitorMutex;
	static std::shared_ptr<fx::ResourceMonitor> gResourceMonitor;

	static bool resourceTimeWarningShown;
	static std::chrono::microseconds warningLastShown;
	static std::string resourceTimeWarningText;
	static std::mutex mutex;

	static bool taskMgrEnabled;
	static bool m_enabledPools;
	static bool m_enabledPgRawStreamerStats;

	static ConVar<bool> taskMgrVar("resmon", ConVar_Archive | ConVar_UserPref, false, &taskMgrEnabled);

	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* manager)
	{
		manager->OnTick.Connect([]()
		{
			bool hadMonitor = false;

			{
				std::shared_lock _(gResourceMonitorMutex);
				hadMonitor = gResourceMonitor.get() != nullptr;
			}

			// #TODO: SDK should explicitly notify the resource monitor is opened
			if (taskMgrEnabled || launch::IsSDKGuest())
			{
				if (!hadMonitor)
				{
					std::unique_lock _(gResourceMonitorMutex);
					gResourceMonitor = std::make_shared<fx::ResourceMonitor>();

					gResourceMonitor->SetShouldGetMemory(true);
				}
			}
			else
			{
				if (hadMonitor)
				{
					std::unique_lock _(gResourceMonitorMutex);
					gResourceMonitor = {};
				}
			}
		}, INT32_MIN);
	});

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
#endif

		std::shared_ptr<fx::ResourceMonitor> resourceMonitor;

		{
			std::shared_lock _(gResourceMonitorMutex);
			resourceMonitor = gResourceMonitor;
		}

#ifndef IS_FXSERVER
		if (launch::IsSDKGuest() && resourceMonitor)
		{
			const auto& resourceDatas = resourceMonitor->GetResourceDatas();

			if (resourceDatas.size() >= 2)
			{
				static std::chrono::microseconds lastSendTime;
				static std::chrono::milliseconds maxPause(333);

				// only send data thrice a second
				if (usec() - lastSendTime >= maxPause)
				{
					std::vector<std::tuple<std::string, double, double, int64_t, int64_t>> resourceDatasClean;
					for (const auto& data : resourceDatas)
					{
						if (std::get<0>(data) == "_cfx_internal")
						{
							continue;
						}

						resourceDatasClean.push_back(tuple_slice<0, std::tuple_size_v<decltype(resourceDatasClean)::value_type>>(data));
					}

					SendSdkMessage("fxdk:clientResourcesData", resourceDatasClean);

					lastSendTime = usec();
				}
			}
		}
#endif

		if (taskMgrEnabled && resourceMonitor)
		{
			// the table scrolls by itself, so the profiler buttons below it stay visible
			if (ImGui::Begin("Resource Monitor", &taskMgrEnabled) && ImGui::BeginTable("##resmon", 8, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollY, ImVec2(0.0f, -ImGui::GetFrameHeightWithSpacing())))
			{
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableSetupColumn("Resource");
				ImGui::TableSetupColumn("CPU msec", ImGuiTableColumnFlags_PreferSortDescending);
				ImGui::TableSetupColumn("SQL ms/s", ImGuiTableColumnFlags_PreferSortDescending);
				ImGui::TableSetupColumn("Time %", ImGuiTableColumnFlags_PreferSortDescending);
				ImGui::TableSetupColumn("CPU (inclusive)", ImGuiTableColumnFlags_PreferSortDescending);
				ImGui::TableSetupColumn("Memory", ImGuiTableColumnFlags_PreferSortDescending);
				ImGui::TableSetupColumn("JSON mem/s", ImGuiTableColumnFlags_PreferSortDescending);
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

				// SQL ms/s: time queries took (send until result) per second, while the profiler records
				auto sqlStats = GetSqlStats();
				SqlStats totalSql;

				for (const auto& [name, stats] : sqlStats)
				{
					totalSql.msPerSec += stats.msPerSec;
					totalSql.queriesPerSec += stats.queriesPerSec;
				}

				auto getSqlMs = [&sqlStats](const std::string& resourceName)
				{
					auto it = sqlStats.find(resourceName);
					return (it != sqlStats.end()) ? it->second.msPerSec : -1.0;
				};

				ImGui::TableSetColumnIndex(2);

				if (sqlStats.empty())
				{
					ImGui::Text("-");
				}
				else
				{
					ImGui::Text("%.2f (%.1f q/s)", totalSql.msPerSec, totalSql.queriesPerSec);
				}

				// JSON mem/s: bytes allocated by json.encode/json.decode per second, while the profiler records
				auto jsonRates = GetJsonAllocRates();

				auto getJsonRate = [&jsonRates](const std::string& resourceName)
				{
					auto it = jsonRates.find(resourceName);
					return (it != jsonRates.end()) ? it->second : -1.0;
				};

				ImGui::TableSetColumnIndex(6);

				if (jsonRates.empty())
				{
					ImGui::Text("-");
				}
				else
				{
					double totalJson = 0.0;

					for (const auto& [name, rate] : jsonRates)
					{
						totalJson += rate;
					}

					ImGui::Text("%s", FormatBytes(totalJson).c_str());
				}

				ImGui::TableSetColumnIndex(3);

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
						std::sort(resourceDatas.begin(), resourceDatas.end(), [sortSpecs, &getSqlMs, &getJsonRate](const auto& left, const auto& right)
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
								const ImGuiTableColumnSortSpecs* sortSpec = &sortSpecs->Specs[n];
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
										delta = compare(getSqlMs(std::get<0>(left)), getSqlMs(std::get<0>(right)));
										break;
									case 3:
										delta = compare(std::get<2>(left), std::get<2>(right));
										break;
									case 4:
										delta = compare(std::get<6>(left), std::get<6>(right));
										break;
									case 5:
										delta = compare(std::get<3>(left), std::get<3>(right));
										break;
									case 6:
										delta = compare(getJsonRate(std::get<0>(left)), getJsonRate(std::get<0>(right)));
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

				for (const auto& [resourceName, avgTickMs, avgFrameFraction, memorySize, streamingUsage, recentTicks, avgTotalMs, recentTotals] : resourceDatas)
				{
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGui::Text("%s", resourceName.c_str());

					ImGui::TableSetColumnIndex(1);

					if (avgTickMs >= 0.0)
					{
						using TTick = decltype(recentTicks->tickTimes);
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
						(void*)recentTicks->tickTimes,
						std::size(TTick{}) / sampleCount,
						*recentTicks->curTickTime / sampleCount,
						nullptr,
						0.0f,
						2.5f,
						ImVec2(
							100.0f,
							ImGui::GetTextLineHeight()
						));

						if (ImGui::IsItemHovered())
						{
							ShowTopLinesTooltip(resourceName, TooltipMode::Cpu);
						}
					}
					else
					{
						ImGui::Text("-");
					}

					ImGui::TableSetColumnIndex(2);

					if (auto it = sqlStats.find(resourceName); it != sqlStats.end())
					{
						ImGui::TextColored(GetColorForRange(10.0f, 200.0f, it->second.msPerSec), "%.2f (%.1f q/s)", it->second.msPerSec, it->second.queriesPerSec);

						if (ImGui::IsItemHovered())
						{
							ShowTopLinesTooltip(resourceName, TooltipMode::Sql);
						}
					}
					else
					{
						ImGui::Text("-");
					}

					ImGui::TableSetColumnIndex(3);

					if (avgFrameFraction < 0.0f)
					{
						ImGui::Text("-");
					}
					else
					{
						ImGui::Text("%.2f%%", avgFrameFraction * 100.0);
					}

					ImGui::TableSetColumnIndex(4);

					if (avgTotalMs >= 0.0)
					{
						ImGui::TextColored(GetColorForRange(1.0f, 8.0f, avgTotalMs), "%.2f ms", avgTotalMs);
					}
					else
					{
						ImGui::Text("-");
					}

					ImGui::TableSetColumnIndex(5);

					int64_t totalBytes = memorySize;

					if (totalBytes == 0 || totalBytes == -1)
					{
						ImGui::Text("?");
					}
					else
					{
						std::string humanSize;
						BytesToHumanReadable(humanSize, totalBytes);

						ImGui::Text("%s+", humanSize.c_str());
					}

					if (ImGui::IsItemHovered())
					{
						ShowTopLinesTooltip(resourceName, TooltipMode::Memory);
					}

					ImGui::TableSetColumnIndex(6);

					if (auto it = jsonRates.find(resourceName); it != jsonRates.end())
					{
						ImGui::Text("%s", FormatBytes(it->second).c_str());

						if (ImGui::IsItemHovered())
						{
							ShowTopLinesTooltip(resourceName, TooltipMode::Json);
						}
					}
					else
					{
						ImGui::Text("-");
					}

					ImGui::TableSetColumnIndex(7);

					if (streamingUsage > 0)
					{
						std::string humanSize;
						BytesToHumanReadable(humanSize, streamingUsage);

						ImGui::Text("%s", humanSize.c_str());
					}
					else
					{
						ImGui::Text("-");
					}
				}

				ImGui::EndTable();

				// line profiler: feeds the SQL and JSON columns and the CPU/memory tooltips
				if (fx::lineprof::g_enabled)
				{
					if (ImGui::Button("Stop Profiler Record"))
					{
						fx::lineprof::SetEnabled(false);
					}

					ImGui::SameLine();
					ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Recording %.0fs - hover the CPU graph to see the costly lines", fx::lineprof::GetRecordedNs() / 1e9);
				}
				else
				{
					if (ImGui::Button("Start Profiler Record"))
					{
						fx::lineprof::Reset();
						fx::lineprof::SetEnabled(true);
					}

					if (auto recordedNs = fx::lineprof::GetRecordedNs(); recordedNs > 0)
					{
						ImGui::SameLine();
						ImGui::TextDisabled("Showing last record (%.0fs)", recordedNs / 1e9);
					}
				}
			}

			ImGui::End();
		}
	});

#ifndef IS_FXSERVER
	static ConVar<bool> poolVar("net_showPools", ConVar_Archive | ConVar_UserPref, false, &m_enabledPools);

	ConHost::OnShouldDrawGui.Connect([this](bool* should)
	{
		*should = *should || m_enabledPools;
	});

	ConHost::OnDrawGui.Connect([this]()
	{
		if (!m_enabledPools)
		{
			return;
		}

		if (ImGui::Begin("Pool List", &m_enabledPools))
		{
			static char search[100];
			ImGui::InputText("Search", search, IM_ARRAYSIZE(search));
			int totalSizeAllocated = 0;
			int totalSizeUsed = 0;
			auto pools = rage::GetPools();
			struct PoolInfo
			{
				std::string name;
				size_t itemSize;
				size_t totalSize;
				size_t items;
				size_t maxItems;
				float used;

				PoolInfo(const std::string& _name, size_t _itemSize, size_t _totalSize, size_t _items, size_t _maxItems, float _used)
					: name(_name), itemSize(_itemSize), totalSize(_totalSize), items(_items), maxItems(_maxItems), used(_used)
				{
				}
			};
			std::vector<PoolInfo> poolsInfo;
			poolsInfo.reserve(pools.size());

			for (auto [key, pool] : pools)
			{
				totalSizeAllocated += pool->GetSize() * pool->GetEntrySize();
				totalSizeUsed += pool->GetCount() * pool->GetEntrySize();

				float usedPercent = pool->GetSize() == 0 ? 0.f : (static_cast<float>(pool->GetCount()) / pool->GetSize() * 100.f);
				PoolInfo poolInfo(key, pool->GetEntrySize(), pool->GetEntrySize() * pool->GetSize(), pool->GetCount(), pool->GetSize(), usedPercent);
				poolsInfo.push_back(poolInfo);
			}

			std::string humanSizeAllocated;
			BytesToHumanReadable(humanSizeAllocated, totalSizeAllocated);
			ImGui::Text("Total size allocated: %s", humanSizeAllocated.c_str());

			std::string humanSizeUsed;
			BytesToHumanReadable(humanSizeUsed, totalSizeUsed);
			ImGui::Text("Total size used: %s", humanSizeUsed.c_str());

			bool shouldExport = false;
			if (ImGui::Button("Export"))
			{
				shouldExport = true;
			}

			if (ImGui::BeginTable("Pool values", 6, ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable))
			{
				ImGui::TableSetupColumn("Pool name", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Item size", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending, 200);
				ImGui::TableSetupColumn("Total size", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending, 200);
				ImGui::TableSetupColumn("Items", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending, 200);
				ImGui::TableSetupColumn("Max items", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending, 200);
				ImGui::TableSetupColumn("Used", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending, 200);
				ImGui::TableHeadersRow();

				{
					auto sortSpecs = ImGui::TableGetSortSpecs();

					if (sortSpecs && sortSpecs->SpecsCount > 0)
					{
						std::sort(poolsInfo.begin(), poolsInfo.end(), [sortSpecs](const PoolInfo& left, const PoolInfo& right)
						{
							for (int n = 0; n < sortSpecs->SpecsCount; n++)
							{
								const ImGuiTableColumnSortSpecs* sortSpec = &sortSpecs->Specs[n];
								int delta = 0;
								switch (sortSpec->ColumnIndex)
								{
									case 0:
										delta = strcmpi(left.name.c_str(), right.name.c_str());
										break;
									case 1:
										delta = left.itemSize - right.itemSize;
										break;
									case 2:
										delta = left.totalSize - right.totalSize;
										break;
									case 3:
										delta = left.items - right.items;
										break;
									case 4:
										delta = left.maxItems - right.maxItems;
										break;
									case 5:
										delta = static_cast<int>(left.used * 1000) - static_cast<int>(right.used * 1000);
										break;
								}
								if (delta > 0)
									return (sortSpec->SortDirection == ImGuiSortDirection_Ascending) ? false : true;
								if (delta < 0)
									return (sortSpec->SortDirection == ImGuiSortDirection_Ascending) ? true : false;
							}

							return strcmpi(left.name.c_str(), right.name.c_str()) < 0;
						});
					}
				}

				for (const auto& poolData : poolsInfo)
				{
					if (search[0] != '\0')
					{
						std::string nameLower = poolData.name;
						std::string itemsStr = std::to_string(poolData.items);
						std::string maxItemsStr = std::to_string(poolData.maxItems);
						std::string searchLower = search;

						std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
						std::transform(itemsStr.begin(), itemsStr.end(), itemsStr.begin(), ::tolower);
						std::transform(maxItemsStr.begin(), maxItemsStr.end(), maxItemsStr.begin(), ::tolower);
						std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);

						if (nameLower.find(searchLower) == std::string::npos &&
							itemsStr.find(searchLower) == std::string::npos &&
							maxItemsStr.find(searchLower) == std::string::npos)
						{
							continue;
						}
					}

					std::string humanSize;
					BytesToHumanReadable(humanSize, poolData.totalSize);

					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text("%s", poolData.name.c_str());
					ImGui::TableNextColumn();
					ImGui::Text("%d", poolData.itemSize);
					ImGui::TableNextColumn();
					ImGui::Text("%s", humanSize.c_str());
					ImGui::TableNextColumn();
					ImGui::Text("%d", poolData.items);
					ImGui::TableNextColumn();
					ImGui::Text("%d", poolData.maxItems);
					ImGui::TableNextColumn();
					ImGui::Text("%.2f%%", poolData.used);
				}
				ImGui::EndTable();
			}

			if (shouldExport)
			{
				// Properly sort it before export
				std::sort(poolsInfo.begin(), poolsInfo.end(), [](const PoolInfo& left, const PoolInfo& right)
				{
					return strcmpi(left.name.c_str(), right.name.c_str()) < 0;
				});

				trace("--- Pools memory usage report ---\n");
				trace("Allocated: %s\n", humanSizeAllocated.c_str());
				trace("Used: %s\n", humanSizeUsed.c_str());
				trace("Pools details:\n");

				for (const auto& poolData : poolsInfo)
				{
					std::string humanSize;
					BytesToHumanReadable(humanSize, poolData.totalSize);

					trace(" - %s, item size = %d, total size = %s, items count = %d, pool size = %d\n", poolData.name.c_str(), poolData.itemSize, humanSize.c_str(), poolData.items, poolData.maxItems);
				}
				trace("--- Pools memory usage report end ---\n");
			}
		}
		ImGui::End();
	});


	static ConVar<bool> pgStatsVar("net_pgStats", ConVar_Archive | ConVar_UserPref, false, &m_enabledPgRawStreamerStats);

	ConHost::OnShouldDrawGui.Connect([this](bool* should)
	{
		*should = *should || m_enabledPgRawStreamerStats;
	});

	ConHost::OnDrawGui.Connect([this]()
	{
		if (!m_enabledPgRawStreamerStats)
		{
			return;
		}

		if (ImGui::Begin("Loaded pgRawStreamer assets", &m_enabledPgRawStreamerStats))
		{
			static char search[100] = "";
			ImGui::InputText("Search", search, IM_ARRAYSIZE(search));
			auto assets = rage::GetPgRawStreamerEntries();
			ImGui::LabelText("Assets total", "%d/65535", assets.GetCount());

			ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Sortable;
			if (ImGui::BeginTable("Assets", 2, tableFlags))
			{
				ImGui::TableSetupColumn("Asset path", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Add timestamp", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_PreferSortDescending, 200);
				ImGui::TableHeadersRow();

				if (assets.GetCount() > 1)
				{
					std::vector<uint32_t> filteredIndices;
					for (uint32_t i = 0; i < assets.GetCount(); i++)
					{
						if (assets[i].fileName && strstr(assets[i].fileName, search) != nullptr)
						{
							filteredIndices.push_back(i);
						}
					}

					ImGuiListClipper clipper;

					// -1 because 1st item in Entries always dummy in GTAV and RDR3
					clipper.Begin(static_cast<int>(filteredIndices.size()));
					while (clipper.Step())
					{
						for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
						{
							const uint32_t assetsIndex = filteredIndices[i];

							const std::string datetime = PrettyFormatFileTime(assets[assetsIndex].timestamp);

							ImGui::TableNextRow();
							ImGui::TableNextColumn();
							ImGui::Text("%s", assets[assetsIndex].fileName);
							ImGui::TableNextColumn();
							ImGui::Text("%s", datetime.c_str());
						}
					}
					clipper.End();
				}

				ImGui::EndTable();
			}
		}

		ImGui::End();
	});
#endif
});

