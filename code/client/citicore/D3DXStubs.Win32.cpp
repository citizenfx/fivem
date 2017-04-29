#include <StdInc.h>
#include <DirectXMath.h>

#include <d3dcompiler.h>

using namespace DirectX;

using D3DXMATRIX = XMFLOAT4X4;
using D3DXVECTOR3 = XMFLOAT3;
using D3DXVECTOR4 = XMFLOAT4;

extern "C"
{
	__declspec(dllexport) D3DXMATRIX* WINAPI D3DXMatrixMultiply(D3DXMATRIX *pOut, CONST D3DXMATRIX *pM1, CONST D3DXMATRIX *pM2)
	{
		XMStoreFloat4x4(pOut, XMMatrixMultiply(XMLoadFloat4x4(pM1), XMLoadFloat4x4(pM2)));

		return pOut;
	}

	__declspec(dllexport) D3DXMATRIX* WINAPI D3DXMatrixLookAtLH(D3DXMATRIX *pOut, CONST D3DXVECTOR3 *pEye, CONST D3DXVECTOR3 *pAt, CONST D3DXVECTOR3 *pUp)
	{
		XMStoreFloat4x4(pOut, XMMatrixLookAtLH(XMLoadFloat3(pEye), XMLoadFloat3(pAt), XMLoadFloat3(pUp)));

		return pOut;
	}
	
	__declspec(dllexport) D3DXMATRIX* WINAPI D3DXMatrixPerspectiveOffCenterLH(D3DXMATRIX *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn, FLOAT zf)
	{
		XMStoreFloat4x4(pOut, XMMatrixPerspectiveOffCenterLH(l, r, b, t, zn, zf));

		return pOut;
	}

	__declspec(dllexport) D3DXMATRIX* WINAPI D3DXMatrixOrthoOffCenterLH(D3DXMATRIX *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn, FLOAT zf)
	{
		XMStoreFloat4x4(pOut, XMMatrixOrthographicOffCenterLH(l, r, b, t, zn, zf));

		return pOut;
	}

	__declspec(dllexport) D3DXVECTOR3* WINAPI D3DXVec3TransformCoord(D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DXMATRIX *pM)
	{
		XMStoreFloat3(pOut, XMVector3TransformCoord(XMLoadFloat3(pV), XMLoadFloat4x4(pM)));

		return pOut;
	}

	__declspec(dllexport) D3DXVECTOR4* WINAPI D3DXVec4Transform(D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV, CONST D3DXMATRIX *pM)
	{
		XMStoreFloat4(pOut, XMVector4Transform(XMLoadFloat4(pV), XMLoadFloat4x4(pM)));

		return pOut;
	}

	__declspec(dllexport) D3DXMATRIX* WINAPI D3DXMatrixInverse(D3DXMATRIX *pOut, FLOAT *pDeterminant, CONST D3DXMATRIX *pM)
	{
		XMVECTOR determinant;

		XMStoreFloat4x4(pOut, XMMatrixInverse(&determinant, XMLoadFloat4x4(pM)));

		if (pDeterminant)
		{
			XMStoreFloat(pDeterminant, determinant);
		}

		return pOut;
	}
}