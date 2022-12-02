/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include "CefOverlay.h"
#include "NUIApp.h"
#include "NUIClient.h"
#include "NUIWindowManager.h"

#include <CL2LaunchMode.h>

#include <shared_mutex>
#include <unordered_set>

#include "memdbgon.h"

enum class FeatureStage
{
	AlwaysDisabled,
	DisabledByDefault,
	EnabledByDefault,
	AlwaysEnabled,
};

namespace details
{
struct EnabledState
{
	bool configured = false;
	bool enabled = false;
};

struct FeatureStateCache
{
	EnabledState state;

	// TODO: replace with something more inherently lockfree like WIL does?
	std::once_flag of;
};

template<typename Traits>
class FeatureStorage : private Traits
{
public:
	static constexpr auto GetStage()
	{
		return kStage;
	}

	static constexpr auto GetId()
	{
		return kId;
	}

	auto& GetFeatureStateCache()
	{
		return m_featureStateCache;
	}

private:
	FeatureStateCache m_featureStateCache;
};

template<typename Traits>
class FeatureImpl
{
	using TStorage = FeatureStorage<Traits>;

public:
	EnabledState GetCurrentFeatureEnabledState()
	{
		FeatureStateCache& cache = m_storage.GetFeatureStateCache();
		std::call_once(cache.of, [&cache]()
		{
			auto id = TStorage::GetId();
			// TODO
			//cache.state.configured = true;
		});
	}

	bool __private_IsEnabled()
	{
		if constexpr (TStorage::GetStage() == FeatureStage::AlwaysDisabled)
		{
			return false;
		}
		else if constexpr (TStorage::GetStage() == FeatureStage::AlwaysEnabled)
		{
			return true;
		}

		auto enabledState = GetCurrentFeatureEnabledState();

		if (enabledState.configured)
		{
			return enabledState.enabled;
		}
		else if constexpr (TStorage::GetStage() == FeatureStage::DisabledByDefault)
		{
			return false;
		}
		else if constexpr (TStorage::GetStage() == FeatureStage::EnabledByDefault)
		{
			return true;
		}

		return false;
	}

private:
	TStorage m_storage;
};
}

template<typename Traits>
class Feature
{
public:
	template<typename Func, typename... Args>
	static inline bool RunIfEnabled(Func&& func, Args&&... args)
	{
		if (IsEnabled())
		{
			func(args...);

			return true;
		}

		return false;
	}

	static bool IsEnabled()
	{
		return GetImpl().__private_IsEnabled();
	}

private:
	static auto& GetImpl()
	{
		static details::FeatureImpl<Traits> impl;
		return impl;
	}
};

struct FeatureTraits_TopLevelMPMenu
{
	static constexpr const FeatureStage kStage = FeatureStage::AlwaysEnabled;
	static constexpr const uint32_t kId = 10001;
};

using Feature_TopLevelMPMenu = Feature<FeatureTraits_TopLevelMPMenu>;

bool g_mainUIFlag = true;
bool g_shouldHideCursor;

fwEvent<const wchar_t*, const wchar_t*> nui::OnInvokeNative;
fwEvent<bool> nui::OnDrawBackground;

extern bool g_shouldCreateRootWindow;
extern nui::GameInterface* g_nuiGi;

static std::map<std::string, std::vector<CefRefPtr<CefProcessMessage>>> g_processMessageQueue;
static std::mutex g_processMessageQueueMutex;

static concurrency::concurrent_queue<std::function<void()>> g_offThreadNuiQueue;
static HANDLE g_offthreadNuiQueueWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

void TriggerLoadEnd(const std::string& name)
{
	auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();

	if (rootWindow.GetRef() && rootWindow->GetBrowser() && rootWindow->GetBrowser()->GetMainFrame())
	{
		std::unique_lock _(g_processMessageQueueMutex);

		if (auto it = g_processMessageQueue.find(name); it != g_processMessageQueue.end())
		{
			for (auto& processMessage : it->second)
			{
				rootWindow->GetBrowser()->GetMainFrame()->SendProcessMessage(PID_RENDERER, processMessage);
			}

			g_processMessageQueue.erase(it);
		}
	}
}

namespace nui
{
	__declspec(dllexport) fwRefContainer<NUIWindow> GetWindow()
	{
		auto nuiWm = Instance<NUIWindowManager>::Get();

		if (Feature_TopLevelMPMenu::IsEnabled())
		{
			fwRefContainer<NUIWindow> mpMenu = nullptr;

			nuiWm->ForAllWindows([&mpMenu](fwRefContainer<NUIWindow> window)
			{
				if (window->GetName() == "nui_mpMenu")
				{
					mpMenu = window;
				}
			});

			if (mpMenu.GetRef())
			{
				return mpMenu;
			}
		}

		auto rootWindow = nuiWm->GetRootWindow();

		if (!rootWindow.GetRef())
		{
			return nullptr;
		}

		return rootWindow;
	}

	__declspec(dllexport) CefBrowser* GetBrowser()
	{
		auto window = GetWindow();

		return (window.GetRef()) ? window->GetBrowser() : nullptr;
	}

	bool HasMainUI()
	{
		return g_mainUIFlag;
	}

	std::unordered_map<std::string, fwRefContainer<NUIWindow>> windowList;
	std::shared_mutex windowListMutex;
	
	fwRefContainer<NUIWindow> FindNUIWindow(fwString windowName)
	{
		{
			std::unique_lock<std::shared_mutex> lock(windowListMutex);
			auto windowIt = windowList.find(windowName);

			if (windowIt == windowList.end())
			{
				return nullptr;
			}

			return windowIt->second;
		}
	}

	__declspec(dllexport) void ExecuteRootScript(const std::string& scriptBit)
	{
		auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();

		if (rootWindow.GetRef() && rootWindow->GetBrowser() && rootWindow->GetBrowser()->GetMainFrame())
		{
			rootWindow->GetBrowser()->GetMainFrame()->ExecuteJavaScript(scriptBit, "internal", 1);
		}
	}

	static void PostJSEvent(const std::string& type, std::vector<std::string>&& args)
	{
		auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();

		static auto initThread = []
		{
			std::thread([]
			{
				SetThreadName(-1, "NuiMessage");

				while (true)
				{
					WaitForSingleObject(g_offthreadNuiQueueWakeEvent, INFINITE);

					std::function<void()> fn;
					while (g_offThreadNuiQueue.try_pop(fn))
					{
						if (fn)
						{
							fn();
						}
					}
				}
			})
			.detach();

			return true;
		}();

		g_offThreadNuiQueue.push([rootWindow, type, args = std::move(args)]()
		{
			auto processMessage = CefProcessMessage::Create("pushEvent");
			auto argumentList = processMessage->GetArgumentList();

			argumentList->SetString(0, type);

			int argIdx = 1;

			for (const auto& arg : args)
			{
				argumentList->SetString(argIdx, CefStringUTF8{ arg.data(), arg.size(), false }.ToString16());

				argIdx++;
			}

			if (rootWindow.GetRef() && rootWindow->GetBrowser() && rootWindow->GetBrowser()->GetMainFrame())
			{
				rootWindow->GetBrowser()->GetMainFrame()->SendProcessMessage(PID_RENDERER, processMessage);
			}
			else
			{
				std::unique_lock _(g_processMessageQueueMutex);
				g_processMessageQueue[(type != "rootCall") ? processMessage->GetArgumentList()->GetString(0) : "__root"].push_back(processMessage);
			}
		});

		SetEvent(g_offthreadNuiQueueWakeEvent);
	}

	__declspec(dllexport) void PostRootMessage(const std::string& jsonData)
	{
		PostJSEvent("rootCall", { jsonData });
	}

	DLL_EXPORT void PostFrameMessage(const std::string& frame, const std::string& jsonData)
	{
		if (frame == "mpMenu" && Feature_TopLevelMPMenu::RunIfEnabled([&frame, &jsonData]()
			{
				auto rootWindow = FindNUIWindow(fmt::sprintf("nui_%s", frame));

				if (rootWindow.GetRef())
				{
					bool passed = false;

					auto sendMessage = [frame, jsonData]()
					{
						auto rootWindow = FindNUIWindow(fmt::sprintf("nui_%s", frame));
						if (!rootWindow.GetRef())
						{
							return;
						}

						rootWindow->TouchMessage();

						auto processMessage = CefProcessMessage::Create("pushEvent");
						auto argumentList = processMessage->GetArgumentList();

						argumentList->SetString(0, "frameCall");
						argumentList->SetString(1, jsonData);

						rootWindow->GetBrowser()->GetMainFrame()->SendProcessMessage(PID_RENDERER, processMessage);
					};

					if (rootWindow->GetBrowser() && rootWindow->GetBrowser()->GetMainFrame())
					{
						if (rootWindow->GetBrowser()->GetHost())
						{
							auto client = rootWindow->GetBrowser()->GetHost()->GetClient();

							if (client)
							{
								auto nuiClient = (NUIClient*)client.get();

								if (nuiClient->HasLoadedMainFrame())
								{
									sendMessage();
									passed = true;
								}
							}
						}
					}

					if (!rootWindow->GetBrowser())
					{
						rootWindow->DeferredCreate();
					}

					if (!passed)
					{
						rootWindow->PushLoadQueue(std::move(sendMessage));
					}
				}
			}))
		{
			return;
		}

		PostJSEvent("frameCall", { frame, jsonData });
	}

	__declspec(dllexport) void ReloadNUI()
	{
		auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();

		rootWindow->GetBrowser()->ReloadIgnoreCache();
	}

	__declspec(dllexport) void SetMainUI(bool enable)
	{
		g_mainUIFlag = enable;
		nui::GiveFocus("mpMenu", enable);
	}

	DLL_EXPORT void SetHideCursor(bool hide)
	{
		g_shouldHideCursor = hide;
	}

	extern std::string GetContext();

	fwRefContainer<NUIWindow> CreateNUIWindow(fwString windowName, int width, int height, fwString windowURL, bool rawBlit/* = false*/, bool instant)
	{
		auto window = NUIWindow::Create(rawBlit, width, height, windowURL, instant, (windowName != "nui_mpMenu") ? nui::GetContext() : "");

		std::unique_lock<std::shared_mutex> lock(windowListMutex);
		windowList[windowName] = window;

		return window;
	}

	DLL_EXPORT fwRefContainer<NUIWindow> CreateNUIWindow(fwString windowName, int width, int height, fwString windowURL, bool rawBlit)
	{
		return CreateNUIWindow(windowName, width, height, windowURL, rawBlit, true);
	}

	__declspec(dllexport) void DestroyNUIWindow(fwString windowName)
	{
		std::unique_lock<std::shared_mutex> lock(windowListMutex);

		auto windowIt = windowList.find(windowName);

		if (windowIt != windowList.end())
		{
			Instance<NUIWindowManager>::Get()->RemoveWindow(windowIt->second.GetRef());

			windowList.erase(windowName);
		}
	}

	__declspec(dllexport) CefBrowser* GetNUIWindowBrowser(fwString windowName)
	{
		auto window = FindNUIWindow(windowName);

		if (window.GetRef())
		{
			return window->GetBrowser();
		}

		return nullptr;
	}

	__declspec(dllexport) void SetNUIWindowURL(fwString windowName, fwString url)
	{
		fwRefContainer<NUIWindow> window = FindNUIWindow(windowName);

		if (!window.GetRef())
		{
			return;
		}

		if (window->GetBrowser() && window->GetBrowser()->GetMainFrame())
		{
			window->GetBrowser()->GetMainFrame()->LoadURL(url);
		}
	}

	OVERLAY_DECL fwRefContainer<nui::GITexture> GetWindowTexture(fwString windowName)
	{
		fwRefContainer<NUIWindow> window = FindNUIWindow(windowName);

		if (!window.GetRef())
		{
			return nullptr;
		}

		return window->GetTexture();
	}

	__declspec(dllexport) void ExecuteWindowScript(const std::string& windowName, const std::string& scriptBit)
	{
		fwRefContainer<NUIWindow> window = FindNUIWindow(windowName);

		if (!window.GetRef())
		{
			return;
		}

		if (window.GetRef() && window->GetBrowser() && window->GetBrowser()->GetMainFrame())
		{
			window->GetBrowser()->GetMainFrame()->ExecuteJavaScript(scriptBit, "internal", 1);
		}
	}

	static std::unordered_map<std::string, std::string> frameList;
	static std::shared_mutex frameListMutex;

	static bool rootWindowTerminated;

	static void CreateFrame(const std::string& frameName, const std::string& frameURLArg, bool instant)
	{
		std::string frameURL = frameURLArg;

		if (frameName == "mpMenu" && launch::IsSDKGuest())
		{
			frameURL = "https://nui-game-internal/sdk-root/shell/mpMenu.html";
		}

		bool exists = false;

		{
			std::shared_lock<std::shared_mutex> lock(frameListMutex);
			exists = frameList.find(frameName) != frameList.end();
		}

		if (!exists)
		{
			if (frameName != "mpMenu" || !Feature_TopLevelMPMenu::RunIfEnabled([&frameName, &frameURL, instant]
			{
				auto winName = fmt::sprintf("nui_%s", frameName);

				auto window = CreateNUIWindow(winName, 1919, 1079, frameURL, true, instant);
				window->SetPaintType(NUIPaintTypePostRender);
				window->SetName(winName);
			}))
			{
				auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();
				if (rootWindow.GetRef())
				{
					auto browser = rootWindow->GetBrowser();

					if (browser)
					{
						auto procMessage = CefProcessMessage::Create("createFrame");
						auto argumentList = procMessage->GetArgumentList();

						argumentList->SetSize(2);
						argumentList->SetString(0, frameName.c_str());
						argumentList->SetString(1, frameURL.c_str());

						browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, procMessage);
					}
				}
			}

			std::unique_lock<std::shared_mutex> lock(frameListMutex);
			frameList.insert({ frameName, frameURL });
		}
	}

	DLL_EXPORT void CreateFrame(const std::string& frameName, const std::string& frameURL)
	{
		CreateFrame(frameName, frameURL, true);
	}

	DLL_EXPORT void PrepareFrame(const std::string& frameName, const std::string& frameURL)
	{
		CreateFrame(frameName, frameURL, false);
	}

	__declspec(dllexport) void DestroyFrame(const std::string& frameName)
	{
		if (frameName == "mpMenu" && Feature_TopLevelMPMenu::RunIfEnabled([&frameName]
		{
			auto winName = fmt::sprintf("nui_%s", frameName);
			DestroyNUIWindow(winName);

			std::unique_lock<std::shared_mutex> lock(frameListMutex);
			frameList.erase(frameName);
		}))
		{
			return;
		}

		auto procMessage = CefProcessMessage::Create("destroyFrame");
		auto argumentList = procMessage->GetArgumentList();

		argumentList->SetSize(1);
		argumentList->SetString(0, frameName.c_str());

		auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();

		if (rootWindow.GetRef())
		{
			auto browser = rootWindow->GetBrowser();

			if (browser)
			{
				browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, procMessage);
			}

			std::unique_lock<std::shared_mutex> lock(frameListMutex);
			frameList.erase(frameName);
		}
	}

	bool HasFrame(const std::string& frameName)
	{
		std::shared_lock<std::shared_mutex> lock(frameListMutex);
		return frameList.find(frameName) != frameList.end();
	}

	void RecreateFrames()
	{
		std::vector<std::pair<std::string, std::string>> frameData;

		{
			std::unique_lock lock(frameListMutex);

			for (auto it = frameList.begin(); it != frameList.end();)
			{
				auto frame = *it;

				if (Feature_TopLevelMPMenu::IsEnabled() && frame.first == "mpMenu")
				{
					++it;					
				}
				else
				{
					frameData.push_back(frame);
					it = frameList.erase(it);
				}
			}
		}

		for (auto& frame : frameData)
		{
			CreateFrame(frame.first, frame.second);
		}
	}

	__declspec(dllexport) void SignalPoll(fwString frameName)
	{
		auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();
		rootWindow->SignalPoll(std::string(frameName.c_str()));
	}
}
