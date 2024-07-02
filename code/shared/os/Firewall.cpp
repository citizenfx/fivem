#include "Firewall.h"

namespace fx::os
{
std::wstring Firewall::GetActiveName(std::wstring_view defaultName)
{
#ifdef _WIN32_WINNT >= 0x0602 // Need at least windows 8 to compile this

	using namespace Microsoft;

	// IWSCProductList and IWscProduct are Windows 8+ Desktop
	if (IsWindows8OrGreater())
	{
		(void)CoInitializeEx(0, COINIT_APARTMENTTHREADED);

		{
			WRL::ComPtr<IWSCProductList> productList;
			if (SUCCEEDED(CoCreateInstance(__uuidof(WSCProductList), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(productList.GetAddressOf()))))
			{
				LONG productCount = 0;
				if (SUCCEEDED(productList->Initialize(WSC_SECURITY_PROVIDER_FIREWALL)) && SUCCEEDED(productList->get_Count(&productCount)))
				{
					for (ULONG i = 0; i < productCount; ++i)
					{
						WRL::ComPtr<IWscProduct> product;
						WSC_SECURITY_PRODUCT_STATE state;
						BSTR bstrName;

						if (SUCCEEDED(productList->get_Item(i, product.GetAddressOf()))
							&& SUCCEEDED(product->get_ProductState(&state))
							&& state == WSC_SECURITY_PRODUCT_STATE_ON
							&& SUCCEEDED(product->get_ProductName(&bstrName)))
						{
							auto name = std::wstring((wchar_t*)bstrName, SysStringLen(bstrName));
							SysFreeString(bstrName);

							return name;
						}
					}
				}
			}
		}

		CoUninitialize();
	}
#endif

	return std::wstring(defaultName);
}
}
