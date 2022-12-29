/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIApp.h"
#include "NUIClient.h"
#include "NUISchemeHandlerFactory.h"
#include "NUIWindowManager.h"

#include <CL2LaunchMode.h>

#include <CefOverlay.h>

#include <psapi.h>
#include <delayimp.h>

#include <include/cef_origin_whitelist.h>

#include "ResumeComponent.h"
#include "HookFunction.h"

#include <CfxSubProcess.h>

#include <HostSharedData.h>

#include <Error.h>

#include <MinHook.h>

#include "memdbgon.h"

#include <CoreConsole.h>
#include <Hooking.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <VFSManager.h>
#include <VFSZipFile.h>

#include <include/cef_api_hash.h>
#include <include/cef_version.h>

#include "DeferredInitializer.h"
#include <Error.h>

namespace nui
{
fwRefContainer<NUIWindow> FindNUIWindow(fwString windowName);
}

std::wstring GetNUIStoragePath();

nui::GameInterface* g_nuiGi;

struct GameRenderData
{
	HANDLE handle;
	int width;
	int height;
	bool requested;

	GameRenderData()
		: requested(false), handle(NULL), width(0), height(0)
	{

	}
};

static GLuint g_curGlTexture;
static std::set<GLuint> g_backBufferTextures;

static void BindGameRenderHandle();

static std::map<GLuint, EGLSurface> g_pbuffers;

static void (*g_origglDeleteTextures)(GLsizei n, const GLuint* textures);

static void glDeleteTexturesHook(GLsizei n, const GLuint* textures)
{
	for (int i = 0; i < n; i++)
	{
		GLuint texture = textures[i];
		g_backBufferTextures.erase(texture);

		if (g_pbuffers.find(texture) != g_pbuffers.end())
		{
			static auto _eglGetCurrentDisplay = (decltype(&eglGetCurrentDisplay))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglGetCurrentDisplay"));
			static auto _eglDestroySurface = (decltype(&eglDestroySurface))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglDestroySurface"));
			static auto _eglReleaseTexImage = (decltype(&eglReleaseTexImage))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglReleaseTexImage"));
			static auto _eglGetError = (decltype(&eglGetError))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglGetError"));

			auto m_display = _eglGetCurrentDisplay();
			_eglReleaseTexImage(m_display, g_pbuffers[texture], EGL_BACK_BUFFER);
			_eglDestroySurface(m_display, g_pbuffers[texture]);
			g_pbuffers.erase(texture);
		}
	}

	g_origglDeleteTextures(n, textures);
}

static void (*g_origglBindTexture)(GLenum target, GLuint texture);

static void glBindTextureHook(GLenum target, GLuint texture)
{
	// this gets called really frequently but do we want to do so here?
	static HANDLE lastBackbufHandle;
	static HostSharedData<GameRenderData> handleData(launch::IsSDK() ? "CfxGameRenderHandleFxDK" : "CfxGameRenderHandle");

	if (handleData->handle != lastBackbufHandle)
	{
		lastBackbufHandle = handleData->handle;

		for (auto textureId : g_backBufferTextures)
		{
			g_curGlTexture = textureId;

			g_origglBindTexture(GL_TEXTURE_2D, textureId);
			BindGameRenderHandle();
		}
	}

	if (target == GL_TEXTURE_2D)
	{
		g_curGlTexture = texture;
	}

	g_origglBindTexture(target, texture);
}

EGLConfig ChooseCompatibleConfig()
{
	static auto _eglChooseConfig = (decltype(&eglChooseConfig))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglChooseConfig"));
	static auto _eglGetConfigAttrib = (decltype(&eglGetConfigAttrib))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglGetConfigAttrib"));
	static auto _eglGetCurrentDisplay = (decltype(&eglGetCurrentDisplay))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglGetCurrentDisplay"));

	const EGLint buffer_bind_to_texture = EGL_BIND_TO_TEXTURE_RGBA;
	const EGLint buffer_size = 32;
	EGLint const attrib_list[] = { EGL_RED_SIZE,
		8,
		EGL_GREEN_SIZE,
		8,
		EGL_BLUE_SIZE,
		8,
		EGL_SURFACE_TYPE,
		EGL_PBUFFER_BIT,
		buffer_bind_to_texture,
		EGL_TRUE,
		EGL_BUFFER_SIZE,
		buffer_size,
		EGL_NONE };

	EGLint num_config;
	EGLDisplay display = _eglGetCurrentDisplay();
	EGLBoolean result = _eglChooseConfig(display, attrib_list, nullptr, 0, &num_config);
	if (result != EGL_TRUE)
		return nullptr;
	std::vector<EGLConfig> all_configs(num_config);
	result = _eglChooseConfig(display, attrib_list,
	all_configs.data(), num_config, &num_config);
	if (result != EGL_TRUE)
		return nullptr;
	for (EGLConfig config : all_configs)
	{
		EGLint bits;
		if (!_eglGetConfigAttrib(display, config, EGL_RED_SIZE, &bits) || bits != 8)
		{
			continue;
		}

		if (!_eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &bits) || bits != 8)
		{
			continue;
		}

		if (!_eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &bits) || bits != 8)
		{
			continue;
		}

		if (!_eglGetConfigAttrib(display, config, EGL_ALPHA_SIZE, &bits) || bits != 8)
		{
			continue;
		}

		return config;
	}
	return nullptr;
}

static void BindGameRenderHandle()
{
	static auto _eglGetCurrentDisplay = (decltype(&eglGetCurrentDisplay))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglGetCurrentDisplay"));
	static auto _eglDestroySurface = (decltype(&eglDestroySurface))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglDestroySurface"));
	static auto _eglChooseConfig = (decltype(&eglChooseConfig))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglChooseConfig"));
	static auto _eglGetConfigs = (decltype(&eglGetConfigs))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglGetConfigs"));
	static auto _eglCreatePbufferFromClientBuffer = (decltype(&eglCreatePbufferFromClientBuffer))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglCreatePbufferFromClientBuffer"));
	static auto _eglBindTexImage = (decltype(&eglBindTexImage))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglBindTexImage"));
	static auto _eglGetError = (decltype(&eglGetError))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglGetError"));
	static auto _eglReleaseTexImage = (decltype(&eglReleaseTexImage))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglReleaseTexImage"));

	auto m_display = _eglGetCurrentDisplay();

	static HostSharedData<GameRenderData> handleData(launch::IsSDK() ? "CfxGameRenderHandleFxDK" : "CfxGameRenderHandle");

	// not existent yet, but we will retry this later on
	if (!handleData->handle)
	{
		return;
	}

	EGLint pbuffer_attributes[] = {
		EGL_WIDTH, handleData->width,
		EGL_HEIGHT, handleData->height,
		EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
		EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,
		EGL_NONE
	};

	auto config = ChooseCompatibleConfig();

	if (!config)
	{
		return;
	}

	EGLSurface pbuffer = _eglCreatePbufferFromClientBuffer(
	m_display,
	EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE,
	(EGLClientBuffer)handleData->handle,
	config,
	pbuffer_attributes);

	if (g_pbuffers.find(g_curGlTexture) != g_pbuffers.end())
	{
		_eglReleaseTexImage(m_display, g_pbuffers[g_curGlTexture], EGL_BACK_BUFFER);
		_eglDestroySurface(m_display, g_pbuffers[g_curGlTexture]);
		g_pbuffers.erase(g_curGlTexture);
	}

	g_pbuffers.insert({ g_curGlTexture, pbuffer });

	handleData->requested = true;

	_eglBindTexImage(m_display, pbuffer, EGL_BACK_BUFFER);
}

static void(*g_origglTexParameterf)(GLenum target, GLenum pname, GLfloat param);

static void glTexParameterfHook(GLenum target, GLenum pname, GLfloat param)
{
	// 'secret' activation sequence
	static std::map<GLuint, int> stages;
	int& stage = stages[g_curGlTexture];

	if (target == GL_TEXTURE_2D && pname == GL_TEXTURE_WRAP_T)
	{
		switch (stage)
		{
		case 0:
			if (param == GL_CLAMP_TO_EDGE)
			{
				stage = 1;
			}

			break;
		case 1:
			if (param == GL_MIRRORED_REPEAT)
			{
				stage = 2;
			}
			else if (param != GL_CLAMP_TO_EDGE)
			{
				stage = 0;
			}

			break;
		case 2:
			if (param == GL_REPEAT)
			{
				stage = 3;
			}
			else
			{
				stage = 0;
			}

			break;
		}
	}
	else
	{
		stage = 0;
	}

	if (stage == 3)
	{
		stage = 0;

		BindGameRenderHandle();
		g_backBufferTextures.insert(g_curGlTexture);
	}
	else if (stage <= 1)
	{
		g_origglTexParameterf(target, pname, param);
	}
}

static HRESULT(*g_origD3D11CreateDevice)(_In_opt_ IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, _COM_Outptr_opt_ ID3D11Device** ppDevice, _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel, _COM_Outptr_opt_ ID3D11DeviceContext** ppImmediateContext);

static HRESULT (*g_origCreateTexture2D)(
	ID3D11Device* device,
	const D3D11_TEXTURE2D_DESC* pDesc,
	const D3D11_SUBRESOURCE_DATA* pInitialData,
	ID3D11Texture2D** ppTexture2D
);

#include <unordered_set>

#include <wrl.h>
#include <dxgi1_3.h>

#include <HostSharedData.h>
#include <CfxState.h>

namespace WRL = Microsoft::WRL;

MIDL_INTERFACE("035f3ab4-482e-4e50-b41f-8a7f8bd8960b")
IDXGIResourceHack : public IDXGIDeviceSubObject
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetSharedHandle(
		/* [annotation][out] */
		_Out_  HANDLE * pSharedHandle) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetUsage(
		/* [out] */ DXGI_USAGE* pUsage) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetEvictionPriority2(
		/* [in] */ UINT EvictionPriority) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetEvictionPriority(
		/* [annotation][retval][out] */
		_Out_  UINT* pEvictionPriority) = 0;

};

MIDL_INTERFACE("30961379-4609-4a41-998e-54fe567ee0c1")
IDXGIResource1Hack : public IDXGIResourceHack
{
public:
	virtual HRESULT STDMETHODCALLTYPE CreateSubresourceSurface(
	UINT index,
	/* [annotation][out] */
	_COM_Outptr_ IDXGISurface2 * *ppSurface)
	= 0;

	virtual HRESULT STDMETHODCALLTYPE CreateSharedHandle(
	/* [annotation][in] */
	_In_opt_ const SECURITY_ATTRIBUTES* pAttributes,
	/* [annotation][in] */
	_In_ DWORD dwAccess,
	/* [annotation][in] */
	_In_opt_ LPCWSTR lpName,
	/* [annotation][out] */
	_Out_ HANDLE* pHandle)
	= 0;
};

MIDL_INTERFACE("9d8e1289-d7b3-465f-8126-250e349af85d")
IDXGIKeyedMutexHack : public IDXGIDeviceSubObject
{
public:
	virtual HRESULT STDMETHODCALLTYPE AcquireSync(
	/* [in] */ UINT64 Key,
	/* [in] */ DWORD dwMilliseconds)
	= 0;

	virtual HRESULT STDMETHODCALLTYPE ReleaseSync(
	/* [in] */ UINT64 Key)
	= 0;
};

MIDL_INTERFACE("EF93CE51-2C32-488A-ADC1-3453BC0D1664")
IMyTexture : public IUnknown
{
	virtual void GetOriginal(ID3D11Resource** dst) = 0;
};

class Texture2DWrap : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IDXGIResourceHack, IDXGIResource1Hack, IDXGIKeyedMutexHack, ID3D11Texture2D, IMyTexture>
{
private:
	WRL::ComPtr<ID3D11Texture2D> m_texture;
	WRL::ComPtr<IDXGIResource> m_resource;
	WRL::ComPtr<IDXGIResource1> m_resource1;
	WRL::ComPtr<IDXGIKeyedMutex> m_keyedMutex;

public:
	Texture2DWrap(ID3D11Texture2D* res)
	{
		m_texture.Attach(res);
		m_texture.As(&m_resource);
		m_texture.As(&m_resource1);
		m_texture.As(&m_keyedMutex);

		InitializeDeleter();
	}

	~Texture2DWrap();

	// Inherited via RuntimeClass
	virtual HRESULT STDMETHODCALLTYPE AcquireSync(UINT64 Key, DWORD dwMilliseconds) override
	{
		if (m_keyedMutex)
		{
			return m_keyedMutex->AcquireSync(Key, dwMilliseconds);
		}

		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE ReleaseSync(UINT64 Key) override
	{
		if (m_keyedMutex)
		{
			return m_keyedMutex->ReleaseSync(Key);
		}

		return S_OK;
	}

	virtual HRESULT __stdcall SetPrivateData(REFGUID Name, UINT DataSize, const void* pData) override
	{
		return m_texture->SetPrivateData(Name, DataSize, pData);
	}

	virtual HRESULT __stdcall SetPrivateDataInterface(REFGUID Name, const IUnknown* pUnknown) override
	{
		return m_texture->SetPrivateDataInterface(Name, pUnknown);
	}

	virtual HRESULT __stdcall GetPrivateData(REFGUID Name, UINT* pDataSize, void* pData) override
	{
		return m_texture->GetPrivateData(Name, pDataSize, pData);
	}

	virtual HRESULT __stdcall GetParent(REFIID riid, void** ppParent) override
	{
		return m_resource->GetParent(riid, ppParent);
	}

	virtual HRESULT __stdcall GetDevice(REFIID riid, void** ppDevice) override
	{
		return m_resource->GetDevice(riid, ppDevice);
	}

	virtual HRESULT __stdcall GetSharedHandle(HANDLE* pSharedHandle) override;

	virtual HRESULT __stdcall GetUsage(DXGI_USAGE* pUsage) override
	{
		return m_resource->GetUsage(pUsage);
	}

	virtual HRESULT __stdcall SetEvictionPriority2(UINT EvictionPriority) override
	{
		return m_resource->SetEvictionPriority(EvictionPriority);
	}

	virtual void __stdcall SetEvictionPriority(UINT EvictionPriority) override
	{
		m_resource->SetEvictionPriority(EvictionPriority);
	}

	virtual void __stdcall GetDevice(ID3D11Device** ppDevice) override
	{
		return m_texture->GetDevice(ppDevice);
	}

	virtual void __stdcall GetType(D3D11_RESOURCE_DIMENSION* pResourceDimension) override
	{
		return m_texture->GetType(pResourceDimension);
	}

	virtual HRESULT __stdcall GetEvictionPriority(UINT*) override
	{
		return S_OK;
	}

	virtual UINT __stdcall GetEvictionPriority(void) override
	{
		return 0;
	}

	virtual void __stdcall GetDesc(D3D11_TEXTURE2D_DESC* pDesc) override
	{
		return m_texture->GetDesc(pDesc);
	}

	virtual void GetOriginal(ID3D11Resource** dst) override
	{
		*dst = m_texture.Get();
	}

	// IDXGIResource1
	virtual HRESULT STDMETHODCALLTYPE CreateSubresourceSurface(
	UINT index,
	/* [annotation][out] */
	_COM_Outptr_ IDXGISurface2** ppSurface)
	{
		return m_resource1->CreateSubresourceSurface(index, ppSurface);
	}

	virtual HRESULT STDMETHODCALLTYPE CreateSharedHandle(
	/* [annotation][in] */
	_In_opt_ const SECURITY_ATTRIBUTES* pAttributes,
	/* [annotation][in] */
	_In_ DWORD dwAccess,
	/* [annotation][in] */
	_In_opt_ LPCWSTR lpName,
	/* [annotation][out] */
	_Out_ HANDLE* pHandle)
	{
		return E_FAIL;
	}

	HANDLE m_handle = NULL;

private:
	static void InitializeDeleter();

	static concurrency::concurrent_queue<std::tuple<uint64_t, HANDLE>> deletionQueue;
};

static std::mutex g_textureLock;
static std::map<HANDLE, WRL::ComPtr<ID3D11Texture2D>> g_textureHacks;
concurrency::concurrent_queue<std::tuple<uint64_t, HANDLE>> Texture2DWrap::deletionQueue;

struct TextureRefQueue
{
	struct TextureRef
	{
		uint64_t handle = 0;
		long refcount = 0;
	};

	TextureRef textures[32];
};

auto GetWakeEvent()
{
	static HANDLE wakeEvent = CreateEventW(NULL, FALSE, FALSE, ToWide(fmt::sprintf("CFX_%s_%s_NUITextureWake", launch::GetLaunchModeKey(), launch::GetProductKey())).c_str());
	return wakeEvent;
}

void NUI_AcceptTexture(uint64_t handle)
{
	HostSharedData<TextureRefQueue> refQueue("NUITextureRefs");
	for (auto& texture : refQueue->textures)
	{
		if (texture.handle == handle)
		{
			InterlockedDecrement(&texture.refcount);
			break;
		}
	}

	SetEvent(GetWakeEvent());
}

void NUI_AddTexture(HANDLE handleH)
{
	auto handle = (uint64_t)handleH;

	HostSharedData<TextureRefQueue> refQueue("NUITextureRefs");
	for (auto& texture : refQueue->textures)
	{
		if (texture.handle == handle)
		{
			InterlockedIncrement(&texture.refcount);
			return;
		}
	}

	// not found
	for (auto& texture : refQueue->textures)
	{
		if (!texture.handle)
		{
			texture.handle = handle;
			texture.refcount = 1;
			break;
		}
	}
}

void Texture2DWrap::InitializeDeleter()
{
	static std::thread* deletionThread = new std::thread([]()
	{
		SetThreadName(-1, "GPU Deletion Workaround");

		auto wakeEvent = GetWakeEvent();

		HostSharedData<TextureRefQueue> refQueue("NUITextureRefs");

		while (true)
		{
			WaitForSingleObject(wakeEvent, 2500);

			for (auto& entry : refQueue->textures)
			{
				if (entry.handle && entry.refcount <= 0)
				{
					g_textureHacks.erase(HANDLE(entry.handle));

					entry.handle = 0;
				}
			}

			decltype(deletionQueue)::value_type item;
			std::vector<decltype(item)> toAdd;

			while (deletionQueue.try_pop(item))
			{
				if (GetTickCount64() > (std::get<0>(item) + 7500))
				{
					for (auto& entry : refQueue->textures)
					{
						if (entry.handle == uint64_t(std::get<1>(item)))
						{
							entry.handle = 0;
						}
					}

					std::lock_guard<std::mutex> _(g_textureLock);
					g_textureHacks.erase(std::get<1>(item));
				}
				else
				{
					toAdd.push_back(item);
				}
			}

			for (auto& entry : toAdd)
			{
				deletionQueue.push(entry);
			}
		}
	});
}

Texture2DWrap::~Texture2DWrap()
{
	if (m_handle)
	{
		deletionQueue.push({ GetTickCount64(), m_handle });
	}
}

HRESULT Texture2DWrap::GetSharedHandle(HANDLE* pSharedHandle)
{
#if !defined(GTA_FIVE)
	HANDLE handle;
	HRESULT hr = m_resource1->CreateSharedHandle(NULL, DXGI_SHARED_RESOURCE_READ, NULL, &handle);

	static HostSharedData<CfxState> initState("CfxInitState");

	if (SUCCEEDED(hr))
	{
		auto proc = OpenProcess(PROCESS_DUP_HANDLE, FALSE, (initState->gamePid) ? initState->gamePid : initState->GetInitialPid());
		DuplicateHandle(GetCurrentProcess(), handle, proc, pSharedHandle, 0, FALSE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
		CloseHandle(proc);

		m_handle = *pSharedHandle;
		g_textureHacks.insert({ *pSharedHandle, m_texture });
		NUI_AddTexture(m_handle);
	}

	return hr;
#else
	auto hr = m_resource->GetSharedHandle(&m_handle);

	if (SUCCEEDED(hr))
	{
		*pSharedHandle = m_handle;
		g_textureHacks.insert({ m_handle, m_texture });
		NUI_AddTexture(m_handle);
	}

	return hr;
#endif

	return E_NOTIMPL;
}

static HRESULT CreateTexture2DHook(
	ID3D11Device* device,
	D3D11_TEXTURE2D_DESC* pDesc,
	const D3D11_SUBRESOURCE_DATA* pInitialData,
	ID3D11Texture2D** ppTexture2D
)
{
	auto& desc = *pDesc;

	bool wrapped = false;

#if !defined(GTA_FIVE)
	if (IsWindows8OrGreater())
	{
		if ((desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED) || (desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX))
		{
			if (desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM)
			{
				desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			}

			desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
			wrapped = true;
		}
	}
#else
	if ((desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED) || (desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX))
	{
		wrapped = true;
	}
#endif

	auto rv = g_origCreateTexture2D(device, &desc, pInitialData, ppTexture2D);

	if (wrapped && *ppTexture2D)
	{
		auto retVal = WRL::Make<Texture2DWrap>(*ppTexture2D);
		retVal.CopyTo(ppTexture2D);
	}

	return rv;
}

#include <d3d11_1.h>

static HRESULT OpenSharedResource1Hook(ID3D11Device1* device, HANDLE hRes, REFIID iid, void** ppRes)
{
	if (auto data = (HANDLE*)MapViewOfFile(hRes, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(HANDLE)))
	{
		HRESULT hr = device->OpenSharedResource(*data, iid, ppRes);
		// we don't release here as the resource will still get read by a target process
		UnmapViewOfFile(data);

		if (iid == __uuidof(ID3D11Texture2D) && *ppRes)
		{
			auto retVal = WRL::Make<Texture2DWrap>((ID3D11Texture2D*)*ppRes);
			retVal.CopyTo(iid, ppRes);
		}

		return hr;
	}

	return E_INVALIDARG;
}

static HRESULT (*g_origOpenSharedResourceHook)(ID3D11Device* device, HANDLE hRes, REFIID iid, void** ppRes);

static HRESULT OpenSharedResourceHook(ID3D11Device* device, HANDLE hRes, REFIID iid, void** ppRes)
{
	auto it = g_textureHacks.find(hRes);

	if (it != g_textureHacks.end())
	{
		return it->second->QueryInterface(iid, ppRes);
	}

	return g_origOpenSharedResourceHook(device, hRes, iid, ppRes);
}

static HRESULT (*g_origCreateShaderResourceView)(ID3D11Device* device, ID3D11Resource* resource, const D3D11_SHADER_RESOURCE_VIEW_DESC* desc, ID3D11ShaderResourceView** out);

static HRESULT CreateShaderResourceViewHook(ID3D11Device* device, ID3D11Resource* resource, const D3D11_SHADER_RESOURCE_VIEW_DESC* desc, ID3D11ShaderResourceView** out)
{
	WRL::ComPtr<ID3D11Resource> resourceRef(resource);
	WRL::ComPtr<IMyTexture> mt;

	if (SUCCEEDED(resourceRef.As(&mt)))
	{
		mt->GetOriginal(&resource);
	}

	return g_origCreateShaderResourceView(device, resource, desc, out);
}

static HRESULT (*g_origCreateRenderTargetView)(ID3D11Device* device, ID3D11Resource* resource, const D3D11_RENDER_TARGET_VIEW_DESC* desc, ID3D11RenderTargetView** out);

static HRESULT CreateRenderTargetViewHook(ID3D11Device* device, ID3D11Resource* resource, const D3D11_RENDER_TARGET_VIEW_DESC* desc, ID3D11RenderTargetView** out)
{
	WRL::ComPtr<ID3D11Resource> resourceRef(resource);
	WRL::ComPtr<IMyTexture> mt;

	if (SUCCEEDED(resourceRef.As(&mt)))
	{
		mt->GetOriginal(&resource);
	}

	return g_origCreateRenderTargetView(device, resource, desc, out);
}

static HRESULT (*g_origCopySubresourceRegion)(ID3D11DeviceContext* cxt, ID3D11Resource* pDstResource,
	UINT DstSubresource, UINT DstX, UINT DstY, UINT DstZ,
	ID3D11Resource* pSrcResource, UINT SrcSubresource, const D3D11_BOX* pSrcBox);

static HRESULT CopySubresourceRegionHook(ID3D11DeviceContext* cxt, ID3D11Resource* dst,
	UINT DstSubresource, UINT DstX, UINT DstY, UINT DstZ,
	ID3D11Resource* src, UINT SrcSubresource, const D3D11_BOX* pSrcBox)
{
	WRL::ComPtr<ID3D11Resource> dstRef(dst);
	WRL::ComPtr<IMyTexture> mt;

	if (SUCCEEDED(dstRef.As(&mt)))
	{
		mt->GetOriginal(&dst);
	}

	WRL::ComPtr<ID3D11Resource> srcRef(src);

	if (SUCCEEDED(srcRef.As(&mt)))
	{
		mt->GetOriginal(&src);
	}

	return g_origCopySubresourceRegion(cxt, dst, DstSubresource, DstX, DstY, DstZ, src, SrcSubresource, pSrcBox);
}

static HRESULT(*g_origCopyResource)(ID3D11DeviceContext* cxt, ID3D11Resource* dst, ID3D11Resource* src);

static HRESULT CopyResourceHook(ID3D11DeviceContext* cxt, ID3D11Resource* dst, ID3D11Resource* src)
{
	WRL::ComPtr<ID3D11Resource> dstRef(dst);
	WRL::ComPtr<IMyTexture> mt;

	if (SUCCEEDED(dstRef.As(&mt)))
	{
		mt->GetOriginal(&dst);
	}

	WRL::ComPtr<ID3D11Resource> srcRef(src);

	if (SUCCEEDED(srcRef.As(&mt)))
	{
		mt->GetOriginal(&src);
	}

	return g_origCopyResource(cxt, dst, src);
}

#include <dxgi1_6.h>

#pragma comment(lib, "dxgi.lib")

static ID3D11DeviceContext* g_origImContext;
static ID3D11Device* g_origDevice;

DLL_EXPORT ID3D11Device* GetRawD3D11Device()
{
	return g_origDevice;
}

static void PatchAdapter(IDXGIAdapter** pAdapter)
{
	if (!*pAdapter)
	{
		WRL::ComPtr<IDXGIFactory1> dxgiFactory;
		CreateDXGIFactory1(IID_IDXGIFactory1, &dxgiFactory);

		WRL::ComPtr<IDXGIAdapter1> adapter;
		WRL::ComPtr<IDXGIFactory6> factory6;
		HRESULT hr = dxgiFactory.As(&factory6);
		if (SUCCEEDED(hr))
		{
			for (UINT adapterIndex = 0;
				 DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()));
				 adapterIndex++)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					continue;
				}

				adapter.CopyTo(pAdapter);

				break;
			}
		}
	}
}

#include <d3d11_1.h>

static bool g_reshit;

template<typename TFnLeft, typename TFnRight>
void VHook(intptr_t& ref, TFnLeft fn, TFnRight out)
{
	if (ref == (intptr_t)fn)
	{
		return;
	}

	if (*out)
	{
		return;
	}

	*out = (decltype(*out))ref;
	ref = (intptr_t)fn;
}

// primarily to prevent Flush() calls while another thread is creating a device
static std::shared_mutex g_d3d11Mutex;
static void (__stdcall *g_origFlush)(void*);

static void __stdcall FlushHook(void* cxt)
{
	std::unique_lock _(g_d3d11Mutex);
	g_origFlush(cxt);
}

static void PatchCreateResults(ID3D11Device** ppDevice, ID3D11DeviceContext** ppImmediateContext, bool forceGPU)
{
	bool can = true;

#if !defined(IS_RDR3)
	can = wcsstr(GetCommandLineW(), L"type=gpu") != nullptr || forceGPU;
#endif

	if (ppDevice && ppImmediateContext && can && *ppDevice && *ppImmediateContext)
	{
		auto vtbl = **(intptr_t***)ppDevice;
		auto vtblCxt = **(intptr_t***)ppImmediateContext;

		auto ourVtbl = new intptr_t[640];
		memcpy(ourVtbl, vtbl, 640 * sizeof(intptr_t));
		VHook(ourVtbl[5], &CreateTexture2DHook, &g_origCreateTexture2D);
		VHook(ourVtbl[7], &CreateShaderResourceViewHook, &g_origCreateShaderResourceView);
		VHook(ourVtbl[9], &CreateRenderTargetViewHook, &g_origCreateRenderTargetView);
		VHook(ourVtbl[28], &OpenSharedResourceHook, &g_origOpenSharedResourceHook);

		auto ourVtblCxt = new intptr_t[640];
		memcpy(ourVtblCxt, vtblCxt, 640 * sizeof(intptr_t));

		VHook(ourVtblCxt[111], &FlushHook, &g_origFlush);
		VHook(ourVtblCxt[46], &CopySubresourceRegionHook, &g_origCopySubresourceRegion);
		VHook(ourVtblCxt[47], &CopyResourceHook, &g_origCopyResource);

		**(intptr_t***)ppDevice = ourVtbl;
		**(intptr_t***)ppImmediateContext = ourVtblCxt;
	}

	if (ppImmediateContext && *ppImmediateContext)
	{
		g_origImContext = *ppImmediateContext;
		g_origImContext->AddRef();
	}
	else if (ppDevice && *ppDevice)
	{
		(*ppDevice)->GetImmediateContext(&g_origImContext);
	}

	if (ppDevice && ppImmediateContext && *ppDevice)
	{
		g_origDevice = *ppDevice;
	}

	// horrible hack as 'reshade' doesn't give us our original device back
	if (g_reshit && ppDevice && *ppDevice)
	{
		// get context
		WRL::ComPtr<ID3D11DeviceContext> cxt;
		(*ppDevice)->GetImmediateContext(&cxt);

		{
			// get annotation, which will return the original device
			WRL::ComPtr<ID3DUserDefinedAnnotation> ann;
			if (SUCCEEDED(cxt.As(&ann)))
			{
				// get real cxt
				WRL::ComPtr<ID3D11DeviceContext> realCxt;
				if (SUCCEEDED(ann.As(&realCxt)))
				{
					realCxt->GetDevice(&g_origDevice);
				}
			}
		}
	}
}

static HRESULT (*g_origD3D11CreateDeviceAndSwapChain)(_In_opt_ IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, _In_opt_ CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, _COM_Outptr_opt_ IDXGISwapChain** ppSwapChain, _COM_Outptr_opt_ ID3D11Device** ppDevice, _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel, _COM_Outptr_opt_ ID3D11DeviceContext** ppImmediateContext);

#include <variant>

struct ModuleData
{
	using TSet = std::map<uintptr_t, uintptr_t>;

	const TSet dataSet;

	ModuleData(std::initializer_list<std::variant<std::wstring, HMODULE>> moduleNames)
		: dataSet(CreateSet(moduleNames))
	{
	}

	bool IsInSet(void* address)
	{
		uintptr_t ptr = (uintptr_t)address;

		auto it = dataSet.upper_bound(ptr);

		if (it != dataSet.begin())
		{
			it--;

			if (ptr >= it->first && ptr < it->second)
			{
				return true;
			}
		}

		return false;
	}

private:
	TSet CreateSet(std::initializer_list<std::variant<std::wstring, HMODULE>> moduleNames)
	{
		TSet set;

		for (auto& name : moduleNames)
		{
			HMODULE hMod = (name.index() == 0) ? GetModuleHandle(std::get<0>(name).c_str()) : std::get<1>(name);
			
			if (hMod)
			{
				MODULEINFO mi;
				GetModuleInformation(GetCurrentProcess(), hMod, &mi, sizeof(mi));

				set.insert({ (uintptr_t)hMod, (uintptr_t)hMod + mi.SizeOfImage });
			}
		}

		return std::move(set);
	}
};

static std::unique_ptr<ModuleData> g_libgl;
static std::unique_ptr<ModuleData> g_d3d11;
static HMODULE g_sysD3D11;

static HRESULT D3D11CreateDeviceAndSwapChainHook(_In_opt_ IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, _In_opt_ CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, _COM_Outptr_opt_ IDXGISwapChain** ppSwapChain, _COM_Outptr_opt_ ID3D11Device** ppDevice, _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel, _COM_Outptr_opt_ ID3D11DeviceContext** ppImmediateContext)
{
	std::shared_lock _(g_d3d11Mutex);

	PatchAdapter(&pAdapter);

	if (pAdapter)
	{
		// if we ended up setting adapter, driver type should be unknown, or queries will fail
		DriverType = D3D_DRIVER_TYPE_UNKNOWN;
	}

	auto hr = g_origD3D11CreateDeviceAndSwapChain(pAdapter, DriverType, Software, Flags | D3D11_CREATE_DEVICE_BGRA_SUPPORT, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);

	PatchCreateResults(ppDevice, ppImmediateContext, g_libgl->IsInSet(_ReturnAddress()));

	return hr;
}

static HRESULT D3D11CreateDeviceHook(_In_opt_ IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, _COM_Outptr_opt_ ID3D11Device** ppDevice, _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel, _COM_Outptr_opt_ ID3D11DeviceContext** ppImmediateContext)
{
	std::shared_lock _(g_d3d11Mutex);

	// if this is the OS calling us, we need to be special and *somehow* convince any hook to give up their true colors
	// since D3D11CoreCreateDevice is super obscure, we'll use *that*
	if (g_d3d11->IsInSet(_ReturnAddress()))
	{
		static auto D3D11CoreCreateDevice = (HRESULT(WINAPI*)(IDXGIFactory * pFactory,
		IDXGIAdapter * pAdapter,
		D3D_DRIVER_TYPE DriverType,
		HMODULE Software, UINT Flags,
		const D3D_FEATURE_LEVEL* pFeatureLevels,
		UINT FeatureLevels,
		UINT SDKVersion,
		ID3D11Device** ppDevice,
		D3D_FEATURE_LEVEL* pFeatureLevel)) GetProcAddress(g_sysD3D11, "D3D11CoreCreateDevice");

		if (D3D11CoreCreateDevice)
		{
			if (!pFeatureLevels)
			{
				static const D3D_FEATURE_LEVEL fls[] = 
				{
					D3D_FEATURE_LEVEL_11_0
				};

				pFeatureLevels = fls;
				FeatureLevels = 1;
			}

			HRESULT hr = D3D11CoreCreateDevice(NULL, pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel);

			if (SUCCEEDED(hr))
			{
				if (ppDevice && *ppDevice && ppImmediateContext)
				{
					(*ppDevice)->GetImmediateContext(ppImmediateContext);
				}
				
				return hr;
			}
		}
	}

	PatchAdapter(&pAdapter);

	if (pAdapter)
	{
		// if we ended up setting adapter, driver type should be unknown, or queries will fail
		DriverType = D3D_DRIVER_TYPE_UNKNOWN;
	}

	auto hr = g_origD3D11CreateDevice(pAdapter, DriverType, Software, Flags | D3D11_CREATE_DEVICE_BGRA_SUPPORT, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);

	PatchCreateResults(ppDevice, ppImmediateContext, g_libgl->IsInSet(_ReturnAddress()));

	return hr;
}

static HRESULT (*g_origD3D11CreateDeviceMain)(_In_opt_ IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, _COM_Outptr_opt_ ID3D11Device** ppDevice, _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel, _COM_Outptr_opt_ ID3D11DeviceContext** ppImmediateContext);

static HRESULT D3D11CreateDeviceHookMain(_In_opt_ IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, _COM_Outptr_opt_ ID3D11Device** ppDevice, _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel, _COM_Outptr_opt_ ID3D11DeviceContext** ppImmediateContext)
{
	auto hr = g_origD3D11CreateDeviceMain(pAdapter, DriverType, Software, Flags | D3D11_CREATE_DEVICE_BGRA_SUPPORT, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);

	// hook e.g. GetImmediateContext here if needed

	// if we didn't get g_origDevice set from the system DLL we will set it here anyway
	// this for example happens with 'ReShade' which completely hides the system DLL from us like a thieving retard employed by NVIDIA of course would do
	if (!g_origDevice && ppDevice && *ppDevice && ppImmediateContext)
	{
		g_origDevice = *ppDevice;
	}

	return hr;
}

#include <psapi.h>

void HookLibGL(HMODULE libGL)
{
#if !GTA_NY
	wchar_t systemDir[MAX_PATH];
	GetSystemDirectoryW(systemDir, std::size(systemDir));

	HMODULE realSysDll = NULL;
	auto sysDllName = va(L"%s\\d3d11.dll", systemDir);
	auto sysDll = LoadLibraryW(sysDllName);
	auto mainDll = LoadLibraryW(L"d3d11.dll");

    HMODULE hMods[1024];
	DWORD cbNeeded;

	if (EnumProcessModules(GetCurrentProcess(), hMods, sizeof(hMods), &cbNeeded))
	{
		for (size_t i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			wchar_t szModName[MAX_PATH];

			if (GetModuleFileNameExW(GetCurrentProcess(), hMods[i], szModName,
				sizeof(szModName) / sizeof(wchar_t)))
			{
				if (_wcsicmp(szModName, sysDllName) == 0)
				{
					realSysDll = hMods[i];
					break;
				}
			}
		}
	}

	g_sysD3D11 = realSysDll;
	g_d3d11 = std::unique_ptr<ModuleData>(new ModuleData{ realSysDll });

	// maybe
	g_reshit = true;

	MH_Initialize();
	MH_CreateHook(GetProcAddress(libGL, "glTexParameterf"), glTexParameterfHook, (void**)&g_origglTexParameterf);
	MH_CreateHook(GetProcAddress(libGL, "glBindTexture"), glBindTextureHook, (void**)&g_origglBindTexture);
	MH_CreateHook(GetProcAddress(libGL, "glDeleteTextures"), glDeleteTexturesHook, (void**)&g_origglDeleteTextures);
	if (sysDll != realSysDll)
	{
		trace("You're using a broken 'graphics mod' that tries to hide D3D11.dll from us. Why?\n");
		MH_CreateHook(GetProcAddress(realSysDll, "D3D11CreateDevice"), D3D11CreateDeviceHook, (void**)&g_origD3D11CreateDevice);
	}
	else
	{
		MH_CreateHook(GetProcAddress(sysDll, "D3D11CreateDevice"), D3D11CreateDeviceHook, (void**)&g_origD3D11CreateDevice);
	}

	// as reshade suuuucks we have to duplicate logic as they pass D3D11CreateDevice on there
	MH_CreateHook(GetProcAddress(sysDll, "D3D11CreateDeviceAndSwapChain"), D3D11CreateDeviceAndSwapChainHook, (void**)&g_origD3D11CreateDeviceAndSwapChain);

	// hook any wrapper dll too
	if (mainDll != sysDll || mainDll != realSysDll)
	{
		MH_CreateHook(GetProcAddress(mainDll, "D3D11CreateDevice"), D3D11CreateDeviceHookMain, (void**)&g_origD3D11CreateDeviceMain);
	}

	MH_EnableHook(MH_ALL_HOOKS);
#endif
}

extern bool g_inited;

void Component_RunPreInit()
{
	// Don't run CEF init if we want node
	if (wcsstr(GetCommandLine(), L"--start-node"))
	{
		return;
	}

#ifdef _M_AMD64
	// again, a Win7 SP1 check (Chromium x64 isn't supported below this operating level)
	if (!IsWindows7SP1OrGreater())
	{
		FatalError("CitizenFX requires Windows 7 SP1 or higher. Please upgrade to this operating system version to run CitizenFX.");
	}
#endif

	// CEF keeps loading/unloading this - load it ourselves to make the refcount always 1
	LoadLibrary(L"bluetoothapis.dll");

	// load Chrome dependencies ourselves so that the system won't try loading from other paths
	LoadLibrary(MakeRelativeCitPath(L"bin/chrome_elf.dll").c_str());

	// we don't need to load ANGLE if this is utility or renderer
	if (wcsstr(GetCommandLineW(), L"type=utility") == nullptr && wcsstr(GetCommandLineW(), L"type=renderer") == nullptr)
	{
		LoadLibrary(MakeRelativeCitPath(L"bin/libEGL.dll").c_str());
		HMODULE libGL = LoadLibrary(MakeRelativeCitPath(L"bin/libGLESv2.dll").c_str());

		// hook libGLESv2 for Cfx purposes
		HookLibGL(libGL);
	}

	// load the CEF library
	HMODULE libcef = LoadLibraryW(MakeRelativeCitPath(L"bin/libcef.dll").c_str());
	g_libgl = std::unique_ptr<ModuleData>(new ModuleData{ L"libGLESv2.dll", L"libEGL.dll", L"libcef.dll" });

	if (!libcef)
	{
		auto gle = GetLastError();

		_wunlink(MakeRelativeCitPath(L"content_index.xml").c_str());
		FatalError("Could not load bin/libcef.dll.\nLoadLibraryW failed for reason %d.", gle);
	}

	if (wcsstr(GetCommandLineW(), L"type=gpu"))
	{
		auto mojo__Connector__RaiseError = (uint8_t*)((char*)libcef + 0x2d12f4a);

		if (*mojo__Connector__RaiseError == 0xB2)
		{
			hook::putVP<uint8_t>(mojo__Connector__RaiseError, 0xCC);
		}
	}

	{
		// echo network::SetFetchMetadataHeaders | pdbdump -r libcef.dll.pdb:0x180000000
		auto network__SetFetchMetadataHeaders = (uint8_t*)((char*)libcef + 0x3d36e0c);

		if (*network__SetFetchMetadataHeaders == 0x41)
		{
			hook::putVP<uint8_t>(network__SetFetchMetadataHeaders, 0xC3);

			// GL robust error handling
			hook::putVP<uint32_t>((char*)libcef + 0x26C818A, 0x90C3C033);
		}
	}

	__HrLoadAllImportsForDll("libcef.dll");

	// verify if the CEF API hash is screwed
	{
		const char* apiHash = cef_api_hash(0);
		const char* apiHashUniversal = cef_api_hash(1);
		if (strcmp(apiHash, CEF_API_HASH_PLATFORM) != 0 || strcmp(apiHashUniversal, CEF_API_HASH_UNIVERSAL) != 0)
		{
			_wunlink(MakeRelativeCitPath(L"content_index.xml").c_str());
			FatalError("CEF API hash mismatch\nA mismatch was detected between `nui-core.dll` and `bin/libcef.dll`. Please restart the game and try again.\n\nPlatform hash:\n%s\n%s\n\nUniversal hash:\n%s\n%s",
			apiHash,
			CEF_API_HASH_PLATFORM,
			apiHashUniversal,
			CEF_API_HASH_UNIVERSAL);
		}
	}

	Instance<NUIApp>::Set(new NUIApp());

	// instantiate a NUIApp
	auto selfApp = Instance<NUIApp>::Get();

	CefMainArgs args(GetModuleHandle(NULL));
	static CefRefPtr<CefApp> app(selfApp);

    auto schemeHandlerFactory = new NUISchemeHandlerFactory();
    schemeHandlerFactory->AddRef();
    Instance<NUISchemeHandlerFactory>::Set(schemeHandlerFactory);

    InitFunctionBase::RunAll();
	g_inited = true;

	// try to execute as a CEF process
	int exitCode = CefExecuteProcess(args, app, nullptr);

	// and exit if we did
	if (exitCode >= 0)
	{
		TerminateProcess(GetCurrentProcess(), exitCode);
	}
}

namespace nui
{
std::string GetContext();
}

void CreateRootWindow()
{
	int resX, resY;
	g_nuiGi->GetGameResolution(&resX, &resY);

	static ConVar<std::string> rootUrl("ui_rootUrl", ConVar_None, "nui://game/ui/root.html");

	auto rootWindow = NUIWindow::Create(true, resX, resY, rootUrl.GetValue(), true, nui::GetContext());
	rootWindow->SetPaintType(NUIPaintTypePostRender);
	rootWindow->SetName("root");

	Instance<NUIWindowManager>::Get()->SetRootWindow(rootWindow);
}

bool g_shouldCreateRootWindow;

namespace nui
{
fwEvent<> OnInitialize;

static std::mutex g_rcHandlerMutex;
static std::vector<std::tuple<CefString, CefString, CefRefPtr<CefSchemeHandlerFactory>>> g_schemeHandlers;
static std::set<CefRefPtr<CefRequestContext>> g_requestContexts;

void AddSchemeHandlerFactories(CefRefPtr<CefRequestContext> rc)
{
	std::unique_lock _(g_rcHandlerMutex);

	for (const auto& [name, domain, factory] : g_schemeHandlers)
	{
		rc->RegisterSchemeHandlerFactory(name, domain, factory);
	}

	g_requestContexts.insert(rc);
}

void RegisterSchemeHandlerFactory(const CefString& scheme_name, const CefString& domain_name, CefRefPtr<CefSchemeHandlerFactory> factory)
{
	std::unique_lock _(g_rcHandlerMutex);

	CefRegisterSchemeHandlerFactory(scheme_name, domain_name, factory);

	for (auto& rc : g_requestContexts)
	{
		rc->RegisterSchemeHandlerFactory(scheme_name, domain_name, factory);
	}

	g_schemeHandlers.push_back({ scheme_name, domain_name, factory });
}

extern std::unordered_map<std::string, fwRefContainer<NUIWindow>> windowList;
extern std::shared_mutex windowListMutex;
static std::string g_currentContext;
bool g_rendererInit;

std::string GetContext()
{
	std::shared_lock _(windowListMutex);
	return g_currentContext;
}

void SwitchContext(const std::string& contextId)
{
	bool switched = false;

	{
		std::unique_lock _(windowListMutex);

		if (contextId != g_currentContext)
		{
			g_currentContext = contextId;
			switched = true;
		}
	}

	if (switched)
	{
		{
			auto rw = Instance<NUIWindowManager>::Get()->GetRootWindow().GetRef();

			if (rw)
			{
				Instance<NUIWindowManager>::Get()->RemoveWindow(rw);
				Instance<NUIWindowManager>::Get()->SetRootWindow({});
			}
		}

		if (!contextId.empty())
		{
			CreateRootWindow();
		}
	}
}

void Initialize(nui::GameInterface* gi)
{
	g_nuiGi = gi;

    if (getenv("CitizenFX_ToolMode"))
    {
        return;
    }

	static ConVar<std::string> uiUrlVar("ui_url", ConVar_None, "https://nui-game-internal/ui/app/index.html");

	auto deferredInitializer = DeferredInitializer::Create([]()
	{
		std::wstring cachePath = GetNUIStoragePath();
		CreateDirectory(cachePath.c_str(), nullptr);

		// delete any old CEF logs
		DeleteFile(MakeRelativeCitPath(L"cef.log").c_str());
		DeleteFile(MakeRelativeCitPath(L"cef_console.txt").c_str());

		auto selfApp = Instance<NUIApp>::Get();

		CefMainArgs args(GetModuleHandle(NULL));
		CefRefPtr<CefApp> app(selfApp);

		CefSettings cSettings;

		cSettings.multi_threaded_message_loop = true;
		cSettings.remote_debugging_port = 13172;
		cSettings.windowless_rendering_enabled = true;
		cSettings.log_severity = LOGSEVERITY_DEFAULT;
		cSettings.background_color = 0;

		// read the platform version
		FILE* f = _wfopen(MakeRelativeCitPath(L"citizen/release.txt").c_str(), L"r");
		int version = 0;

		if (f)
		{
			char ver[128];

			fgets(ver, sizeof(ver), f);
			fclose(f);

			version = atoi(ver);
		}

		// #TODONY: why is this missing from official CEF?
#ifndef GTA_NY
		CefString(&cSettings.user_agent_product).FromWString(fmt::sprintf(L"Chrome/%d.%d.%d.%d CitizenFX/1.0.0.%d", cef_version_info(4), cef_version_info(5), cef_version_info(6), cef_version_info(7), version));
#endif

		CefString(&cSettings.log_file).FromWString(MakeRelativeCitPath(L"cef_console.txt"));

		CefString(&cSettings.browser_subprocess_path).FromWString(MakeCfxSubProcess(L"ChromeBrowser", L"chrome"));

		CefString(&cSettings.locale).FromASCII("en-US");

		CefString(&cSettings.cookieable_schemes_list).FromString("nui");

		std::wstring resPath = MakeRelativeCitPath(L"bin/cef/");

		CefString(&cSettings.resources_dir_path).FromWString(resPath);
		CefString(&cSettings.locales_dir_path).FromWString(resPath);
		CefString(&cSettings.cache_path).FromWString(cachePath);

		// 2014-06-30: sandbox disabled as it breaks scheme handler factories (results in blank page being loaded)
		CefInitialize(args, cSettings, app.get(), /*cefSandbox*/ nullptr);
		nui::RegisterSchemeHandlerFactory("nui", "", Instance<NUISchemeHandlerFactory>::Get());
		CefAddCrossOriginWhitelistEntry("nui://game", "https", "", true);
		CefAddCrossOriginWhitelistEntry("nui://game", "http", "", true);
		CefAddCrossOriginWhitelistEntry("nui://game", "nui", "", true);

		nui::RegisterSchemeHandlerFactory("https", "nui-game-internal", Instance<NUISchemeHandlerFactory>::Get());
		CefAddCrossOriginWhitelistEntry("https://nui-game-internal", "https", "", true);
		CefAddCrossOriginWhitelistEntry("https://nui-game-internal", "http", "", true);
		CefAddCrossOriginWhitelistEntry("https://nui-game-internal", "nui", "", true);

		nui::RegisterSchemeHandlerFactory("ws", "", Instance<NUISchemeHandlerFactory>::Get());
		nui::RegisterSchemeHandlerFactory("wss", "", Instance<NUISchemeHandlerFactory>::Get());

		HookFunctionBase::RunAll();

		{
			auto zips = { "citizen:/ui.zip", "citizen:/ui-big.zip" };

			for (auto zip : zips)
			{
				static std::map<std::string, std::vector<uint8_t>> storedFiles;

				const void* thisData = nullptr;
				size_t thisDataSize = 0;

				{
					auto stream = vfs::OpenRead(zip);

					if (stream.GetRef())
					{
						storedFiles[zip] = stream->ReadToEnd();

						thisData = storedFiles[zip].data();
						thisDataSize = storedFiles[zip].size();
					}
				}

				if (thisData)
				{
					fwRefContainer<vfs::ZipFile> file = new vfs::ZipFile();

					if (file->OpenArchive(fmt::sprintf("memory:$%016llx,%d,0:%s", (uintptr_t)thisData, thisDataSize, "ui")))
					{
						vfs::Mount(file, "citizen:/ui/");
					}
				}
			}
		}

		if (nui::HasMainUI())
		{
			nui::CreateFrame("mpMenu", uiUrlVar.GetValue());
		}

		nui::OnInitialize();
	});
	
	static ConsoleCommand devtoolsCmd("nui_devtools", []()
	{
		auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();

		if (rootWindow.GetRef())
		{
			auto browser = rootWindow->GetBrowser();

			if (browser)
			{
				CefWindowInfo wi;
				wi.SetAsPopup(NULL, "NUI DevTools");

				CefBrowserSettings s;

				browser->GetHost()->ShowDevTools(wi, new NUIClient(nullptr), s, {});
			}
		}
	});

	static ConsoleCommand devtoolsWindowCmd("nui_devtools", [](const std::string& windowName)
	{
		auto browser = nui::GetNUIWindowBrowser(windowName);

		if (!browser)
		{
			browser = nui::GetNUIWindowBrowser(fmt::sprintf("nui_%s", windowName));
		}

		if (browser)
		{
			CefWindowInfo wi;
			wi.SetAsPopup(NULL, fmt::sprintf("NUI DevTools - %s", windowName));

			CefBrowserSettings s;

			browser->GetHost()->ShowDevTools(wi, new NUIClient(nullptr), s, {});
		}
		else
		{
			trace("No such NUI window: %s\n", windowName);
		}
	});

	g_nuiGi->OnInitRenderer.Connect([deferredInitializer]()
	{
		deferredInitializer->Wait();

		g_rendererInit = true;
		Instance<NUIWindowManager>::Get()->ForAllWindows([](auto window)
		{
			window->InitializeRenderBacking();
		});
	});

	g_nuiGi->OnRender.Connect([deferredInitializer]()
	{
		deferredInitializer->Wait();

		if (g_shouldCreateRootWindow)
		{
			if (!nui::HasMainUI())
			{
				{
					auto rw = Instance<NUIWindowManager>::Get()->GetRootWindow().GetRef();

					if (rw)
					{
						Instance<NUIWindowManager>::Get()->RemoveWindow(rw);
						Instance<NUIWindowManager>::Get()->SetRootWindow({});
					}
				}

				CreateRootWindow();
			}
			else
			{
				nui::DestroyFrame("mpMenu");

				static ConVar<std::string> uiUrlVar("ui_url", ConVar_None, "https://nui-game-internal/ui/app/index.html");
				nui::CreateFrame("mpMenu", uiUrlVar.GetValue());
			}

			g_shouldCreateRootWindow = false;
		}
	});
}
}

std::wstring GetNUIStoragePath()
{
	std::wstring lmSuffix;

	if (launch::IsSDKGuest())
	{
		lmSuffix = L"-guest";
	}

	return MakeRelativeCitPath(fmt::sprintf(L"data\\nui-storage%s%s", ToWide(launch::GetPrefixedLaunchModeKey("-")), lmSuffix));
}
