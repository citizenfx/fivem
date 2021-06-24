#include <StdInc.h>
#include "DiagLocal.h"

#include <wscapi.h>
#include <iwscapi.h>

#include <OleAuto.h>

// from https://github.com/Microsoft/Windows-classic-samples/blob/master/Samples/WebSecurityCenter/cpp/WscApiSample.cpp

static std::string GetSecurityCenterInfo(WSC_SECURITY_PROVIDER provider)
{
	std::stringstream outStream;

	HRESULT                         hr = S_OK;
	IWscProduct* PtrProduct = nullptr;
	IWSCProductList* PtrProductList = nullptr;
	BSTR                            PtrVal = nullptr;
	LONG                            ProductCount = 0;
	WSC_SECURITY_PRODUCT_STATE      ProductState;
	WSC_SECURITY_SIGNATURE_STATUS   ProductStatus;

	if (provider != WSC_SECURITY_PROVIDER_FIREWALL &&
		provider != WSC_SECURITY_PROVIDER_ANTIVIRUS &&
		provider != WSC_SECURITY_PROVIDER_ANTISPYWARE)
	{
		hr = E_INVALIDARG;
		goto exit;
	}

	//
	// Initialize can only be called once per instance, so you need to
	// CoCreateInstance for each security product type you want to query.
	//
	hr = CoCreateInstance(
		__uuidof(WSCProductList),
		NULL,
		CLSCTX_INPROC_SERVER,
		__uuidof(IWSCProductList),
		reinterpret_cast<LPVOID*> (&PtrProductList));
	if (FAILED(hr))
	{
		outStream << ToNarrow(fmt::sprintf(L"CoCreateInstance returned error = 0x%d \n", hr));
		goto exit;
	}

	//
	// Initialize the product list with the type of security product you're 
	// interested in.
	//
	hr = PtrProductList->Initialize(provider);
	if (FAILED(hr))
	{
		outStream << ToNarrow(fmt::sprintf(L"Initialize failed with error: 0x%d\n", hr));
		goto exit;
	}

	//
	// Get the number of security products of that type.
	//
	hr = PtrProductList->get_Count(&ProductCount);
	if (FAILED(hr))
	{
		outStream << ToNarrow(fmt::sprintf(L"get_Count failed with error: 0x%d\n", hr));
		goto exit;
	}

	if (provider == WSC_SECURITY_PROVIDER_FIREWALL)
	{
		outStream << ToNarrow(fmt::sprintf(L"\n\nFirewall Products:\n"));
	}
	else if (provider == WSC_SECURITY_PROVIDER_ANTIVIRUS)
	{
		outStream << ToNarrow(fmt::sprintf(L"\n\nAntivirus Products:\n"));
	}
	else
	{
		outStream << ToNarrow(fmt::sprintf(L"\n\nAntispyware Products:\n"));
	}

	//
	// Loop over each product, querying the specific attributes.
	//
	for (LONG i = 0; i < ProductCount; i++)
	{
		//
		// Get the next security product
		//
		hr = PtrProductList->get_Item(i, &PtrProduct);
		if (FAILED(hr))
		{
			outStream << ToNarrow(fmt::sprintf(L"get_Item failed with error: 0x%d\n", hr));
			goto exit;
		}

		//
		// Get the product name
		//
		hr = PtrProduct->get_ProductName(&PtrVal);
		if (FAILED(hr))
		{
			outStream << ToNarrow(fmt::sprintf(L"get_ProductName failed with error: 0x%d\n", hr));
			goto exit;
		}
		outStream << ToNarrow(fmt::sprintf(L"\nProduct name: %s\n", PtrVal));
		// Caller is responsible for freeing the string
		SysFreeString(PtrVal);
		PtrVal = nullptr;

		//
		// Get the product state
		//
		hr = PtrProduct->get_ProductState(&ProductState);
		if (FAILED(hr))
		{
			outStream << ToNarrow(fmt::sprintf(L"get_ProductState failed with error: 0x%d\n", hr));
			goto exit;
		}

		LPWSTR pszState;
		if (ProductState == WSC_SECURITY_PRODUCT_STATE_ON)
		{
			pszState = L"On";
		}
		else if (ProductState == WSC_SECURITY_PRODUCT_STATE_OFF)
		{
			pszState = L"Off";
		}
		else if (ProductState == WSC_SECURITY_PRODUCT_STATE_SNOOZED)
		{
			pszState = L"Snoozed";
		}
		else
		{
			pszState = L"Expired";
		}
		outStream << ToNarrow(fmt::sprintf(L"Product state: %s\n", pszState));

		//
		// Get the signature status (not applicable to firewall products)
		//
		if (provider != WSC_SECURITY_PROVIDER_FIREWALL)
		{
			hr = PtrProduct->get_SignatureStatus(&ProductStatus);
			if (FAILED(hr))
			{
				outStream << ToNarrow(fmt::sprintf(L"get_SignatureStatus failed with error: 0x%d\n", hr));
				goto exit;
			}
			LPWSTR pszStatus = (ProductStatus == WSC_SECURITY_PRODUCT_UP_TO_DATE) ?
				L"Up-to-date" : L"Out-of-date";
			outStream << ToNarrow(fmt::sprintf(L"Product status: %s\n", pszStatus));
		}

		//
		// Get the remediation path for the security product
		//
		hr = PtrProduct->get_RemediationPath(&PtrVal);
		if (FAILED(hr))
		{
			wprintf(L"get_RemediationPath failed with error: 0x%d\n", hr);
			goto exit;
		}
		outStream << ToNarrow(fmt::sprintf(L"Product remediation path: %s\n", PtrVal));
		// Caller is responsible for freeing the string
		SysFreeString(PtrVal);
		PtrVal = nullptr;

		//
		// Get the product state timestamp (updated when product changes its 
		// state), and only applicable for AV products (NULL is returned for
		// AS and FW products)
		//
		if (provider == WSC_SECURITY_PROVIDER_ANTIVIRUS)
		{
			hr = PtrProduct->get_ProductStateTimestamp(&PtrVal);
			if (FAILED(hr))
			{
				wprintf(L"get_ProductStateTimestamp failed with error: 0x%d\n", hr);
				goto exit;
			}
			outStream << ToNarrow(fmt::sprintf(L"Product state timestamp: %s\n", PtrVal));
			// Caller is responsible for freeing the string
			SysFreeString(PtrVal);
			PtrVal = nullptr;
		}

		PtrProduct->Release();
		PtrProduct = nullptr;
	}

exit:

	if (nullptr != PtrVal)
	{
		SysFreeString(PtrVal);
	}
	if (nullptr != PtrProductList)
	{
		PtrProductList->Release();
	}
	if (nullptr != PtrProduct)
	{
		PtrProduct->Release();
	}

	return outStream.str();
}

static InitFunction initFunction([]()
{
	RegisterCfxDiagnosticsCommand("SecurityCenter", [](const std::shared_ptr<CfxDiagnosticsOutputBuffer> & buffer)
	{
		buffer->AppendText(GetSecurityCenterInfo(WSC_SECURITY_PROVIDER_FIREWALL));
		buffer->AppendText(GetSecurityCenterInfo(WSC_SECURITY_PROVIDER_ANTIVIRUS));
		buffer->AppendText(GetSecurityCenterInfo(WSC_SECURITY_PROVIDER_ANTISPYWARE));
	});
});
