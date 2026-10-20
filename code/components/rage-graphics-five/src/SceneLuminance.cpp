#include "StdInc.h"
#include "GameViewCapture.h"
#include "SceneLuminance.h"

#include <DrawCommands.h>
#include <Hooking.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <mutex>
#include <string>

#pragma comment(lib, "d3dcompiler.lib")

using Microsoft::WRL::ComPtr;

namespace
{
struct CellSpan
{
	float from;
	float to;

	uint32_t first;
	uint32_t end;

	CellSpan(float start, float stop, uint32_t cells) : from(start * cells), to(stop * cells), first(static_cast<uint32_t>(from)), end(std::min(cells, static_cast<uint32_t>(std::ceil(to))))
	{
		//
	}

	float Overlap(uint32_t cell) const
	{
		float edge = cell;
		return std::min(to, edge + 1) - std::max(from, edge);
	}
};

// converts linear luminance to perceived lightness (CIE L*, scaled to 0..1), so 0.5 reads as a medium grey
float Lightness(float luminance)
{
	return luminance <= 0.008856f ? luminance * 9.033f : 1.16f * std::cbrt(luminance) - 0.16f;
}

constexpr uint32_t GridWidth = 32;
constexpr uint32_t GridHeight = 18;
constexpr uint32_t CellCount = GridWidth * GridHeight;

constexpr uint64_t IdleMs = 1000;
constexpr uint64_t MaxAgeMs = 500;

// reduces the scene to a GRID_WIDTH x GRID_HEIGHT grid of average luminance
constexpr char ShaderSource[] = R"hlsl(
Texture2D<float4> scene : register(t0);
RWStructuredBuffer<float> results : register(u0);

// each cell averages Samples x Samples evenly spaced pixels rather than every pixel it covers
static const uint Samples = 4;

// Rec. 709 weights giving the relative luminance of linear RGB
static const float3 LuminanceWeights = float3(0.2126, 0.7152, 0.0722);

// standard sRGB decoding curve, from encoded values to linear light
float3 SrgbToLinear(float3 encoded)
{
	return encoded <= 0.04045 ? encoded / 12.92 : pow(max((encoded + 0.055) / 1.055, 0.0), 2.4);
}

[numthreads(GRID_WIDTH, GRID_HEIGHT, 1)]
void main(uint3 cell : SV_GroupThreadID)
{
	uint width;
	uint height;
	scene.GetDimensions(width, height);

	// sample points form a uniform grid over the whole scene, each read at its center
	float2 pointCount = float2(GRID_WIDTH, GRID_HEIGHT) * Samples;
	float2 pixelsPerPoint = float2(width, height) / pointCount;
	float sum = 0;

	[unroll] for (uint y = 0; y < Samples; ++y)
	{
		[unroll] for (uint x = 0; x < Samples; ++x)
		{
			float2 samplePoint = float2(cell.xy * Samples + uint2(x, y));
			float3 rgb = scene.Load(int3((samplePoint + 0.5) * pixelsPerPoint, 0)).rgb;
			// the scene view is plain 8-bit, so its values are still sRGB encoded and must be decoded before averaging
			sum += dot(SrgbToLinear(rgb), LuminanceWeights);
		}
	}

	results[cell.y * GRID_WIDTH + cell.x] = saturate(sum / (Samples * Samples));
}
)hlsl";

uint64_t g_gridTime = 0;
uint64_t g_frame = 0;

bool g_initialized = false;
bool (*g_isSystemUiShowing)();

ComPtr<ID3DBlob> g_bytecode;
ComPtr<ID3D11ComputeShader> g_shader;
ComPtr<ID3D11Buffer> g_output;
ComPtr<ID3D11UnorderedAccessView> g_outputUav;

std::array<ComPtr<ID3D11Buffer>, 3> g_staging;
std::array<uint64_t, 3> g_stagingTime{};
std::array<float, CellCount> g_grid;
std::atomic<uint64_t> g_lastRequest{ 0 };
std::mutex g_gridMutex;

bool CompileShader()
{
	const std::string gridWidth = std::to_string(GridWidth);
	const std::string gridHeight = std::to_string(GridHeight);

	D3D_SHADER_MACRO defines[] = {
		{ "GRID_WIDTH", gridWidth.c_str() },
		{ "GRID_HEIGHT", gridHeight.c_str() },
		{ nullptr, nullptr }
	};

	return SUCCEEDED(D3DCompile(ShaderSource, sizeof(ShaderSource) - 1, "SceneLuminance", defines, nullptr, "main", "cs_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &g_bytecode, nullptr));
}

void CreateResources(ID3D11Device* device)
{
	D3D11_BUFFER_DESC desc{};
	desc.ByteWidth = CellCount * sizeof(float);
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = sizeof(float);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.NumElements = CellCount;

	if (FAILED(device->CreateBuffer(&desc, nullptr, &g_output)) || FAILED(device->CreateUnorderedAccessView(g_output.Get(), &uavDesc, &g_outputUav)))
	{
		return;
	}

	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	for (auto& staging : g_staging)
	{
		if (FAILED(device->CreateBuffer(&desc, nullptr, &staging)))
		{
			return;
		}
	}

	device->CreateComputeShader(g_bytecode->GetBufferPointer(), g_bytecode->GetBufferSize(), nullptr, &g_shader);
}

void ReadBack(ID3D11DeviceContext* context, ID3D11Buffer* staging, uint64_t capturedAt)
{
	D3D11_MAPPED_SUBRESOURCE mapped{};
	if (FAILED(context->Map(staging, 0, D3D11_MAP_READ, D3D11_MAP_FLAG_DO_NOT_WAIT, &mapped)))
	{
		return;
	}

	{
		std::lock_guard<std::mutex> lock(g_gridMutex);
		memcpy(g_grid.data(), mapped.pData, sizeof(g_grid));
		g_gridTime = capturedAt;
	}

	context->Unmap(staging, 0);
}

void Dispatch(ID3D11DeviceContext* context, ID3D11ShaderResourceView* scene, ID3D11Buffer* staging)
	{
	std::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> targets{};
	ComPtr<ID3D11DepthStencilView> depth;
	ComPtr<ID3D11ComputeShader> oldShader;
	ComPtr<ID3D11ShaderResourceView> oldSrv;
	ComPtr<ID3D11UnorderedAccessView> oldUav;

	context->OMGetRenderTargets(static_cast<uint32_t>(targets.size()), targets.data(), &depth);
	context->CSGetShader(&oldShader, nullptr, nullptr);
	context->CSGetShaderResources(0, 1, &oldSrv);
	context->CSGetUnorderedAccessViews(0, 1, &oldUav);

	// only restore up to the last bound target, pixel shader UAVs share the slots after it
	uint32_t targetCount = 0;

	for (uint32_t i = 0; i < targets.size(); ++i)
	{
		if (targets[i])
		{
			targetCount = i + 1;
		}
	}

	context->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, 0, D3D11_KEEP_UNORDERED_ACCESS_VIEWS, nullptr, nullptr);
	context->CSSetShader(g_shader.Get(), nullptr, 0);
	context->CSSetShaderResources(0, 1, &scene);
	context->CSSetUnorderedAccessViews(0, 1, g_outputUav.GetAddressOf(), nullptr);
	context->Dispatch(1, 1, 1);

	context->CSSetShader(oldShader.Get(), nullptr, 0);
	context->CSSetShaderResources(0, 1, oldSrv.GetAddressOf());
	context->CSSetUnorderedAccessViews(0, 1, oldUav.GetAddressOf(), nullptr);
	context->OMSetRenderTargetsAndUnorderedAccessViews(targetCount, targets.data(), depth.Get(), 0, D3D11_KEEP_UNORDERED_ACCESS_VIEWS, nullptr, nullptr);

	for (auto target : targets)
	{
		if (target)
		{
			target->Release();
		}
	}

	context->CopyResource(staging, g_output.Get());
}

void Capture()
{
	if (g_lastRequest + IdleMs < GetTickCount64())
	{
		return;
	}

	auto backBuffer = GetBackbuf();
	if (!backBuffer || !backBuffer->m_srv2 || !IsOnRenderThread())
	{
		return;
	}

	ComPtr<IUnknown> srvIdentity;
	ComPtr<ID3D11ShaderResourceView> scene;

	if (FAILED(backBuffer->m_srv2->QueryInterface(IID_PPV_ARGS(&srvIdentity))) || FAILED(srvIdentity.As(&scene)))
	{
		return;
	}

	// the shader decodes sRGB itself and reads a single-sampled texture, any other view would give wrong values
	D3D11_SHADER_RESOURCE_VIEW_DESC view{};
	scene->GetDesc(&view);

	if (view.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D || (view.Format != DXGI_FORMAT_B8G8R8A8_UNORM && view.Format != DXGI_FORMAT_R8G8B8A8_UNORM))
	{
		return;
	}

	if (!g_initialized)
	{
		g_initialized = true;
		CreateResources(GetInvariantD3D11Device().Get());
	}

	if (!g_shader)
	{
		return;
	}

	auto context = GetInvariantD3D11DeviceContext();
	const size_t slot = g_frame++ % g_staging.size();

	if (g_stagingTime[slot])
	{
		ReadBack(context.Get(), g_staging[slot].Get(), g_stagingTime[slot]);
	}

	Dispatch(context.Get(), scene.Get(), g_staging[slot].Get());
	g_stagingTime[slot] = GetTickCount64();
}

bool BeforePhotoOverlays()
{
	Capture();
	return g_isSystemUiShowing();
}

static HookFunction hookFunction([]()
{
	// system UI check right before photo overlays, which runs before HUD, Scaleform and NUI are drawn
	auto matches = hook::pattern("E8 ? ? ? ? 84 C0 75 ? 33 C9 E8 ? ? ? ? 84 C0 75 ? 44 38 2D ? ? ? ? 74 ? E8 ? ? ? ? 84 C0 75 ? 48 8B 1D").count_hint(2);
	if (matches.size() != 1)
	{
		trace("Scene luminance unavailable: no unique pre-UI capture boundary\n");
		return;
	}

	if (!CompileShader())
	{
		trace("Scene luminance unavailable: shader compilation failed\n");
		return;
	}

	auto location = matches.get(0).get<void>();
	hook::set_call(&g_isSystemUiShowing, location);
	hook::call(location, BeforePhotoOverlays);
});
}

bool GetSceneLuminance(float left, float top, float right, float bottom, float& value)
{
	const auto now = GetTickCount64();
	g_lastRequest = now;

	std::lock_guard<std::mutex> lock(g_gridMutex);
	if (!g_gridTime || g_gridTime + MaxAgeMs < now)
	{
		return false;
	}

	const CellSpan columns(left, right, GridWidth);
	const CellSpan rows(top, bottom, GridHeight);
	float sum = 0;

	for (uint32_t cy = rows.first; cy < rows.end; ++cy)
	{
		for (uint32_t cx = columns.first; cx < columns.end; ++cx)
		{
			sum += g_grid[cy * GridWidth + cx] * columns.Overlap(cx) * rows.Overlap(cy);
		}
	}

	value = Lightness(sum / ((columns.to - columns.from) * (rows.to - rows.from)));
	return true;
}
