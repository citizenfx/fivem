#pragma once

#include <iostream>
#include <string>

#ifdef _WIN32
#include <iwscapi.h>
#include <wscapi.h>
#include <versionhelpers.h>
#include <wrl.h>

using namespace Microsoft;

#endif

namespace os
{
std::wstring GetActiveFirewallName(std::wstring defaultName = L"Unknown")
{
#ifdef _WIN32_WINNT >= 0x0602 // Need at least windows 8 to compile this

	// IWSCProductList and IWscProduct are Windows 8+ Desktop
	if (IsWindows8OrGreater())
	{
		CoInitializeEx(0, COINIT_APARTMENTTHREADED);

		{
			Microsoft::WRL::ComPtr<IWSCProductList> productList;
			if (SUCCEEDED(CoCreateInstance(__uuidof(WSCProductList), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(productList.GetAddressOf()))))
			{
				LONG productCount = 0;
				if (SUCCEEDED(productList->Initialize(WSC_SECURITY_PROVIDER_FIREWALL)) && SUCCEEDED(productList->get_Count(&productCount)))
				{
					for (ULONG i = 0; i < productCount; ++i)
					{
						Microsoft::WRL::ComPtr<IWscProduct> product;
						WSC_SECURITY_PRODUCT_STATE state;
						BSTR bstrName;

						if (SUCCEEDED(productList->get_Item(i, product.GetAddressOf()))
							&& SUCCEEDED(product->get_ProductState(&state))
							&& state == WSC_SECURITY_PRODUCT_STATE_ON
							&& SUCCEEDED(product->get_ProductName(&bstrName)))
						{
							defaultName = std::wstring((wchar_t*)bstrName, SysStringLen(bstrName));

							SysFreeString(bstrName);

							break;
						}
					}
				}
			}
		}

		CoUninitialize();
	}
#endif

	return defaultName;
}
}
