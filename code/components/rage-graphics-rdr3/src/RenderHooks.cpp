#include "StdInc.h"

#include <d3d12.h>
#include <wrl/client.h>
#include <vulkan/vulkan.h>

#include <Hooking.h>
#include <Error.h>
#include <dxerr.h>

#include <CoreConsole.h>

#include <VulkanHelper.h>
#include <CrossBuildRuntime.h>
#include <DrawCommands.h>
#include <Hooking.Stubs.h>

#pragma comment(lib, "vulkan-1.lib")

static VkInstance g_vkInstance = nullptr;
static bool g_enableVulkanValidation = false;

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
		return;

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


// the game only checks if VK_ERROR_DEVICE_LOST for certain calls. For consistentcy sake we do the same
// just providing the user with the real error code rather then ERR_GFX_STATE
static VkResult VulkanDeviceLostCheck(VkResult result)
{
	if (result == VK_ERROR_DEVICE_LOST)
	{
		FatalError("Vulkan call failed with VK_ERROR_DEVICE_LOST: The logical or physical device has been lost.");
		return result;
	}

	return result;
}

static VkResult VulkanFailed(VkResult result)
{
	if (result != VK_SUCCESS)
	{
		FatalError("Vulkan Call failed with %s", ResultToString(result));
		return result;
	}

	return result;
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

	return result;
}

static void __declspec(noinline) DisplayD3DCrashMessageGeneric(const std::string& errorBody)
{
	FatalError("DirectX encountered an unrecoverable error: %s", errorBody);
}

static void D3D12ResultFailed(HRESULT hr)
{
	constexpr auto errorBufferCount = 8192;
	auto errorBuffer = std::unique_ptr<wchar_t[]>(new wchar_t[errorBufferCount]);
	memset(errorBuffer.get(), 0, errorBufferCount * sizeof(wchar_t));
	DXGetErrorDescriptionW(hr, errorBuffer.get(), errorBufferCount);

	auto errorString = DXGetErrorStringW(hr);

	if (!errorString)
	{
		errorString = va(L"0x%08x", hr);
	}

	std::string removedError;

	if (hr == DXGI_ERROR_DEVICE_REMOVED)
	{
		HRESULT removedReason = ((ID3D12Device*)GetGraphicsDriverHandle())->GetDeviceRemovedReason();

		auto errorBuffer = std::unique_ptr<wchar_t[]>(new wchar_t[errorBufferCount]);
		memset(errorBuffer.get(), 0, errorBufferCount * sizeof(wchar_t));
		DXGetErrorDescriptionW(removedReason, errorBuffer.get(), errorBufferCount);

		auto removedString = DXGetErrorStringW(removedReason);

		if (!removedString)
		{
			removedString = va(L"0x%08x", hr);
		}

		removedError = ToNarrow(fmt::sprintf(L"\nGetDeviceRemovedReason returned %s - %s", removedString, errorBuffer.get()));
	}

	auto errorBody = fmt::sprintf("%s - %s%s", ToNarrow(errorString), ToNarrow(errorBuffer.get()), removedError);
	DisplayD3DCrashMessageGeneric(errorBody);
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

	// D3D12: Instead of throwing generic ERR_GFX_STATE properly catch FAILED calls and provide a slightly more useful error.
	hook::call(hook::get_pattern("E8 ? ? ? ? 8B C3 48 83 C4 ? 5B C3 48 8B C4 48 89 58 ? 88 50"), D3D12ResultFailed);

	// Same for Vulkan, Vulkan has three ERR_GFX_STATE errors, one for VK_ERROR_DEVICE_LOST, one for Swapchain Creation and an unknown one.
	// VK_ERROR_DEVICE_LOST exclusively, called after most vk functions.
	hook::trampoline(hook::get_pattern("40 53 48 83 EC ? 8B D9 83 F9 ? 75 ? 48 8B 0D"), VulkanDeviceLostCheck);

	// Vulkan Swapchain creation. (VK_NOT_READY, VK_TIMEOUT, VK_ERROR_SURFACE_LOST_KHR etc falsely reported this as a D3D crash)
	{
		auto location = hook::get_pattern<char>("E8 ? ? ? ? 85 C0 0F 85 ? ? ? ? 41 8A 87");
		hook::nop(location, 6);
		hook::call(location, VulkanFailed);
	}
});
