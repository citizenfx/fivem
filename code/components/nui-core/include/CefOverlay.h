/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

//#define USE_NUI_ROOTLESS

#include "ComponentExport.h"

#define OVERLAY_DECL COMPONENT_EXPORT(NUI_CORE)

#if defined(COMPILING_NUI_CORE) || defined(COMPILING_NUI_RESOURCES) || defined(COMPILING_GLUE)
#define WANT_CEF_INTERNALS
#endif

#include <d3d11.h>

#include <memory>

#ifdef WANT_CEF_INTERNALS
#include "CfxRGBA.h"
#include "CfxRect.h"

#include <include/cef_app.h>
#include <include/cef_browser.h>
#include <include/cef_client.h>
#endif

#include <SharedInput.h>

#include <queue>

#ifdef WANT_CEF_INTERNALS
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
#endif

#ifndef COMPILING_NUI_CORE
class NUIWindow : public fwRefCountable
{

};
#else
class NUIWindow;
#endif

namespace nui
{
#ifdef WANT_CEF_INTERNALS
	struct GILockedTexture
	{
		int level;
		void* pBits;
		int pitch;
		int width;
		int height;
		int format;
		int numSubLevels;
	};

	enum class GILockFlags : int
	{
		Read = 1,
		Write = 2,
		Unknown = 4,
		WriteDiscard = 8,
		NoOverwrite = 16
	};

	class OVERLAY_DECL GITexture : public fwRefCountable
	{
	public:
		virtual ~GITexture() = default;

		virtual void* GetNativeTexture() = 0;

		virtual void* GetHostTexture() = 0;

		virtual bool Map(int numSubLevels, int subLevel, GILockedTexture* lockedTexture, GILockFlags flags) = 0;

		virtual void Unmap(GILockedTexture* lockedTexture) = 0;

		virtual void WithHostTexture(std::function<void(void*)>&& callback)
		{
			callback(GetHostTexture());
		}
	};

	enum class GITextureFormat
	{
		ARGB
	};

	struct ResultingRectangle
	{
		CRect rectangle;
		CRGBA color;
	};

	///
	// Represents the state of a setting.
	///
	/*--cef()--*/
	typedef enum
	{
		///
		// No permission
		///
		NUI_MEDIA_PERMISSION_NONE = 0,

		///
		// Audio capture permission
		///
		NUI_MEDIA_PERMISSION_DEVICE_AUDIO_CAPTURE = 1 << 0,

		///
		// Video capture permission
		///
		NUI_MEDIA_PERMISSION_DEVICE_VIDEO_CAPTURE = 1 << 1,

		///
		// Desktop audio capture permission
		///
		NUI_MEDIA_PERMISSION_DESKTOP_AUDIO_CAPTURE = 1 << 2,

		///
		// Desktop video capture permission
		///
		NUI_MEDIA_PERMISSION_DESKTOP_VIDEO_CAPTURE = 1 << 3,
	} media_access_permission_types_t;

	class OVERLAY_DECL GameInterface
	{
	public:
		virtual void GetGameResolution(int* width, int* height) = 0;

		virtual fwRefContainer<GITexture> CreateTexture(int width, int height, GITextureFormat format, void* pixelData) = 0;

		virtual fwRefContainer<GITexture> CreateTextureBacking(int width, int height, GITextureFormat format) = 0;

		virtual fwRefContainer<GITexture> CreateTextureFromShareHandle(HANDLE shareHandle) = 0;

		virtual fwRefContainer<GITexture> CreateTextureFromShareHandle(HANDLE shareHandle, int width, int height)
		{
			return CreateTextureFromShareHandle(shareHandle);
		}

		virtual void SetTexture(fwRefContainer<GITexture> texture, bool pm = false) = 0;

		virtual void DrawRectangles(int numRectangles, const ResultingRectangle* rectangles) = 0;

		virtual void UnsetTexture() = 0;

		virtual void SetGameMouseFocus(bool val) = 0;

		virtual HWND GetHWND() = 0;

		virtual void BlitTexture(fwRefContainer<GITexture> dst, fwRefContainer<GITexture> src) = 0;

		virtual ID3D11Device* GetD3D11Device() = 0;

		virtual ID3D11DeviceContext* GetD3D11DeviceContext() = 0;

		virtual fwRefContainer<GITexture> CreateTextureFromD3D11Texture(ID3D11Texture2D* texture) = 0;

		virtual bool RequestMediaAccess(const std::string& frameOrigin, const std::string& url, int permissions, const std::function<void(bool /* success */, int /* allowed mask */)>& onComplete)
		{
			return false;
		}

		virtual void SetHostCursorEnabled(bool enabled)
		{
		}

		virtual void SetHostCursor(HCURSOR cursor)
		{
		}

		virtual bool CanDrawHostCursor()
		{
			return false;
		}

		fwEvent<HWND, UINT, WPARAM, LPARAM, bool&, LRESULT&> OnWndProc;

		fwEvent<std::vector<InputTarget*>&> QueryInputTarget;

		fwEvent<int&> QueryMayLockCursor;

		fwEvent<> OnInitVfs;

		fwEvent<> OnInitRenderer;

		fwEvent<> OnRender;
		
		fwEvent<bool&> QueryShouldMute;
	};

	void OVERLAY_DECL Initialize(nui::GameInterface* gi);
#endif

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

		virtual void ProcessPacket(const float** data, int frames, int64_t pts) = 0;
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

	void OVERLAY_DECL CreateFrame(const std::string& frameName, const std::string& frameURL);
	void OVERLAY_DECL PrepareFrame(const std::string& frameName, const std::string& frameURL);
	void OVERLAY_DECL DestroyFrame(const std::string& frameName);
	bool OVERLAY_DECL HasFrame(const std::string& frameName);
	void OVERLAY_DECL SignalPoll(fwString frameName);

	bool OVERLAY_DECL HasFocus();
	bool OVERLAY_DECL HasFocusKeepInput();
	void OVERLAY_DECL GiveFocus(const std::string& frameName, bool hasFocus, bool hasCursor = false);
	void OVERLAY_DECL OverrideFocus(bool hasFocus);
	void OVERLAY_DECL KeepInput(bool keepInput);
	bool OVERLAY_DECL HasMainUI();
	void OVERLAY_DECL SetMainUI(bool enable);
	void OVERLAY_DECL SetHideCursor(bool hide);

	void ProcessInput();

	void OVERLAY_DECL ExecuteRootScript(const std::string& scriptBit);

	void OVERLAY_DECL PostFrameMessage(const std::string& frameName, const std::string& jsonData);

	void OVERLAY_DECL PostRootMessage(const std::string& jsonData);

#ifdef WANT_CEF_INTERNALS
	OVERLAY_DECL CefBrowser* GetBrowser();

	OVERLAY_DECL fwRefContainer<NUIWindow> GetWindow();

	OVERLAY_DECL CefBrowser* GetFocusBrowser();

	// window API
	OVERLAY_DECL CefBrowser* GetNUIWindowBrowser(fwString windowName);
#endif

	OVERLAY_DECL fwRefContainer<NUIWindow> CreateNUIWindow(fwString windowName, int width, int height, fwString windowURL, bool rawBlit = false);
	OVERLAY_DECL void DestroyNUIWindow(fwString windowName);
	OVERLAY_DECL void ExecuteWindowScript(const std::string& windowName, const std::string& scriptBit);
	OVERLAY_DECL void SetNUIWindowURL(fwString windowName, fwString url);

	OVERLAY_DECL void SwitchContext(const std::string& contextId);

#ifdef WANT_CEF_INTERNALS
	OVERLAY_DECL void RegisterSchemeHandlerFactory(const CefString& scheme_name,
	const CefString& domain_name,
	CefRefPtr<CefSchemeHandlerFactory> factory);

	OVERLAY_DECL fwRefContainer<GITexture> GetWindowTexture(fwString windowName);
#endif

	extern
		OVERLAY_DECL
		fwEvent<const wchar_t*, const wchar_t*> OnInvokeNative;

	extern
		OVERLAY_DECL
		fwEvent<bool> OnDrawBackground;

	extern OVERLAY_DECL
		fwEvent<std::function<void(bool, const char*, size_t)>>
		RequestNUIBlocklist;

	extern OVERLAY_DECL fwEvent<> OnInitialize;

	OVERLAY_DECL void SetAudioSink(IAudioSink* sinkRef);

	using TResourceLookupFn = std::function<std::string(const std::string&, const std::string&)>;

	OVERLAY_DECL void SetResourceLookupFunction(const TResourceLookupFn& fn);
}

#define REQUIRE_IO_THREAD()   assert(CefCurrentlyOn(TID_IO));

struct nui_s
{
	bool initialized;

	DWORD nuiWidth;
	DWORD nuiHeight;
};

#ifdef WANT_CEF_INTERNALS
extern
	OVERLAY_DECL
	fwEvent<const char*, CefRefPtr<CefRequest>, CefRefPtr<CefResourceHandler>&> OnSchemeCreateRequest;
#endif
