/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifdef COMPILING_NUI_CORE
#define OVERLAY_DECL __declspec(dllexport)
#else
#define OVERLAY_DECL __declspec(dllimport)
#endif

#include <memory>

#include "grcTexture.h"

#include <include/cef_app.h>
#include <include/cef_browser.h>
#include <include/cef_client.h>

#include <queue>

class NUIExtensionHandler : public CefV8Handler
{
public:
	NUIExtensionHandler();

	virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception) override;

	void InvokeNUICallback(const char* type, const CefString& name, const CefV8ValueList& arguments);

	void EnterV8Context(const char* type);
	void ExitV8Context(const char* type);

private:
	std::map<std::string, CefRefPtr<CefV8Value>> _callbackFunctions;
	std::map<std::string, CefRefPtr<CefV8Context>> _callbackContexts;

	IMPLEMENT_REFCOUNTING(NUIExtensionHandler);
};

namespace nui
{
	enum class CefChannelLayout
	{
		CEF_CHANNEL_LAYOUT_NONE = 0,
		CEF_CHANNEL_LAYOUT_UNSUPPORTED = 1,

		// Front C
		CEF_CHANNEL_LAYOUT_MONO = 2,

		// Front L, Front R
		CEF_CHANNEL_LAYOUT_STEREO = 3,

		// Front L, Front R, Back C
		CEF_CHANNEL_LAYOUT_2_1 = 4,

		// Front L, Front R, Front C
		CEF_CHANNEL_LAYOUT_SURROUND = 5,

		// Front L, Front R, Front C, Back C
		CEF_CHANNEL_LAYOUT_4_0 = 6,

		// Front L, Front R, Side L, Side R
		CEF_CHANNEL_LAYOUT_2_2 = 7,

		// Front L, Front R, Back L, Back R
		CEF_CHANNEL_LAYOUT_QUAD = 8,

		// Front L, Front R, Front C, Side L, Side R
		CEF_CHANNEL_LAYOUT_5_0 = 9,

		// Front L, Front R, Front C, LFE, Side L, Side R
		CEF_CHANNEL_LAYOUT_5_1 = 10,

		// Front L, Front R, Front C, Back L, Back R
		CEF_CHANNEL_LAYOUT_5_0_BACK = 11,

		// Front L, Front R, Front C, LFE, Back L, Back R
		CEF_CHANNEL_LAYOUT_5_1_BACK = 12,

		// Front L, Front R, Front C, Side L, Side R, Back L, Back R
		CEF_CHANNEL_LAYOUT_7_0 = 13,

		// Front L, Front R, Front C, LFE, Side L, Side R, Back L, Back R
		CEF_CHANNEL_LAYOUT_7_1 = 14,

		// Front L, Front R, Front C, LFE, Side L, Side R, Front LofC, Front RofC
		CEF_CHANNEL_LAYOUT_7_1_WIDE = 15,

		// Stereo L, Stereo R
		CEF_CHANNEL_LAYOUT_STEREO_DOWNMIX = 16,

		// Stereo L, Stereo R, LFE
		CEF_CHANNEL_LAYOUT_2POINT1 = 17,

		// Stereo L, Stereo R, Front C, LFE
		CEF_CHANNEL_LAYOUT_3_1 = 18,

		// Stereo L, Stereo R, Front C, Rear C, LFE
		CEF_CHANNEL_LAYOUT_4_1 = 19,

		// Stereo L, Stereo R, Front C, Side L, Side R, Back C
		CEF_CHANNEL_LAYOUT_6_0 = 20,

		// Stereo L, Stereo R, Side L, Side R, Front LofC, Front RofC
		CEF_CHANNEL_LAYOUT_6_0_FRONT = 21,

		// Stereo L, Stereo R, Front C, Rear L, Rear R, Rear C
		CEF_CHANNEL_LAYOUT_HEXAGONAL = 22,

		// Stereo L, Stereo R, Front C, LFE, Side L, Side R, Rear Center
		CEF_CHANNEL_LAYOUT_6_1 = 23,

		// Stereo L, Stereo R, Front C, LFE, Back L, Back R, Rear Center
		CEF_CHANNEL_LAYOUT_6_1_BACK = 24,

		// Stereo L, Stereo R, Side L, Side R, Front LofC, Front RofC, LFE
		CEF_CHANNEL_LAYOUT_6_1_FRONT = 25,

		// Front L, Front R, Front C, Side L, Side R, Front LofC, Front RofC
		CEF_CHANNEL_LAYOUT_7_0_FRONT = 26,

		// Front L, Front R, Front C, LFE, Back L, Back R, Front LofC, Front RofC
		CEF_CHANNEL_LAYOUT_7_1_WIDE_BACK = 27,

		// Front L, Front R, Front C, Side L, Side R, Rear L, Back R, Back C.
		CEF_CHANNEL_LAYOUT_OCTAGONAL = 28,

		// Channels are not explicitly mapped to speakers.
		CEF_CHANNEL_LAYOUT_DISCRETE = 29,

		// Front L, Front R, Front C. Front C contains the keyboard mic audio. This
		// layout is only intended for input for WebRTC. The Front C channel
		// is stripped away in the WebRTC audio input pipeline and never seen outside
		// of that.
		CEF_CHANNEL_LAYOUT_STEREO_AND_KEYBOARD_MIC = 30,

		// Front L, Front R, Side L, Side R, LFE
		CEF_CHANNEL_LAYOUT_4_1_QUAD_SIDE = 31,

		// Actual channel layout is specified in the bitstream and the actual channel
		// count is unknown at Chromium media pipeline level (useful for audio
		// pass-through mode).
		CEF_CHANNEL_LAYOUT_BITSTREAM = 32,

		// Max value, must always equal the largest entry ever logged.
		CEF_CHANNEL_LAYOUT_MAX = CEF_CHANNEL_LAYOUT_BITSTREAM
	};

	struct AudioStreamParams
	{
		int channels;
		CefChannelLayout channelLayout;
		int sampleRate;
		int framesPerBuffer;

		std::string frameName;
		std::string categoryName;
	};

	class IAudioStream
	{
	public:
		virtual ~IAudioStream() = default;

		virtual void ProcessPacket(const float** data, int frames, int64 pts) = 0;
	};

	class IAudioSink
	{
	public:
		virtual std::shared_ptr<IAudioStream> CreateAudioStream(const AudioStreamParams& params) = 0;
	};

	//void EnterV8Context(const char* type);
	//void LeaveV8Context(const char* type);
	//void InvokeNUICallback(const char* type, const CefString& name, const CefV8ValueList& arguments);
	void OVERLAY_DECL ReloadNUI();

	void OVERLAY_DECL CreateFrame(fwString frameName, fwString frameURL);
	void OVERLAY_DECL DestroyFrame(fwString frameName);
	bool OVERLAY_DECL HasFrame(const std::string& frameName);
	void OVERLAY_DECL SignalPoll(fwString frameName);

	void OVERLAY_DECL GiveFocus(bool hasFocus, bool hasCursor = false);
	void OVERLAY_DECL OverrideFocus(bool hasFocus);
	bool OVERLAY_DECL HasMainUI();
	void OVERLAY_DECL SetMainUI(bool enable);

	void ProcessInput();

	void OVERLAY_DECL ExecuteRootScript(const std::string& scriptBit);

	void OVERLAY_DECL PostFrameMessage(const std::string& frameName, const std::string& jsonData);

	void OVERLAY_DECL PostRootMessage(const std::string& jsonData);

	OVERLAY_DECL CefBrowser* GetBrowser();

	bool OnPreLoadGame(void* cefSandbox);

	// window API
	OVERLAY_DECL CefBrowser* GetNUIWindowBrowser(fwString windowName);

	OVERLAY_DECL void CreateNUIWindow(fwString windowName, int width, int height, fwString windowURL);
	OVERLAY_DECL void DestroyNUIWindow(fwString windowName);
	OVERLAY_DECL void ExecuteWindowScript(const std::string& windowName, const std::string& scriptBit);
	OVERLAY_DECL void SetNUIWindowURL(fwString windowName, fwString url);

	OVERLAY_DECL rage::grcTexture* GetWindowTexture(fwString windowName);

	extern
		OVERLAY_DECL
		fwEvent<const wchar_t*, const wchar_t*> OnInvokeNative;

	extern
		OVERLAY_DECL
		fwEvent<bool> OnDrawBackground;

	OVERLAY_DECL void SetAudioSink(IAudioSink* sinkRef);
}

#define REQUIRE_IO_THREAD()   assert(CefCurrentlyOn(TID_IO));

struct nui_s
{
	bool initialized;

	DWORD nuiWidth;
	DWORD nuiHeight;
};

extern
	OVERLAY_DECL
	fwEvent<const char*, CefRefPtr<CefRequest>, CefRefPtr<CefResourceHandler>&> OnSchemeCreateRequest;
