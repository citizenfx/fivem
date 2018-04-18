/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIApp.h"
#include "memdbgon.h"

void NUIApp::OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar)
{
	// add the 'nui://' internal scheme
	registrar->AddCustomScheme("nui", true, false, false, true, false, true);
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
	auto manager = CefCookieManager::GetGlobalManager(nullptr);
	manager->SetSupportedSchemes({ "nui" }, nullptr);
}

void NUIApp::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
	CefRefPtr<CefV8Value> window = context->GetGlobal();

	window->SetValue("registerPollFunction", CefV8Value::CreateFunction("registerPollFunction", this), V8_PROPERTY_ATTRIBUTE_READONLY);
	window->SetValue("registerFrameFunction", CefV8Value::CreateFunction("registerFrameFunction", this), V8_PROPERTY_ATTRIBUTE_READONLY);
	window->SetValue("invokeNative", CefV8Value::CreateFunction("invokeNative", this), V8_PROPERTY_ATTRIBUTE_READONLY);
}

void NUIApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
	command_line->AppendSwitch("enable-experimental-web-platform-features");
	command_line->AppendSwitch("enable-media-stream");
	command_line->AppendSwitch("use-fake-ui-for-media-stream");
	command_line->AppendSwitch("enable-speech-input");
	command_line->AppendSwitch("ignore-gpu-blacklist");
	command_line->AppendSwitch("enable-usermedia-screen-capture");
	command_line->AppendSwitch("disable-direct-composition");
	command_line->AppendSwitchWithValue("default-encoding", "utf-8");
	//command_line->AppendSwitch("disable-gpu-vsync");
	command_line->AppendSwitchWithValue("autoplay-policy", "no-user-gesture-required");
	command_line->AppendSwitch("force-gpu-rasterization");

	// some GPUs are in the GPU blacklist as 'forcing D3D9'
	// this just forces D3D11 anyway.
	command_line->AppendSwitchWithValue("use-angle", "d3d11");

	// M65 enables these by default, but CEF doesn't pass the required phase data at this time (2018-03-31)
	// this breaks scrolling 'randomly' - after a middle click, and some other scenarios
	command_line->AppendSwitchWithValue("disable-features", "TouchpadAndWheelScrollLatching,AsyncWheelEvents");

	// M66 enables this by default, this breaks scrolling in iframes, however only in the Cfx embedder scenario (2018-03-31)
	// cefclient is not affected, code was compared with cefclient but not that different.
	command_line->AppendSwitchWithValue("disable-blink-features", "RootLayerScrolling");
}

bool NUIApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
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
