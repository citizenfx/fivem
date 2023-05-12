/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <queue>
#include <shared_mutex>

#include <include/cef_client.h>
#include <include/cef_v8.h>

#include <concurrent_queue.h>

#include <wrl.h>

enum NUIPaintType
{
	NUIPaintTypeDummy,
	NUIPaintTypePostRender
};

#include <CefOverlay.h>

class
#ifdef COMPILING_NUI_CORE
	__declspec(dllexport)
#endif
	NUIWindow : public fwRefCountable
{
private:
	CefRefPtr<CefClient> m_client;

	void(__cdecl* m_onClientCreated)(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context);

	void Initialize(CefString url);

	concurrency::concurrent_queue<std::function<void()>> m_onLoadQueue;

public:
	NUIWindow(bool primary, int width, int height, const std::string& windowContext);

private:
	std::string m_windowContext;

	bool m_rawBlit;
	int m_width;
	int m_height;

	int m_roundedWidth;
	int m_roundedHeight;

	uint32_t m_lastFrameTime;
	uint32_t m_lastMessageTime;

	unsigned long m_dirtyFlag;
	RECT m_lastDirtyRect;
	std::shared_mutex m_renderBufferLock;
	char* m_renderBuffer;

	std::queue<CefRect> m_dirtyRects;

	std::set<std::string> m_pollQueue;

	fwRefContainer<nui::GITexture> m_nuiTexture;

	fwRefContainer<nui::GITexture> m_popupTexture;

	NUIPaintType m_paintType;

	uint64_t m_syncKey;

	std::map<CefRenderHandler::PaintElementType, fwRefContainer<nui::GITexture>> m_parentTextures;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_swapTexture;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_swapRtv;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_swapSrv;

	std::map<CefRenderHandler::PaintElementType, HANDLE> m_lastParentHandle;

	bool m_dereferencedNuiTexture;

	CefRect m_popupRect;

	std::shared_mutex m_textureMutex;

public:
	inline int		GetWidth() { return m_width; }
	inline int		GetHeight() { return m_height; }

	inline auto GetRenderBufferLock()
	{
		return std::unique_lock{ m_renderBufferLock };
	}

	inline void*	GetRenderBuffer() { return m_renderBuffer; }
	inline int		GetRoundedWidth() { return m_roundedWidth; }

	void TouchMessage();

	void InitializeRenderBacking();

	inline const std::string& GetName()
	{
		return m_name;
	}

	inline void SetName(const std::string& name)
	{
		m_name = name;
	}

	inline bool IsPrimary()
	{
		return m_rawBlit;
	}

	inline void ProcessLoadQueue()
	{
		std::function<void()> fn;

		while (m_onLoadQueue.try_pop(fn))
		{
			fn();
		}
	}

	inline void PushLoadQueue(std::function<void()>&& fn)
	{
		m_onLoadQueue.push(std::move(fn));
	}

private:
	std::string m_name;

public:
	void			AddDirtyRect(const CefRect& rect);

	inline void		MarkRenderBufferDirty() { InterlockedIncrement(&m_dirtyFlag); }

public:
	static fwRefContainer<NUIWindow> Create(bool primary, int width, int height, CefString url, bool instant, const std::string& context = {});

	void DeferredCreate();

private:
	CefString m_initUrl;

	bool m_isMuted = false;

public:
	~NUIWindow();

	void UpdateFrame();

	void Invalidate();

	void SetPaintType(NUIPaintType type);

	CefBrowser* GetBrowser();

	void SignalPoll(std::string& argument);

	void UpdateSharedResource(void* sharedHandle, uint64_t syncKey, const CefRenderHandler::RectList& rects, CefRenderHandler::PaintElementType type);

	inline void SetClientContextCreated(void(__cdecl* cb)(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context))
	{
		m_onClientCreated = cb;
	}

	inline void OnClientContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
	{
		if (m_onClientCreated)
		{
			m_onClientCreated(browser, frame, context);
		}
	}

	inline fwRefContainer<nui::GITexture> GetTexture() 
	{
		std::shared_lock<std::shared_mutex> _(m_textureMutex);
		return m_nuiTexture;
	}

	inline fwRefContainer<nui::GITexture> GetPopupTexture()
	{
		std::shared_lock<std::shared_mutex> _(m_textureMutex);
		return m_popupTexture;
	}

	inline NUIPaintType GetPaintType() { return m_paintType; }

	inline fwRefContainer<nui::GITexture> GetParentTexture(CefRenderHandler::PaintElementType type)
	{
		return m_parentTextures[type];
	}

	inline void SetParentTexture(CefRenderHandler::PaintElementType type, fwRefContainer<nui::GITexture> texture)
	{
		m_parentTextures[type] = texture;
	}

	CefRect GetPopupRect();

	void SetPopupRect(const CefRect& rect);

	void HandlePopupShow(bool show);

	bool IsFixedSizeWindow() const;
};
