#include <StdInc.h>

#include <Shlwapi.h>
#include <shobjidl.h>

#if !defined(_M_IX86)
#include <wrl.h>

#include <wbemidl.h>
#include <propvarutil.h>

#include <variant>

#ifdef trace
#undef trace
#define trace(...)
#endif

namespace WRL = Microsoft::WRL;

enum class QueryType
{
	DiskDrive,
	OperatingSystem,
	BaseBoard,
	VideoController,
	IDEController,
	Processor,
};

class WbemClassObject : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IWbemClassObject, IClientSecurity>
{
	QueryType type;
	int idx;

	std::map<std::wstring, std::variant<std::wstring>> m_members;

public:
	WbemClassObject(QueryType type, int idx)
		: type(type), idx(idx)
	{
		Init();
	}

	void Init()
	{
		if (type == QueryType::DiskDrive)
		{
			m_members[L"SerialNumber"] = L"000000000000";
		}
		else if (type == QueryType::OperatingSystem)
		{
			// Microsoft and Windows are registered trademarks of Microsoft Corporation
			// Pro is a registered trademark of Apple Inc.
			m_members[L"Caption"] = L"Microsoft Windows 10 Pro";
		}
		else if (type == QueryType::BaseBoard)
		{
			m_members[L"Product"] = L"Microsoft Corporation Virtual Machine";
		}
		else if (type == QueryType::VideoController)
		{
			m_members[L"PNPDeviceID"] = L"PCI\\VEN_10DE&DEV_2204&SUBSYS_161319DA&REV_A1";
		}
		else if (type == QueryType::Processor)
		{
			// AMD, Ryzen and 'Processor' are registered trademarks of Advanced Micro Devices
			m_members[L"Name"] = L"AMD Ryzen 7 5800X 8-Core Processor";
		}
	}

	// Inherited via RuntimeClass
	virtual HRESULT __stdcall GetQualifierSet(IWbemQualifierSet** ppQualSet) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall Get(LPCWSTR wszName, long lFlags, VARIANT* pVal, CIMTYPE* pType, long* plFlavor) override
	{
		if (auto it = m_members.find(wszName); it != m_members.end())
		{
			if (it->second.index() == 0)
			{
				InitVariantFromString(std::get<std::wstring>(it->second).c_str(), pVal);
			}

			return S_OK;
		}

		return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
	}
	virtual HRESULT __stdcall Put(LPCWSTR wszName, long lFlags, VARIANT* pVal, CIMTYPE Type) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall Delete(LPCWSTR wszName) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetNames(LPCWSTR wszQualifierName, long lFlags, VARIANT* pQualifierVal, SAFEARRAY** pNames) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall BeginEnumeration(long lEnumFlags) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall Next(long lFlags, BSTR* strName, VARIANT* pVal, CIMTYPE* pType, long* plFlavor) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EndEnumeration(void) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetPropertyQualifierSet(LPCWSTR wszProperty, IWbemQualifierSet** ppQualSet) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall Clone(IWbemClassObject** ppCopy) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetObjectText(long lFlags, BSTR* pstrObjectText) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall SpawnDerivedClass(long lFlags, IWbemClassObject** ppNewClass) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall SpawnInstance(long lFlags, IWbemClassObject** ppNewInstance) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall CompareTo(long lFlags, IWbemClassObject* pCompareTo) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetPropertyOrigin(LPCWSTR wszName, BSTR* pstrClassName) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall InheritsFrom(LPCWSTR strAncestor) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetMethod(LPCWSTR wszName, long lFlags, IWbemClassObject** ppInSignature, IWbemClassObject** ppOutSignature) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall PutMethod(LPCWSTR wszName, long lFlags, IWbemClassObject* pInSignature, IWbemClassObject* pOutSignature) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall DeleteMethod(LPCWSTR wszName) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall BeginMethodEnumeration(long lEnumFlags) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall NextMethod(long lFlags, BSTR* pstrName, IWbemClassObject** ppInSignature, IWbemClassObject** ppOutSignature) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall EndMethodEnumeration(void) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetMethodQualifierSet(LPCWSTR wszMethod, IWbemQualifierSet** ppQualSet) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetMethodOrigin(LPCWSTR wszMethodName, BSTR* pstrClassName) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall QueryBlanket(IUnknown* pProxy, DWORD* pAuthnSvc, DWORD* pAuthzSvc, OLECHAR** pServerPrincName, DWORD* pAuthnLevel, DWORD* pImpLevel, void** pAuthInfo, DWORD* pCapabilites) override
	{
		return S_OK;
	}
	virtual HRESULT __stdcall SetBlanket(IUnknown* pProxy, DWORD dwAuthnSvc, DWORD dwAuthzSvc, OLECHAR* pServerPrincName, DWORD dwAuthnLevel, DWORD dwImpLevel, void* pAuthInfo, DWORD dwCapabilities) override
	{
		return S_OK;
	}
	virtual HRESULT __stdcall CopyProxy(IUnknown* pProxy, IUnknown** ppCopy) override
	{
		return E_NOTIMPL;
	}
};

class EnumWbemClassObject : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IEnumWbemClassObject, IClientSecurity>
{
	QueryType type;
	int i = 0;
	int num = 1;

public:
	EnumWbemClassObject(QueryType type)
		: type(type)
	{
		if (type == QueryType::IDEController)
		{
			// we only have NVMe disk
			num = 0;
		}
	}

	// Inherited via RuntimeClass
	virtual HRESULT __stdcall Reset(void) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall Next(long lTimeout, ULONG uCount, IWbemClassObject** apObjects, ULONG* puReturned) override
	{
		if (i >= num)
		{
			*puReturned = 0;
			return S_FALSE;
		}

		int n = 0;

		while (n < uCount)
		{
			auto obj = WRL::Make<WbemClassObject>(type, i);
			obj.CopyTo(&apObjects[n]);

			n++;
			i++;
		}

		*puReturned = n;
		return S_OK;
	}
	virtual HRESULT __stdcall NextAsync(ULONG uCount, IWbemObjectSink* pSink) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall Clone(IEnumWbemClassObject** ppEnum) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall Skip(long lTimeout, ULONG nCount) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall QueryBlanket(IUnknown* pProxy, DWORD* pAuthnSvc, DWORD* pAuthzSvc, OLECHAR** pServerPrincName, DWORD* pAuthnLevel, DWORD* pImpLevel, void** pAuthInfo, DWORD* pCapabilites) override
	{
		return S_OK;
	}
	virtual HRESULT __stdcall SetBlanket(IUnknown* pProxy, DWORD dwAuthnSvc, DWORD dwAuthzSvc, OLECHAR* pServerPrincName, DWORD dwAuthnLevel, DWORD dwImpLevel, void* pAuthInfo, DWORD dwCapabilities) override
	{
		return S_OK;
	}
	virtual HRESULT __stdcall CopyProxy(IUnknown* pProxy, IUnknown** ppCopy) override
	{
		return E_NOTIMPL;
	}
};

class WbemServices : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IWbemServices, IClientSecurity>
{
public:
	virtual HRESULT __stdcall OpenNamespace(const BSTR strNamespace, long lFlags, IWbemContext* pCtx, IWbemServices** ppWorkingNamespace, IWbemCallResult** ppResult) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall CancelAsyncCall(IWbemObjectSink* pSink) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall QueryObjectSink(long lFlags, IWbemObjectSink** ppResponseHandler) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetObjectAsync(const BSTR strObjectPath, long lFlags, IWbemContext* pCtx, IWbemObjectSink* pResponseHandler) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall PutClass(IWbemClassObject* pObject, long lFlags, IWbemContext* pCtx, IWbemCallResult** ppCallResult) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall PutClassAsync(IWbemClassObject* pObject, long lFlags, IWbemContext* pCtx, IWbemObjectSink* pResponseHandler) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall DeleteClass(const BSTR strClass, long lFlags, IWbemContext* pCtx, IWbemCallResult** ppCallResult) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall DeleteClassAsync(const BSTR strClass, long lFlags, IWbemContext* pCtx, IWbemObjectSink* pResponseHandler) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall CreateClassEnum(const BSTR strSuperclass, long lFlags, IWbemContext* pCtx, IEnumWbemClassObject** ppEnum) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall CreateClassEnumAsync(const BSTR strSuperclass, long lFlags, IWbemContext* pCtx, IWbemObjectSink* pResponseHandler) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall PutInstance(IWbemClassObject* pInst, long lFlags, IWbemContext* pCtx, IWbemCallResult** ppCallResult) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall PutInstanceAsync(IWbemClassObject* pInst, long lFlags, IWbemContext* pCtx, IWbemObjectSink* pResponseHandler) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall DeleteInstance(const BSTR strObjectPath, long lFlags, IWbemContext* pCtx, IWbemCallResult** ppCallResult) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall DeleteInstanceAsync(const BSTR strObjectPath, long lFlags, IWbemContext* pCtx, IWbemObjectSink* pResponseHandler) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall CreateInstanceEnum(const BSTR strFilter, long lFlags, IWbemContext* pCtx, IEnumWbemClassObject** ppEnum) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall CreateInstanceEnumAsync(const BSTR strFilter, long lFlags, IWbemContext* pCtx, IWbemObjectSink* pResponseHandler) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall ExecQuery(const BSTR strQueryLanguage, const BSTR strQuery, long lFlags, IWbemContext* pCtx, IEnumWbemClassObject** ppEnum) override
	{
		trace("WMI: ExecQuery %s\n", ToNarrow(strQuery));

		if (wcscmp(strQuery, L"SELECT * FROM Win32_DiskDrive") == 0)
		{
			auto query = WRL::Make<EnumWbemClassObject>(QueryType::DiskDrive);
			query.CopyTo(ppEnum);
		}
		else if (wcscmp(strQuery, L"SELECT * FROM Win32_OperatingSystem") == 0)
		{
			auto query = WRL::Make<EnumWbemClassObject>(QueryType::OperatingSystem);
			query.CopyTo(ppEnum);
		}
		else if (wcscmp(strQuery, L"SELECT * FROM Win32_BaseBoard") == 0)
		{
			auto query = WRL::Make<EnumWbemClassObject>(QueryType::BaseBoard);
			query.CopyTo(ppEnum);
		}
		else if (wcsstr(strQuery, L"SELECT * FROM Win32_VideoController") != nullptr)
		{
			auto query = WRL::Make<EnumWbemClassObject>(QueryType::VideoController);
			query.CopyTo(ppEnum);
		}
		else if (wcsstr(strQuery, L"SELECT * FROM Win32_IDEController") != nullptr)
		{
			auto query = WRL::Make<EnumWbemClassObject>(QueryType::IDEController);
			query.CopyTo(ppEnum);
		}
		else if (wcsstr(strQuery, L"SELECT * FROM Win32_Processor") != nullptr)
		{
			auto query = WRL::Make<EnumWbemClassObject>(QueryType::Processor);
			query.CopyTo(ppEnum);
		}

		return S_OK;
	}
	virtual HRESULT __stdcall ExecQueryAsync(const BSTR strQueryLanguage, const BSTR strQuery, long lFlags, IWbemContext* pCtx, IWbemObjectSink* pResponseHandler) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall ExecNotificationQuery(const BSTR strQueryLanguage, const BSTR strQuery, long lFlags, IWbemContext* pCtx, IEnumWbemClassObject** ppEnum) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall ExecNotificationQueryAsync(const BSTR strQueryLanguage, const BSTR strQuery, long lFlags, IWbemContext* pCtx, IWbemObjectSink* pResponseHandler) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall ExecMethod(const BSTR strObjectPath, const BSTR strMethodName, long lFlags, IWbemContext* pCtx, IWbemClassObject* pInParams, IWbemClassObject** ppOutParams, IWbemCallResult** ppCallResult) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall ExecMethodAsync(const BSTR strObjectPath, const BSTR strMethodName, long lFlags, IWbemContext* pCtx, IWbemClassObject* pInParams, IWbemObjectSink* pResponseHandler) override
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}
	virtual HRESULT GetObjectW(BSTR strObjectPath, long lFlags, IWbemContext* pCtx, IWbemClassObject** ppObject, IWbemCallResult** ppCallResult)
	{
		trace("WMI: %s\n", __func__);
		return E_NOTIMPL;
	}

	// Inherited via IClientSecurity
	virtual HRESULT __stdcall QueryBlanket(IUnknown* pProxy, DWORD* pAuthnSvc, DWORD* pAuthzSvc, OLECHAR** pServerPrincName, DWORD* pAuthnLevel, DWORD* pImpLevel, void** pAuthInfo, DWORD* pCapabilites) override
	{
		return S_OK;
	}
	virtual HRESULT __stdcall SetBlanket(IUnknown* pProxy, DWORD dwAuthnSvc, DWORD dwAuthzSvc, OLECHAR* pServerPrincName, DWORD dwAuthnLevel, DWORD dwImpLevel, void* pAuthInfo, DWORD dwCapabilities) override
	{
		return S_OK;
	}
	virtual HRESULT __stdcall CopyProxy(IUnknown* pProxy, IUnknown** ppCopy) override
	{
		return S_OK;
	}
};

class WbemLocator : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IWbemLocator>
{
public:
	virtual HRESULT ConnectServer(
	const BSTR strNetworkResource,
	const BSTR strUser,
	const BSTR strPassword,
	const BSTR strLocale,
	long lSecurityFlags,
	const BSTR strAuthority,
	IWbemContext* pCtx,
	IWbemServices** ppNamespace)
	{
		auto services = WRL::Make<WbemServices>();
		services.CopyTo(ppNamespace);

		return S_OK;
	}
};
#endif

HRESULT WINAPI CoCreateInstanceStub(_In_ REFCLSID rclsid, _In_opt_ LPUNKNOWN pUnkOuter, _In_ DWORD dwClsContext, _In_ REFIID riid, _COM_Outptr_ _At_(*ppv, _Post_readable_size_(_Inexpressible_(varies))) LPVOID FAR* ppv)
{
#if !defined(GTA_NY)
	if (riid == __uuidof(IWbemLocator))
	{
		auto locator = WRL::Make<WbemLocator>();
		locator.CopyTo(riid, ppv);

		return S_OK;
	}
#endif

	if (getenv("CitizenFX_ToolMode") && riid == __uuidof(IShellLink))
	{
		return E_NOINTERFACE;
	}

	return CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}

BOOL __stdcall CreateProcessAStub(_In_opt_ LPCSTR lpApplicationName, _Inout_opt_ LPSTR lpCommandLine, _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes, _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes, _In_ BOOL bInheritHandles, _In_ DWORD dwCreationFlags, _In_opt_ LPVOID lpEnvironment, _In_opt_ LPCSTR lpCurrentDirectory, _In_ LPSTARTUPINFOA lpStartupInfo, _Out_ LPPROCESS_INFORMATION lpProcessInformation)
{
	std::string fakeData;

	if (lpCommandLine)
	{
		if (StrStrIA(lpCommandLine, "wmic.exe") != nullptr)
		{
			fakeData = R"(UUID                                  
12345678-9abc-0000-0000-000000000000  
)";
		}
		else if (StrStrIA(lpCommandLine, "hostname.exe") != nullptr)
		{
			fakeData = R"(PHONE-NGNFJS
)";
		}
		else if (StrStrIA(lpCommandLine, "whoami.exe") != nullptr)
		{
			fakeData = R"(phone-ngnfjs\admin
)";
		}
		else if (StrStrIA(lpCommandLine, "dxdiag") != nullptr)
		{
			fakeData = R"(
)";
		}
	}

	if (!fakeData.empty())
	{
		DWORD bw;
		WriteFile(lpStartupInfo->hStdOutput, fakeData.data(), fakeData.size() * sizeof(fakeData[0]), &bw, NULL);

		HANDLE hEvent = CreateEventW(NULL, TRUE, TRUE, NULL);
		HANDLE hEvent2 = CreateEventW(NULL, TRUE, TRUE, NULL);

		lpProcessInformation->hProcess = hEvent;
		lpProcessInformation->hThread = hEvent2;
		lpProcessInformation->dwProcessId = 8;
		lpProcessInformation->dwThreadId = 12;
		return TRUE;
	}

	return CreateProcessA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}

BOOL __stdcall CreateProcessWStub(_In_opt_ LPCWSTR lpApplicationName, _Inout_opt_ LPWSTR lpCommandLine, _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes, _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes, _In_ BOOL bInheritHandles, _In_ DWORD dwCreationFlags, _In_opt_ LPVOID lpEnvironment, _In_opt_ LPCWSTR lpCurrentDirectory, _In_ LPSTARTUPINFOW lpStartupInfo, _Out_ LPPROCESS_INFORMATION lpProcessInformation)
{
	std::string fakeData;

	if (lpCommandLine)
	{
		if (StrStrIW(lpCommandLine, L"dxdiag") != nullptr)
		{
			fakeData = R"(
)";
		}
	}

	if (!fakeData.empty())
	{
		DWORD bw;
		WriteFile(lpStartupInfo->hStdOutput, fakeData.data(), fakeData.size() * sizeof(fakeData[0]), &bw, NULL);

		HANDLE hEvent = CreateEventW(NULL, TRUE, TRUE, NULL);
		HANDLE hEvent2 = CreateEventW(NULL, TRUE, TRUE, NULL);

		lpProcessInformation->hProcess = hEvent;
		lpProcessInformation->hThread = hEvent2;
		lpProcessInformation->dwProcessId = 8;
		lpProcessInformation->dwThreadId = 12;
		return TRUE;
	}

	return CreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}
