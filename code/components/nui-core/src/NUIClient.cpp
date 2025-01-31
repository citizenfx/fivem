/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIClient.h"
#include "NUIRenderHandler.h"
#include "NUISchemeHandlerFactory.h"
#include "CefOverlay.h"
#include "memdbgon.h"

#include <IteratorView.h>

#include <CoreConsole.h>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <rapidjson/document.h>
#include "include/cef_parser.h"

#include <sstream>
#include <regex>

extern nui::GameInterface* g_nuiGi;
bool shouldHaveRootWindow;

static nui::IAudioSink* g_audioSink;

namespace nui
{
	void SetAudioSink(IAudioSink* sinkRef)
	{
		g_audioSink = sinkRef;
	}

	void RecreateFrames();
}

NUIClient::NUIClient(NUIWindow* window)
	: m_window(window), m_windowValid(false), m_loadedMainFrame(false)
{
	if (m_window)
	{
		m_windowValid = true;
	}

	m_renderHandler = new NUIRenderHandler(this);
}

void NUIClient::Initialize()
{
	CefRefPtr<NUIClient> thisRef(this);

	nui::RequestNUIBlocklist([thisRef](bool success, const char* data, size_t length)
	{
		if (success)
		{
			rapidjson::Document doc;
			doc.Parse(data, length);

			if (!doc.HasParseError())
			{
				if (doc.IsArray())
				{
					for (auto it = doc.Begin(); it != doc.End(); ++it)
					{
						if (it->IsString())
						{
							std::unique_lock _(thisRef->m_requestBlocklistLock);
							thisRef->m_requestBlocklist.emplace_back(it->GetString(), std::regex_constants::ECMAScript | std::regex_constants::icase);
						}
					}
				}
			}

			Instance<NUISchemeHandlerFactory>::Get()->SetRequestBlocklist(thisRef->m_requestBlocklist);
		}
	});
}

void NUIClient::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transitionType)
{
	if (frame->IsMain() && m_window && m_window->GetName() == "nui_mpMenu")
	{
		m_loadedMainFrame = false;
	}

	if (g_audioSink)
	{
		browser->GetHost()->SetAudioMuted(true);
	}

	auto window = GetWindow();

	if (window)
	{
		window->OnClientContextCreated(browser, frame, nullptr);
	}
}

extern void TriggerLoadEnd(const std::string& name);

void NUIClient::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
	auto url = frame->GetURL();
	TriggerLoadEnd((url == "nui://game/ui/root.html") ? "__root" : frame->GetName());

	if (auto parent = frame->GetParent(); parent && parent->IsMain())
	{
		frame->ExecuteJavaScript(R"(
const doHook = () => {
	const oldConsoleLog = console.log;

	Object.defineProperty(console, 'log', {
		get: () => {
			return (...args) => {
				for (const arg of args) {
					if (arg instanceof HTMLElement) {
						globalThis.dummy = arg.id + '';
					}
				}
				return oldConsoleLog(...args);
			};
		}
	});
};

for (const old of ['profile', 'profileEnd']) {
	const oldProfile = console[old];
	Object.defineProperty(console, old, {
		get: () => {
			return (...args) => {
				for (const arg of args) {
					try {
						arg.toString();
					} catch {
					}
				}

				return oldProfile(...args);
			};
		}
	});
}

const oldDefineGetter = Object.prototype.__defineGetter__;
Object.prototype.__defineGetter__ = function(prop, func) {
	if (prop === 'id') {
		doHook();
	}
	return oldDefineGetter.call(this, prop, func);
};
)",
		"nui://patches", 0);
	}

	if (url == "nui://game/ui/root.html")
	{
		shouldHaveRootWindow = true;
		nui::RecreateFrames();
	}

	// enter push function
	if (frame->IsMain() && m_window && m_window->GetName() == "nui_mpMenu")
	{
		frame->ExecuteJavaScript(R"(
(() => {
	let nuiMessageQueue = [];

	window.nuiInternalCallMessages = () => {
		const mq = nuiMessageQueue;
		nuiMessageQueue = [];

		setTimeout(() => {
			for (const msg of mq) {
				window.postMessage(msg, '*');
			}
		}, 50);
	};

	window.registerPushFunction(function(type, ...args) {
		switch (type) {
			case 'frameCall': {
				const [ dataString ] = args;

				try {
					const data = JSON.parse(dataString);

					window.postMessage(data, '*');

					if (!window.nuiInternalHandledMessages) {
						nuiMessageQueue.push(data);
					}
				} catch (e) {
					console.log('frameCall data that caused the following error', dataString);
					console.error(e);
					return;
				}

				break;
			}
		}
	});
})();
)",
		"nui://handler", 0);

		m_loadedMainFrame = true;

		if (m_window)
		{
			m_window->ProcessLoadQueue();
		}
	}
}

void NUIClient::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	m_browser = browser;

	OnClientCreated(this);
}

bool NUIClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
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

void NUIClient::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model)
{
	model->Clear();
}

extern HCURSOR g_defaultCursor;

bool NUIClient::OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, cef_cursor_type_t type, const CefCursorInfo& custom_cursor_info)
{
	if (!cursor || type == CT_POINTER)
	{
		g_nuiGi->SetHostCursor(g_defaultCursor);
	}
	else
	{
		g_nuiGi->SetHostCursor(cursor);
	}

	return true;
}

bool NUIClient::OnConsoleMessage(CefRefPtr<CefBrowser> browser, cef_log_severity_t level, const CefString& message, const CefString& source, int line)
{
	std::wstring sourceStr = source.ToWString();
	std::wstring messageStr = message.ToWString();

	// skip websocket error messages and mpMenu messages
	// some of these can't be blocked from script and users get very confused by them appearing in console
	if (messageStr.find(L"WebSocket connection to") != std::string::npos || sourceStr.find(L"nui-game-internal") != std::string::npos || sourceStr.find(L"chrome-devtools") != std::string::npos ||
		// some weird loading screen embeds use this, but newer Chrome writes to log in that case
		// users get confused, again, as well: 'I'm stuck in loading due to this error, what is this?'
		messageStr.find(L"target-densitydpi") != std::string::npos)
	{
		return false;
	}

	std::string channel = "nui:console";

	if (sourceStr.find(L"nui://") == 0)
	{
		static std::wregex re{ L"nui://(.*?)/(.*)" };
		std::wsmatch match;

		if (std::regex_search(sourceStr, match, re))
		{
			channel = fmt::sprintf("script:%s:nui", ToNarrow(match[1].str()));
			sourceStr = fmt::sprintf(L"@%s/%s", match[1].str(), match[2].str());
		}
	}
	else if (sourceStr.find(L"https://cfx-nui-") == 0)
	{
		static std::wregex re{ L"https://cfx-nui-(.*?)/(.*)" };
		std::wsmatch match;

		if (std::regex_search(sourceStr, match, re))
		{
			channel = fmt::sprintf("script:%s:nui", ToNarrow(match[1].str()));
			sourceStr = fmt::sprintf(L"@%s/%s", match[1].str(), match[2].str());
		}
	}

	console::Printf(channel, "%s\n", ToNarrow(fmt::sprintf(L"%s (%s:%d)", messageStr, sourceStr, line)));

	return false;
}

auto NUIClient::OnBeforePopup(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	const CefString& target_url,
	const CefString& target_frame_name,
	CefLifeSpanHandler::WindowOpenDisposition target_disposition,
	bool user_gesture,
	const CefPopupFeatures& popupFeatures,
	CefWindowInfo& windowInfo,
	CefRefPtr<CefClient>& client,
	CefBrowserSettings& settings,
	CefRefPtr<CefDictionaryValue>& extra_info,
	bool* no_javascript_access) -> bool
{
	if (target_disposition == WOD_NEW_FOREGROUND_TAB || target_disposition == WOD_NEW_BACKGROUND_TAB || target_disposition == WOD_NEW_POPUP || target_disposition == WOD_NEW_WINDOW )
	{
		return true;
	}
	return false;
}

auto NUIClient::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) -> ReturnValue
{
	auto url = request->GetURL().ToString();

	if (boost::algorithm::to_lower_copy(url).find("file://") != std::string::npos)
	{
		return RV_CANCEL;
	}

	// 'code.jquery.com' has reliability concerns for some end users, redirect these to googleapis instead
	{
		CefURLParts parts;
		if (CefParseURL(request->GetURL(), parts))
		{
			auto hostString = CefString(&parts.host).ToString();

			if (hostString == "code.jquery.com")
			{
				std::smatch match;
				static std::regex re{
					R"(code.jquery.com/jquery-([0-9]+\.[0-9]+\.[0-9]+)(\..*?)?\.js)"
				};
				static std::regex reUI{
					R"(code.jquery.com/ui/(.*?)/(.*?)$)"
				};

				auto url = request->GetURL().ToString();

				if (std::regex_search(url, match, re))
				{
					auto version = match[1].str();

					// "3.3.0, 2.1.2, 1.2.5 and 1.2.4 are not hosted due to their short and unstable lives in the wild."
					if (version != "3.3.0" && version != "2.1.2" && version != "1.2.5" && version != "1.2.4")
					{
						request->SetURL(fmt::sprintf("https://ajax.googleapis.com/ajax/libs/jquery/%s/jquery%s.js",
							version,
							match.size() >= 3 ? match[2].str() : ""));
					}
				}
				else if (std::regex_search(url, match, reUI))
				{
					request->SetURL(fmt::sprintf("https://ajax.googleapis.com/ajax/libs/jqueryui/%s/%s",
						match[1].str(),
						match[2].str()));
				}
			}
		}
	}


	// DiscordApp breaks as of late and affects end users, tuning the headers seems to fix it
	{
		CefURLParts parts;
		if (CefParseURL(request->GetURL(), parts))
		{
			auto hostString = CefString(&parts.host).ToString();

			if (boost::algorithm::ends_with(hostString, "discordapp.com") ||
				boost::algorithm::ends_with(hostString, "discordapp.net"))
			{
				CefRequest::HeaderMap headers;
				request->GetHeaderMap(headers);

				headers.erase("User-Agent");
				headers.emplace("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/112.0.0.0 Safari/537.36");

				headers.erase("sec-ch-ua");
				headers.emplace("sec-ch-ua", R"("Chromium";v="112", "Google Chrome";v="112", "Not:A-Brand";v="99")");

				headers.erase("sec-ch-ua-mobile");
				headers.emplace("sec-ch-ua-mobile", R"(?0)");

				headers.erase("sec-ch-ua-platform");
				headers.emplace("sec-ch-ua-platform", R"("Windows")");

				request->SetHeaderMap(headers);
				request->SetReferrer("https://discord.com/channels/@me", CefRequest::ReferrerPolicy::REFERRER_POLICY_DEFAULT);
			}
		}
	}

#if !defined(_DEBUG)
	if (frame->IsMain())
	{
		if (frame->GetURL().ToString().find("nui://game/ui/") == 0 && url.find("nui://game/ui/") != 0)
		{
			trace("Blocked a request for root breaking URI %s\n", url);
			return RV_CANCEL;
		}
	}
#endif

	{
		std::shared_lock _(m_requestBlocklistLock);

		for (auto& reg : m_requestBlocklist)
		{
			try
			{
				if (std::regex_search(url, reg))
				{
					trace("Blocked a request for blocklisted URI %s\n", url);
					return RV_CANCEL;
				}
			}
			catch (std::exception& e)
			{
			}
		}
	}

	// to whoever may read this in charge of blocking `https://nui-game-internal*` as referrer on the `mastodon.social` instance:
	// -> instead of outright blocking, it would've been possible to contact us as well (hydrogen@fivem.net et al.) before resorting to
	//    outright oddity affecting actual users. even if server load is the issue.
	// -> maybe don't pretend to support some open protocol if you block anyone using the API on your federated instance?
	if (url.find("mastodon.social/") != std::string::npos)
	{
		request->SetReferrer("", CefRequest::ReferrerPolicy::REFERRER_POLICY_NO_REFERRER);
		request->SetHeaderByName("origin", "http://localhost", true);
		request->SetHeaderByName("user-agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.48 Safari/537.36", true);
	}

	return RV_CONTINUE;
}

void NUIClient::AddProcessMessageHandler(std::string key, TProcessMessageHandler handler)
{
	m_processMessageHandlers[key] = handler;
}

void NUIClient::OnAudioCategoryConfigure(const std::string& frame, const std::string& category)
{
	m_audioFrameCategories[frame] = category;

	if (!g_audioSink)
	{
		return;
	}

	// recreate any streams
	for (auto& streamEntry : fx::GetIteratorView(m_audioStreamsByFrame.equal_range(frame)))
	{
		auto streamIt = m_audioStreams.find(streamEntry.second);

		if (streamIt != m_audioStreams.end())
		{
			auto params = std::get<1>(streamIt->second);
			params.categoryName = category;

			m_audioStreams.erase(streamIt);

			auto stream = g_audioSink->CreateAudioStream(params);
			m_audioStreams.insert({ streamEntry.second, { stream, params } });
		}
	}
}

#if 0
void NUIClient::OnAudioStreamStarted(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int audio_stream_id, int channels, ChannelLayout channel_layout, int sample_rate, int frames_per_buffer)
{
	if (g_audioSink)
	{
		nui::AudioStreamParams params;
		params.channelLayout = (nui::CefChannelLayout)channel_layout;
		params.channels = channels;
		params.sampleRate = sample_rate;
		params.framesPerBuffer = frames_per_buffer;

		// try finding the second-topmost frame
		auto refFrame = frame;

		for (auto f = refFrame; f->GetParent(); f = f->GetParent())
		{
			if (!f->GetParent()->GetParent())
			{
				refFrame = f;
				break;
			}
		}

		// get frame name
		auto frameName = ToNarrow(refFrame->GetName().ToWString());
		params.frameName = std::move(frameName);

		auto categoryIt = m_audioFrameCategories.find(params.frameName);

		if (categoryIt != m_audioFrameCategories.end())
		{
			params.categoryName = categoryIt->second;
		}

		// and create audio stream
		auto stream = g_audioSink->CreateAudioStream(params);

		m_audioStreams.insert({ { browser->GetIdentifier(), audio_stream_id }, { stream, params } });
		m_audioStreamsByFrame.insert({ params.frameName, { browser->GetIdentifier(), audio_stream_id } });
	}
}

void NUIClient::OnAudioStreamPacket(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int audio_stream_id, const float** data, int frames, int64 pts)
{
	auto it = m_audioStreams.find({ browser->GetIdentifier(), audio_stream_id });

	if (it != m_audioStreams.end())
	{
		std::get<0>(it->second)->ProcessPacket(data, frames, pts);
	}
}

void NUIClient::OnAudioStreamStopped(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int audio_stream_id)
{
	auto refFrame = frame;

	for (auto f = refFrame; f->GetParent(); f = f->GetParent())
	{
		if (!f->GetParent()->GetParent())
		{
			refFrame = f;
			break;
		}
	}

	// get frame name
	auto frameName = ToNarrow(refFrame->GetName().ToWString());
	m_audioStreamsByFrame.erase(frameName);

	m_audioStreams.erase({ browser->GetIdentifier(), audio_stream_id });
}
#endif

extern bool g_shouldCreateRootWindow;

void NUIClient::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status)
{
	if (browser->GetMainFrame()->GetURL() == "nui://game/ui/root.html" || (m_windowValid && m_window && m_window->GetName() == "nui_mpMenu"))
	{
		browser->GetHost()->CloseBrowser(true);

		g_shouldCreateRootWindow = true;
	}
}

bool NUIClient::OnOpenURLFromTab(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url, CefRequestHandler::WindowOpenDisposition target_disposition, bool user_gesture)
{
	// Discards middle mouse clicks / ctrl-clicks of links
	// Default behavior is to open them in a new tab and switch to it, with no back button the player had no way to go back to CfxUI
	if (target_disposition == CefRequestHandler::WindowOpenDisposition::WOD_NEW_BACKGROUND_TAB && user_gesture)
	{
		return true;
	}
	return false;
}

void NUIClient::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
	m_browser = nullptr;
}

CefRefPtr<CefLifeSpanHandler> NUIClient::GetLifeSpanHandler()
{
	return this;
}

CefRefPtr<CefDisplayHandler> NUIClient::GetDisplayHandler()
{
	return this;
}

CefRefPtr<CefContextMenuHandler> NUIClient::GetContextMenuHandler()
{
	return this;
}

CefRefPtr<CefLoadHandler> NUIClient::GetLoadHandler()
{
	return this;
}

CefRefPtr<CefRequestHandler> NUIClient::GetRequestHandler()
{
	return this;
}

CefRefPtr<CefRenderHandler> NUIClient::GetRenderHandler()
{
	return m_renderHandler;
}

extern nui::GameInterface* g_nuiGi;

#ifdef NUI_WITH_MEDIA_ACCESS
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "include/base/cef_callback_helpers.h"

static void AcceptCallback(CefRefPtr<CefMediaAccessCallback> callback, bool noCancel, int mask)
{
	if (noCancel)
	{
		callback->Continue(mask);
	}
	else
	{
		callback->Cancel();
	}
}

bool NUIClient::OnRequestMediaAccessPermission(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& requesting_url, uint32_t requested_permissions, CefRefPtr<CefMediaAccessCallback> callback)
{
	return g_nuiGi->RequestMediaAccess(frame->GetName(), requesting_url, requested_permissions, [callback](bool noCancel, int mask)
	{
		CefPostTask(TID_UI, base::BindOnce(&AcceptCallback, callback, noCancel, mask));
	});
}

CefRefPtr<CefPermissionHandler> NUIClient::GetPermissionHandler()
{
	return this;
}
#endif

fwEvent<NUIClient*> NUIClient::OnClientCreated;
fwEvent<std::function<void(bool, const char*, size_t)>> nui::RequestNUIBlocklist;

