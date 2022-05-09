#include <StdInc.h>
#include <Hooking.h>

#include <DrawCommands.h>
#include <d3d11.h>

#include <CoreConsole.h>

extern bool IsValidGraphicsLibrary(const std::wstring& path);

namespace hook
{
template<typename T>
inline T* getRVA(HMODULE hModule, uintptr_t rva)
{
	return (T*)((char*)hModule + rva);
}

template<typename TOrdinal>
inline bool iat_matches_ordinal(HMODULE hModule, uintptr_t* nameTableEntry, TOrdinal ordinal)
{
}

template<>
inline bool iat_matches_ordinal(HMODULE hModule, uintptr_t* nameTableEntry, int ordinal)
{
	if (IMAGE_SNAP_BY_ORDINAL(*nameTableEntry))
	{
		return IMAGE_ORDINAL(*nameTableEntry) == ordinal;
	}

	return false;
}

template<>
inline bool iat_matches_ordinal(HMODULE hModule, uintptr_t* nameTableEntry, const char* ordinal)
{
	if (!IMAGE_SNAP_BY_ORDINAL(*nameTableEntry))
	{
		auto import = getRVA<IMAGE_IMPORT_BY_NAME>(hModule, *nameTableEntry);

		return !_stricmp(import->Name, ordinal);
	}

	return false;
}

template<typename T, typename TOrdinal>
T iat(HMODULE hModule, const char* moduleName, T function, TOrdinal ordinal)
{
	IMAGE_DOS_HEADER* imageHeader = (IMAGE_DOS_HEADER*)hModule;
	IMAGE_NT_HEADERS* ntHeader = getRVA<IMAGE_NT_HEADERS>(hModule, imageHeader->e_lfanew);

	IMAGE_IMPORT_DESCRIPTOR* descriptor = getRVA<IMAGE_IMPORT_DESCRIPTOR>(hModule, ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	while (descriptor->Name)
	{
		const char* name = getRVA<char>(hModule, descriptor->Name);

		if (_stricmp(name, moduleName))
		{
			descriptor++;

			continue;
		}

		if (descriptor->OriginalFirstThunk == 0)
		{
			return nullptr;
		}

		auto nameTableEntry = getRVA<uintptr_t>(hModule, descriptor->OriginalFirstThunk);
		auto addressTableEntry = getRVA<uintptr_t>(hModule, descriptor->FirstThunk);

		while (*nameTableEntry)
		{
			if (iat_matches_ordinal(hModule, nameTableEntry, ordinal))
			{
				T origEntry = (T)*addressTableEntry;

				DWORD oldProtect;
				VirtualProtect(addressTableEntry, sizeof(T), PAGE_READWRITE, &oldProtect);

				*addressTableEntry = (uintptr_t)function;

				VirtualProtect(addressTableEntry, sizeof(T), oldProtect, &oldProtect);

				return origEntry;
			}

			nameTableEntry++;
			addressTableEntry++;
		}

		return nullptr;
	}

	return nullptr;
}
}

// this does *not* work, myID3D11*Shader classes store some internal state that's depended on at runtime
#ifdef ENBSERIES_CACHE_ATTEMPT
template<typename TFnLeft, typename TFnRight>
void VHook(intptr_t& ref, TFnLeft fn, TFnRight out)
{
	if (ref == (intptr_t)fn)
	{
		return;
	}

	if (*out)
	{
		return;
	}

	*out = (decltype(*out))ref;
	ref = (intptr_t)fn;
}

static bool g_inShader;
static ID3D11Device* g_origDevice;
static size_t g_shaderHash;
static std::map<size_t, std::vector<uint8_t>> g_shaderCache;

#include <msgpack.hpp>

static bool LoadShaderCache()
{
	FILE* f = _wfopen(MakeRelativeCitPath(L"enbcache.bin").c_str(), L"rb");

	if (f)
	{
		fseek(f, 0, SEEK_END);
		int flen = ftell(f);
		fseek(f, 0, SEEK_SET);

		std::vector<char> data(flen);
		fread(&data[0], 1, data.size(), f);
		fclose(f);

		auto up = msgpack::unpack(data.data(), data.size());
		up->convert(g_shaderCache);
	}

	return true;
}

static void DumpShaderCache()
{
	FILE* f = _wfopen(MakeRelativeCitPath(L"enbcache.bin").c_str(), L"wb");
	if (f)
	{
		msgpack::sbuffer s;
		msgpack::pack(s, g_shaderCache);
		fwrite(s.data(), 1, s.size(), f);
		fclose(f);
	}
}

static const void* g_pShaderBytecode;
static SIZE_T g_BytecodeLength;

static HRESULT (*g_origCreateVertexShader)(ID3D11Device* self, const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11VertexShader** ppVertexShader);

static HRESULT CreateVertexShaderHook(ID3D11Device* self, const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11VertexShader** ppVertexShader)
{
	if (!pShaderBytecode && g_pShaderBytecode)
	{
		pShaderBytecode = g_pShaderBytecode;
		BytecodeLength = g_BytecodeLength;

		g_pShaderBytecode = NULL;
		g_BytecodeLength = 0;
	}
	else if (g_inShader)
	{
		g_shaderCache[g_shaderHash] = { (const char*)pShaderBytecode, (const char*)pShaderBytecode + BytecodeLength };
	}

	return g_origCreateVertexShader(self, pShaderBytecode, BytecodeLength, pClassLinkage, ppVertexShader);
}

static HRESULT (*g_origCreatePixelShader)(ID3D11Device* self, const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11PixelShader** ppShader);

static HRESULT CreatePixelShaderHook(ID3D11Device* self, const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11PixelShader** ppShader)
{
	if (!pShaderBytecode && g_pShaderBytecode)
	{
		pShaderBytecode = g_pShaderBytecode;
		BytecodeLength = g_BytecodeLength;

		g_pShaderBytecode = NULL;
		g_BytecodeLength = 0;
	}
	else if (g_inShader)
	{
		g_shaderCache[g_shaderHash] = { (const char*)pShaderBytecode, (const char*)pShaderBytecode + BytecodeLength };
	}

	return g_origCreatePixelShader(self, pShaderBytecode, BytecodeLength, pClassLinkage, ppShader);
}

static HRESULT (*g_origCreateVertexShaderOuter)(ID3D11Device* self, const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11VertexShader** ppVertexShader);

static HRESULT CreateVertexShaderHookOuter(ID3D11Device* self, const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11VertexShader** ppVertexShader)
{
	static auto a = LoadShaderCache();

	g_inShader = true;
	g_shaderHash = std::hash<std::string_view>()({ (const char*)pShaderBytecode, BytecodeLength });

	HRESULT hr = S_OK;

	if (auto it = g_shaderCache.find(g_shaderHash); it != g_shaderCache.end())
	{
		g_pShaderBytecode = it->second.data();
		g_BytecodeLength = it->second.size();

		hr = g_origCreateVertexShaderOuter(self, NULL, 0, pClassLinkage, ppVertexShader);
	}
	else
	{
		hr = g_origCreateVertexShaderOuter(self, pShaderBytecode, BytecodeLength, pClassLinkage, ppVertexShader);
	}

	g_inShader = false;

	return hr;
}

static HRESULT (*g_origCreatePixelShaderOuter)(ID3D11Device* self, const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11PixelShader** ppShader);

static HRESULT CreatePixelShaderHookOuter(ID3D11Device* self, const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11PixelShader** ppShader)
{
	static auto a = LoadShaderCache();

	g_inShader = true;
	g_shaderHash = std::hash<std::string_view>()({ (const char*)pShaderBytecode, BytecodeLength });

	HRESULT hr = S_OK;

	if (auto it = g_shaderCache.find(g_shaderHash); it != g_shaderCache.end())
	{
		g_pShaderBytecode = it->second.data();
		g_BytecodeLength = it->second.size();

		hr = g_origCreatePixelShaderOuter(self, NULL, 0, pClassLinkage, ppShader);
	}
	else
	{
		hr = g_origCreatePixelShaderOuter(self, pShaderBytecode, BytecodeLength, pClassLinkage, ppShader);
	}

	g_inShader = false;

	return hr;
}

static HRESULT D3D11CreateDeviceWoo(_In_opt_ IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, _COM_Outptr_opt_ ID3D11Device** ppDevice, _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel, _COM_Outptr_opt_ ID3D11DeviceContext** ppImmediateContext)
{
	HRESULT hr = D3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);

	if (SUCCEEDED(hr))
	{
		auto vtbl = **(intptr_t***)ppDevice;
		auto vtblCxt = **(intptr_t***)ppImmediateContext;

		auto ourVtbl = new intptr_t[640];
		memcpy(ourVtbl, vtbl, 640 * sizeof(intptr_t));
		
		auto ourVtblCxt = new intptr_t[640];
		memcpy(ourVtblCxt, vtblCxt, 640 * sizeof(intptr_t));

		VHook(ourVtbl[12], &CreateVertexShaderHook, &g_origCreateVertexShader);
		VHook(ourVtbl[15], &CreatePixelShaderHook, &g_origCreatePixelShader);

		**(intptr_t***)ppDevice = ourVtbl;
		**(intptr_t***)ppImmediateContext = ourVtblCxt;

		g_origDevice = *ppDevice;
	}

	return hr;
}
#else
static HRESULT D3D11CreateDeviceWoo(_In_opt_ IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, _COM_Outptr_opt_ ID3D11Device** ppDevice, _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel, _COM_Outptr_opt_ ID3D11DeviceContext** ppImmediateContext)
{
	return D3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
}
#endif

static FARPROC GetProcAddressStub(HMODULE hModule, LPCSTR name)
{
	if (strcmp(name, "D3D11CreateDevice") == 0)
	{
		return (FARPROC)&D3D11CreateDeviceWoo;
	}

	return GetProcAddress(hModule, name);
}

static bool presented;
extern void D3DPresent(int syncInterval, int flags);

static bool g_isModded;

void RootCheckPresented(int& flags)
{
	if (g_isModded && !presented)
	{
		trace("^2The following two Present errors are perfectly normal and are to work around a bug in ENBSeries.^7\n\n");
		flags |= DXGI_PRESENT_ALLOW_TEARING;
	}
}

void RootSetPresented()
{
	if (g_isModded && !presented)
	{
		presented = true;

		D3DPresent(0, DXGI_PRESENT_ALLOW_TEARING);

		ID3D11RenderTargetView* rtv = nullptr;
		GetD3D11DeviceContext()->OMGetRenderTargets(1, &rtv, NULL);

		if (rtv)
		{
			CD3D11_VIEWPORT vp = CD3D11_VIEWPORT(0.0f, 0.0f, 8192.0f, 8192.0f);
			GetD3D11DeviceContext()->RSSetViewports(1, &vp);

			float blank[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
			GetD3D11DeviceContext()->ClearRenderTargetView(rtv, blank);

			rtv->Release();
		}
	}
}

static DWORD GetTickCountStub()
{
	auto count = GetTickCount();
	static size_t calls = 0;

	if (presented)
	{
		count += 45000;
	}

	calls++;

	return count;
}

static HANDLE FindFirstFileAStub()
{
	return INVALID_HANDLE_VALUE;
}

static HMODULE LoadLibraryAStub(LPCSTR modName)
{
	if (strstr(modName, "enbhelper.dll") != nullptr)
	{
		return NULL;
	}

	return LoadLibraryA(modName);
}

HRESULT RootD3D11CreateDevice(_In_opt_ IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, _COM_Outptr_opt_ ID3D11Device** ppDevice, _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel, _COM_Outptr_opt_ ID3D11DeviceContext** ppImmediateContext)
{
	auto hookDll = MakeRelativeGamePath(L"d3d11.dll");

	if (GetFileAttributesW(hookDll.c_str()) == INVALID_FILE_ATTRIBUTES || !IsValidGraphicsLibrary(hookDll))
	{
		return D3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
	}

	HMODULE hookModule = LoadLibraryW(hookDll.c_str());

	if (!hookModule)
	{
		return D3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
	}

	g_isModded = true;

	// for getting and sandwiching the original D3D device
	hook::iat(hookModule, "kernel32.dll", GetProcAddressStub, "GetProcAddress");

	// for being less ugly
	hook::iat(hookModule, "kernel32.dll", GetTickCountStub, "GetTickCount");

	// for not being an ASI loader
	hook::iat(hookModule, "kernel32.dll", FindFirstFileAStub, "FindFirstFileA");
	hook::iat(hookModule, "kernel32.dll", LoadLibraryAStub, "LoadLibraryA");

	auto cd = (decltype(&D3D11CreateDevice))GetProcAddress(hookModule, "D3D11CreateDevice");
	HRESULT hr = cd(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);

#ifdef ENBSERIES_CACHE_ATTEMPT
	if (SUCCEEDED(hr))
	{
		auto vtbl = **(intptr_t***)ppDevice;
		auto vtblCxt = **(intptr_t***)ppImmediateContext;

		auto ourVtbl = new intptr_t[640];
		memcpy(ourVtbl, vtbl, 640 * sizeof(intptr_t));

		auto ourVtblCxt = new intptr_t[640];
		memcpy(ourVtblCxt, vtblCxt, 640 * sizeof(intptr_t));

		VHook(ourVtbl[12], &CreateVertexShaderHookOuter, &g_origCreateVertexShaderOuter);
		VHook(ourVtbl[15], &CreatePixelShaderHookOuter, &g_origCreatePixelShaderOuter);

		**(intptr_t***)ppDevice = ourVtbl;
		**(intptr_t***)ppImmediateContext = ourVtblCxt;
	}

	static ConsoleCommand c("cc", []()
	{
		DumpShaderCache();
	});
#endif

	return hr;
}
