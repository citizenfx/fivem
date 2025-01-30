#include <StdInc.h>

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"
#include "include/cef_app.h"
#include "include/cef_parser.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/base/cef_callback_helpers.h"

#include "include/wrapper/cef_stream_resource_handler.h"

#include <boost/property_tree/xml_parser.hpp>
#include <sstream>
#include <json.hpp>

#include <CfxSubProcess.h>

#include "FormData.h"

enum class ScuiAuthFlow
{
	LegitimacyNui,
	SteamAuth,
	EpicAuth,
};

class SimpleApp : public CefApp, public CefBrowserProcessHandler {
public:
	SimpleApp(ScuiAuthFlow flow, const std::string& extraJson);

	// CefApp methods:
	virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler()
		override {
		return this;
	}

	// CefBrowserProcessHandler methods:
	virtual void OnContextInitialized() override;

	virtual void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) override;

private:
	ScuiAuthFlow flow_;
	std::string extra_json_;

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(SimpleApp);
};

SimpleApp::SimpleApp(ScuiAuthFlow flow, const std::string& extraJson)
	: flow_(flow), extra_json_(extraJson)
{

}

void SimpleApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
	command_line->AppendSwitch("disable-extensions");
	command_line->AppendSwitch("disable-pdf-extension");
	command_line->AppendSwitch("disable-gpu");
	command_line->AppendSwitch("ignore-certificate-errors");
	command_line->AppendSwitch("disable-site-isolation-trials");
	command_line->AppendSwitchWithValue("disable-blink-features", "AutomationControlled");
}

namespace {

	// When using the Views framework this object provides the delegate
	// implementation for the CefWindow that hosts the Views-based browser.
	class SimpleWindowDelegate : public CefWindowDelegate {
	public:
		explicit SimpleWindowDelegate(CefRefPtr<CefBrowserView> browser_view)
			: browser_view_(browser_view) {}

		void OnWindowCreated(CefRefPtr<CefWindow> window) override
		{
			// Add the browser view and show the window.
			window->AddChildView(browser_view_);
			//window->CenterWindow(CefSize(705, 535));
			window->CenterWindow(CefSize(1280, 720));

			// Do not show by default
			//window->Show();

			// Give keyboard focus to the browser view.
			browser_view_->RequestFocus();
		}

		void OnWindowDestroyed(CefRefPtr<CefWindow> window) override {
			browser_view_ = nullptr;
		}

		bool CanClose(CefRefPtr<CefWindow> window) override {
			// Allow the window to close if the browser says it's OK.
			CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
			if (browser)
				return browser->GetHost()->TryCloseBrowser();
			return true;
		}

	private:
		CefRefPtr<CefBrowserView> browser_view_;

		IMPLEMENT_REFCOUNTING(SimpleWindowDelegate);
		DISALLOW_COPY_AND_ASSIGN(SimpleWindowDelegate);
	};

}  // namespace

class SimpleHandler : public CefClient,
	public CefDisplayHandler,
	public CefLifeSpanHandler,
	public CefLoadHandler,
	public CefRequestHandler,
	public CefResourceRequestHandler{
public:
	explicit SimpleHandler(ScuiAuthFlow flow, const std::string& extraJson);
	~SimpleHandler();

	// Provide access to the single global instance of this object.
	static SimpleHandler* GetInstance();

	// CefClient methods:
	virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override {
		return this;
	}
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override {
		return this;
	}
	virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override {
		return this;
	}
	virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }

	virtual CefRefPtr<CefResourceRequestHandler> GetResourceRequestHandler(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_navigation, bool is_download, const CefString& request_initiator, bool& disable_default_handling) override
	{
		return this;
	}

	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override;

	// CefDisplayHandler methods:
	virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
		const CefString& title) override;

	// CefLifeSpanHandler methods:
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
	virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

	// CefLoadHandler methods:
	virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type) override;

	virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) override;

	virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		ErrorCode errorCode,
		const CefString& errorText,
		const CefString& failedUrl) override;

	virtual ReturnValue OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) override;

	// Request that all existing browser windows close.
	void CloseAllBrowsers(bool force_close);

	bool IsClosing() const { return is_closing_; }

	void SetWindow(CefRefPtr<CefWindow> window)
	{
		window_ = window;
	}

	CefRefPtr<CefWindow> GetWindow()
	{
		return window_;
	}

	virtual CefRefPtr<CefResourceHandler> GetResourceHandler(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request) override
	{
		if (request->GetURL() == "https://rgl.rockstargames.com/temp.css")
		{
			static std::string file = fmt::sprintf(R"(
.rememberContainer, p[class^="AuthHeader__signUpLink"]
{
	display: none;
}

.UI__Alert__info .UI__Alert__text
{
	display: none;
}

.UI__Alert__info .UI__Alert__content:after
{
	content: 'A Rockstar Games Social Club account owning %s is required to play %s.';
	max-width: 600px;
	display: inline-block;
}
)",
#ifdef GTA_FIVE
			"Grand Theft Auto V",
			"FiveM"
#elif defined(IS_RDR3)
			"Red Dead Redemption 2 or Red Dead Online",
			"RedM"
#else
			"Grand Theft Auto IV: Complete Edition",
			"LibertyM"
#endif
			);

			return new CefStreamResourceHandler("text/css", CefStreamReader::CreateForData(file.data(), file.size()));
		}

		return nullptr;
	}

private:
	std::string GetRgscInitCode();

	// List of existing browser windows. Only accessed on the CEF UI thread.
	typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
	BrowserList browser_list_;

	CefRefPtr<CefWindow> window_;

	bool is_closing_;

	ScuiAuthFlow flow_;
	std::string extra_json_;

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(SimpleHandler);
};

void SimpleApp::OnContextInitialized()
{
	CEF_REQUIRE_UI_THREAD();

	CefRefPtr<SimpleHandler> handler(new SimpleHandler(flow_, extra_json_));

	// Specify CEF browser settings here.
	CefBrowserSettings browser_settings;

	std::string url = "https://rgl.rockstargames.com/launcher";

	// Create the BrowserView.
	CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
	handler, url, browser_settings, {}, nullptr, nullptr);

	// Create the Window. It will show itself after creation.
	handler->SetWindow(CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(browser_view)));

	// Show window for normal flow, i.e. not steam/epic
	if (flow_ == ScuiAuthFlow::LegitimacyNui)
	{
		handler->GetWindow()->Show();
	}
}

namespace {

	SimpleHandler* g_instance = NULL;

}  // namespace

SimpleHandler::SimpleHandler(ScuiAuthFlow flow, const std::string& extraJson)
	: is_closing_(false), flow_(flow), extra_json_(extraJson) {
	DCHECK(!g_instance);
	g_instance = this;
}

SimpleHandler::~SimpleHandler() {
	g_instance = NULL;
}

// static
SimpleHandler* SimpleHandler::GetInstance() {
	return g_instance;
}

void SimpleHandler::OnTitleChange(CefRefPtr<CefBrowser> browser,
	const CefString& title) {
	CEF_REQUIRE_UI_THREAD();

	// Set the title of the window using the Views framework.
	CefRefPtr<CefBrowserView> browser_view =
		CefBrowserView::GetForBrowser(browser);
	if (browser_view) {
		CefRefPtr<CefWindow> window = browser_view->GetWindow();
		if (window)
			window->SetTitle(title);
	}
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	// Add to the list of existing browsers.
	browser_list_.push_back(browser);
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	// Closing the main window requires special handling. See the DoClose()
	// documentation in the CEF header for a detailed description of this
	// process.
	if (browser_list_.size() == 1) {
		// Set a flag to indicate that the window close should be allowed.
		is_closing_ = true;
	}

	// Allow the close. For windowed browsers this will result in the OS close
	// event being sent.
	return false;
}

void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	// Remove from the list of existing browsers.
	BrowserList::iterator bit = browser_list_.begin();
	for (; bit != browser_list_.end(); ++bit) {
		if ((*bit)->IsSame(browser)) {
			browser_list_.erase(bit);
			break;
		}
	}

	if (browser_list_.empty()) {
		// All browser windows have closed. Quit the application message loop.
		CefQuitMessageLoop();
	}
}

void SimpleHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	ErrorCode errorCode,
	const CefString& errorText,
	const CefString& failedUrl) {
	CEF_REQUIRE_UI_THREAD();

	// Don't display an error for downloaded files.
	if (errorCode == ERR_ABORTED)
		return;
}

void SimpleHandler::CloseAllBrowsers(bool force_close) {
	if (!CefCurrentlyOn(TID_UI)) {
		// Execute on the UI thread.
		CefPostTask(TID_UI, base::BindOnce(&SimpleHandler::CloseAllBrowsers, this,
			force_close));
		return;
	}

	if (browser_list_.empty())
		return;

	BrowserList::const_iterator it = browser_list_.begin();
	for (; it != browser_list_.end(); ++it)
		(*it)->GetHost()->CloseBrowser(force_close);

	CefQuitMessageLoop();
}

auto SimpleHandler::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) -> ReturnValue
{
	CefRequest::HeaderMap hm;
	request->GetHeaderMap(hm);

	for (auto it = hm.begin(); it != hm.end(); )
	{
		if (it->first.ToString().find("sec-") == 0 || it->first.ToString().find("Sec-") == 0)
		{
			it = hm.erase(it);
		}
		else
		{
			it++;
		}
	}

	request->SetHeaderMap(hm);

	return RV_CONTINUE;
}

extern std::string g_rosData;
extern std::string g_rosData2;
extern bool g_oldAge;
extern std::string g_rosEmail;

extern std::string g_tpaId;
extern std::string g_tpaToken;

extern std::string GetAuthSessionTicket(uint32_t appID);

bool SimpleHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
	if (message->GetName() == "invokeNative")
	{
		auto args = message->GetArgumentList();
		auto nativeType = args->GetString(0);
		auto messageData = args->GetString(1);

		if (nativeType == "ui")
		{
			if (messageData == "Close")
			{
				CloseAllBrowsers(false);
			}
			else if (messageData == "Minimize")
			{

			}
			else if (messageData == "Maximize")
			{

			}
		}
		else if (nativeType == "refreshSteamTicket")
		{
			frame->ExecuteJavaScript(fmt::sprintf("RGSC_JS_REFRESH_STEAM_TICKET_RESULT(JSON.stringify({ Ticket: '%s' }));",
				GetAuthSessionTicket(0)), "https://rgl.rockstargames.com/scui.js", 0);
		}
		else if (nativeType == "signinFailed")
		{
			trace(__FUNCTION__ ": Auto-SignIn failed, showing login window.\n");

			GetWindow()->Show();
		}
		else if (nativeType == "signin")
		{
			trace(__FUNCTION__ ": Processing NUI sign-in.\n");

			auto json = nlohmann::json::parse(messageData.ToString());
			auto response = json["XMLResponse"];

			auto age = json["Age"].get<int>();
			g_rosEmail = json["Email"].get<std::string>();
			g_tpaToken = json.value<std::string>("TpaToken", "");

			// 1900 age
			if (age >= 120)
			{
				g_oldAge = true;
			}

			std::string responseDec;
			net::UrlDecode(response, responseDec);

			std::istringstream stream(responseDec);

			boost::property_tree::ptree tree;
			boost::property_tree::read_xml(stream, tree);

			auto obj = nlohmann::json::object({
				{ "Ticket", json["ticket"] },
				{ "SessionKey", json["sessionKey"] },
				{ "RockstarId", tree.get<std::string>("Response.RockstarAccount.RockstarId") },
				{ "SessionTicket", tree.get<std::string>("Response.SessionTicket") },
				{ "OrigNickname", json["Nickname"] },
			});

			auto obj2 = nlohmann::json::object({
			{ "Ticket", json["ticket"] },
			{ "SessionKey", json["sessionKey"] },
			{ "RockstarId", std::stoull(tree.get<std::string>("Response.RockstarAccount.RockstarId")) },
			{ "SessionTicket", tree.get<std::string>("Response.SessionTicket") },
			{ "Nickname", json["Nickname"] },
			{ "Email", g_rosEmail },
			});

			g_tpaId = tree.get<std::string>("Response.RockstarAccount.RockstarId");
			g_rosData = obj.dump();
			g_rosData2 = obj2.dump();

			trace(__FUNCTION__ ": Processed NUI sign-in - closing all browsers.\n");

			CloseAllBrowsers(false);
		}
	}

	return true;
}

std::string SimpleHandler::GetRgscInitCode()
{
	std::string flowString = "base";
	
	if (flow_ == ScuiAuthFlow::SteamAuth)
	{
		flowString = "steam";
	}
	else if (flow_ == ScuiAuthFlow::EpicAuth)
	{
		flowString = "epic";
	}	

	return fmt::sprintf(R"(
const flowType = '%s';
const flowData = %s;

function RGSC_GET_PROFILE_LIST()
{
	return JSON.stringify({
		Profiles: [],//profileList.profiles,
		NumProfiles: 0//profileList.profiles.length
	});
}

function RGSC_UI_STATE_RESPONSE(arg)
{
	var data = JSON.parse(arg);

	if (!data.Visible)
	{
		$("#scuiPanelInstruction").hide();
		$('#headerWrapper').hide();
	}
}

function RGSC_GET_TITLE_ID()
{
	let titleId = {
		RosTitleName: 'gta5',
		RosEnvironment: 'prod',
		RosTitleVersion: 11,
		RosPlatform: 'pcros',
		Platform: 'viveport',
		IsLauncher: true,
		Language: 'en-US'
	};

	if (flowType == 'steam')
	{
		titleId = {...titleId, RosTitleName: 'launcher', Platform: 'Steam', ...flowData};
	}
	else if (flowType == 'epic')
	{
		titleId = {...titleId, RosTitleName: 'launcher', Platform: 'Epic', epicAccessToken: flowData.epicAccessToken};
	}

	return JSON.stringify(titleId);
}

function RGSC_GET_VERSION_INFO()
{
	return JSON.stringify({
		Version: '9.9.9.9',
		TitleVersion: ''
	});
}

function RGSC_GET_COMMAND_LINE_ARGUMENTS()
{
	return JSON.stringify({
		Arguments: []
	});
}

function RGSC_SIGN_IN(s)
{
	var data = JSON.parse(s);

	if (data.XMLResponse)
	{
		window.invokeNative('signin', s);
	}
	else if (data.Error)
	{
		window.invokeNative('signinFailed', '');
	}
	else
	{
		data = {
    "SignedIn": false,
    "SignedOnline": false,
    "ScAuthToken": "",
    "ScAuthTokenError": false,
    "ProfileSaved": false,
    "AchievementsSynced": false,
    "FriendsSynced": false,
    "Local": false,
    "NumFriendsOnline": 0,
    "NumFriendsPlayingSameTitle": 0,
    "NumBlocked": 0,
    "NumFriends": 0,
    "NumInvitesReceieved": 0,
    "NumInvitesSent": 0,
    "CallbackData": 0
};
	}

	RGSC_JS_FINISH_SIGN_IN(JSON.stringify(data));
}

function RGSC_RAISE_UI_EVENT(a)
{
	const d = JSON.parse(a);

	if (d.EventId === 1) {
		window.invokeNative('ui', d.Data.Action);
	}
}

function RGSC_MODIFY_PROFILE(a)
{
	RGSC_JS_FINISH_MODIFY_PROFILE(a);
}

function RGSC_SIGN_OUT()
{
}

function RGSC_DELETE_PROFILE(a)
{
}

function RGSC_REQUEST_PLAYER_LIST_COUNTS(st)
{
}

function RGSC_REQUEST_PLAYER_LIST(st)
{
}

// yes, this is mistyped as RSGC
function RSGC_LAUNCH_EXTERNAL_WEB_BROWSER(a)
{
	var d = JSON.parse(a);
}

function RGSC_GET_ROS_CREDENTIALS()
{
	return JSON.stringify(rosCredentials);
}

function RGSC_REQUEST_UI_STATE(a)
{
	var data = JSON.parse(a);

	RGSC_UI_STATE_RESPONSE(a);

	data.Online = true;

	RGSC_JS_UI_STATE_RESPONSE(JSON.stringify(data));
}

function RGSC_READY_TO_ACCEPT_COMMANDS()
{
	RGSC_JS_REQUEST_UI_STATE(JSON.stringify({ Visible: true, Online: true, State: flowType === 'base' ? "SIGNIN" : "MAIN" }));

	if (flowType === 'steam' || flowType === 'epic')
	{
		const ORIG_fetch = window.fetch;

		window.fetch = (...args) => {
			const requestUrl = args[0];

			if (typeof requestUrl === 'string') {
				if (requestUrl.endsWith('autologinsteam') || requestUrl.endsWith('autologinepic')) {
					const newBody = {
						...JSON.parse(args[1].body),
						tpaTokens: flowData.TpaTokens,
					};

					if (flowType === 'steam') {
						newBody.AppId = flowData.SteamAppId;
						newBody.authTicket = flowData.SteamAuthTicket;
					}
					if (flowType === 'epic') {
						newBody.authTicket = flowData.epicAccessToken;
						newBody.platformUserId = flowData.EpicAccountId;
					}

					args[1].body = JSON.stringify(newBody);
				}	
			}

			return ORIG_fetch(...args);
		};

		RGSC_JS_SIGN_IN(JSON.stringify({
			"RockstarId": "0",
			"Nickname": "",
			"LastSignInTime": "0",
			"AutoSignIn": false,
			"Local": false,
			"SaveEmail": false,
			"SavePassword": false,
			"Ticket": "",
			"AvatarUrl": "",
			"RememberedMachineToken": ""
		}));
	}

	return true;
}

function RGSC_REFRESH_STEAM_TICKET()
{
	window.invokeNative('refreshSteamTicket', '');
}

function RGSC_GET_PROFILE_LIST_FILTERED()
{
	return RGSC_GET_PROFILE_LIST();
}

function RGSC_DEBUG_EVENT(e)
{
	console.log(e);
}

RGSC_JS_READY_TO_ACCEPT_COMMANDS();

if (!localStorage.getItem('loadedOnce')) {
	localStorage.setItem('loadedOnce', true);
	setTimeout(() => {
		location.reload();
	}, 500);
}

globals.isLog = true;

const head	= document.getElementsByTagName('head')[0];
const link	= document.createElement('link');
link.rel	= 'stylesheet';
link.type	= 'text/css';
link.href	= 'https://rgl.rockstargames.com/temp.css';
head.appendChild(link);
)",
	flowString,
	extra_json_
	);
};

void SimpleHandler::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type)
{
	frame->ExecuteJavaScript("window.rgscAddSubscription = () => {};", "https://rgl.rockstargames.com/temp.js", 0);
}

void SimpleHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
	if (frame->GetURL().ToString().find("/launcher") != std::string::npos)
	{
		frame->ExecuteJavaScript(GetRgscInitCode(), "https://rgl.rockstargames.com/temp.js", 0);
	}
}

static bool RunScuiAuthFlow(ScuiAuthFlow flow, const std::string& extraJson = "{}")
{
	SetEnvironmentVariable(L"CitizenFX_ToolMode", nullptr);

	// Provide CEF with command-line arguments.
	CefMainArgs main_args(GetModuleHandle(nullptr));

	// Specify CEF global settings here.
	CefSettings settings;
	settings.no_sandbox = true;

	settings.remote_debugging_port = 13173;
	settings.log_severity = LOGSEVERITY_DEFAULT;

	CefString(&settings.log_file).FromWString(MakeRelativeCitPath(L"cef_console.txt"));

	CefString(&settings.browser_subprocess_path).FromWString(MakeCfxSubProcess(L"AuthBrowser", L"chrome"));

	CefString(&settings.locale).FromASCII("en-US");

	std::wstring resPath = MakeRelativeCitPath(L"bin/cef/");

	std::wstring cachePath = MakeRelativeCitPath(L"data\\cache\\authbrowser\\");
	CreateDirectory(cachePath.c_str(), nullptr);

	CefString(&settings.resources_dir_path).FromWString(resPath);
	CefString(&settings.locales_dir_path).FromWString(resPath);
	CefString(&settings.cache_path).FromWString(cachePath);
	CefString(&settings.user_agent).FromWString(L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/73.0.3683.75 RockstarGames/2.0.7.5/1.0.33.319/launcher/PC Safari/537.36");

	// SimpleApp implements application-level callbacks for the browser process.
	// It will create the first browser instance in OnContextInitialized() after
	// CEF has initialized.
	CefRefPtr<SimpleApp> app(new SimpleApp(flow, extraJson));

	trace(__FUNCTION__ ": Initializing CEF.\n");

	// Initialize CEF.
	CefInitialize(main_args, settings, app.get(), nullptr);

	trace(__FUNCTION__ ": Initialized CEF.\n");

	// Run the CEF message loop. This will block until CefQuitMessageLoop() is
	// called.
	CefRunMessageLoop();

	trace(__FUNCTION__ ": Shutting down CEF.\n");

	// Shut down CEF.
	//CefShutdown();

	trace(__FUNCTION__ ": Shut down CEF.\n");

	return !g_rosData.empty();
}

void RunLegitimacyNui()
{
	RunScuiAuthFlow(ScuiAuthFlow::LegitimacyNui);
}

bool RunSteamAuthUi(const nlohmann::json& json)
{
	return RunScuiAuthFlow(ScuiAuthFlow::SteamAuth, json.dump());
}

bool RunEpicAuthUi(const nlohmann::json& json)
{
	return RunScuiAuthFlow(ScuiAuthFlow::EpicAuth, json.dump());
}
