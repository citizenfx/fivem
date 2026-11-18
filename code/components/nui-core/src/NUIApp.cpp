/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIApp.h"
#include "CefOverlay.h"
#include <CoreConsole.h>
#include "memdbgon.h"
#include "NUIDummyClient.h"
#include <CrossBuildRuntime.h>
#include <PureModeState.h>

#include <include/cef_parser.h>
#include <EpoxyScript.h>

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.UserProfile.h>
#pragma comment(lib, "runtimeobject")

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::System::UserProfile;

std::string g_handoverJson = "null";
std::string g_handoverServerAddress = "127.0.0.1:30120";

static std::string ToJsString(const std::string& s)
{
	auto v = CefValue::Create();
	v->SetString(s);
	return CefWriteJSON(v, JSON_WRITER_DEFAULT).ToString();
}

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
	CefRefPtr<CefRequestContext> requestContext = CefRequestContext::GetGlobalContext();

	constexpr std::array<std::string_view, 21> kDisabledPreferences{
		"translate",
		"autologin.enabled",
		"printing.enabled",
		"shopping_list_enabled",
		"search.suggest_enabled",
		"side_panel.google_search_side_panel_enabled",
		"side_search.enabled",
		"signin.allowed",
		"signin.allowed_on_next_startup",
		"url_keyed_anonymized_data_collection.enabled",
		"autofill.credit_card_enabled",
		"autofill.enabled",
		"autofill.iban_enabled",
		"autofill.payment_card_benefits",
		"autofill.payment_cvc_storage",
		"autofill.profile_enabled",
		"browser_labs_enabled",
		"credentials_enable_autosignin",
		"credentials_enable_service",
		"payments.can_make_payment_enabled",
		"browser.enable_spellchecking"
	};

	CefRefPtr<CefValue> featureOption = CefValue::Create();
	featureOption->SetBool(false);

	CefString errorMessage;
	for (std::string_view feature : kDisabledPreferences)
	{
		requestContext->SetPreference(feature.data(), featureOption.get(), errorMessage);
	}

	featureOption->SetBool(true);
	// Extra toggles to further disable extensions
	requestContext->SetPreference("extensions.block_external_extensions", featureOption.get(), errorMessage);
	requestContext->SetPreference("extensions.disabled", featureOption.get(), errorMessage);
}

void NUIApp::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
	CefRefPtr<CefV8Value> window = context->GetGlobal();

	window->SetValue("registerPollFunction", CefV8Value::CreateFunction("registerPollFunction", this), V8_PROPERTY_ATTRIBUTE_READONLY);
	window->SetValue("registerFrameFunction", CefV8Value::CreateFunction("registerFrameFunction", this), V8_PROPERTY_ATTRIBUTE_READONLY);
	window->SetValue("registerPushFunction", CefV8Value::CreateFunction("registerPushFunction", this), V8_PROPERTY_ATTRIBUTE_READONLY);

	// register epoxy functions in order to provide CORS compliant messaging between game root and resource windows without breaking backwards compatibility.
	window->SetValue("sendEpoxyMessage", CefV8Value::CreateFunction("sendEpoxyMessage", this), V8_PROPERTY_ATTRIBUTE_READONLY);
	window->SetValue("registerEpoxyHandler", CefV8Value::CreateFunction("registerEpoxyHandler", this), V8_PROPERTY_ATTRIBUTE_NONE);

    // Load epoxy on context creation (and not devtools)
	if (auto parent = frame->GetParent(); parent && parent->IsMain())
	{
		frame->ExecuteJavaScript(fmt::sprintf(g_epoxyScript,
								 frame->GetName().ToString(), ToJsString(g_handoverJson), g_handoverServerAddress),
		"nui://epoxy", 0);
	}	                     
	frame->ExecuteJavaScript(g_gameViewScript, "nui://game-view-wrapper", 0);

	{
		winrt::init_apartment();

		std::vector<std::wstring> langList;
		for (const auto& lang : GlobalizationPreferences::Languages())
		{
			langList.push_back(std::wstring{ lang });
		}

		auto languages = CefV8Value::CreateArray(int(langList.size()));
		for (size_t i = 0; i < langList.size(); i++)
		{
			languages->SetValue(int(i), CefV8Value::CreateString(CefString{ (char16_t*)langList[i].data(), langList[i].length(), true }));
		}

		window->SetValue("nuiSystemLanguages", languages, V8_PROPERTY_ATTRIBUTE_READONLY);
	}

	window->SetValue("invokeNative", CefV8Value::CreateFunction("invokeNative", this), V8_PROPERTY_ATTRIBUTE_READONLY);
#ifdef NUI_WITH_AUDIO_SINKS
	window->SetValue("nuiSetAudioCategory", CefV8Value::CreateFunction("nuiSetAudioCategory", this), V8_PROPERTY_ATTRIBUTE_READONLY);
#endif
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
	window->SetValue("nuiTargetGameBuild", CefV8Value::CreateInt(xbr::GetRequestedGameBuild()), V8_PROPERTY_ATTRIBUTE_READONLY);
	window->SetValue("nuiTargetGamePureLevel", CefV8Value::CreateInt(fx::client::GetPureLevel()), V8_PROPERTY_ATTRIBUTE_READONLY);

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
			"fxdkClipboardRead",
			"fxdkClipboardWrite"
		};

		for (auto const& handler : fxdkHandlers)
		{
			window->SetValue(handler, CefV8Value::CreateFunction(handler, this), V8_PROPERTY_ATTRIBUTE_READONLY);
		}
	}
}

void NUIApp::OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
{
	for (auto& handler : m_v8ReleaseHandlers)
	{
		handler(context);
	}
}
	
void NUIApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
#if defined(IS_RDR3)
	// on RedM, an in-process GPU makes CEF's D3D11 device race other devices for the nui-core texture hooks,
	// and losing that race fails the Vulkan import at startup ("Failed to allocate memory for Vulkan")
	static ConVar<bool> nuiUseInProcessGpu("nui_useInProcessGpu", ConVar_Archive, false);
#else
	static ConVar<bool> nuiUseInProcessGpu("nui_useInProcessGpu", ConVar_Archive, true);
#endif

	static std::string defaultUiUrl = "https://nui-game-internal/ui/app/index.html";
	static ConVar<std::string> uiUrlVar("ui_url", ConVar_UserPref, defaultUiUrl);

	if (uiUrlVar.GetValue() != defaultUiUrl)
	{
		CefString uiUrl(uiUrlVar.GetValue());
		CefURLParts uiUrlParts;

		if (CefParseURL(uiUrl, uiUrlParts) && uiUrlParts.origin.length > 0)
		{
			// Allow secure context for insecure localhost
			command_line->AppendSwitchWithValue("unsafely-treat-insecure-origin-as-secure", uiUrlParts.origin.str);
		}
	}

	// GPU Flags
	if (nuiUseInProcessGpu.GetValue())
	{
		// In process GPU also disables the GPU watchdog.
		command_line->AppendSwitch("in-process-gpu");
	}

	// FxDK makes use of the Views Framework within CEF
	// which depends on direct composition in order to draw.
	if (!launch::IsSDK())
	{
		command_line->AppendSwitch("disable-direct-composition");
	}
	command_line->AppendSwitch("ignore-gpu-blocklist");
	command_line->AppendSwitch("enable-gpu-rasterization");
	command_line->AppendSwitch("disable-gpu-process-crash-limit");
	// some GPUs are in the GPU blacklist as 'forcing D3D9'
	// this just forces D3D11 anyway.
	command_line->AppendSwitchWithValue("use-angle", "d3d11");
	//

	// Previously we would enable *all* experimental features 
	// that were bundled with the chromium version
	// This shouldn't have been the case, rather any experimental features 
	// should be added on a case-by-case basis.
	//command_line->AppendSwitch("enable-experimental-web-platform-features");

	command_line->AppendSwitchWithValue("disable-blink-features", 
		// These experimental features are currently broken as of M144 (October 2026 build)
		// While experimental web features are disabled, it's worth keeping a list of ones that **are** broken in NUI.
		"WidthAndHeightAsPresentationAttributesOnNestedSvg,"  // Breaks SVG's with custom height/width.
															  // see https://issues.chromium.org/issues/449170647 
		"SelectionAndFocusedVisiblePositionMatch,"			  // Known bad feature, can lead to UI/GPU process hangs
															  // Issue details are currently private.

		// Other functionality that aren't required in NUI.
		"SpellcheckAPI,"                                      // Breaks assumptions made by a majority of FiveM resources
		                                                      // As pre-m144 Spellcheck did not work as intended.
		"DocumentPictureInPictureAPI,"                        // Not needed in NUI

	);

	// Disable features that aren't desired in NUI.
	command_line->AppendSwitchWithValue("disable-features", 
		"HardwareMediaKeyHandling," // Don't let NUI hijack hardware media key handling from other processes
		"PrintCompositor," // NUI don't need print compositor (for printing pages)
		"AutofillServerCommunication," // NUI doesn't need autofill
		"AutofillEnableAccountWalletStorage," // NUI also doesn't need google wallet autofill
		"CalculateNativeWinOcclusion," // Not relevant in OSR
		"WebUSB," // NUI contexts shouldn't have access to USB API's
	    "WebBluetooth," // Same with WebBluetooth API's
		"WebHID," // Same with HID API's
		"SerialAPI," // Same with SerialAPI
		"OptimizationHints," // fetch hints for preloading don't work in NUI and make no sense being enabled
		"OptimizationHintsFetching," // ^, should already be partially no-op in CEF. But disable it anyway
		"MediaRouter," // NUI does not need anything related to presentation or casting to a TV.
		"DialMediaRouteProvider," // ^
		"MetricsReporting,"
		"SpellcheckService,"
		"Spellcheck,"
		"GCMDriver,"
		"StorageNotificationService,"// Shows a popup when running low on cache space.
									 // See https://github.com/chromiumembedded/cef/issues/3966
		"LiveCaption," // This doesn't work in CEF. Disable it so NUI resources are aware.
		"ComputePressure," // Disables PressureObserver API in Chromium. Introduced in M125
						   // https://developer.mozilla.org/en-US/docs/Web/API/PressureObserver
		"CpuPerformanceProbe," // Causes CEF to give misleading errors.
		                       // PressureObserver depends on this so that has to be disabled as well.
		"ThirdPartyStoragePartitioning" // This breaks migration of NUI localStorage and similar from M103
	);


	command_line->AppendSwitchWithValue("default-encoding", "utf-8");
	command_line->AppendSwitchWithValue("autoplay-policy", "no-user-gesture-required");
	// For lower end systems with fewer cores we want to limit the amount of threads.
	// On lower end systems in busy scenarios could lead to a negative impact on the game performance
	{
		constexpr int kMinRasterThreads = 1;
		constexpr int kMaxRasterThreads = 4;

		int totalCores = std::thread::hardware_concurrency();

		if (totalCores < 8)
		{
			// For a majority of the work, Rasterisation is handled on the GPU.
			// However according to https://www.chromium.org/developers/design-documents/chromium-graphics/how-to-get-gpu-rasterization/
			// it can veto it self and force rasterisation work to be handled on the CPU.
			command_line->AppendSwitchWithValue("num-raster-threads", std::to_string(std::clamp<int>(totalCores / 4, kMinRasterThreads, kMaxRasterThreads)));
		}
	}
	
	// null-route urls that we don't want in CEF (tied to chrome services)
	command_line->AppendSwitchWithValue("connectivity-check-url", "http://0.0.0.0");
	command_line->AppendSwitchWithValue("lso-url", "http://0.0.0.0");
	command_line->AppendSwitchWithValue("sync-url", "http://0.0.0.0");
	command_line->AppendSwitchWithValue("crash-server-url", "http://0.0.0.0");

	// Disable features forced into CEF by Chrome runtime/bootstrap
	command_line->AppendSwitch("disable-gaia-services");
	command_line->AppendSwitch("disable-sync");
	command_line->AppendSwitch("disable-extensions");
	command_line->AppendSwitch("disable-spell-checking");
	command_line->AppendSwitch("disable-autocorrect");
	command_line->AppendSwitch("no-spell-check-suggestions");
	command_line->AppendSwitch("hide-crash-restore-bubble");
	command_line->AppendSwitch("disable-background-networking");
	command_line->AppendSwitch("disable-default-apps");
	command_line->AppendSwitchWithValue("allow-browser-signin", "false");

	// disable accelerated video encoding.
	// In M91 this caused hanging when playing media.
	// In M144 it causes inconsistent stuttering/performance issues
	command_line->AppendSwitch("disable-accelerated-video-decode");
	command_line->AppendSwitch("disable-accelerated-video-encode");
	command_line->AppendSwitch("disable-accelerated-mjpeg-decode");

	// Block NUI from having presentation api. https://developer.mozilla.org/en-US/docs/Web/API/Presentation_API
	command_line->AppendSwitch("disable-presentation-api");

	// Don't allow accidental zooming in on a trackpad
	command_line->AppendSwitch("disable-pinch");

	// important switch to prevent users from mentioning 'why are there 50 chromes again'
	command_line->AppendSwitch("disable-site-isolation-trials");

	// TODO: remove this flag in the future
	command_line->AppendSwitch("disable-web-security");
}

bool NUIApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
	auto handler = m_processMessageHandlers.find(message->GetName());
	if (handler == m_processMessageHandlers.end())
	{
		trace("Unknown NUI process message: %s\n", message->GetName().ToString().c_str());
		return false;
	}

	return handler->second(browser, message);
}

CefRefPtr<CefClient> NUIApp::GetDefaultClient()
{
	static CefRefPtr<NUIDummyClient> dummyClient;
	if (!dummyClient)
	{
		dummyClient = new NUIDummyClient();
	}

	// If anything tries to call GetDefaultClient they will get a dummy client that will exit immediately
	return dummyClient;
}

bool NUIApp::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception)
{
	auto handler = m_v8Handlers.find(name);
	if (handler == m_v8Handlers.end())
	{
		trace("Unknown NUI function: %s\n", name.ToString().c_str());
		return false;
	}

	retval = handler->second(arguments, exception);
	return true;
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
