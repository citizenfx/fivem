#include "StdInc.h"

#include <d3d11_1.h>
#include <wrl.h>

#include <Hooking.h>
#include <DrawCommands.h>
#include <ScriptEngine.h>

#include <nutsnbolts.h>

#include <MinHook.h>

#include <stb_image_write.h>
#include <botan/base64.h>

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <emmintrin.h>
#include <tmmintrin.h>

namespace WRL = Microsoft::WRL;

static constexpr int kMaxHandles = 32;
static constexpr int kStagingCount = 4;
static constexpr int kResolveCount = 4;
static constexpr int kMaxInFlight = 4;
static constexpr int kQueueDepth = 4;

static constexpr int kJpegMin = 20;
static constexpr int kJpegMax = 100;

static int ClampJpegQuality(int quality)
{
	if (quality < kJpegMin)
	{
		return kJpegMin;
	}
	if (quality > kJpegMax)
	{
		return kJpegMax;
	}
	return quality;
}

static constexpr int kLatencyFrames = 2;

enum class ScreenshotFormat : uint8_t
{
	Jpeg = 0,
	Png = 1
};

enum class SlotState : uint8_t
{
	Free,
	Pending,
	Capturing,
	Encoding,
	Ready,
	Failed
};

struct ScreenshotSlot
{
	SlotState state = SlotState::Free;
	std::string base64Encoded;
	int readyFrame = 0;
};

static ScreenshotSlot g_slots[kMaxHandles];
static std::mutex g_slotMutex;
static std::atomic<int> g_currentFrame{ 0 };
static std::atomic<int> g_inFlightCount{ 0 };

struct PendingHandleRequest
{
	int slotIndex;
	ScreenshotFormat format;
	int jpegQuality;
};

struct CaptureQueue
{
	std::mutex mutex;
	std::queue<PendingHandleRequest> requests;
	std::atomic<int> count{ 0 };
};

static CaptureQueue g_beforeHud;
static CaptureQueue g_afterHud;

struct StagingEntry
{
	WRL::ComPtr<ID3D11Texture2D> tex;
	uint32_t width = 0;
	uint32_t height = 0;
	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	int issuedFrame = 0;
	int slotIndex = -1;
	bool hudVariant = false;
	bool inFlight = false;
	ScreenshotFormat encodeFormat = ScreenshotFormat::Jpeg;
	int jpegQuality = kJpegMin;
	// mapped lifetime: Map() here, Unmap() when worker signals done via workerDone
	void* mappedPtr = nullptr;
	uint32_t mappedRowPitch = 0;
	bool mapped = false;
	std::atomic<bool> workerDone{ false };

	void ReleaseMapping(ID3D11DeviceContext* dc)
	{
		dc->Unmap(tex.Get(), 0);
		mapped = false;
		mappedPtr = nullptr;
		mappedRowPitch = 0;
		inFlight = false;
		slotIndex = -1;
	}
};

static StagingEntry g_stagingPool[kStagingCount];

// resolve targets for MSAA backbuffers
struct ResolvePoolEntry
{
	WRL::ComPtr<ID3D11Texture2D> tex;
	uint32_t width = 0;
	uint32_t height = 0;
	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	uint32_t sampleCount = 0;
};

static ResolvePoolEntry g_resolvePool[kResolveCount];

struct EncodeJob
{
	const uint8_t* source = nullptr;
	int stagingEntryIdx = -1;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t rowPitch = 0;
	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	int slotIndex = -1;
	bool hudVariant = false;
	bool sentinel = false;
	ScreenshotFormat encodeFormat = ScreenshotFormat::Jpeg;
	int jpegQuality = kJpegMin;
};

static std::mutex g_encodeQueueMutex;
static std::condition_variable g_encodeQueueCv;
static std::queue<EncodeJob> g_encodeQueue;
static std::atomic<bool> g_workerShouldExit{ false };
static std::thread g_workerThread;
static std::once_flag g_workerOnce;

static bool IsValidHandle(int handle)
{
	return handle >= 0 && handle < kMaxHandles;
}

static int AllocateSlot()
{
	for (int i = 0; i < kMaxHandles; i++)
	{
		if (g_slots[i].state == SlotState::Free)
		{
			g_slots[i].state = SlotState::Pending;
			g_slots[i].base64Encoded.clear();
			g_slots[i].readyFrame = g_currentFrame.load(std::memory_order_relaxed);
			return i;
		}
	}

	return -1;
}

static void MarkSlotFailed(int slotIndex)
{
	std::lock_guard lock(g_slotMutex);
	if (g_slots[slotIndex].state != SlotState::Free)
	{
		g_slots[slotIndex].base64Encoded.clear();
		g_slots[slotIndex].state = SlotState::Failed;
		g_slots[slotIndex].readyFrame = g_currentFrame.load(std::memory_order_relaxed);
	}
}

static void TransitionSlotState(int slotIndex, SlotState expected, SlotState next)
{
	std::lock_guard lock(g_slotMutex);
	if (g_slots[slotIndex].state == expected)
	{
		g_slots[slotIndex].state = next;
	}
}

static void StbiWriteAppendCallback(void* context, void* data, int size)
{
	auto* out = static_cast<std::vector<uint8_t>*>(context);
	auto* bytes = static_cast<const uint8_t*>(data);
	out->insert(out->end(), bytes, bytes + size);
}

static void SwizzleBgraToRgb(const uint8_t* srcBase, uint32_t width, uint32_t height, uint32_t rowPitch, bool swapBr, std::vector<uint8_t>& out)
{
	const size_t dstPitch = static_cast<size_t>(width) * 3;
	out.resize(static_cast<size_t>(height) * dstPitch);

	// BGRA -> RGB: swap B/R, drop A
	const __m128i shufSwap = _mm_setr_epi8(2, 1, 0, 6, 5, 4, 10, 9, 8, 14, 13, 12, -1, -1, -1, -1);
	// RGBA -> RGB: keep order, drop A
	const __m128i shufKeep = _mm_setr_epi8(0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, -1, -1, -1, -1);
	const __m128i mask = swapBr ? shufSwap : shufKeep;

	const uint32_t simdGroups = width / 4;
	const uint32_t tailPixels = width % 4;

	for (uint32_t y = 0; y < height; y++)
	{
		const uint8_t* src = srcBase + static_cast<size_t>(rowPitch) * y;
		uint8_t* dst = out.data() + dstPitch * y;

		for (uint32_t g = 0; g < simdGroups; g++)
		{
			__m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src));
			__m128i shuffled = _mm_shuffle_epi8(v, mask);

			_mm_storel_epi64(reinterpret_cast<__m128i*>(dst), shuffled);
			const __m128i hi = _mm_srli_si128(shuffled, 8);
			*reinterpret_cast<uint32_t*>(dst + 8) = static_cast<uint32_t>(_mm_cvtsi128_si32(hi));

			src += 16;
			dst += 12;
		}

		if (swapBr)
		{
			for (uint32_t x = 0; x < tailPixels; x++)
			{
				dst[0] = src[2];
				dst[1] = src[1];
				dst[2] = src[0];
				dst += 3;
				src += 4;
			}
		}
		else
		{
			for (uint32_t x = 0; x < tailPixels; x++)
			{
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
				dst += 3;
				src += 4;
			}
		}
	}
}

static void SwizzleBgraToRgba(const uint8_t* srcBase, uint32_t width, uint32_t height, uint32_t rowPitch, bool swapBr, std::vector<uint8_t>& out)
{
	const size_t dstPitch = static_cast<size_t>(width) * 4;
	out.resize(static_cast<size_t>(height) * dstPitch);

	// BGRA -> RGBA: swap B/R, keep G and A
	const __m128i mask = _mm_setr_epi8(2, 1, 0, 3, 6, 5, 4, 7, 10, 9, 8, 11, 14, 13, 12, 15);

	const uint32_t simdGroups = width / 4;
	const uint32_t tailPixels = width % 4;

	for (uint32_t y = 0; y < height; y++)
	{
		const uint8_t* src = srcBase + static_cast<size_t>(rowPitch) * y;
		uint8_t* dst = out.data() + dstPitch * y;

		if (swapBr)
		{
			for (uint32_t g = 0; g < simdGroups; g++)
			{
				__m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(src));
				__m128i shuffled = _mm_shuffle_epi8(v, mask);
				_mm_storeu_si128(reinterpret_cast<__m128i*>(dst), shuffled);

				src += 16;
				dst += 16;
			}

			for (uint32_t x = 0; x < tailPixels; x++)
			{
				dst[0] = src[2];
				dst[1] = src[1];
				dst[2] = src[0];
				dst[3] = src[3];
				dst += 4;
				src += 4;
			}
		}
		else
		{
			std::memcpy(dst, src, static_cast<size_t>(width) * 4);
		}
	}
}

template<int channels>
static void ConvertR10G10B10A2(const uint8_t* srcBase, uint32_t width, uint32_t height, uint32_t rowPitch, std::vector<uint8_t>& out)
{
	static_assert(channels == 3 || channels == 4, "channels must be 3 or 4");

	const size_t dstPitch = static_cast<size_t>(width) * channels;
	out.resize(static_cast<size_t>(height) * dstPitch);

	for (uint32_t y = 0; y < height; y++)
	{
		const uint32_t* src = reinterpret_cast<const uint32_t*>(srcBase + static_cast<size_t>(rowPitch) * y);
		uint8_t* dst = out.data() + dstPitch * y;

		for (uint32_t x = 0; x < width; x++)
		{
			const uint32_t v = src[x];
			const uint32_t r10 = v & 0x3FF;
			const uint32_t g10 = (v >> 10) & 0x3FF;
			const uint32_t b10 = (v >> 20) & 0x3FF;

			dst[0] = static_cast<uint8_t>((r10 * 255 + 511) / 1023);
			dst[1] = static_cast<uint8_t>((g10 * 255 + 511) / 1023);
			dst[2] = static_cast<uint8_t>((b10 * 255 + 511) / 1023);

			if constexpr (channels == 4)
			{
				const uint32_t a2 = (v >> 30) & 0x3;
				dst[3] = static_cast<uint8_t>((a2 * 255) / 3);
			}

			dst += channels;
		}
	}
}

static inline float Float16ToFloat32(uint16_t h)
{
	const uint32_t sign = (h & 0x8000u) << 16;
	const uint32_t exp = (h >> 10) & 0x1Fu;
	const uint32_t mant = h & 0x3FFu;

	uint32_t bits;
	if (exp == 0)
	{
		if (mant == 0)
		{
			// +/- zero
			bits = sign;
		}
		else
		{
			float f = static_cast<float>(mant) * (1.0f / (1 << 24));
			uint32_t fbits;
			std::memcpy(&fbits, &f, sizeof(fbits));
			bits = sign | fbits;
		}
	}
	else if (exp == 0x1F)
	{
		bits = sign | 0x7F800000u | (mant << 13);
	}
	else
	{
		bits = sign | ((exp + (127 - 15)) << 23) | (mant << 13);
	}

	float f;
	std::memcpy(&f, &bits, sizeof(f));
	return f;
}

template<int channels>
static void ConvertR16G16B16A16Float(const uint8_t* srcBase, uint32_t width, uint32_t height, uint32_t rowPitch, std::vector<uint8_t>& out)
{
	static_assert(channels == 3 || channels == 4, "channels must be 3 or 4");

	const size_t dstPitch = static_cast<size_t>(width) * channels;
	out.resize(static_cast<size_t>(height) * dstPitch);

	for (uint32_t y = 0; y < height; y++)
	{
		const uint16_t* src = reinterpret_cast<const uint16_t*>(srcBase + static_cast<size_t>(rowPitch) * y);
		uint8_t* dst = out.data() + dstPitch * y;

		for (uint32_t x = 0; x < width; x++)
		{
			float r = Float16ToFloat32(src[0]);
			float g = Float16ToFloat32(src[1]);
			float b = Float16ToFloat32(src[2]);

			if (!(r == r)) r = 0.0f;
			if (!(g == g)) g = 0.0f;
			if (!(b == b)) b = 0.0f;

			r = r < 0.0f ? 0.0f : (r > 1.0f ? 1.0f : r);
			g = g < 0.0f ? 0.0f : (g > 1.0f ? 1.0f : g);
			b = b < 0.0f ? 0.0f : (b > 1.0f ? 1.0f : b);

			dst[0] = static_cast<uint8_t>(r * 255.0f + 0.5f);
			dst[1] = static_cast<uint8_t>(g * 255.0f + 0.5f);
			dst[2] = static_cast<uint8_t>(b * 255.0f + 0.5f);

			if constexpr (channels == 4)
			{
				float a = Float16ToFloat32(src[3]);
				if (!(a == a)) a = 0.0f;
				a = a < 0.0f ? 0.0f : (a > 1.0f ? 1.0f : a);
				dst[3] = static_cast<uint8_t>(a * 255.0f + 0.5f);
			}

			dst += channels;
			src += 4;
		}
	}
}

static bool IsSupportedBackbufferFormat(DXGI_FORMAT format)
{
	switch (format)
	{
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R10G10B10A2_UNORM:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return true;
		default:
			return false;
	}
}

template<int channels>
static void DecodeBackbuffer(DXGI_FORMAT format, const uint8_t* src, uint32_t width, uint32_t height, uint32_t rowPitch, std::vector<uint8_t>& out)
{
	static_assert(channels == 3 || channels == 4, "channels must be 3 or 4");

	switch (format)
	{
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			if constexpr (channels == 3)
				SwizzleBgraToRgb(src, width, height, rowPitch, /*swapBr*/ true, out);
			else
				SwizzleBgraToRgba(src, width, height, rowPitch, /*swapBr*/ true, out);
			break;
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			if constexpr (channels == 3)
				SwizzleBgraToRgb(src, width, height, rowPitch, /*swapBr*/ false, out);
			else
				SwizzleBgraToRgba(src, width, height, rowPitch, /*swapBr*/ false, out);
			break;
		case DXGI_FORMAT_R10G10B10A2_UNORM:
			ConvertR10G10B10A2<channels>(src, width, height, rowPitch, out);
			break;
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			ConvertR16G16B16A16Float<channels>(src, width, height, rowPitch, out);
			break;
		default:
			assert(false); // only called for supported formats
			break;
	}
}

static void EncodeWorkerLoop()
{
	for (;;)
	{
		EncodeJob job;

		{
			std::unique_lock lock(g_encodeQueueMutex);
			g_encodeQueueCv.wait(lock, []
			{
				return !g_encodeQueue.empty() || g_workerShouldExit.load(std::memory_order_acquire);
			});

			if (g_workerShouldExit.load(std::memory_order_acquire) && g_encodeQueue.empty())
			{
				return;
			}

			job = std::move(g_encodeQueue.front());
			g_encodeQueue.pop();
		}

		if (job.sentinel)
		{
			return;
		}

		std::vector<uint8_t> encodedBytes;
		encodedBytes.reserve(static_cast<size_t>(job.width) * job.height / 4);

		int writeOk = 0;

		switch (job.encodeFormat)
		{
			case ScreenshotFormat::Jpeg:
			{
				std::vector<uint8_t> rgbData;
				DecodeBackbuffer<3>(job.format, job.source, job.width, job.height, job.rowPitch, rgbData);

				if (rgbData.empty())
				{
					break;
				}

				writeOk = stbi_write_jpg_to_func(
					&StbiWriteAppendCallback,
					&encodedBytes,
					static_cast<int>(job.width),
					static_cast<int>(job.height),
					3,
					rgbData.data(),
					job.jpegQuality);
				break;
			}
			case ScreenshotFormat::Png:
			{
				std::vector<uint8_t> rgbaData;
				DecodeBackbuffer<4>(job.format, job.source, job.width, job.height, job.rowPitch, rgbaData);

				if (rgbaData.empty())
				{
					break;
				}

				writeOk = stbi_write_png_to_func(
					&StbiWriteAppendCallback,
					&encodedBytes,
					static_cast<int>(job.width),
					static_cast<int>(job.height),
					4,
					rgbaData.data(),
					static_cast<int>(job.width) * 4);
				break;
			}
		}

		assert(job.stagingEntryIdx >= 0 && job.stagingEntryIdx < kStagingCount);
		g_stagingPool[job.stagingEntryIdx].workerDone.store(true, std::memory_order_release);

		if (!writeOk || encodedBytes.empty())
		{
			MarkSlotFailed(job.slotIndex);
			g_inFlightCount.fetch_sub(1, std::memory_order_acq_rel);
			continue;
		}

		std::string b64;
		{
			const size_t inputLen = encodedBytes.size();
			const size_t outputLen = ((inputLen + 2) / 3) * 4;
			b64.resize(outputLen);

			constexpr size_t kChunkInput = 48 * 1024;
			static_assert(kChunkInput % 3 == 0, "chunk must be multiple of 3 to avoid mid-stream padding");

			const uint8_t* inPtr = encodedBytes.data();
			size_t inRemaining = inputLen;
			char* outPtr = b64.data();

			while (inRemaining > kChunkInput)
			{
				size_t consumed = 0;
				size_t written = Botan::base64_encode(outPtr, inPtr, kChunkInput, consumed, false);
				outPtr += written;
				inPtr += consumed;
				inRemaining -= consumed;
			}

			size_t consumed = 0;
			Botan::base64_encode(outPtr, inPtr, inRemaining, consumed, true);
		}

		{
			std::lock_guard lock(g_slotMutex);
			if (IsValidHandle(job.slotIndex) && g_slots[job.slotIndex].state == SlotState::Encoding)
			{
				g_slots[job.slotIndex].base64Encoded = std::move(b64);
				g_slots[job.slotIndex].state = SlotState::Ready;
				g_slots[job.slotIndex].readyFrame = g_currentFrame.load(std::memory_order_relaxed);
			}
		}

		g_inFlightCount.fetch_sub(1, std::memory_order_acq_rel);
	}
}

static void EnsureWorkerStarted()
{
	std::call_once(g_workerOnce, []
	{
		g_workerShouldExit.store(false, std::memory_order_release);
		g_workerThread = std::thread(&EncodeWorkerLoop);
	});
}

static bool EnqueueEncodeJob(EncodeJob&& job)
{
	{
		std::lock_guard lock(g_encodeQueueMutex);
		if (g_encodeQueue.size() >= kQueueDepth)
		{
			return false;
		}
		g_encodeQueue.push(std::move(job));
	}
	g_encodeQueueCv.notify_one();
	return true;
}

static StagingEntry* AcquireStagingEntry(ID3D11Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format)
{
	for (auto& entry : g_stagingPool)
	{
		if (!entry.inFlight && !entry.mapped && entry.tex && entry.width == width && entry.height == height && entry.format == format)
		{
			return &entry;
		}
	}

	for (auto& entry : g_stagingPool)
	{
		if (!entry.inFlight && !entry.mapped)
		{
			D3D11_TEXTURE2D_DESC desc = {};
			desc.Width = width;
			desc.Height = height;
			desc.Format = format;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_STAGING;
			desc.BindFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			desc.MiscFlags = 0;

			entry.tex.Reset();
			HRESULT hr = device->CreateTexture2D(&desc, nullptr, &entry.tex);

			if (FAILED(hr))
			{
				return nullptr;
			}

			entry.width = width;
			entry.height = height;
			entry.format = format;
			return &entry;
		}
	}

	return nullptr;
}

static ID3D11Texture2D* AcquireResolveTexture(ID3D11Device* device, const D3D11_TEXTURE2D_DESC& bbDesc)
{
	for (auto& entry : g_resolvePool)
	{
		if (entry.tex && entry.width == bbDesc.Width && entry.height == bbDesc.Height && entry.format == bbDesc.Format && entry.sampleCount == bbDesc.SampleDesc.Count)
		{
			return entry.tex.Get();
		}
	}

	for (auto& entry : g_resolvePool)
	{
		if (!entry.tex || entry.width != bbDesc.Width || entry.height != bbDesc.Height || entry.format != bbDesc.Format || entry.sampleCount != bbDesc.SampleDesc.Count)
		{
			D3D11_TEXTURE2D_DESC resolveDesc = bbDesc;
			resolveDesc.SampleDesc.Count = 1;
			resolveDesc.SampleDesc.Quality = 0;
			resolveDesc.Usage = D3D11_USAGE_DEFAULT;
			resolveDesc.BindFlags = 0;
			resolveDesc.CPUAccessFlags = 0;
			resolveDesc.MiscFlags = 0;

			entry.tex.Reset();
			HRESULT hr = device->CreateTexture2D(&resolveDesc, nullptr, &entry.tex);

			if (FAILED(hr))
			{
				return nullptr;
			}

			entry.width = bbDesc.Width;
			entry.height = bbDesc.Height;
			entry.format = bbDesc.Format;
			entry.sampleCount = bbDesc.SampleDesc.Count;
			return entry.tex.Get();
		}
	}

	return nullptr;
}

static void IssueScreenshotCopy(int slotIndex, bool hudVariant, ScreenshotFormat encodeFormat, int jpegQuality)
{
	{
		std::lock_guard lock(g_slotMutex);
		if (!IsValidHandle(slotIndex) || g_slots[slotIndex].state != SlotState::Pending)
		{
			return;
		}
	}

	struct SlotFailGuard
	{
		int slotIndex;
		bool armed = true;
		~SlotFailGuard()
		{
			if (armed)
			{
				MarkSlotFailed(slotIndex);
				g_inFlightCount.fetch_sub(1, std::memory_order_acq_rel);
			}
		}
	};

	SlotFailGuard guard{ slotIndex };

	auto device = GetD3D11Device();
	auto dc = GetD3D11DeviceContext();

	if (!device || !dc)
	{
		return;
	}

	WRL::ComPtr<ID3D11RenderTargetView> rtv;
	dc->OMGetRenderTargets(1, &rtv, nullptr);

	if (!rtv)
	{
		return;
	}

	WRL::ComPtr<ID3D11Resource> rtResource;
	rtv->GetResource(&rtResource);

	WRL::ComPtr<ID3D11Texture2D> backbufferTex;
	rtResource.As(&backbufferTex);

	if (!backbufferTex)
	{
		return;
	}

	D3D11_TEXTURE2D_DESC bbDesc;
	backbufferTex->GetDesc(&bbDesc);

	if (bbDesc.Width == 0 || bbDesc.Height == 0)
	{
		return;
	}

	if (!IsSupportedBackbufferFormat(bbDesc.Format))
	{
		return;
	}

	StagingEntry* entry = AcquireStagingEntry(device, bbDesc.Width, bbDesc.Height, bbDesc.Format);

	if (!entry)
	{
		return;
	}

	if (bbDesc.SampleDesc.Count > 1)
	{
		ID3D11Texture2D* resolveTex = AcquireResolveTexture(device, bbDesc);

		if (!resolveTex)
		{
			return;
		}

		dc->ResolveSubresource(resolveTex, 0, backbufferTex.Get(), 0, bbDesc.Format);
		dc->CopyResource(entry->tex.Get(), resolveTex);
	}
	else
	{
		dc->CopyResource(entry->tex.Get(), backbufferTex.Get());
	}

	entry->inFlight = true;
	entry->issuedFrame = g_currentFrame.load(std::memory_order_relaxed);
	entry->slotIndex = slotIndex;
	entry->hudVariant = hudVariant;
	entry->encodeFormat = encodeFormat;
	entry->jpegQuality = jpegQuality;

	TransitionSlotState(slotIndex, SlotState::Pending, SlotState::Capturing);
	guard.armed = false;
}

static void ReclaimCompletedStaging(ID3D11DeviceContext* dc)
{
	for (auto& entry : g_stagingPool)
	{
		if (entry.mapped && entry.workerDone.load(std::memory_order_acquire))
		{
			entry.ReleaseMapping(dc);
			entry.workerDone.store(false, std::memory_order_relaxed);
		}
	}
}

static void IssueNewReadbacks(ID3D11DeviceContext* dc)
{
	const int frameNow = g_currentFrame.load(std::memory_order_relaxed);

	for (int idx = 0; idx < kStagingCount; idx++)
	{
		auto& entry = g_stagingPool[idx];

		if (!entry.inFlight || entry.mapped)
		{
			continue;
		}

		if ((frameNow - entry.issuedFrame) < kLatencyFrames)
		{
			continue;
		}

		D3D11_MAPPED_SUBRESOURCE mapped = {};
		HRESULT hr = dc->Map(entry.tex.Get(), 0, D3D11_MAP_READ, D3D11_MAP_FLAG_DO_NOT_WAIT, &mapped);

		if (hr == DXGI_ERROR_WAS_STILL_DRAWING)
		{
			continue;
		}

		if (FAILED(hr) || !mapped.pData)
		{
			MarkSlotFailed(entry.slotIndex);
			g_inFlightCount.fetch_sub(1, std::memory_order_acq_rel);
			entry.inFlight = false;
			entry.slotIndex = -1;
			continue;
		}

		entry.mapped = true;
		entry.mappedPtr = mapped.pData;
		entry.mappedRowPitch = mapped.RowPitch;

		EncodeJob job;
		job.source = static_cast<const uint8_t*>(mapped.pData);
		job.stagingEntryIdx = idx;
		job.width = entry.width;
		job.height = entry.height;
		job.rowPitch = mapped.RowPitch;
		job.format = entry.format;
		job.slotIndex = entry.slotIndex;
		job.hudVariant = entry.hudVariant;
		job.encodeFormat = entry.encodeFormat;
		job.jpegQuality = entry.jpegQuality;

		const int slotIndexCaptured = entry.slotIndex;

		TransitionSlotState(slotIndexCaptured, SlotState::Capturing, SlotState::Encoding);

		if (!EnqueueEncodeJob(std::move(job)))
		{
			// queue full: Unmap inline so the staging entry stays usable
			entry.ReleaseMapping(dc);

			MarkSlotFailed(slotIndexCaptured);
			g_inFlightCount.fetch_sub(1, std::memory_order_acq_rel);
		}
	}
}

static void PollStagingReadbacks()
{
	auto dc = GetD3D11DeviceContext();
	if (!dc)
	{
		return;
	}

	ReclaimCompletedStaging(dc);
	IssueNewReadbacks(dc);
}

static void ProcessPendingScreenshots(CaptureQueue& cq, bool hudVariant)
{
	std::queue<PendingHandleRequest> handleRequests;

	{
		std::lock_guard lock(cq.mutex);
		std::swap(handleRequests, cq.requests);
	}

	const int drained = static_cast<int>(handleRequests.size());
	if (drained > 0)
	{
		cq.count.fetch_sub(drained, std::memory_order_acq_rel);
	}

	while (!handleRequests.empty())
	{
		const auto& req = handleRequests.front();
		IssueScreenshotCopy(req.slotIndex, hudVariant, req.format, req.jpegQuality);
		handleRequests.pop();
	}
}

static void OnAfterHudCapture()
{
	if (g_afterHud.count.load(std::memory_order_acquire) == 0)
	{
		return;
	}

	uintptr_t a1 = 0, a2 = 0;
	EnqueueGenericDrawCommand(
	[](uintptr_t, uintptr_t)
	{
		ProcessPendingScreenshots(g_afterHud, true);
	},
	&a1, &a2);
}

static std::atomic<bool> g_skipHudForCapture{ false };

static void (*g_origRenderPhaseHudBuildDrawList)(void*);

static void RenderPhaseHudBuildDrawListHook(void* thisptr)
{
	if (g_skipHudForCapture.load(std::memory_order_relaxed))
	{
		return;
	}

	g_origRenderPhaseHudBuildDrawList(thisptr);
}

static void ShutdownWorker()
{
	if (!g_workerThread.joinable())
	{
		return;
	}

	{
		std::lock_guard lock(g_encodeQueueMutex);
		g_workerShouldExit.store(true, std::memory_order_release);
		EncodeJob sentinel;
		sentinel.sentinel = true;
		g_encodeQueue.push(std::move(sentinel));
	}
	g_encodeQueueCv.notify_all();

	g_workerThread.join();

	auto dc = GetD3D11DeviceContext();
	if (!dc)
	{
		return;
	}

	for (auto& entry : g_stagingPool)
	{
		if (entry.mapped && entry.tex)
		{
			dc->Unmap(entry.tex.Get(), 0);
			entry.mapped = false;
			entry.mappedPtr = nullptr;
			entry.mappedRowPitch = 0;
		}
	}
}

static int TakeScreenshotImpl(fx::ScriptContext& context, CaptureQueue& cq)
{
	if (g_inFlightCount.load(std::memory_order_acquire) >= kMaxInFlight)
	{
		return -1;
	}

	const ScreenshotFormat format = (context.GetArgument<int>(0) == 1)
		? ScreenshotFormat::Png : ScreenshotFormat::Jpeg;

	int quality = ClampJpegQuality(context.GetArgument<int>(1));

	int slot;
	{
		std::lock_guard slotLock(g_slotMutex);
		slot = AllocateSlot();
	}

	if (slot < 0)
	{
		return -1;
	}

	g_inFlightCount.fetch_add(1, std::memory_order_acq_rel);

	{
		std::lock_guard pendingLock(cq.mutex);
		cq.requests.push({ slot, format, quality });
		cq.count.fetch_add(1, std::memory_order_acq_rel);
	}

	return slot;
}

static InitFunction initFunction([]()
{
	EnsureWorkerStarted();

	fx::ScriptEngine::RegisterNativeHandler("TAKE_SCREENSHOT_ASYNC", [](fx::ScriptContext& context)
	{
		context.SetResult<int>(TakeScreenshotImpl(context, g_beforeHud));
	});

	fx::ScriptEngine::RegisterNativeHandler("TAKE_SCREENSHOT_HUD_ASYNC", [](fx::ScriptContext& context)
	{
		context.SetResult<int>(TakeScreenshotImpl(context, g_afterHud));
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_SCREENSHOT_READY", [](fx::ScriptContext& context)
	{
		int handle = context.GetArgument<int>(0);

		if (!IsValidHandle(handle))
		{
			context.SetResult<int>(0);
			return;
		}

		std::lock_guard lock(g_slotMutex);
		context.SetResult<int>(g_slots[handle].state == SlotState::Ready ? 1 : 0);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_SCREENSHOT_ENCODED", [](fx::ScriptContext& context)
	{
		static std::string lastEncodedResult;
		static const char* const kEmpty = "";

		lastEncodedResult.clear();

		int handle = context.GetArgument<int>(0);

		if (!IsValidHandle(handle))
		{
			context.SetResult<const char*>(kEmpty);
			return;
		}

		std::lock_guard lock(g_slotMutex);

		if (g_slots[handle].state != SlotState::Ready)
		{
			if (g_slots[handle].state == SlotState::Failed)
			{
				g_slots[handle].state = SlotState::Free;
				g_slots[handle].readyFrame = 0;
			}

			context.SetResult<const char*>(kEmpty);
			return;
		}

		lastEncodedResult = std::move(g_slots[handle].base64Encoded);
		g_slots[handle].state = SlotState::Free;
		g_slots[handle].readyFrame = 0;

		context.SetResult<const char*>(lastEncodedResult.c_str());
	});

	OnGameFrame.Connect([]()
	{
		g_currentFrame.fetch_add(1, std::memory_order_relaxed);

		const int frameNow = g_currentFrame.load(std::memory_order_relaxed);

		std::lock_guard lock(g_slotMutex);

		for (int i = 0; i < kMaxHandles; i++)
		{
			const auto st = g_slots[i].state;
			const int age = frameNow - g_slots[i].readyFrame;

			if ((st == SlotState::Ready || st == SlotState::Failed) && age > 300)
			{
				g_slots[i].base64Encoded.clear();
				g_slots[i].state = SlotState::Free;
				g_slots[i].readyFrame = 0;
				continue;
			}

			if (st == SlotState::Pending && age > 120)
			{
				g_slots[i].base64Encoded.clear();
				g_slots[i].state = SlotState::Free;
				g_slots[i].readyFrame = 0;
				g_inFlightCount.fetch_sub(1, std::memory_order_acq_rel);
			}
		}
	});

	// flip-flop (g_skipHudForCapture) set flag on this frame so the HUD hook skips next frame,
	// capture then, remove flag. One-frame delay is unavoidable, HUD is already drawn by the
	// time OnPostFrontendRender fires.
	OnPostFrontendRender.Connect([]()
	{
		if (g_inFlightCount.load(std::memory_order_acquire) > 0
			|| g_beforeHud.count.load(std::memory_order_acquire) > 0
			|| g_skipHudForCapture.load(std::memory_order_relaxed))
		{
			uintptr_t a1 = 0, a2 = 0;
			EnqueueGenericDrawCommand(
			[](uintptr_t, uintptr_t)
			{
				PollStagingReadbacks();

				if (g_skipHudForCapture.load(std::memory_order_relaxed))
				{
					ProcessPendingScreenshots(g_beforeHud, false);
					g_skipHudForCapture.store(false, std::memory_order_relaxed);
				}
				else if (g_beforeHud.count.load(std::memory_order_acquire) > 0)
				{
					g_skipHudForCapture.store(true, std::memory_order_relaxed);
				}
			},
			&a1, &a2);
		}

		OnAfterHudCapture();
	},
	INT32_MIN + 6);
});

static HookFunction hookFunction([]()
{
	// CRenderPhaseHud::BuildDrawList
	//
	// mov     [rsp+10h], rbx
	// mov     [rsp+18h], rdi
	// push    rbp
	// mov     rbp, rsp
	// sub     rsp, 30h
	// mov     rax, [rcx+4C8h]  ; m_viewport
	// mov     rdi, rcx
	// test    byte ptr [rax+5E2h], 1

	MH_Initialize();

	auto hudBuildDrawList = hook::get_pattern(
	"48 89 5C 24 10 48 89 7C 24 18 55 48 8B EC 48 83 EC 30 48 8B 81 ? ? 00 00");

	MH_CreateHook(hudBuildDrawList, RenderPhaseHudBuildDrawListHook, (void**)&g_origRenderPhaseHudBuildDrawList);
	MH_EnableHook(hudBuildDrawList);

	std::atexit(&ShutdownWorker);
});
