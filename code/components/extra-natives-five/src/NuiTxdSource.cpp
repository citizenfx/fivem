#include <StdInc.h>

#include <DrawCommands.h>

#define WANT_CEF_INTERNALS
#include <CefOverlay.h>
#include <NUISchemeHandlerFactory.h>

#include <include/cef_parser.h>

#include <Streaming.h>

#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE

#define RAGE_FORMATS_IN_GAME
#include <gtaDrawable.h>

#include <wrl.h>
#include <tbb/concurrent_queue.h>

#include <DirectXTex/DirectXTex.h>

#include <CrossBuildRuntime.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>

namespace WRL = Microsoft::WRL;

class NuiTxdResourceHandler;

struct TxdRequest
{
	CefRefPtr<NuiTxdResourceHandler> self;
	CefRefPtr<CefCallback> callback;

	uint32_t txdId;
	rage::grcTexture* texture;
};

static tbb::concurrent_queue<TxdRequest> g_txdRequests;

class NuiTxdResourceHandler : public CefResourceHandler
{
public:
	virtual bool ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) override
	{
		CefURLParts parts;
		CefParseURL(request->GetURL(), parts);

		std::string path = CefString(&parts.path).ToString().substr(1);
		std::string txd = path.substr(0, path.find('/'));
		std::string txn = path.substr(path.find('/') + 1);

		auto str = streaming::Manager::GetInstance();

		if (!str)
		{
			callback->Continue();
			return true;
		}

		auto txdStore = str->moduleMgr.GetStreamingModule("ytd");

		uint32_t id;
		txdStore->FindSlot(&id, txd.c_str());

		if (id == -1)
		{
			callback->Continue();
			return true;
		}

		auto txdRef = (rage::five::pgDictionary<rage::grcTexture>*)txdStore->GetPtr(id);

		if (!txdRef)
		{
			callback->Continue();
			return true;
		}

		auto tex = txdRef->Get(txn.c_str());

		if (!tex)
		{
			callback->Continue();
			return true;
		}

		// keep a reference alive for now
		txdStore->AddRef(id);

		TxdRequest req;
		req.callback = callback;
		req.self = this;
		req.texture = tex;
		req.txdId = id;

		g_txdRequests.push(req);

		return true;
	}

	virtual void GetResponseHeaders(CefRefPtr<CefResponse> response, int64& response_length, CefString& redirectUrl) override
	{
		response->SetMimeType("image/png");

		if (!m_found)
		{
			response->SetStatus(404);
		}
		else
		{
			response->SetStatus(200);
		}

		CefResponse::HeaderMap map;
		response->GetHeaderMap(map);

		map.insert({ "cache-control", "max-age=7200" });
		map.insert({ "access-control-allow-origin", "*" });
		map.insert({ "access-control-allow-methods", "POST, GET, OPTIONS" });

		response->SetHeaderMap(map);

		if (m_found)
		{
			response_length = m_result.GetBufferSize();
		}
		else
		{
			response_length = 0;
		}

		m_cursor = 0;
	}

	virtual void Cancel() override
	{
	}

	virtual bool ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback) override
	{
		if (m_found)
		{
			int toRead = std::min((unsigned int)m_result.GetBufferSize() - m_cursor, (unsigned int)bytes_to_read);
			memcpy(data_out, (char*)m_result.GetBufferPointer() + m_cursor, toRead);

			m_cursor += toRead;
			bytes_read = toRead;

			return (bytes_read > 0);
		}

		return false;
	}

	void SubmitCompletion(DirectX::Blob&& bitmap)
	{
		m_result = std::move(bitmap);
		m_found = true;
	}

private:
	bool m_found = false;
	DirectX::Blob m_result;
	uint32_t m_cursor = 0;

	IMPLEMENT_REFCOUNTING(NuiTxdResourceHandler);
};

static void ProcessNuiTxdQueue()
{
	TxdRequest req;

	while (g_txdRequests.try_pop(req))
	{
		// for now, we use D3D11-specific behavior to correctly convert
		auto d3d11Res = req.texture->texture;
		WRL::ComPtr<ID3D11Texture2D> d3d11Tex;

		bool hadResult = false;
		
		if (SUCCEEDED(d3d11Res->QueryInterface(d3d11Tex.GetAddressOf())))
		{
			auto dev = GetD3D11Device();
			auto dc = GetD3D11DeviceContext();

			D3D11_TEXTURE2D_DESC texDesc;
			d3d11Tex->GetDesc(&texDesc);
			
			D3D11_MAPPED_SUBRESOURCE mapRes;

			texDesc.Usage = D3D11_USAGE_STAGING;
			texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			texDesc.BindFlags = 0;

			WRL::ComPtr<ID3D11Texture2D> tempTex;
			if (SUCCEEDED(dev->CreateTexture2D(&texDesc, NULL, tempTex.GetAddressOf())))
			{
				dc->CopyResource(tempTex.Get(), d3d11Tex.Get());

				if (SUCCEEDED(dc->Map(tempTex.Get(), 0, D3D11_MAP_READ, 0, &mapRes)))
				{
					DirectX::Image image;
					image.format = texDesc.Format;
					image.width = texDesc.Width;
					image.height = texDesc.Height;
					image.rowPitch = mapRes.RowPitch;
					image.slicePitch = mapRes.DepthPitch;
					image.pixels = (uint8_t*)mapRes.pData;

					DirectX::ScratchImage tgtImage;

					// first uncompress
					HRESULT hr = S_OK;

					if (DirectX::IsCompressed(texDesc.Format))
					{
						hr = DirectX::Decompress(image, DXGI_FORMAT_UNKNOWN, tgtImage);
					}
					else
					{
						hr = tgtImage.InitializeFromImage(image);
					}

					dc->Unmap(tempTex.Get(), 0);

					// then convert to RGBA8
					//DirectX::ScratchImage rawImage;
					//hr = SUCCEEDED(hr) ? DirectX::Convert(tgtImage.GetImages(), tgtImage.GetImageCount(), tgtImage.GetMetadata(), DXGI_FORMAT_R8G8B8A8_UNORM, DirectX::TEX_FILTER_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT, rawImage) : hr;

					struct WorkItemRequest
					{
						CefRefPtr<NuiTxdResourceHandler> txdHandler;
						CefRefPtr<CefCallback> callback;
						DirectX::ScratchImage tgtImage;

						WorkItemRequest(const CefRefPtr<NuiTxdResourceHandler>& handler, const CefRefPtr<CefCallback>& callback, DirectX::ScratchImage&& image)
							: txdHandler(handler), callback(callback), tgtImage(std::move(image))
						{
						
						}
					};

					if (SUCCEEDED(hr))
					{
						auto workItemRequest = new WorkItemRequest(req.self, req.callback, std::move(tgtImage));

						// and finally, wrap it in a PNG
						QueueUserWorkItem([](LPVOID arg) -> DWORD
						{
							auto workItemRequest = (WorkItemRequest*)arg;

							DirectX::Blob blob;
							HRESULT hr = DirectX::SaveToWICMemory(*workItemRequest->tgtImage.GetImage(0, 0, 0), DirectX::WIC_FLAGS_FORCE_SRGB, DirectX::GetWICCodec(DirectX::WIC_CODEC_PNG), blob);

							if (SUCCEEDED(hr))
							{
								workItemRequest->txdHandler->SubmitCompletion(std::move(blob));
							}

							workItemRequest->callback->Continue();

							delete workItemRequest;
							return 0;
						},
						workItemRequest, 0);

						hadResult = true;
					}
				}
			}
		}

		auto str = streaming::Manager::GetInstance();
		auto txdStore = str->moduleMgr.GetStreamingModule("ytd");
		txdStore->RemoveRef(req.txdId);

		if (!hadResult)
		{
			req.callback->Continue();
		}
	}
}

static InitFunction initFunction([]()
{
	OnSchemeCreateRequest.Connect([](const char* name, CefRefPtr<CefRequest> request, CefRefPtr<CefResourceHandler>& handler)
	{
		if (!strcmp(name, "http") || !strcmp(name, "https"))
		{
			// parse the URL to get the hostname
			CefString url = request->GetURL();
			CefURLParts urlParts;

			if (CefParseURL(url, urlParts))
			{
				CefString hostString = &urlParts.host;

				if (hostString == "nui-img")
				{
					handler = new NuiTxdResourceHandler();

					return false;
				}
			}
		}

		return true;
	});

	OnPostFrontendRender.Connect([]()
	{
		ProcessNuiTxdQueue();
	}, -500);
});

__int64(__fastcall* g_origAlterPedHeadshotTransparentPSConstants_Hook)(__m128*, __m128*);

// Since b2189's shader changes `RegisterPedheadshotTransparent`'s texture colors are in the [0 .. 0.6] range, let's inverse it
// Sets	PS constant buffer 3rd and 4th? (unchecked) float4/vec4/__m128 values, where the 3rd is referred to as `GeneralParams0`
__int64 __fastcall AlterPedHeadshotTransparentPSConstants_Hook(__m128* a3, __m128* a4)
{
	*a3 = _mm_set1_ps(1.f / 0.6f); // *a3 is stack allocated
	return g_origAlterPedHeadshotTransparentPSConstants_Hook(a3, a4);
}

static HookFunction hookFunction([]()
{
	OnGrcCreateDevice.Connect([]()
	{
		nui::RegisterSchemeHandlerFactory("http", "nui-img", Instance<NUISchemeHandlerFactory>::Get());
		nui::RegisterSchemeHandlerFactory("https", "nui-img", Instance<NUISchemeHandlerFactory>::Get());
	});

	if (xbr::IsGameBuildOrGreater<2189>())
	{
		g_origAlterPedHeadshotTransparentPSConstants_Hook = hook::trampoline(
			hook::get_pattern("48 8D 4D F7 0F 29 4D F7 0F 29 45 E7", 0xC),
			AlterPedHeadshotTransparentPSConstants_Hook);
	}
});
