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
#include "HttpClient.h"

#include <IteratorView.h>

#include <CoreConsole.h>

#include <rapidjson/document.h>

#include <sstream>

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

	CefRefPtr<NUIClient> thisRef(this);

	auto httpClient = Instance<HttpClient>::Get();
	httpClient->DoGetRequest("https://runtime.fivem.net/nui-blacklist.json", [thisRef](bool success, const char* data, size_t length)
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
							thisRef->m_requestBlacklist.emplace_back(it->GetString(), std::regex_constants::ECMAScript | std::regex_constants::icase);
						}
					}
				}
			}

			Instance<NUISchemeHandlerFactory>::Get()->SetRequestBlacklist(thisRef->m_requestBlacklist);
		}
	});
}

void NUIClient::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transitionType)
{
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

void NUIClient::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
	auto url = frame->GetURL();

#ifndef USE_NUI_ROOTLESS
	if (url == "nui://game/ui/root.html")
	{
		static ConVar<std::string> uiUrlVar("ui_url", ConVar_None, "https://nui-game-internal/ui/app/index.html");

		nui::RecreateFrames();

		if (nui::HasMainUI())
		{
			nui::CreateFrame("mpMenu", uiUrlVar.GetValue());
		}
	}
#else
	// enter push function
	if (frame->IsMain())
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
				const data = JSON.parse(dataString);

				window.postMessage(data, '*');

				if (!window.nuiInternalHandledMessages) {
					nuiMessageQueue.push(data);
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
#endif

	// replace any segoe ui symbol font
	frame->ExecuteJavaScript("[].concat.apply([], Array.from(document.styleSheets).map(a => Array.from(a.rules).filter(b => b.style && b.style.fontFamily))).forEach(a => a.style.fontFamily = a.style.fontFamily.replace(/Segoe UI Symbol/g, 'Segoe UI Emoji'));", "nui://patches", 0);
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

bool NUIClient::OnConsoleMessage(CefRefPtr<CefBrowser> browser, cef_log_severity_t level, const CefString& message, const CefString& source, int line)
{
	std::wstring sourceStr = source.ToWString();
	std::wstring messageStr = message.ToWString();

	// skip websocket error messages and mpMenu messages
	// some of these can't be blocked from script and users get very confused by them appearing in console
	if (messageStr.find(L"WebSocket connection to") != std::string::npos || sourceStr.find(L"nui-game-internal") != std::string::npos || sourceStr.find(L"chrome-devtools") != std::string::npos)
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

auto NUIClient::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefRequestCallback> callback) -> ReturnValue
{
	for (auto& reg : m_requestBlacklist)
	{
		std::string url = request->GetURL().ToString();

		try
		{
			if (std::regex_search(url, reg))
			{
				trace("Blocked a request for blacklisted URI %s\n", url);
				return RV_CANCEL;
			}
		}
		catch (std::exception& e)
		{
		}
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

#ifdef USE_NUI_ROOTLESS
extern std::set<std::string> g_recreateBrowsers;
extern std::shared_mutex g_recreateBrowsersMutex;
#endif

void NUIClient::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status)
{
#ifndef USE_NUI_ROOTLESS
	if (browser->GetMainFrame()->GetURL() == "nui://game/ui/root.html")
	{
		browser->GetHost()->CloseBrowser(true);

		g_shouldCreateRootWindow = true;
	}
#else
	std::unique_lock<std::shared_mutex> _(g_recreateBrowsersMutex);

	g_recreateBrowsers.insert(m_window->GetName());
#endif
}

void NUIClient::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
	m_browser = {};
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

fwEvent<NUIClient*> NUIClient::OnClientCreated;
