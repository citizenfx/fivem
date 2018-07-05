#include "StdInc.h"

#include <Streaming.h>

#include <scrBind.h>

#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE

#define RAGE_FORMATS_IN_GAME
#include <gtaDrawable.h>

#include <Resource.h>
#include <fxScripting.h>
#include <VFSManager.h>

#include <grcTexture.h>

#include <wrl.h>
#include <wincodec.h>

#include <CefOverlay.h>

using Microsoft::WRL::ComPtr;

class RuntimeTex
{
public:
	RuntimeTex(const char* name, int width, int height);

	RuntimeTex(rage::grcTexture* texture, const void* data, size_t size);

	RuntimeTex(rage::grcTexture* texture);

	virtual ~RuntimeTex();

	int GetWidth();

	int GetHeight();

	int GetPitch();

	void SetPixel(int x, int y, int r, int g, int b, int a);

	bool SetPixelData(const void* data, size_t length);

	void Commit();

	inline rage::grcTexture* GetTexture()
	{
		return m_texture;
	}

private:
	rage::grcTexture* m_texture;

	int m_pitch;

	std::vector<uint8_t> m_backingPixels;
};

class RuntimeTxd
{
public:
	RuntimeTxd(const char* name);

	RuntimeTex* CreateTexture(const char* name, int width, int height);

	RuntimeTex* CreateTextureFromImage(const char* name, const char* fileName);

	RuntimeTex* CreateTextureFromDui(const char* name, const char* duiHandle);

private:
	uint32_t m_txdIndex;
	std::string m_name;

	std::unordered_map<std::string, std::shared_ptr<RuntimeTex>> m_textures;

	rage::five::pgDictionary<rage::grcTexture>* m_txd;
};

RuntimeTex::RuntimeTex(const char* name, int width, int height)
{
	rage::grcManualTextureDef textureDef;
	memset(&textureDef, 0, sizeof(textureDef));
	textureDef.isStaging = 0;
	textureDef.arraySize = 1;

	m_texture = rage::grcTextureFactory::getInstance()->createManualTexture(width, height, 2, nullptr, true, &textureDef);

	rage::grcLockedTexture lockedTexture;

	if (m_texture->Map(1, 0, &lockedTexture, rage::grcLockFlags::Write))
	{
		memset(lockedTexture.pBits, 0, lockedTexture.pitch * lockedTexture.height);
		m_backingPixels.resize(lockedTexture.pitch * lockedTexture.height);

		m_pitch = lockedTexture.pitch;

		m_texture->Unmap(&lockedTexture);
	}
}

RuntimeTex::RuntimeTex(rage::grcTexture* texture, const void* data, size_t size)
	: m_texture(texture)
{
	m_backingPixels.resize(size);
	memcpy(&m_backingPixels[0], data, m_backingPixels.size());
}

RuntimeTex::RuntimeTex(rage::grcTexture* texture)
	: m_texture(texture)
{
	m_backingPixels.resize(0);
}

RuntimeTex::~RuntimeTex()
{
	delete m_texture;
}

int RuntimeTex::GetWidth()
{
	return m_texture->GetWidth();
}

int RuntimeTex::GetHeight()
{
	return m_texture->GetHeight();
}

int RuntimeTex::GetPitch()
{
	return m_pitch;
}

void RuntimeTex::SetPixel(int x, int y, int r, int g, int b, int a)
{
	auto offset = (y * m_pitch) + (x * 4);

	if (offset < 0 || offset >= m_backingPixels.size() - 4)
	{
		return;
	}

	auto start = &m_backingPixels[offset];

	start[3] = b;
	start[2] = g;
	start[1] = r;
	start[0] = a;
}

bool RuntimeTex::SetPixelData(const void* data, size_t length)
{
	if (length != m_backingPixels.size())
	{
		return false;
	}

	rage::grcLockedTexture lockedTexture;

	if (m_texture->Map(1, 0, &lockedTexture, rage::grcLockFlags::Write))
	{
		memcpy(lockedTexture.pBits, data, length);
		memcpy(m_backingPixels.data(), data, length);
		m_texture->Unmap(&lockedTexture);
	}

	return true;
}

void RuntimeTex::Commit()
{
	rage::grcLockedTexture lockedTexture;

	if (m_texture->Map(1, 0, &lockedTexture, rage::grcLockFlags::Write))
	{
		memcpy(lockedTexture.pBits, m_backingPixels.data(), m_backingPixels.size());
		m_texture->Unmap(&lockedTexture);
	}
}

RuntimeTxd::RuntimeTxd(const char* name)
{
	streaming::Manager* streaming = streaming::Manager::GetInstance();
	auto txdStore = streaming->moduleMgr.GetStreamingModule("ytd");

	txdStore->GetOrCreate(&m_txdIndex, name);

	if (m_txdIndex != 0xFFFFFFFF)
	{
		auto& entry = streaming->Entries[txdStore->baseIdx + m_txdIndex];

		if (!entry.handle)
		{
			m_name = name;
			m_txd = new rage::five::pgDictionary<rage::grcTexture>();

			streaming::strAssetReference ref;
			ref.asset = m_txd;

			txdStore->SetAssetReference(m_txdIndex, ref);
			entry.flags = (512 << 8) | 1;
		}
	}
}

RuntimeTex* RuntimeTxd::CreateTexture(const char* name, int width, int height)
{
	if (!m_txd)
	{
		return nullptr;
	}

	if (m_textures.find(name) != m_textures.end())
	{
		return nullptr;
	}

	auto tex = std::make_shared<RuntimeTex>(name, width, height);
	m_txd->Add(name, tex->GetTexture());

	m_textures[name] = tex;

	scrBindAddSafePointer(tex.get());
	return tex.get();
}

RuntimeTex* RuntimeTxd::CreateTextureFromDui(const char* name, const char* duiHandle)
{
	if (!m_txd)
	{
		return nullptr;
	}

	if (m_textures.find(name) != m_textures.end())
	{
		return nullptr;
	}

	auto tex = std::make_shared<RuntimeTex>(nui::GetWindowTexture(duiHandle));
	m_txd->Add(name, tex->GetTexture());

	m_textures[name] = tex;

	scrBindAddSafePointer(tex.get());
	return tex.get();
}

#pragma comment(lib, "windowscodecs.lib")

ComPtr<IWICImagingFactory> g_imagingFactory;

class VfsStream : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, IStream>
{
private:
	fwRefContainer<vfs::Stream> m_stream;

public:
	VfsStream(fwRefContainer<vfs::Stream> stream)
	{
		m_stream = stream;
	}

	// Inherited via RuntimeClass
	virtual HRESULT Read(void * pv, ULONG cb, ULONG * pcbRead) override
	{
		*pcbRead = m_stream->Read(pv, cb);

		return S_OK;
	}
	virtual HRESULT Write(const void * pv, ULONG cb, ULONG * pcbWritten) override
	{
		*pcbWritten = m_stream->Write(pv, cb);
		return S_OK;
	}
	virtual HRESULT Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER * plibNewPosition) override
	{
		auto p = m_stream->Seek(dlibMove.QuadPart, dwOrigin);

		if (plibNewPosition)
		{
			plibNewPosition->QuadPart = p;
		}

		return S_OK;
	}

	virtual HRESULT SetSize(ULARGE_INTEGER libNewSize) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT CopyTo(IStream * pstm, ULARGE_INTEGER cb, ULARGE_INTEGER * pcbRead, ULARGE_INTEGER * pcbWritten) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT Commit(DWORD grfCommitFlags) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT Revert(void) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT Stat(STATSTG * pstatstg, DWORD grfStatFlag) override
	{
		pstatstg->cbSize.QuadPart = m_stream->GetLength();
		pstatstg->type = STGTY_STREAM;
		pstatstg->grfMode = STGM_READ;

		return S_OK;
	}
	virtual HRESULT Clone(IStream ** ppstm) override
	{
		return E_NOTIMPL;
	}
};

static ComPtr<IStream> CreateComStream(fwRefContainer<vfs::Stream> stream)
{
	return Microsoft::WRL::Make<VfsStream>(stream);
}

RuntimeTex* RuntimeTxd::CreateTextureFromImage(const char* name, const char* fileName)
{
	if (!m_txd)
	{
		return nullptr;
	}

	if (!g_imagingFactory)
	{
		HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (void**)g_imagingFactory.GetAddressOf());
	}

	fx::OMPtr<IScriptRuntime> runtime;

	if (!FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
	{
		return nullptr;
	}

	fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

	ComPtr<IWICBitmapDecoder> decoder;

	ComPtr<IStream> stream = CreateComStream(vfs::OpenRead(resource->GetPath() + "/" + fileName));

	HRESULT hr = g_imagingFactory->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf());

	if (SUCCEEDED(hr))
	{
		ComPtr<IWICBitmapFrameDecode> frame;

		hr = decoder->GetFrame(0, frame.GetAddressOf());

		if (SUCCEEDED(hr))
		{
			ComPtr<IWICBitmapSource> source;
			ComPtr<IWICBitmapSource> convertedSource;

			UINT width = 0, height = 0;

			frame->GetSize(&width, &height);

			// try to convert to a pixel format we like
			frame.As(&source);

			hr = WICConvertBitmapSource(GUID_WICPixelFormat32bppBGRA, source.Get(), convertedSource.GetAddressOf());

			if (SUCCEEDED(hr))
			{
				source = convertedSource;
			}

			// create a pixel data buffer
			uint32_t* pixelData = new uint32_t[width * height];

			hr = source->CopyPixels(nullptr, width * 4, width * height * 4, reinterpret_cast<BYTE*>(pixelData));

			if (SUCCEEDED(hr))
			{
				rage::grcTextureReference reference;
				memset(&reference, 0, sizeof(reference));
				reference.width = width;
				reference.height = height;
				reference.depth = 1;
				reference.stride = width * 4;
				reference.format = 11; // should correspond to DXGI_FORMAT_B8G8R8A8_UNORM
				reference.pixelData = (uint8_t*)pixelData;

				auto tex = std::make_shared<RuntimeTex>(rage::grcTextureFactory::getInstance()->createImage(&reference, nullptr), pixelData, width * height * 4);
				m_txd->Add(name, tex->GetTexture());

				m_textures[name] = tex;

				scrBindAddSafePointer(tex.get());
				return tex.get();
			}
		}
	}

	return nullptr;
}

static InitFunction initFunction([]()
{
	scrBindClass<RuntimeTxd>()
		.AddConstructor<void(*)(const char*)>("CREATE_RUNTIME_TXD")
		.AddMethod("CREATE_RUNTIME_TEXTURE", &RuntimeTxd::CreateTexture)
		.AddMethod("CREATE_RUNTIME_TEXTURE_FROM_IMAGE", &RuntimeTxd::CreateTextureFromImage)
		.AddMethod("CREATE_RUNTIME_TEXTURE_FROM_DUI_HANDLE", &RuntimeTxd::CreateTextureFromDui);

	scrBindClass<RuntimeTex>()
		.AddMethod("GET_RUNTIME_TEXTURE_WIDTH", &RuntimeTex::GetWidth)
		.AddMethod("GET_RUNTIME_TEXTURE_HEIGHT", &RuntimeTex::GetHeight)
		.AddMethod("GET_RUNTIME_TEXTURE_PITCH", &RuntimeTex::GetPitch)
		.AddMethod("SET_RUNTIME_TEXTURE_PIXEL", &RuntimeTex::SetPixel)
		.AddMethod("SET_RUNTIME_TEXTURE_ARGB_DATA", &RuntimeTex::SetPixelData)
		.AddMethod("COMMIT_RUNTIME_TEXTURE", &RuntimeTex::Commit);
});
