#pragma once

#include <d3d11.h>
#include <wrl/client.h>

namespace rage
{
class grcRenderTargetDX11;
}

Microsoft::WRL::ComPtr<ID3D11Device> GetInvariantD3D11Device();
Microsoft::WRL::ComPtr<ID3D11DeviceContext> GetInvariantD3D11DeviceContext();

rage::grcRenderTargetDX11* GetBackbuf();
