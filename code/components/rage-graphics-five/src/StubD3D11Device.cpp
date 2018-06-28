#include "StdInc.h"
#include <d3d11.h>
#include <wrl.h>

namespace WRL = Microsoft::WRL;

static std::vector<char> g_mapScratchBuffer;

class StubD3D11DeviceContext : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11DeviceContext>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
	virtual void VSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer * const * ppConstantBuffers) override
	{
	}
	virtual void PSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView * const * ppShaderResourceViews) override
	{
	}
	virtual void PSSetShader(ID3D11PixelShader * pPixelShader, ID3D11ClassInstance * const * ppClassInstances, UINT NumClassInstances) override
	{
	}
	virtual void PSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState * const * ppSamplers) override
	{
	}
	virtual void VSSetShader(ID3D11VertexShader * pVertexShader, ID3D11ClassInstance * const * ppClassInstances, UINT NumClassInstances) override
	{
	}
	virtual void DrawIndexed(UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation) override
	{
	}
	virtual void Draw(UINT VertexCount, UINT StartVertexLocation) override
	{
	}
	virtual HRESULT Map(ID3D11Resource * pResource, UINT Subresource, D3D11_MAP MapType, UINT MapFlags, D3D11_MAPPED_SUBRESOURCE * pMappedResource) override
	{
		// lazy-initialize the map scratch buffer
		if (g_mapScratchBuffer.size() == 0)
		{
			g_mapScratchBuffer.resize(128 * 1024 * 1024);
		}

		pMappedResource->DepthPitch = 512;
		pMappedResource->pData = g_mapScratchBuffer.data();
		pMappedResource->RowPitch = 512;
		return S_OK;
	}
	virtual void Unmap(ID3D11Resource * pResource, UINT Subresource) override
	{
	}
	virtual void PSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer * const * ppConstantBuffers) override
	{
	}
	virtual void IASetInputLayout(ID3D11InputLayout * pInputLayout) override
	{
	}
	virtual void IASetVertexBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer * const * ppVertexBuffers, const UINT * pStrides, const UINT * pOffsets) override
	{
	}
	virtual void IASetIndexBuffer(ID3D11Buffer * pIndexBuffer, DXGI_FORMAT Format, UINT Offset) override
	{
	}
	virtual void DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation) override
	{
	}
	virtual void DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation) override
	{
	}
	virtual void GSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer * const * ppConstantBuffers) override
	{
	}
	virtual void GSSetShader(ID3D11GeometryShader * pShader, ID3D11ClassInstance * const * ppClassInstances, UINT NumClassInstances) override
	{
	}
	virtual void IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY Topology) override
	{
	}
	virtual void VSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView * const * ppShaderResourceViews) override
	{
	}
	virtual void VSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState * const * ppSamplers) override
	{
	}
	virtual void Begin(ID3D11Asynchronous * pAsync) override
	{
	}
	virtual void End(ID3D11Asynchronous * pAsync) override
	{
	}
	virtual HRESULT GetData(ID3D11Asynchronous * pAsync, void * pData, UINT DataSize, UINT GetDataFlags) override
	{
		*(int*)pData = 1;
		return S_OK;
	}
	virtual void SetPredication(ID3D11Predicate * pPredicate, BOOL PredicateValue) override
	{
	}
	virtual void GSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView * const * ppShaderResourceViews) override
	{
	}
	virtual void GSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState * const * ppSamplers) override
	{
	}
	virtual void OMSetRenderTargets(UINT NumViews, ID3D11RenderTargetView * const * ppRenderTargetViews, ID3D11DepthStencilView * pDepthStencilView) override
	{
	}
	virtual void OMSetRenderTargetsAndUnorderedAccessViews(UINT NumRTVs, ID3D11RenderTargetView * const * ppRenderTargetViews, ID3D11DepthStencilView * pDepthStencilView, UINT UAVStartSlot, UINT NumUAVs, ID3D11UnorderedAccessView * const * ppUnorderedAccessViews, const UINT * pUAVInitialCounts) override
	{
	}
	virtual void OMSetBlendState(ID3D11BlendState * pBlendState, const FLOAT BlendFactor[4], UINT SampleMask) override
	{
	}
	virtual void OMSetDepthStencilState(ID3D11DepthStencilState * pDepthStencilState, UINT StencilRef) override
	{
	}
	virtual void SOSetTargets(UINT NumBuffers, ID3D11Buffer * const * ppSOTargets, const UINT * pOffsets) override
	{
	}
	virtual void DrawAuto(void) override
	{
	}
	virtual void DrawIndexedInstancedIndirect(ID3D11Buffer * pBufferForArgs, UINT AlignedByteOffsetForArgs) override
	{
	}
	virtual void DrawInstancedIndirect(ID3D11Buffer * pBufferForArgs, UINT AlignedByteOffsetForArgs) override
	{
	}
	virtual void Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ) override
	{
	}
	virtual void DispatchIndirect(ID3D11Buffer * pBufferForArgs, UINT AlignedByteOffsetForArgs) override
	{
	}
	virtual void RSSetState(ID3D11RasterizerState * pRasterizerState) override
	{
	}
	virtual void RSSetViewports(UINT NumViewports, const D3D11_VIEWPORT * pViewports) override
	{
	}
	virtual void RSSetScissorRects(UINT NumRects, const D3D11_RECT * pRects) override
	{
	}
	virtual void CopySubresourceRegion(ID3D11Resource * pDstResource, UINT DstSubresource, UINT DstX, UINT DstY, UINT DstZ, ID3D11Resource * pSrcResource, UINT SrcSubresource, const D3D11_BOX * pSrcBox) override
	{
	}
	virtual void CopyResource(ID3D11Resource * pDstResource, ID3D11Resource * pSrcResource) override
	{
	}
	virtual void UpdateSubresource(ID3D11Resource * pDstResource, UINT DstSubresource, const D3D11_BOX * pDstBox, const void * pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch) override
	{
	}
	virtual void CopyStructureCount(ID3D11Buffer * pDstBuffer, UINT DstAlignedByteOffset, ID3D11UnorderedAccessView * pSrcView) override
	{
	}
	virtual void ClearRenderTargetView(ID3D11RenderTargetView * pRenderTargetView, const FLOAT ColorRGBA[4]) override
	{
	}
	virtual void ClearUnorderedAccessViewUint(ID3D11UnorderedAccessView * pUnorderedAccessView, const UINT Values[4]) override
	{
	}
	virtual void ClearUnorderedAccessViewFloat(ID3D11UnorderedAccessView * pUnorderedAccessView, const FLOAT Values[4]) override
	{
	}
	virtual void ClearDepthStencilView(ID3D11DepthStencilView * pDepthStencilView, UINT ClearFlags, FLOAT Depth, UINT8 Stencil) override
	{
	}
	virtual void GenerateMips(ID3D11ShaderResourceView * pShaderResourceView) override
	{
	}
	virtual void SetResourceMinLOD(ID3D11Resource * pResource, FLOAT MinLOD) override
	{
	}
	virtual FLOAT GetResourceMinLOD(ID3D11Resource * pResource) override
	{
		return FLOAT();
	}
	virtual void ResolveSubresource(ID3D11Resource * pDstResource, UINT DstSubresource, ID3D11Resource * pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format) override
	{
	}
	virtual void ExecuteCommandList(ID3D11CommandList * pCommandList, BOOL RestoreContextState) override
	{
	}
	virtual void HSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView * const * ppShaderResourceViews) override
	{
	}
	virtual void HSSetShader(ID3D11HullShader * pHullShader, ID3D11ClassInstance * const * ppClassInstances, UINT NumClassInstances) override
	{
	}
	virtual void HSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState * const * ppSamplers) override
	{
	}
	virtual void HSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer * const * ppConstantBuffers) override
	{
	}
	virtual void DSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView * const * ppShaderResourceViews) override
	{
	}
	virtual void DSSetShader(ID3D11DomainShader * pDomainShader, ID3D11ClassInstance * const * ppClassInstances, UINT NumClassInstances) override
	{
	}
	virtual void DSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState * const * ppSamplers) override
	{
	}
	virtual void DSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer * const * ppConstantBuffers) override
	{
	}
	virtual void CSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView * const * ppShaderResourceViews) override
	{
	}
	virtual void CSSetUnorderedAccessViews(UINT StartSlot, UINT NumUAVs, ID3D11UnorderedAccessView * const * ppUnorderedAccessViews, const UINT * pUAVInitialCounts) override
	{
	}
	virtual void CSSetShader(ID3D11ComputeShader * pComputeShader, ID3D11ClassInstance * const * ppClassInstances, UINT NumClassInstances) override
	{
	}
	virtual void CSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState * const * ppSamplers) override
	{
	}
	virtual void CSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer * const * ppConstantBuffers) override
	{
	}
	virtual void VSGetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer ** ppConstantBuffers) override
	{
	}
	virtual void PSGetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView ** ppShaderResourceViews) override
	{
	}
	virtual void PSGetShader(ID3D11PixelShader ** ppPixelShader, ID3D11ClassInstance ** ppClassInstances, UINT * pNumClassInstances) override
	{
	}
	virtual void PSGetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState ** ppSamplers) override
	{
	}
	virtual void VSGetShader(ID3D11VertexShader ** ppVertexShader, ID3D11ClassInstance ** ppClassInstances, UINT * pNumClassInstances) override
	{
	}
	virtual void PSGetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer ** ppConstantBuffers) override
	{
	}
	virtual void IAGetInputLayout(ID3D11InputLayout ** ppInputLayout) override
	{
	}
	virtual void IAGetVertexBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer ** ppVertexBuffers, UINT * pStrides, UINT * pOffsets) override
	{
	}
	virtual void IAGetIndexBuffer(ID3D11Buffer ** pIndexBuffer, DXGI_FORMAT * Format, UINT * Offset) override
	{
	}
	virtual void GSGetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer ** ppConstantBuffers) override
	{
	}
	virtual void GSGetShader(ID3D11GeometryShader ** ppGeometryShader, ID3D11ClassInstance ** ppClassInstances, UINT * pNumClassInstances) override
	{
	}
	virtual void IAGetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY * pTopology) override
	{
	}
	virtual void VSGetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView ** ppShaderResourceViews) override
	{
	}
	virtual void VSGetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState ** ppSamplers) override
	{
	}
	virtual void GetPredication(ID3D11Predicate ** ppPredicate, BOOL * pPredicateValue) override
	{
	}
	virtual void GSGetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView ** ppShaderResourceViews) override
	{
	}
	virtual void GSGetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState ** ppSamplers) override
	{
	}
	virtual void OMGetRenderTargets(UINT NumViews, ID3D11RenderTargetView ** ppRenderTargetViews, ID3D11DepthStencilView ** ppDepthStencilView) override
	{
	}
	virtual void OMGetRenderTargetsAndUnorderedAccessViews(UINT NumRTVs, ID3D11RenderTargetView ** ppRenderTargetViews, ID3D11DepthStencilView ** ppDepthStencilView, UINT UAVStartSlot, UINT NumUAVs, ID3D11UnorderedAccessView ** ppUnorderedAccessViews) override
	{
	}
	virtual void OMGetBlendState(ID3D11BlendState ** ppBlendState, FLOAT BlendFactor[4], UINT * pSampleMask) override
	{
	}
	virtual void OMGetDepthStencilState(ID3D11DepthStencilState ** ppDepthStencilState, UINT * pStencilRef) override
	{
	}
	virtual void SOGetTargets(UINT NumBuffers, ID3D11Buffer ** ppSOTargets) override
	{
	}
	virtual void RSGetState(ID3D11RasterizerState ** ppRasterizerState) override
	{
	}
	virtual void RSGetViewports(UINT * pNumViewports, D3D11_VIEWPORT * pViewports) override
	{
	}
	virtual void RSGetScissorRects(UINT * pNumRects, D3D11_RECT * pRects) override
	{
	}
	virtual void HSGetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView ** ppShaderResourceViews) override
	{
	}
	virtual void HSGetShader(ID3D11HullShader ** ppHullShader, ID3D11ClassInstance ** ppClassInstances, UINT * pNumClassInstances) override
	{
	}
	virtual void HSGetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState ** ppSamplers) override
	{
	}
	virtual void HSGetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer ** ppConstantBuffers) override
	{
	}
	virtual void DSGetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView ** ppShaderResourceViews) override
	{
	}
	virtual void DSGetShader(ID3D11DomainShader ** ppDomainShader, ID3D11ClassInstance ** ppClassInstances, UINT * pNumClassInstances) override
	{
	}
	virtual void DSGetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState ** ppSamplers) override
	{
	}
	virtual void DSGetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer ** ppConstantBuffers) override
	{
	}
	virtual void CSGetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView ** ppShaderResourceViews) override
	{
	}
	virtual void CSGetUnorderedAccessViews(UINT StartSlot, UINT NumUAVs, ID3D11UnorderedAccessView ** ppUnorderedAccessViews) override
	{
	}
	virtual void CSGetShader(ID3D11ComputeShader ** ppComputeShader, ID3D11ClassInstance ** ppClassInstances, UINT * pNumClassInstances) override
	{
	}
	virtual void CSGetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState ** ppSamplers) override
	{
	}
	virtual void CSGetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer ** ppConstantBuffers) override
	{
	}
	virtual void ClearState(void) override
	{
	}
	virtual void Flush(void) override
	{
	}
	virtual D3D11_DEVICE_CONTEXT_TYPE GetType(void) override
	{
		return D3D11_DEVICE_CONTEXT_TYPE();
	}
	virtual UINT GetContextFlags(void) override
	{
		return 0;
	}
	virtual HRESULT FinishCommandList(BOOL RestoreDeferredContextState, ID3D11CommandList ** ppCommandList) override
	{
		return E_NOTIMPL;
	}
};

class StubD3D11Buffer : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11Buffer>
{
	D3D11_BUFFER_DESC desc;

public:
	StubD3D11Buffer(D3D11_BUFFER_DESC desc)
		: desc(desc)
	{

	}

	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
	virtual void GetType(D3D11_RESOURCE_DIMENSION * pResourceDimension) override
	{
	}
	virtual void SetEvictionPriority(UINT EvictionPriority) override
	{
	}
	virtual UINT GetEvictionPriority(void) override
	{
		return 0;
	}
	virtual void GetDesc(D3D11_BUFFER_DESC * pDesc) override
	{
		*pDesc = desc;
	}
};

class StubD3D11Texture1D : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11Texture1D>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
	virtual void GetType(D3D11_RESOURCE_DIMENSION * pResourceDimension) override
	{
	}
	virtual void SetEvictionPriority(UINT EvictionPriority) override
	{
	}
	virtual UINT GetEvictionPriority(void) override
	{
		return 0;
	}
	D3D11_TEXTURE1D_DESC desc;

public:
	StubD3D11Texture1D(D3D11_TEXTURE1D_DESC desc)
		: desc(desc)
	{

	}

	virtual void GetDesc(D3D11_TEXTURE1D_DESC * pDesc) override
	{
		*pDesc = desc;
	}
};

class StubD3D11Texture2D : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11Texture2D>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
	virtual void GetType(D3D11_RESOURCE_DIMENSION * pResourceDimension) override
	{
	}
	virtual void SetEvictionPriority(UINT EvictionPriority) override
	{
	}
	virtual UINT GetEvictionPriority(void) override
	{
		return 0;
	}

	D3D11_TEXTURE2D_DESC desc;

public:
	StubD3D11Texture2D(D3D11_TEXTURE2D_DESC desc)
		: desc(desc)
	{

	}

	virtual void GetDesc(D3D11_TEXTURE2D_DESC * pDesc) override
	{
		*pDesc = desc;
	}
};

class StubD3D11Texture3D : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11Texture3D>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
	virtual void GetType(D3D11_RESOURCE_DIMENSION * pResourceDimension) override
	{
	}
	virtual void SetEvictionPriority(UINT EvictionPriority) override
	{
	}
	virtual UINT GetEvictionPriority(void) override
	{
		return 0;
	}

	D3D11_TEXTURE3D_DESC desc;

public:
	StubD3D11Texture3D(D3D11_TEXTURE3D_DESC desc)
		: desc(desc)
	{

	}

	virtual void GetDesc(D3D11_TEXTURE3D_DESC * pDesc) override
	{
		*pDesc = desc;
	}
};

class StubD3D11ShaderResourceView : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11ShaderResourceView>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
	virtual void GetResource(ID3D11Resource ** ppResource) override
	{
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;

public:
	StubD3D11ShaderResourceView(D3D11_SHADER_RESOURCE_VIEW_DESC desc)
		: desc(desc)
	{

	}

	virtual void GetDesc(D3D11_SHADER_RESOURCE_VIEW_DESC * pDesc) override
	{
		*pDesc = desc;
	}
};

class StubD3D11UnorderedAccessView : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11UnorderedAccessView>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
	virtual void GetResource(ID3D11Resource ** ppResource) override
	{
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc;

public:
	StubD3D11UnorderedAccessView(D3D11_UNORDERED_ACCESS_VIEW_DESC desc)
		: desc(desc)
	{

	}
	virtual void GetDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC * pDesc) override
	{
		*pDesc = desc;
	}
};

class StubD3D11RenderTargetView : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11RenderTargetView>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
	virtual void GetResource(ID3D11Resource ** ppResource) override
	{
	}
	D3D11_RENDER_TARGET_VIEW_DESC desc;

public:
	StubD3D11RenderTargetView(D3D11_RENDER_TARGET_VIEW_DESC desc)
		: desc(desc)
	{

	}

	virtual void GetDesc(D3D11_RENDER_TARGET_VIEW_DESC * pDesc) override
	{
		*pDesc = desc;
	}
};

class StubD3D11DepthStencilView : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11DepthStencilView>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
	virtual void GetResource(ID3D11Resource ** ppResource) override
	{
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC desc;

public:
	StubD3D11DepthStencilView(D3D11_DEPTH_STENCIL_VIEW_DESC desc)
		: desc(desc)
	{

	}

	virtual void GetDesc(D3D11_DEPTH_STENCIL_VIEW_DESC * pDesc) override
	{
		*pDesc = desc;
	}
};

class StubD3D11InputLayout : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11InputLayout>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
};

class StubD3D11VertexShader : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11VertexShader>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
};

class StubD3D11GeometryShader : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11GeometryShader>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
};

class StubD3D11HullShader : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11HullShader>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
};

class StubD3D11DomainShader : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11DomainShader>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
};

class StubD3D11PixelShader : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11PixelShader>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
};

class StubD3D11ComputeShader : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11ComputeShader>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
};

class StubD3D11ClassLinkage : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11ClassLinkage>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT GetClassInstance(LPCSTR pClassInstanceName, UINT InstanceIndex, ID3D11ClassInstance ** ppInstance) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT CreateClassInstance(LPCSTR pClassTypeName, UINT ConstantBufferOffset, UINT ConstantVectorOffset, UINT TextureOffset, UINT SamplerOffset, ID3D11ClassInstance ** ppInstance) override
	{
		return E_NOTIMPL;
	}
};

class StubD3D11BlendState : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11BlendState>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
	D3D11_BLEND_DESC desc;

public:
	StubD3D11BlendState(D3D11_BLEND_DESC desc)
		: desc(desc)
	{

	}

	virtual void GetDesc(D3D11_BLEND_DESC * pDesc) override
	{
		*pDesc = desc;
	}
};

class StubD3D11DepthStencilState : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11DepthStencilState>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}

	D3D11_DEPTH_STENCIL_DESC desc;

public:
	StubD3D11DepthStencilState(D3D11_DEPTH_STENCIL_DESC desc)
		: desc(desc)
	{

	}

	virtual void GetDesc(D3D11_DEPTH_STENCIL_DESC * pDesc) override
	{
		*pDesc = desc;
	}
};

class StubD3D11RasterizerState : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11RasterizerState>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}

	D3D11_RASTERIZER_DESC desc;

public:
	StubD3D11RasterizerState(D3D11_RASTERIZER_DESC desc)
		: desc(desc)
	{

	}

	virtual void GetDesc(D3D11_RASTERIZER_DESC * pDesc) override
	{
		*pDesc = desc;
	}
};

class StubD3D11SamplerState : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11SamplerState>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
	virtual void GetDesc(D3D11_SAMPLER_DESC * pDesc) override
	{
		*pDesc = desc;
	}

	D3D11_SAMPLER_DESC desc;

public:
	StubD3D11SamplerState(D3D11_SAMPLER_DESC desc)
		: desc(desc)
	{

	}
};

class StubD3D11Query : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11Query>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
	virtual UINT GetDataSize(void) override
	{
		return 0;
	}
	virtual void GetDesc(D3D11_QUERY_DESC * pDesc) override
	{
	}
};

class StubD3D11Predicate : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11Predicate>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
	virtual UINT GetDataSize(void) override
	{
		return 0;
	}
	virtual void GetDesc(D3D11_QUERY_DESC * pDesc) override
	{
	}
};

class StubD3D11Counter : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11Counter>
{
	// Inherited via RuntimeClass
	virtual void GetDevice(ID3D11Device ** ppDevice) override
	{
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
	virtual UINT GetDataSize(void) override
	{
		return 0;
	}
	virtual void GetDesc(D3D11_COUNTER_DESC * pDesc) override
	{
	}
};

class StubD3D11Device : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, ID3D11Device>
{
public:
	virtual HRESULT CreateBuffer(const D3D11_BUFFER_DESC * pDesc, const D3D11_SUBRESOURCE_DATA * pInitialData, ID3D11Buffer ** ppBuffer) override
	{
		auto buffer = WRL::Make<StubD3D11Buffer>(*pDesc);
		return buffer.CopyTo(ppBuffer);
	}
	virtual HRESULT CreateTexture1D(const D3D11_TEXTURE1D_DESC * pDesc, const D3D11_SUBRESOURCE_DATA * pInitialData, ID3D11Texture1D ** ppTexture1D) override
	{
		auto texture = WRL::Make<StubD3D11Texture1D>(*pDesc);
		return texture.CopyTo(ppTexture1D);
	}
	virtual HRESULT CreateTexture2D(const D3D11_TEXTURE2D_DESC * pDesc, const D3D11_SUBRESOURCE_DATA * pInitialData, ID3D11Texture2D ** ppTexture2D) override
	{
		auto texture = WRL::Make<StubD3D11Texture2D>(*pDesc);
		return texture.CopyTo(ppTexture2D);
	}
	virtual HRESULT CreateTexture3D(const D3D11_TEXTURE3D_DESC * pDesc, const D3D11_SUBRESOURCE_DATA * pInitialData, ID3D11Texture3D ** ppTexture3D) override
	{
		auto texture = WRL::Make<StubD3D11Texture3D>(*pDesc);
		return texture.CopyTo(ppTexture3D);
	}
	virtual HRESULT CreateShaderResourceView(ID3D11Resource * pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC * pDesc, ID3D11ShaderResourceView ** ppSRView) override
	{
		auto obj = WRL::Make<StubD3D11ShaderResourceView>(*pDesc);
		return obj.CopyTo(ppSRView);
	}
	virtual HRESULT CreateUnorderedAccessView(ID3D11Resource * pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC * pDesc, ID3D11UnorderedAccessView ** ppUAView) override
	{
		auto obj = WRL::Make<StubD3D11UnorderedAccessView>(*pDesc);
		return obj.CopyTo(ppUAView);
	}
	virtual HRESULT CreateRenderTargetView(ID3D11Resource * pResource, const D3D11_RENDER_TARGET_VIEW_DESC * pDesc, ID3D11RenderTargetView ** ppRTView) override
	{
		auto obj = WRL::Make<StubD3D11RenderTargetView>(*pDesc);
		return obj.CopyTo(ppRTView);
	}
	virtual HRESULT CreateDepthStencilView(ID3D11Resource * pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC * pDesc, ID3D11DepthStencilView ** ppDepthStencilView) override
	{
		auto obj = WRL::Make<StubD3D11DepthStencilView>(*pDesc);
		return obj.CopyTo(ppDepthStencilView);
	}
	virtual HRESULT CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC * pInputElementDescs, UINT NumElements, const void * pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength, ID3D11InputLayout ** ppInputLayout) override
	{
		auto obj = WRL::Make<StubD3D11InputLayout>();
		return obj.CopyTo(ppInputLayout);
	}
	virtual HRESULT CreateVertexShader(const void * pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage * pClassLinkage, ID3D11VertexShader ** ppVertexShader) override
	{
		auto obj = WRL::Make<StubD3D11VertexShader>();
		return obj.CopyTo(ppVertexShader);
	}
	virtual HRESULT CreateGeometryShader(const void * pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage * pClassLinkage, ID3D11GeometryShader ** ppGeometryShader) override
	{
		auto obj = WRL::Make<StubD3D11GeometryShader>();
		return obj.CopyTo(ppGeometryShader);
	}
	virtual HRESULT CreateGeometryShaderWithStreamOutput(const void * pShaderBytecode, SIZE_T BytecodeLength, const D3D11_SO_DECLARATION_ENTRY * pSODeclaration, UINT NumEntries, const UINT * pBufferStrides, UINT NumStrides, UINT RasterizedStream, ID3D11ClassLinkage * pClassLinkage, ID3D11GeometryShader ** ppGeometryShader) override
	{
		auto obj = WRL::Make<StubD3D11GeometryShader>();
		return obj.CopyTo(ppGeometryShader);
	}
	virtual HRESULT CreatePixelShader(const void * pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage * pClassLinkage, ID3D11PixelShader ** ppPixelShader) override
	{
		auto obj = WRL::Make<StubD3D11PixelShader>();
		return obj.CopyTo(ppPixelShader);
	}
	virtual HRESULT CreateHullShader(const void * pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage * pClassLinkage, ID3D11HullShader ** ppHullShader) override
	{
		auto obj = WRL::Make<StubD3D11HullShader>();
		return obj.CopyTo(ppHullShader);
	}
	virtual HRESULT CreateDomainShader(const void * pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage * pClassLinkage, ID3D11DomainShader ** ppDomainShader) override
	{
		auto obj = WRL::Make<StubD3D11DomainShader>();
		return obj.CopyTo(ppDomainShader);
	}
	virtual HRESULT CreateComputeShader(const void * pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage * pClassLinkage, ID3D11ComputeShader ** ppComputeShader) override
	{
		auto obj = WRL::Make<StubD3D11ComputeShader>();
		return obj.CopyTo(ppComputeShader);
	}
	virtual HRESULT CreateClassLinkage(ID3D11ClassLinkage ** ppLinkage) override
	{
		auto obj = WRL::Make<StubD3D11ClassLinkage>();
		return obj.CopyTo(ppLinkage);
	}
	virtual HRESULT CreateBlendState(const D3D11_BLEND_DESC * pBlendStateDesc, ID3D11BlendState ** ppBlendState) override
	{
		auto obj = WRL::Make<StubD3D11BlendState>(*pBlendStateDesc);
		return obj.CopyTo(ppBlendState);
	}
	virtual HRESULT CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC * pDepthStencilDesc, ID3D11DepthStencilState ** ppDepthStencilState) override
	{
		auto obj = WRL::Make<StubD3D11DepthStencilState>(*pDepthStencilDesc);
		return obj.CopyTo(ppDepthStencilState);
	}
	virtual HRESULT CreateRasterizerState(const D3D11_RASTERIZER_DESC * pRasterizerDesc, ID3D11RasterizerState ** ppRasterizerState) override
	{
		auto obj = WRL::Make<StubD3D11RasterizerState>(*pRasterizerDesc);
		return obj.CopyTo(ppRasterizerState);
	}
	virtual HRESULT CreateSamplerState(const D3D11_SAMPLER_DESC * pSamplerDesc, ID3D11SamplerState ** ppSamplerState) override
	{
		auto obj = WRL::Make<StubD3D11SamplerState>(*pSamplerDesc);
		return obj.CopyTo(ppSamplerState);
	}
	virtual HRESULT CreateQuery(const D3D11_QUERY_DESC * pQueryDesc, ID3D11Query ** ppQuery) override
	{
		auto obj = WRL::Make<StubD3D11Query>();
		return obj.CopyTo(ppQuery);
	}
	virtual HRESULT CreatePredicate(const D3D11_QUERY_DESC * pPredicateDesc, ID3D11Predicate ** ppPredicate) override
	{
		auto obj = WRL::Make<StubD3D11Predicate>();
		return obj.CopyTo(ppPredicate);
	}
	virtual HRESULT CreateCounter(const D3D11_COUNTER_DESC * pCounterDesc, ID3D11Counter ** ppCounter) override
	{
		auto obj = WRL::Make<StubD3D11Counter>();
		return obj.CopyTo(ppCounter);
	}
	virtual HRESULT CreateDeferredContext(UINT ContextFlags, ID3D11DeviceContext ** ppDeferredContext) override
	{
		auto obj = WRL::Make<StubD3D11DeviceContext>();
		return obj.CopyTo(ppDeferredContext);
	}
	virtual HRESULT OpenSharedResource(HANDLE hResource, REFIID ReturnedInterface, void ** ppResource) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT CheckFormatSupport(DXGI_FORMAT Format, UINT * pFormatSupport) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT CheckMultisampleQualityLevels(DXGI_FORMAT Format, UINT SampleCount, UINT * pNumQualityLevels) override
	{
		return E_NOTIMPL;
	}
	virtual void CheckCounterInfo(D3D11_COUNTER_INFO * pCounterInfo) override
	{
	}
	virtual HRESULT CheckCounter(const D3D11_COUNTER_DESC * pDesc, D3D11_COUNTER_TYPE * pType, UINT * pActiveCounters, LPSTR szName, UINT * pNameLength, LPSTR szUnits, UINT * pUnitsLength, LPSTR szDescription, UINT * pDescriptionLength) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT CheckFeatureSupport(D3D11_FEATURE Feature, void * pFeatureSupportData, UINT FeatureSupportDataSize) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT GetPrivateData(REFGUID guid, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateData(REFGUID guid, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID guid, const IUnknown * pData) override
	{
		return E_NOTIMPL;
	}
	virtual D3D_FEATURE_LEVEL GetFeatureLevel(void) override
	{
		return D3D_FEATURE_LEVEL();
	}
	virtual UINT GetCreationFlags(void) override
	{
		return 0;
	}
	virtual HRESULT GetDeviceRemovedReason(void) override
	{
		return S_OK;
	}
	virtual void GetImmediateContext(ID3D11DeviceContext ** ppImmediateContext) override
	{
	}
	virtual HRESULT SetExceptionMode(UINT RaiseFlags) override
	{
		return S_OK;
	}
	virtual UINT GetExceptionMode(void) override
	{
		return 0;
	}
};

class StubDXGISwapChain : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IDXGISwapChain>
{
	// Inherited via RuntimeClass
	virtual HRESULT SetPrivateData(REFGUID Name, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID Name, const IUnknown * pUnknown) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT GetPrivateData(REFGUID Name, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT GetParent(REFIID riid, void ** ppParent) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT GetDevice(REFIID riid, void ** ppDevice) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT Present(UINT SyncInterval, UINT Flags) override
	{
		return S_OK;
	}
	virtual HRESULT GetBuffer(UINT Buffer, REFIID riid, void ** ppSurface) override
	{
		D3D11_TEXTURE2D_DESC desc = { 0 };
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.Width = 256;
		desc.Height = 256;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.SampleDesc.Count = 1;

		auto s = WRL::Make<StubD3D11Texture2D>(desc);
		return s.CopyTo(riid, ppSurface);
	}
	virtual HRESULT SetFullscreenState(BOOL Fullscreen, IDXGIOutput * pTarget) override
	{
		return S_OK;
	}
	virtual HRESULT GetFullscreenState(BOOL * pFullscreen, IDXGIOutput ** ppTarget) override
	{
		return S_OK;
	}

public:
	DXGI_SWAP_CHAIN_DESC desc;

	StubDXGISwapChain(DXGI_SWAP_CHAIN_DESC desc)
		: desc(desc)
	{

	}

	virtual HRESULT GetDesc(DXGI_SWAP_CHAIN_DESC * pDesc) override
	{
		*pDesc = desc;
		return S_OK;
	}
	virtual HRESULT ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) override
	{
		return S_OK;
	}
	virtual HRESULT ResizeTarget(const DXGI_MODE_DESC * pNewTargetParameters) override
	{
		return S_OK;
	}
	virtual HRESULT GetContainingOutput(IDXGIOutput ** ppOutput) override
	{
		return S_OK;
	}
	virtual HRESULT GetFrameStatistics(DXGI_FRAME_STATISTICS * pStats) override
	{
		return S_OK;
	}
	virtual HRESULT GetLastPresentCount(UINT * pLastPresentCount) override
	{
		return S_OK;
	}
};

void MakeDummyDevice(ID3D11Device** device, ID3D11DeviceContext** context, const DXGI_SWAP_CHAIN_DESC* desc, IDXGISwapChain** swapChain)
{
	auto d = WRL::Make<StubD3D11Device>();
	auto c = WRL::Make<StubD3D11DeviceContext>();
	auto s = WRL::Make<StubDXGISwapChain>(*desc);

	d.CopyTo(device);
	c.CopyTo(context);
	s.CopyTo(swapChain);
}
