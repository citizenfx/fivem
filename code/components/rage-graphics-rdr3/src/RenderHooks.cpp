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

static VkInstance g_vkInstance = nullptr;
static bool g_enableVulkanValidation = false;

static bool g_canUseVulkan = true;

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

	// if the vkInstance fails to create the game will try to fallback to D3D12.
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
	SetEvent(g_gameWindowEvent);

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

	return result;
}

static HRESULT D3D12CreateDeviceWrap(IDXGIAdapter* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid, void** ppDevice)
{
	DXGIGetHighPerfAdapter(&pAdapter);

	HRESULT hr = D3D12CreateDevice(pAdapter, MinimumFeatureLevel, riid, ppDevice);
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

static bool WaitForSingleObjectRender(HANDLE handle)
{
	return WaitForSingleObject(handle, 500) == WAIT_OBJECT_0;
}

static bool (*g_initalizeVulkanLibrary)();
static bool InitalizeVulkanLibrary()
{
	if (g_canUseVulkan)
	{
		return g_initalizeVulkanLibrary();
	}

	return false;
}

// Creates a temp file to communicate to RedM on next restart to invalidate pipeline cache
// Helps resolves issues where bad pipeline cache data causes an infinite loop of D3D12/Vulkan Crashes.
static void InvalidatePipelineCache()
{
	FILE* f = _wfopen(MakeRelativeCitPath("data/cache/clearPipelineCache").c_str(), L"ab");
	if (f)
	{
		fclose(f);
	}
}

// the game only checks if VK_ERROR_DEVICE_LOST for certain calls. For consistentcy sake we do the same
// just providing the user with the real error code rather then ERR_GFX_STATE
static VkResult VulkanDeviceLostCheck(VkResult result)
{
	if (result == VK_ERROR_DEVICE_LOST)
	{
		InvalidatePipelineCache();
		FatalError("Vulkan call failed with VK_ERROR_DEVICE_LOST: The logical or physical device has been lost.");
		return result;
	}

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
	FatalError("DirectX encountered an unrecoverable error: %s - %s%s", ToNarrow(errorString), ToNarrow(errorDescription), removedError);
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

	// Replace render sleep and WaitForSingleObject call with just WaitForSingleObject 
	{
		auto location = hook::get_pattern<char>("E8 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 4E ? E8");
		// nop Sleep Call
		hook::nop(location, 5);
		// replace Wait call
		hook::call(hook::get_pattern("E8 ? ? ? ? 84 C0 74 ? 88 9E"), WaitForSingleObjectRender);
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

	// D3D12: Instead of throwing generic ERR_GFX_STATE properly catch FAILED calls and provide a slightly more useful error.
	hook::call(hook::get_pattern("E8 ? ? ? ? 8B C3 48 83 C4 ? 5B C3 48 8B C4 48 89 58 ? 88 50"), D3D12ResultFailed);

	// Same for Vulkan, Vulkan has three ERR_GFX_STATE errors, one for VK_ERROR_DEVICE_LOST, one for Swapchain Creation and an unknown one.
	// VK_ERROR_DEVICE_LOST exclusively, called after most vk functions.
	hook::trampoline(hook::get_pattern("40 53 48 83 EC ? 8B D9 83 F9 ? 75 ? 48 8B 0D"), VulkanDeviceLostCheck);

	// Vulkan Swapchain creation. (VK_NOT_READY, VK_TIMEOUT, VK_ERROR_SURFACE_LOST_KHR etc falsely reported this as a D3D crash)
	{
		auto location = hook::get_pattern<char>("E8 ? ? ? ? 85 C0 0F 85 ? ? ? ? 41 8A 87");
		hook::nop(location, 5);
		hook::call(location, VulkanFailed);
	}
	
	// Don't attempt to use vulkan if the system doesn't properly support it.
	g_initalizeVulkanLibrary = hook::trampoline(hook::get_call(hook::get_pattern("E8 ? ? ? ? 84 C0 75 ? 33 C0 EB ? 41 B8")), InitalizeVulkanLibrary);
});

// Create a dummy Vulkan and D3D12 device to load the UMD for both early.
// We also use this as an opportunity to catch if either graphics API wouldn't be suitable.
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

	std::thread([]()
	{
		struct VulkanHandle
		{
			VkInstance instance = VK_NULL_HANDLE;
			VkDevice device = VK_NULL_HANDLE;

			VulkanHandle(VkInstance instance)
				: instance(instance)
			{
			}

			~VulkanHandle()
			{
				if (device)
				{
					vkDestroyDevice(device, nullptr);
					device = VK_NULL_HANDLE;
				}

				if (instance)
				{
					vkDestroyInstance(instance, nullptr);
					instance = VK_NULL_HANDLE;
				}
			}
		};

		VkInstance instance;
		VkInstanceCreateInfo instanceCreateInfo = {};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

		VkResult result;
		if (result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance); result != VK_SUCCESS)
		{
			trace("Unable to use vulkan driver: Unable to create vulkan instance, VkResult: %s\n", ResultToString(result));
			g_canUseVulkan = false;
			return;
		}

		VulkanHandle handle(instance);
		uint32_t physicalDeviceCount = 0;
		if (result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr); result != VK_SUCCESS || physicalDeviceCount == 0)
		{
			trace("Unable to use vulkan driver: Unable to enumerate physical devices, VkResult: %s\n", ResultToString(result));
			g_canUseVulkan = false;
			return;
		}

		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

		VkPhysicalDevice physicalDevice = physicalDevices[0];

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		if (queueFamilyCount == 0)
		{
			trace("Unable to use vulkan driver: Unable to find any deviceQueueFamily\n");
			g_canUseVulkan = false;
			return;
		}

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		uint32_t selectedQueueFamily = UINT32_MAX;
		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
			if (queueFamilies[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
			{
				selectedQueueFamily = i;
				break;
			}
		}

		if (selectedQueueFamily == UINT32_MAX)
		{
			trace("Unable to use vulkan driver: Unable to find supported deviceQueueFamily\n");
			g_canUseVulkan = false;
			return;
		}

		float queuePriority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = 0;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

		if (!IsDedicatedGPU(physicalDevice))
		{
			ForceDedicatedGPU(g_vkInstance, &deviceCreateInfo, physicalDevice);
		}

		VkDevice vkDevice;
		if (result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &vkDevice); result != VK_SUCCESS)
		{
			trace("Unable to use vulkan driver: Failed to create vkDevice, VkResult: %s\n", ResultToString(result));
			g_canUseVulkan = false;
			return;
		}

		handle.device = vkDevice;

		// RedM requires extra vulkan support, ensure that the GPU used can support the added functionality required.
		// Otherwise this can lead to various hard to debug issues usually some form of "Failed to allocate memory for Vulkan..."	
		static auto _vkGetPhysicalDeviceImageFormatProperties2 = (PFN_vkGetPhysicalDeviceImageFormatProperties2)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceImageFormatProperties2");

		if (!_vkGetPhysicalDeviceImageFormatProperties2)
		{
			trace("Unable to use vulkan driver: Vulkan driver does not include 'vkGetPhysicalDeviceImageFormatProperties2'\n");
			g_canUseVulkan = false;
			return;
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
			g_canUseVulkan = false;
			return;
		}

		// 'VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT' is required by NUI to import D3D11 textures from CEF.
		if (!(externalProps.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT))
		{
			trace("Unable to use vulkan driver: GPU does not support 'VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT' which is required by NUI");
			g_canUseVulkan = false;
			return;
		}

		vkDeviceWaitIdle(vkDevice);
		Sleep(5000);
	}).detach();
});
