/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIApp.h"
#include "CefOverlay.h"
#include <CoreConsole.h>
#include "memdbgon.h"

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.UserProfile.h>
#pragma comment(lib, "runtimeobject")

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::System::UserProfile;

void NUIApp::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar)
{
	// add the 'nui://' internal scheme
	registrar->AddCustomScheme("nui", CEF_SCHEME_OPTION_STANDARD | CEF_SCHEME_OPTION_SECURE | CEF_SCHEME_OPTION_CORS_ENABLED | CEF_SCHEME_OPTION_FETCH_ENABLED);
	//registrar->AddCustomScheme("nui", true, false, false, true, false, true);
}

// null data resource functions
bool NUIApp::GetDataResource(int resourceID, void*& data, size_t& data_size)
{
	return false;
}

bool NUIApp::GetDataResourceForScale(int resource_id, ScaleFactor scale_factor, void*& data, size_t& data_size)
{
	return false;
}

bool NUIApp::GetLocalizedString(int messageID, CefString& string)
{
	string = "";
	return true;
}

void NUIApp::OnContextInitialized()
{
}

void NUIApp::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
	CefRefPtr<CefV8Value> window = context->GetGlobal();

#ifdef USE_NUI_ROOTLESS
	auto et = window->GetValue("EventTarget");

	if (et)
	{
		auto prototype = et->GetValue("prototype");

		if (prototype)
		{
			auto origEventListener = prototype->GetValue("addEventListener");
			m_origEventListeners[frame->GetIdentifier()] = origEventListener;

			prototype->SetValue("addEventListener", CefV8Value::CreateFunction("addEventListener", this), V8_PROPERTY_ATTRIBUTE_NONE);
		}
	}
#endif

	window->SetValue("registerPollFunction", CefV8Value::CreateFunction("registerPollFunction", this), V8_PROPERTY_ATTRIBUTE_READONLY);
	window->SetValue("registerFrameFunction", CefV8Value::CreateFunction("registerFrameFunction", this), V8_PROPERTY_ATTRIBUTE_READONLY);
	window->SetValue("registerPushFunction", CefV8Value::CreateFunction("registerPushFunction", this), V8_PROPERTY_ATTRIBUTE_READONLY);

#ifdef USE_NUI_ROOTLESS
	window->SetValue("GetParentResourceName", CefV8Value::CreateFunction("GetParentResourceName", this), V8_PROPERTY_ATTRIBUTE_READONLY);
#endif

	if (!IsWindows10OrGreater())
	{
		ULONG langs = 0;
		ULONG langSize = 0;
		GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &langs, NULL, &langSize);

		std::vector<wchar_t> uiLangs(langSize);
		if (GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &langs, uiLangs.data(), &langSize))
		{
			std::vector<std::wstring_view> langList;
			wchar_t* ptr = &uiLangs[0];

			for (ULONG i = 0; i < langs; i++)
			{
				auto len = wcslen(ptr);
				langList.emplace_back(ptr, len);

				ptr += len + 1;
			}

			auto languages = CefV8Value::CreateArray(langList.size());

			for (size_t i = 0; i < langList.size(); i++)
			{
				languages->SetValue(i, CefV8Value::CreateString(CefString{ langList[i].data(), langList[i].length(), true }));
			}

			window->SetValue("nuiSystemLanguages", languages, V8_PROPERTY_ATTRIBUTE_READONLY);
		}
	}
	else
	{
		winrt::init_apartment();

		std::vector<std::wstring> langList;

		for (const auto& lang : GlobalizationPreferences::Languages())
		{
			langList.push_back(std::wstring{ lang });
		}

		auto languages = CefV8Value::CreateArray(langList.size());

		for (size_t i = 0; i < langList.size(); i++)
		{
			languages->SetValue(i, CefV8Value::CreateString(CefString{ langList[i].data(), langList[i].length(), true }));
		}

		window->SetValue("nuiSystemLanguages", languages, V8_PROPERTY_ATTRIBUTE_READONLY);
	}

	window->SetValue("invokeNative", CefV8Value::CreateFunction("invokeNative", this), V8_PROPERTY_ATTRIBUTE_READONLY);
	window->SetValue("nuiSetAudioCategory", CefV8Value::CreateFunction("nuiSetAudioCategory", this), V8_PROPERTY_ATTRIBUTE_READONLY);
	window->SetValue("nuiTargetGame", CefV8Value::CreateString(
#ifdef IS_LAUNCHER
		"launcher"
#elif defined(IS_RDR3)
		"rdr3"
#elif defined(GTA_FIVE)
		"gta5"
#elif defined (GTA_NY)
		"ny"
#else
		"unknown"
#endif
	), V8_PROPERTY_ATTRIBUTE_READONLY);


	// FxDK API
	{
		std::vector<std::string> fxdkHandlers{
			"resizeGame",
			"initRGDInput",
			"setRawMouseCapture",
			"sendMouseWheel",
			"setKeyState",
			"setMouseButtonState",
			"openDevTools",
			"setFPSLimit",
			"setInputChar",
			"setWorldEditorControls",
			"setWorldEditorMouse",
			"sendGameClientEvent",
			"fxdkSendApiMessage",
			"fxdkOpenSelectFolderDialog",
			"fxdkOpenSelectFileDialog",
		};

		for (auto const& handler : fxdkHandlers)
		{
			window->SetValue(handler, CefV8Value::CreateFunction(handler, this), V8_PROPERTY_ATTRIBUTE_READONLY);
		}
	}
}

void NUIApp::OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
#ifdef USE_NUI_ROOTLESS
	m_origEventListeners.erase(frame->GetIdentifier());
#endif

	for (auto& handler : m_v8ReleaseHandlers)
	{
		handler(context);
	}
}

void NUIApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
	static ConVar<bool> nuiUseInProcessGpu("nui_useInProcessGpu", ConVar_Archive, false);

	// weird dlls crash
	bool hasWeirdStuff = (GetFileAttributes(MakeRelativeGamePath(L"d3d11.dll").c_str()) != INVALID_FILE_ATTRIBUTES);

	if (nuiUseInProcessGpu.GetValue() && !hasWeirdStuff)
	{
		command_line->AppendSwitch("in-process-gpu");
	}

	command_line->AppendSwitch("enable-experimental-web-platform-features");
	command_line->AppendSwitch("ignore-gpu-blacklist");
	command_line->AppendSwitch("ignore-gpu-blocklist"); // future proofing for when Google disables the above
	command_line->AppendSwitch("disable-direct-composition");
	command_line->AppendSwitch("disable-gpu-driver-bug-workarounds");
	command_line->AppendSwitchWithValue("default-encoding", "utf-8");
	command_line->AppendSwitchWithValue("autoplay-policy", "no-user-gesture-required");

#if !GTA_NY
	command_line->AppendSwitch("enable-gpu-rasterization");
#else
	command_line->AppendSwitch("disable-gpu-vsync");
#endif

	command_line->AppendSwitch("disable-gpu-process-crash-limit");

	// important switch to prevent users from mentioning 'why are there 50 chromes again'
	command_line->AppendSwitch("disable-site-isolation-trials");

	// some GPUs are in the GPU blacklist as 'forcing D3D9'
	// this just forces D3D11 anyway.
	command_line->AppendSwitchWithValue("use-angle", "d3d11");

	// CORB is not handled by CEF CefAddCrossOriginWhitelistEntry, disable CORS entirely
	command_line->AppendSwitch("disable-web-security");

	// register the CitizenFX game view plugin
	// in M73+ it ends up entirely breaking UI rendering
#if !GTA_NY
	command_line->AppendSwitchWithValue("register-pepper-plugins", fmt::sprintf("%s;application/x-cfx-game-view", ToNarrow(MakeRelativeCitPath(L"bin\\d3d_rendering.dll"))));
#endif
}

bool NUIApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
	auto handler = m_processMessageHandlers.find(message->GetName());
	bool success = false;

	if (handler != m_processMessageHandlers.end())
	{
		success = handler->second(browser, message);
	}
	else
	{
		trace("Unknown NUI process message: %s\n", message->GetName().ToString().c_str());
	}

	return success;
}

CefRefPtr<CefRenderProcessHandler> NUIApp::GetRenderProcessHandler()
{
	return this;
}

bool NUIApp::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception)
{
#ifdef USE_NUI_ROOTLESS
	if (name == "addEventListener")
	{
		auto cxt = CefV8Context::GetCurrentContext();
		auto frame = cxt->GetFrame();

		if (frame)
		{
			auto origHandler = m_origEventListeners.find(frame->GetIdentifier());

			if (origHandler != m_origEventListeners.end())
			{
				retval = origHandler->second->ExecuteFunction(object, arguments);

				if (arguments.size() > 0 && arguments[0]->IsString() && arguments[0]->GetStringValue() == "message")
				{
					auto global = cxt->GetGlobal();
					auto fn = global->GetValue("nuiInternalCallMessages");

					if (fn)
					{
						CefV8ValueList a;
						fn->ExecuteFunction(global, a);
					}

					global->SetValue("nuiInternalHandledMessages", CefV8Value::CreateBool(true), V8_PROPERTY_ATTRIBUTE_READONLY);
				}
			}
		}

		return true;
	}
#endif

	auto handler = m_v8Handlers.find(name);
	bool success = false;

	if (handler != m_v8Handlers.end())
	{
		retval = handler->second(arguments, exception);

		success = true;
	}
	else
	{
		trace("Unknown NUI function: %s\n", name.ToString().c_str());
	}

	return success;
}

void NUIApp::AddProcessMessageHandler(std::string key, TProcessMessageHandler handler)
{
	m_processMessageHandlers[key] = handler;
}

void NUIApp::AddV8Handler(std::string key, TV8Handler handler)
{
	m_v8Handlers[key] = handler;
}

void NUIApp::AddContextReleaseHandler(TContextReleaseHandler handler)
{
	m_v8ReleaseHandlers.emplace_back(handler);
}
