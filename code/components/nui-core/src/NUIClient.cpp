/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIClient.h"
#include "NUIRenderHandler.h"
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
	: m_window(window)
{
	m_windowValid = true;

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
		}
	});
}

void NUIClient::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transitionType)
{
	if (g_audioSink)
	{
		browser->GetHost()->SetAudioMuted(true);
	}

	GetWindow()->OnClientContextCreated(browser, frame, nullptr);
}

void NUIClient::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
	auto url = frame->GetURL();

	if (url == "nui://game/ui/root.html")
	{
		static ConVar<std::string> uiUrlVar("ui_url", ConVar_None, "https://nui-game-internal/ui/app/index.html");

		nui::RecreateFrames();

		if (nui::HasMainUI())
		{
			nui::CreateFrame("mpMenu", uiUrlVar.GetValue());
		}
	}

	// replace any segoe ui symbol font
	frame->ExecuteJavaScript("[].concat.apply([], Array.from(document.styleSheets).map(a => Array.from(a.rules).filter(b => b.style && b.style.fontFamily))).forEach(a => a.style.fontFamily = a.style.fontFamily.replace(/Segoe UI Symbol/g, 'Segoe UI Emoji'));", "nui://patches", 0);
}

void NUIClient::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	m_browser = browser;

	OnClientCreated(this);
}

bool NUIClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
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

	// skip websocket error messages
	// these can't be blocked from script and users get very confused by them appearing in console
	if (messageStr.find(L"WebSocket connection to") != std::string::npos)
	{
		return false;
	}

	std::wstringstream msg;
	msg << sourceStr << ":" << line << ", " << messageStr << std::endl;

	OutputDebugString(msg.str().c_str());
	trace("%s", ToNarrow(msg.str()));

	return false;
}

auto NUIClient::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefRequestCallback> callback) -> ReturnValue
{
	for (auto& reg : m_requestBlacklist)
	{
		std::string url = request->GetURL().ToString();

		if (std::regex_search(url, reg))
		{
			trace("Blocked a request for blacklisted URI %s\n", url);
			return RV_CANCEL;
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

extern bool g_shouldCreateRootWindow;

void NUIClient::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status)
{
	if (browser->GetMainFrame()->GetURL() == "nui://game/ui/root.html")
	{
		browser->GetHost()->CloseBrowser(true);

		g_shouldCreateRootWindow = true;
	}
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

CefRefPtr<CefAudioHandler> NUIClient::GetAudioHandler()
{
	// #TODONUI: render process termination does not work this way, needs an owning reference to the browser
	// or otherwise tracking why the browser doesn't become null when playing audio and killing the render process
	return nullptr;
	//return this;
}

CefRefPtr<CefRenderHandler> NUIClient::GetRenderHandler()
{
	return m_renderHandler;
}

fwEvent<NUIClient*> NUIClient::OnClientCreated;
