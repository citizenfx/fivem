#include "StdInc.h"

#include <vulkan/vulkan.h>
#include <d3dcommon.h>
#include <d3d12.h>

#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <CL2LaunchMode.h>
#include <ICoreGameInit.h>
#include <HostSharedData.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <dxerr.h>
#include <Error.h>

#include <CoreConsole.h>

#include <VulkanHelper.h>
#include <CrossBuildRuntime.h>
#include <CfxState.h>
#include <DrawCommands.h>

#pragma comment(lib, "vulkan-1.lib")

VkInstance g_vkInstance = nullptr;
static bool g_enableVulkanValidation = false;

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

namespace WRL = Microsoft::WRL;

static void DXGIGetHighPerfAdapter(IDXGIAdapter** ppAdapter)
{
	{
		WRL::ComPtr<IDXGIFactory1> dxgiFactory;
		CreateDXGIFactory1(IID_IDXGIFactory1, &dxgiFactory);

		WRL::ComPtr<IDXGIAdapter1> adapter;
		WRL::ComPtr<IDXGIFactory6> factory6;
		HRESULT hr = dxgiFactory.As(&factory6);
		if (SUCCEEDED(hr))
		{
			for (UINT adapterIndex = 0;
			DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()));
			adapterIndex++)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					continue;
				}

				AddCrashometry("gpu_name", "%s", ToNarrow(desc.Description));
				AddCrashometry("gpu_id", "%04x:%04x", desc.VendorId, desc.DeviceId);

				adapter.CopyTo(ppAdapter);
				break;
			}
		}
	}
}

static HANDLE g_gameWindowEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

void DLL_EXPORT UiDone()
{
	static HostSharedData<CfxState> initState("CfxInitState");
	WaitForSingleObject(g_gameWindowEvent, INFINITE);

	auto uiExitEvent = CreateEventW(NULL, TRUE, FALSE, va(L"CitizenFX_PreUIExit%s", IsCL2() ? L"CL2" : L""));
	auto uiDoneEvent = CreateEventW(NULL, FALSE, FALSE, va(L"CitizenFX_PreUIDone%s", IsCL2() ? L"CL2" : L""));

	if (uiExitEvent)
	{
		SetEvent(uiExitEvent);
	}

	if (uiDoneEvent)
	{
		WaitForSingleObject(uiDoneEvent, INFINITE);
	}
}

// Function to print the output of the validation layers
VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::string prefix;

	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
	{
		prefix = "VERBOSE: ";
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		prefix = "INFO: ";
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		prefix = "WARNING: ";
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		prefix = "ERROR: ";
	}

	trace("%s[%i][%s]: %s\n", prefix, pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);
	return VK_FALSE;
}

static inline bool IsGPUSupported(VkPhysicalDevice device, VkDeviceCreateInfo* createInfo)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	if (extensionCount < 1)
	{
		return false;
	}

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	uint32_t layerCount = 0;
	for (const auto& extension : availableExtensions)
	{
		for (uint32_t i = 0; i < createInfo->enabledExtensionCount; i++)
		{
			if (strcmp(extension.extensionName, createInfo->ppEnabledExtensionNames[i]) == 0)
			{
				layerCount++;
			}
		}
	}

	return layerCount == createInfo->enabledExtensionCount;
}

static inline std::string_view GetGPUType(VkPhysicalDeviceType type)
{
	switch (type)
	{
		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
			return "Other";
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			return "Integrated GPU";
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			return "Discrete GPU";
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			return "Virtual GPU";
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			return "CPU";
		default:
			return "Unknown";
	}
}

static inline std::string_view GetGPUType(VkPhysicalDevice physicalDevice)
{
	VkPhysicalDeviceProperties props = {};

	vkGetPhysicalDeviceProperties(physicalDevice, &props);

	return GetGPUType(props.deviceType);
}

static inline bool IsDedicatedGPU(VkPhysicalDeviceType type)
{
	switch (type)
	{
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			return true;
		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
		default:
			return false;
	}
}

static inline bool IsDedicatedGPU(VkPhysicalDevice physicalDevice)
{
	VkPhysicalDeviceProperties props = {};

	vkGetPhysicalDeviceProperties(physicalDevice, &props);

	return IsDedicatedGPU(props.deviceType);
}

static inline void ForceDedicatedGPU(VkInstance instance, VkDeviceCreateInfo* createInfo, VkPhysicalDevice& physicalDevice)
{
	std::vector<VkPhysicalDevice> devices;
	uint32_t numDevices = 0;

	vkEnumeratePhysicalDevices(instance, &numDevices, nullptr);
	assert(numDevices > 0);

	devices.resize(numDevices);

	vkEnumeratePhysicalDevices(instance, &numDevices, devices.data());

	for (const auto& device : devices)
	{
		if (IsDedicatedGPU(device) && IsGPUSupported(device, createInfo))
		{
			physicalDevice = device;

			return;
		}
	}

	trace("[WARNING] RedM is using a non dedicated GPU. The GPU type that is in use is: %s\n. Consider updating GPU drivers if you have a dedicated GPU in your system.", GetGPUType(physicalDevice));
}

static VkResult __stdcall vkCreateInstanceHook(VkInstanceCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
	// the validation layers should be disabled when playing, but enabled when encountering a crash
	// we keep the data here, to ensure the data does not go out of scope
	std::vector<const char*> originalLayers(pCreateInfo->enabledLayerCount);
	std::vector<const char*> originalExtensions(pCreateInfo->enabledExtensionCount);

	if (g_enableVulkanValidation)
	{

		for (size_t i = 0; i < originalLayers.size(); i++)
		{
			originalLayers[i] = pCreateInfo->ppEnabledLayerNames[i];
			trace("Vulkan Instance layers: %s\n", originalLayers[i]);
		}

		originalLayers.push_back("VK_LAYER_KHRONOS_validation");

		pCreateInfo->enabledLayerCount = static_cast<uint32_t>(originalLayers.size());
		pCreateInfo->ppEnabledLayerNames = originalLayers.data();

		for (size_t i = 0; i < originalExtensions.size(); i++)
		{
			originalExtensions[i] = pCreateInfo->ppEnabledExtensionNames[i];
			trace("Vulkan Instance Extensions: %s\n", originalExtensions[i]);
		}

		originalExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		pCreateInfo->enabledExtensionCount = static_cast<uint32_t>(originalExtensions.size());
		pCreateInfo->ppEnabledExtensionNames = originalExtensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};

		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = DebugMessageCallback;

		pCreateInfo->pNext = &debugCreateInfo;
	}

	VkResult result = vkCreateInstance(pCreateInfo, pAllocator, pInstance);

	if (result != VK_SUCCESS)
	{
		trace("Vulkan instance creation returned: %s\n", ResultToString(result));
	}

	g_vkInstance = *pInstance;

	// we let rage handle the error (probably defaults back to dx12)
	return result;
}

static inline void SetVulkanCrashometry(const VkPhysicalDevice physicalDevice)
{
	static bool isInitialized = false;

	if (isInitialized)
	{
		return;
	}

	VkPhysicalDeviceProperties props = {};

	vkGetPhysicalDeviceProperties(physicalDevice, &props);

	// version encoding: https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#extendingvulkan-coreversions-versionnumbers
	uint32_t major = static_cast<uint32_t>(props.apiVersion) >> 22U;
	uint32_t minor = (static_cast<uint32_t>(props.apiVersion) >> 12U) & 0x3FFU;
	uint32_t patch = static_cast<uint32_t>(props.apiVersion) & 0xFFFU;

	AddCrashometry("gpu_name", "%s", props.deviceName);
	AddCrashometry("gpu_id", "%04x:%04x", props.vendorID, props.deviceID);
	AddCrashometry("vulkan_api_version", "v%u.%u.%u", major, minor, patch);
	AddCrashometry("validation_layers_enabled", "%i", static_cast<int>(g_enableVulkanValidation));

	// driver version encoding: https://github.com/SaschaWillems/vulkan.gpuinfo.org/blob/1e6ca6e3c0763daabd6a101b860ab4354a07f5d3/functions.php#L298-L321
	// vendor ids: https://www.reddit.com/r/vulkan/comments/4ta9nj/is_there_a_comprehensive_list_of_the_names_and/
	// NVIDIA
	if (props.vendorID == 0x10DE)
	{
		AddCrashometry("nvidia_driver", "v%u.%u.%u.%u", (props.driverVersion >> 22) & 0x3ff, (props.driverVersion >> 14) & 0x0ff, (props.driverVersion >> 6) & 0x0ff, (props.driverVersion) & 0x003f);
	}
	// INTEL
	else if (props.vendorID == 0x8086)
	{
		AddCrashometry("intel_driver", "v%u.%u", (props.driverVersion >> 14), (props.driverVersion) & 0x3fff);
	}
	else
	{
		AddCrashometry("driver", "v%u.%u.%u", (props.driverVersion >> 22), (props.driverVersion >> 12) & 0x3ff, props.driverVersion & 0xfff);
	}

	trace("GPU Name: %s\nGPU type: %s\nDriver version: %u\nAPI version: %u.%u.%u\n", props.deviceName, GetGPUType(props.deviceType), props.driverVersion, major, minor, patch);

	isInitialized = true;
}

static HRESULT vkCreateDeviceHook(VkPhysicalDevice physicalDevice, VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
	// we keep the data here, to ensure the data does not go out of scope
	std::vector<const char*> originalLayers(pCreateInfo->enabledLayerCount);

	if (g_enableVulkanValidation)
	{
		// force validation layers for vulkan, but dont replace the original layers

		for (size_t i = 0; i < originalLayers.size(); i++)
		{
			originalLayers[i] = pCreateInfo->ppEnabledLayerNames[i];
			trace("Vulkan Device Layer: %s", originalLayers[i]);
		}

		originalLayers.push_back("VK_LAYER_KHRONOS_validation");

		pCreateInfo->enabledLayerCount = static_cast<uint32_t>(originalLayers.size());
		pCreateInfo->ppEnabledLayerNames = originalLayers.data();
	}

	if (!IsDedicatedGPU(physicalDevice))
	{
		ForceDedicatedGPU(g_vkInstance, pCreateInfo, physicalDevice);
	}

	SetVulkanCrashometry(physicalDevice);

	VkResult result = vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

	if (result != VK_SUCCESS)
	{
		trace("Vulkan device creation returned: %s\n", ResultToString(result));
	}

	// Check if we can use Vulkan with NUI
    const char* requiredExtensions[] = {
		"VK_KHR_external_memory",
		"VK_KHR_external_memory_win32",
		"VK_KHR_dedicated_allocation",
	};

	uint32_t extCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, extensions.data());

	for (const char* reqExt : requiredExtensions)
	{
		bool found = false;
		for (auto& ext : extensions)
		{
			if (strcmp(ext.extensionName, reqExt) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			trace("Unable to use Vulkan driver: Missing required extension %s\n", reqExt);

			// Fallback to D3D12.
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	static auto _vkGetPhysicalDeviceImageFormatProperties2 = (PFN_vkGetPhysicalDeviceImageFormatProperties2)vkGetInstanceProcAddr(g_vkInstance, "vkGetPhysicalDeviceImageFormatProperties2");
	if (!_vkGetPhysicalDeviceImageFormatProperties2)
	{
		trace("Unable to use vulkan driver: Vulkan driver does not include 'vkGetPhysicalDeviceImageFormatProperties2'\n");
		return VK_ERROR_FEATURE_NOT_PRESENT;
	}

	VkPhysicalDeviceExternalImageFormatInfo externalImageInfo{};
	externalImageInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
	externalImageInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;

	VkPhysicalDeviceImageFormatInfo2 formatInfo2{};
	formatInfo2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
	formatInfo2.pNext = &externalImageInfo;
	formatInfo2.format = VK_FORMAT_B8G8R8A8_UNORM;
	formatInfo2.type = VK_IMAGE_TYPE_2D;
	formatInfo2.tiling = VK_IMAGE_TILING_OPTIMAL;
	formatInfo2.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

	VkExternalImageFormatProperties externalProps{};
	externalProps.sType = VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES;
	VkImageFormatProperties2 formatProperties2{};
	formatProperties2.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
	formatProperties2.pNext = &externalProps;

	if (result = _vkGetPhysicalDeviceImageFormatProperties2(physicalDevice, &formatInfo2, &formatProperties2); result != VK_SUCCESS)
	{
		trace("Unable to use vulkan driver: 'vkGetPhysicalDeviceImageFormatProperties2' returned VkResult: %s\n", ResultToString(result));
		return VK_ERROR_FEATURE_NOT_PRESENT;
	}

	// 'VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT' is required by NUI to import D3D11 textures from CEF.
	if (!(externalProps.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT))
	{
		trace("Unable to use vulkan driver: GPU does not support 'VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT' which is required by NUI\n");
		return VK_ERROR_FEATURE_NOT_PRESENT;
	}

	if (result == VK_SUCCESS)
	{
		SetEvent(g_gameWindowEvent);
	}

	return result;
}

static HRESULT D3D12CreateDeviceWrap(IDXGIAdapter* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid, void** ppDevice)
{
    WRL::ComPtr<IDXGIAdapter> adapter(pAdapter);
	DXGIGetHighPerfAdapter(adapter.ReleaseAndGetAddressOf());

	if (!adapter)
	{
		return E_FAIL;
	}

	HRESULT hr = D3D12CreateDevice(adapter.Get(), MinimumFeatureLevel, riid, ppDevice);
	if (SUCCEEDED(hr))
	{
		SetEvent(g_gameWindowEvent);
	}

	return hr;
}

static void SetCrashometryData(const char* type, const char* details, int unk)
{
	if (!strcmp(type,"d3d12_error") || !strcmp(type, "vulkan_last_error") || !strcmp(type, "pc_last_error"))
	{
		AddCrashometry(type, details);
	}
}

// Creates a temp file to communicate to RedM on next restart to invalidate pipeline cache
// Helps resolves issues where bad pipeline cache data causes an infinite loop of D3D12/Vulkan Crashes.
static void InvalidatePipelineCache()
{
	FILE* f = _wfopen(MakeRelativeCitPath("data\\cache\\clearPipelineCache").c_str(), L"wb");
	if (f)
	{
		fclose(f);
	}
}

// the game only checks if VK_ERROR_DEVICE_LOST for certain calls. For consistentcy sake we do the same
// just providing the user with the real error code rather then ERR_GFX_STATE
static VkResult VulkanDeviceLost(VkResult result)
{
	InvalidatePipelineCache();
	FatalError("Vulkan call failed with VK_ERROR_DEVICE_LOST: The logical or physical device has been lost.");
	return result;
}

static VkResult VulkanFailed(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		InvalidatePipelineCache();
		FatalError("Vulkan Call failed with %s", ResultToString(result));
		return result;
	}

	return result;
}

static void D3D12ResultFailed(HRESULT hr)
{
	constexpr auto errorBufferCount = 4096;
	std::wstring errorDescription(errorBufferCount, L'\0');
	DXGetErrorDescriptionW(hr, errorDescription.data(), errorBufferCount);

	std::wstring errorString = DXGetErrorStringW(hr);
	if (errorString.empty())
	{
		errorString = va(L"0x%08x", hr);
	}

	std::string removedError;
	if (hr == DXGI_ERROR_DEVICE_REMOVED)
	{
		HRESULT removedReason = static_cast<ID3D12Device*>(GetGraphicsDriverHandle())->GetDeviceRemovedReason();

		std::wstring removedDescription(2048, L'\0');
		DXGetErrorDescriptionW(removedReason, removedDescription.data(), 2048);
		removedDescription.resize(wcslen(removedDescription.c_str()));

		std::wstring removedString = DXGetErrorStringW(removedReason);

		if (removedString.empty())
		{
			removedString = va(L"0x%08x", removedReason);
		}

		removedError = ToNarrow(fmt::sprintf(L"\nGetDeviceRemovedReason returned %s - %s", removedString, removedDescription));
	}

	InvalidatePipelineCache();
	FatalError("DirectX encountered an unrecoverable error: %s - %s%s", ToNarrow(errorString).c_str(), ToNarrow(errorDescription).c_str(), removedError.c_str());
}

static HookFunction hookFunction([]()
{
	std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");
	g_enableVulkanValidation = static_cast<bool>(GetPrivateProfileInt(L"Game", L"EnableVulkanValidation", 0, fpath.c_str()));

	// Vulkan create instance hook
	{
		auto location = hook::get_pattern("FF D0 8B F0 89 84 24", -7);
		hook::nop(location, 9);
		hook::call(location, vkCreateInstanceHook);
	}

	// Vulkan CreateDevice hook
	{
		auto location = hook::get_pattern<char>("FF 15 ? ? ? ? 8B C8 E8 ? ? ? ? 85 C0 0F 85 ? ? ? ? 48 8B 0D");
		hook::nop(location, 6);
		hook::call(location, vkCreateDeviceHook);
	}

	// D3D12 CreateDevice Hook
	// 1491.50 calls D3D12CreateDevice up to 4 times when creating the real D3D12Device
	{
		auto p = hook::pattern("48 8B CB FF 15 ? ? ? ? 8B C8").count(4);

		for (int i = 0; i < p.size(); i++)
		{
			auto location = p.get(i).get<char>(3);
			hook::nop(location, 6);
			hook::call(location, D3D12CreateDeviceWrap);
		}
	}

	// no ShowWindow early
	{
		auto location = hook::get_pattern<char>("48 85 C0 0F 45 D1", 9);
		// ShowWindow
		hook::nop(location, 6);
		// UpdateWindow
		hook::nop(location + 9, 6);
		// SetFocus
		hook::nop(location + 18, 6);
	}
	
	// Hook crashometry call that has useful information passed to it related to graphics
	{
		auto location = hook::get_pattern("48 89 54 24 ? 48 89 4C 24 ? 55 56 57 41 54 41 56");
		hook::jump(location, SetCrashometryData);
	}

	// D3D12: Catch FAILED calls and provide an error from DXErr instead of a generic ERR_GFX_STATE error
	{
		auto location = hook::get_pattern("C1 FB ? BA");
		hook::nop(location, 0x1c);
		hook::put<uint32_t>(location, 0xCB8B48); // mov rcx, rbx
		hook::call((uintptr_t)location + 3, D3D12ResultFailed);
	}

	// Same for Vulkan, Vulkan has three ERR_GFX_STATE errors, one for VK_ERROR_DEVICE_LOST, one for Swapchain Creation and an unknown one.
	// VK_ERROR_DEVICE_LOST exclusively, called after most vk functions.
	{
		auto location = hook::get_pattern("FF 90 ? ? ? ? BA ? ? ? ? B9 ? ? ? ? 41 B8");
		hook::nop(location, 27);
		hook::put<uint32_t>(location, 0xCB8B48); // mov rcx, rbx
		hook::call((uintptr_t)location + 3, VulkanDeviceLost);
	}

	// Vulkan Swapchain creation. (VK_NOT_READY, VK_TIMEOUT, VK_ERROR_SURFACE_LOST_KHR etc falsely reported this as a D3D crash)
	{
		auto location = hook::get_pattern<char>("E8 ? ? ? ? 85 C0 0F 85 ? ? ? ? 41 8A 87");
		hook::nop(location, 5);
		hook::call(location, VulkanFailed);
	}
});

// Create D3D12 device to load the UMD early.
// Vulkan originally was loaded here but it appeared to have some side-effects and from testing didn't yield a significant speedup.
static InitFunction initFunction([]()
{
	{
		auto state = CfxState::Get();
		if (!state->IsGameProcess())
		{
			return;
		}
	}

	std::thread([]()
	{
		IDXGIAdapter* adapter = nullptr;
		DXGIGetHighPerfAdapter(&adapter);

		WRL::ComPtr<ID3D12Device> device;
		HRESULT hr = D3D12CreateDevice(
		adapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&device));

		if (adapter)
		{
			adapter->Release();
		}
		Sleep(20000);
	}).detach();
});
