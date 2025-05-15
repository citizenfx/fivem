#include "StdInc.h"

#include <vulkan/vulkan.h>

#include <Hooking.h>
#include <Error.h>

#include <CoreConsole.h>

#pragma comment(lib, "vulkan-1.lib")

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

	trace("%s [%i][%s]: %s\n", prefix, pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);
	return VK_FALSE;
}

static inline std::string GetGPUType(VkPhysicalDeviceType type)
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

static VkResult __stdcall vkCreateInstanceHook(VkInstanceCreateInfo* pCreateInfo, VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
	// the validation layers should be disabled when playing, but enabled when encountering a crash
	// if we disable these now, we fallback to dx12 for some reason
#if 0
	std::vector<const char*> originalLayers(pCreateInfo->enabledLayerCount + 1);

	for (size_t i = 0; i < originalLayers.size() - 1; i++)
	{
		originalLayers[i] = pCreateInfo->ppEnabledLayerNames[i];
		trace("Vulkan Instance layers: %s\n", originalLayers[i]);
	}

	originalLayers.push_back("VK_LAYER_KHRONOS_validation");

	pCreateInfo->enabledLayerCount = static_cast<uint32_t>(originalLayers.size());
	pCreateInfo->ppEnabledLayerNames = originalLayers.data();

	std::vector<const char*> originalExtensions(pCreateInfo->enabledExtensionCount + 1);

	for (size_t i = 0; i < originalExtensions.size() - 1; i++)
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
#endif
	VkResult result = vkCreateInstance(pCreateInfo, pAllocator, pInstance);

	if (result != VK_SUCCESS)
	{
		trace("Vulkan instance creation returned: %i\nSee the vulkan docs for the error code (VkResult)\n", result);
	}

	// we let rage handle the error (probably defaults back to dx12)
	return result;
}

static HRESULT vkCreateDeviceHook(VkPhysicalDevice physicalDevice, VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice)
{
	// Disabled for now
#if 0
	// force validation layers for vulkan, but dont replace the original layers
	std::vector<const char*> originalLayers(pCreateInfo->enabledLayerCount + 1);

	for (size_t i = 0; i < originalLayers.size() - 1; i++)
	{
		originalLayers[i] = pCreateInfo->ppEnabledLayerNames[i];
		trace("Vulkan Device Layer: %s", originalLayers[i]);
	}

	originalLayers.push_back("VK_LAYER_KHRONOS_validation");

	pCreateInfo->enabledLayerCount = static_cast<uint32_t>(originalLayers.size());
	pCreateInfo->ppEnabledLayerNames = originalLayers.data();
#endif

	static auto _ = ([&physicalDevice]
	{
		VkPhysicalDeviceProperties props = {};

		vkGetPhysicalDeviceProperties(physicalDevice, &props);

		// version encoding: https://registry.khronos.org/vulkan/specs/latest/html/vkspec.html#extendingvulkan-coreversions-versionnumbers
		uint32_t major = static_cast<uint32_t>(props.apiVersion) >> 22U;
		uint32_t minor = (static_cast<uint32_t>(props.apiVersion) >> 12U) & 0x3FFU;
		uint32_t patch = static_cast<uint32_t>(props.apiVersion) & 0xFFFU;

		AddCrashometry("gpu_name", "%s", props.deviceName);
		AddCrashometry("gpu_id", "%04x:%04x", props.vendorID, props.deviceID);
		AddCrashometry("vulkan_api_version", "v%u.%u.%u", major, minor, patch);

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

		trace("GPU Name: %s\nGPU type: %s\nDriver version: %u\nAPI version: %u.%u.%u\n", props.deviceName, GetGPUType(props.deviceType).c_str(), props.driverVersion, major, minor, patch);

		if (!IsDedicatedGPU(props.deviceType))
		{
			console::DPrintf("Vulkan GPU selection", "[WARNING] RedM is using a non dedicated GPU. The GPU type that is in use is: %s", GetGPUType(props.deviceType));
		}

		return true;
	})();

	VkResult result = vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);

	if (result != VK_SUCCESS)
	{
		trace("Vulkan device creation returned: %i\nSee the vulkan docs for the error code (VkResult)", result);
	}

	return result;
}

static HookFunction hookFunction([]()
{
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
});
