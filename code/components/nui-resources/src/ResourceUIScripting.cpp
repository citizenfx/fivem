/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <ResourceUI.h>

#include <fxScripting.h>
#include <ScriptEngine.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <scrBind.h>
#include <IteratorView.h>

#include <CL2LaunchMode.h>
#include <HostSharedData.h>
#include <ReverseGameData.h>
#include <CrossBuildRuntime.h>

#include <sstream>
#include <string_view>

#include <Error.h>

#include <boost/algorithm/string/case_conv.hpp>

static std::string CleanURL(const std::string& url)
{
	auto lowerURL = boost::algorithm::to_lower_copy(url);

	if (lowerURL.find("file://") == 0)
	{
		// log an error so that we have insight as to any frequency of use
		FatalError("file:// URI requests in DUI are not supported.\nRequested URL: %s", url);
	}

	if (lowerURL.find("http://") == 0 || lowerURL.find("https://") == 0 || lowerURL.find("nui://") == 0 || lowerURL.find("about:") == 0)
	{
		return url;
	}
	else
	{
		// unknown-ish url, prefix with https://
		return fmt::sprintf("https://%s", url);
	}
}

static InitFunction initFunction([] ()
{
	static auto sendMessageToFrame = [](fx::ScriptContext& context, const char* native, auto& execFn)
	{
		// get the message as JSON and validate it by parsing/recreating (so we won't end up injecting malicious JS into the browser root)
		const char* messageJson = context.GetArgument<const char*>(0);

		rapidjson::Document document;
		document.Parse(messageJson);

		if (!document.HasParseError())
		{
			rapidjson::StringBuffer sb;
			rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

			if (document.Accept(writer))
			{
				// execute in NUI
				execFn(std::string_view(sb.GetString(), sb.GetSize()));

				// and return 'true' to indicate to the script that we succeeded
				context.SetResult(true);
				return;
			}
			else
			{
				trace("%s: writing to JSON writer failed\n", native);
			}
		}
		else
		{
			trace("%s: invalid JSON passed in frame (rapidjson error code %d)\n", native, document.GetParseError());
		}

		context.SetResult(false);
	};

	static auto getExecRootFrameFn = [](const std::string& frameName) -> std::function<void(std::string_view)>
	{
		return [frameName](std::string_view data)
		{
			// somewhat insane sanity check at the last minute
			if (frameName.find('"') == std::string::npos)
			{
				// send to NUI
				nui::PostFrameMessage(frameName, std::string(data));
			}
		};
	};

	fx::ScriptEngine::RegisterNativeHandler("SEND_NUI_MESSAGE", [=] (fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			if (resource)
			{
				fwRefContainer<ResourceUI> resourceUI = resource->GetComponent<ResourceUI>();

				if (resourceUI.GetRef())
				{
					if (resourceUI->HasFrame())
					{
						sendMessageToFrame(context, "SEND_NUI_MESSAGE", getExecRootFrameFn(resource->GetName()));
						return;
					}
					else
					{
						trace("SEND_NUI_MESSAGE: resource %s has no UI frame\n", resource->GetName());
					}
				}
				else
				{
					trace("SEND_NUI_MESSAGE: resource %s has no UI\n", resource->GetName());
				}
			}
			else
			{
				trace("SEND_NUI_MESSAGE: no current resource\n");
			}
		}
		else
		{
			trace("SEND_NUI_MESSAGE called from outside a scripting runtime\n");
		}

		context.SetResult(false);
	});

	fx::ScriptEngine::RegisterNativeHandler("SEND_LOADING_SCREEN_MESSAGE", [=](fx::ScriptContext& context)
	{
		if (nui::HasFrame("loadingScreen"))
		{
			return sendMessageToFrame(context, "SEND_LOADING_SCREEN_MESSAGE", getExecRootFrameFn("loadingScreen"));
		}
		else
		{
			trace("SEND_LOADING_SCREEN_MESSAGE called while the loading screen was shut down\n");
		}

		context.SetResult(false);
	});

	class NUIWindowWrapper;

	static std::multimap<std::string, std::string> resourcesToNuiWindows;
	static std::map<std::string, NUIWindowWrapper*> nuiWindows;
	static int nuiWindowIdx;

	class NUIWindowWrapper
	{
	public:
		NUIWindowWrapper(const char* urlArg, int width, int height)
			: m_mouseX(0), m_mouseY(0)
		{
			fx::OMPtr<IScriptRuntime> runtime;

			if (!FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
			{
				return;
			}

			if (!urlArg)
			{
				return;
			}

			auto url = CleanURL(urlArg);
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			++nuiWindowIdx;
			
			m_autogenHandle = fmt::sprintf("nui_resource_%d", nuiWindowIdx);

			nui::CreateNUIWindow(m_autogenHandle, width, height, url);

			nuiWindows.insert({ m_autogenHandle, this });
			resourcesToNuiWindows.insert({ resource->GetName(), m_autogenHandle });
		}

		void SetURL(const char* url)
		{
			if (!url)
			{
				return;
			}

			nui::SetNUIWindowURL(m_autogenHandle, CleanURL(url));
		}

		void Destroy()
		{
			nui::DestroyNUIWindow(m_autogenHandle);

			nuiWindows.erase(m_autogenHandle);

			// delet this!
			delete this;
		}

		const char* GetHandle()
		{
			return m_autogenHandle.c_str();
		}

		bool SendMessage(const char* jsonString)
		{
			fx::ScriptContextBuffer fakeCxt;
			fakeCxt.Push(jsonString);

			sendMessageToFrame(fakeCxt, "SEND_DUI_MESSAGE", [this](std::string_view data)
			{
				std::stringstream stream;
				stream << "window.postMessage(" << data << ", '*');";

				nui::ExecuteWindowScript(m_autogenHandle, stream.str());
			});

			return fakeCxt.GetResult<bool>();
		}

		bool IsAvailable()
		{
			auto browser = nui::GetNUIWindowBrowser(m_autogenHandle);

			return browser && browser->GetHost();
		}

		void InjectMouseMove(int x, int y)
		{
			auto browser = nui::GetNUIWindowBrowser(m_autogenHandle);

			m_mouseX = x;
			m_mouseY = y;

			if (browser && browser->GetHost())
			{
				CefMouseEvent ev;
				ev.x = x;
				ev.y = y;

				browser->GetHost()->SendMouseMoveEvent(ev, false);
			}
		}

		void InjectMouseDown(const char* button)
		{
			auto browser = nui::GetNUIWindowBrowser(m_autogenHandle);

			if (browser && browser->GetHost())
			{
				CefMouseEvent ev;
				ev.x = m_mouseX;
				ev.y = m_mouseY;

				browser->GetHost()->SendMouseClickEvent(ev, GetButton(button), false, 1);
			}
		}

		void InjectMouseUp(const char* button)
		{
			auto browser = nui::GetNUIWindowBrowser(m_autogenHandle);

			if (browser && browser->GetHost())
			{
				CefMouseEvent ev;
				ev.x = m_mouseX;
				ev.y = m_mouseY;

				browser->GetHost()->SendMouseClickEvent(ev, GetButton(button), true, 1);
			}
		}

		void InjectMouseWheel(int dy, int dx)
		{
			auto browser = nui::GetNUIWindowBrowser(m_autogenHandle);

			if (browser && browser->GetHost())
			{
				CefMouseEvent ev;
				ev.x = m_mouseX;
				ev.y = m_mouseY;

				browser->GetHost()->SendMouseWheelEvent(ev, dx, dy);
			}
		}

	private:
		cef_mouse_button_type_t GetButton(const char* button)
		{
			if (_stricmp(button, "left") == 0)
			{
				return MBT_LEFT;
			}
			else if (_stricmp(button, "right") == 0)
			{
				return MBT_RIGHT;
			}
			else if (_stricmp(button, "middle") == 0)
			{
				return MBT_MIDDLE;
			}
			else
			{
				return MBT_MIDDLE;
			}
		}

	private:
		std::string m_autogenHandle;

		int m_mouseX;
		int m_mouseY;
	};

	scrBindClass<NUIWindowWrapper>()
		.AddConstructor<void(*)(const char*, int, int)>("CREATE_DUI")
		.AddMethod("SET_DUI_URL", &NUIWindowWrapper::SetURL)
		.AddMethod("SEND_DUI_MESSAGE", &NUIWindowWrapper::SendMessage)
		.AddMethod("GET_DUI_HANDLE", &NUIWindowWrapper::GetHandle)
		.AddMethod("IS_DUI_AVAILABLE", &NUIWindowWrapper::IsAvailable)
		.AddMethod("SEND_DUI_MOUSE_MOVE", &NUIWindowWrapper::InjectMouseMove)
		.AddMethod("SEND_DUI_MOUSE_DOWN", &NUIWindowWrapper::InjectMouseDown)
		.AddMethod("SEND_DUI_MOUSE_UP", &NUIWindowWrapper::InjectMouseUp)
		.AddMethod("SEND_DUI_MOUSE_WHEEL", &NUIWindowWrapper::InjectMouseWheel)
		.AddMethod("DESTROY_DUI", &NUIWindowWrapper::Destroy);		

	// this *was* a multiset before but some resources would not correctly pair set/unset and then be stuck in 'set' state
	static std::unordered_set<std::string> focusVotes;
	static std::unordered_set<std::string> focusCursorVotes;
	static std::unordered_set<std::string> focusKeepInputVotes;

	static bool lastFocus = false;
	static bool lastFocusCursor = false;
	static bool lastFocusKeepInput = false;

	static auto updateFocus = []()
	{
		{
			auto shouldFocus = !focusVotes.empty();
			auto shouldCursor = !focusCursorVotes.empty();

			if (shouldFocus != lastFocus || shouldCursor != lastFocusCursor)
			{
				nui::GiveFocus("", shouldFocus, shouldCursor);
				lastFocus = shouldFocus;
				lastFocusCursor = shouldCursor;
			}
		}

		{
			// find if the keep-input list overlaps with the general vote list
			bool shouldKeepInput = false;

			for (const auto& resource : focusKeepInputVotes)
			{
				if (focusVotes.find(resource) != focusVotes.end() ||
					focusCursorVotes.find(resource) != focusCursorVotes.end())
				{
					shouldKeepInput = true;
					break;
				}
			}

			if (shouldKeepInput != lastFocusKeepInput)
			{
				nui::KeepInput(shouldKeepInput);
				lastFocusKeepInput = shouldKeepInput;
			}
		}
	};

	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		resource->OnStop.Connect([resource]()
		{
			auto resourceName = resource->GetName();

			for (auto dui : fx::GetIteratorView(resourcesToNuiWindows.equal_range(resourceName)))
			{
				auto it = nuiWindows.find(dui.second);

				if (it != nuiWindows.end())
				{
					it->second->Destroy();
					nuiWindows.erase(dui.second);
				}
			}

			focusVotes.erase(resourceName);
			focusKeepInputVotes.erase(resourceName);
			focusCursorVotes.erase(resourceName);

			updateFocus();
		});
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_NUI_FOCUSED", [] (fx::ScriptContext& context)
	{
		context.SetResult(nui::HasFocus());
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_NUI_FOCUS_KEEPING_INPUT", [] (fx::ScriptContext& context)
	{
		context.SetResult(nui::HasFocusKeepInput());
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_NUI_FOCUS", [] (fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			if (resource)
			{
				fwRefContainer<ResourceUI> resourceUI = resource->GetComponent<ResourceUI>();

				if (resourceUI.GetRef())
				{
					if (resourceUI->HasFrame())
					{
						if (resource->GetName().find('"') == std::string::npos)
						{
							bool hasFocus = context.GetArgument<bool>(0);
							bool hasCursor = context.GetArgument<bool>(1);

							const char* functionName = (hasFocus) ? "focusFrame" : "blurFrame";
							nui::PostRootMessage(fmt::sprintf(R"({ "type": "%s", "frameName": "%s" } )", functionName, resource->GetName()));

							if (hasFocus)
							{
								focusVotes.insert(resource->GetName());
							}
							else if (auto it = focusVotes.find(resource->GetName()); it != focusVotes.end())
							{
								focusVotes.erase(it);
							}

							if (hasCursor)
							{
								focusCursorVotes.insert(resource->GetName());
							}
							else if (auto it = focusCursorVotes.find(resource->GetName()); it != focusCursorVotes.end())
							{
								focusCursorVotes.erase(it);
							}

							updateFocus();
						}
					}
				}
			}
		}
	});

	if (launch::IsSDKGuest())
	{
		fx::ScriptEngine::RegisterNativeHandler("GET_NUI_CURSOR_POSITION", [](fx::ScriptContext& context)
		{
			static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");

			*context.GetArgument<int*>(0) = rgd->mouseAbsX;
			*context.GetArgument<int*>(1) = rgd->mouseAbsY;
		});
	}
	else
	{
		fx::ScriptEngine::RegisterNativeHandler("GET_NUI_CURSOR_POSITION", [](fx::ScriptContext& context)
		{
			POINT cursorPos;
			GetCursorPos(&cursorPos);
			ScreenToClient(CoreGetGameWindow(), &cursorPos);

			*context.GetArgument<int*>(0) = cursorPos.x;
			*context.GetArgument<int*>(1) = cursorPos.y;
		});
	}

	fx::ScriptEngine::RegisterNativeHandler("SET_NUI_FOCUS_KEEP_INPUT", [](fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			if (resource)
			{
				bool shouldKeepInput = context.GetArgument<bool>(0);

				auto& voteList = focusKeepInputVotes;

				if (shouldKeepInput)
				{
					voteList.insert(resource->GetName());
				}
				else
				{
					// remove just one entry
					if (auto it = voteList.find(resource->GetName()); it != voteList.end())
					{
						voteList.erase(it);
					}
				}

				updateFocus();
			}
		}
	});
});
