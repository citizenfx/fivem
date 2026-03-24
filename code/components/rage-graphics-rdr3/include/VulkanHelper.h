#pragma once

#if IS_RDR3
#include <string_view>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_win32.h>

#include <DrawCommands.h>

inline std::string_view ResultToString(VkResult result)
{
	switch (result)
	{
		case VK_NOT_READY:
			return "VK_NOT_READY A fence or query has not yet completed";
		case VK_INCOMPLETE:
			return "VK_INCOMPLETE A return array was too small for the result";
		case VK_TIMEOUT:
			return "VK_TIMEOUT A wait operation has not completed in the specified time";
		case VK_EVENT_SET:
			return "VK_EVENT_SET An event is signaled";
		case VK_EVENT_RESET:
			return "VK_EVENT_RESET An event is unsignaled";
		case VK_SUBOPTIMAL_KHR:
			return "VK_SUBOPTIMAL_KHR A swapchain no longer matches the surface properties exactly, but can still be used to present to the surface successfully.";
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			return "VK_ERROR_OUT_OF_HOST_MEMORY A host memory allocation has failed.";
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			return "VK_ERROR_OUT_OF_DEVICE_MEMORY A device memory allocation has failed.";
		case VK_ERROR_INITIALIZATION_FAILED:
			return "VK_ERROR_INITIALIZATION_FAILED Initialization of an object could not be completed for implementation-specific reasons.";
		case VK_ERROR_DEVICE_LOST:
			return "VK_ERROR_DEVICE_LOST The logical or physical device has been lost.";
		case VK_ERROR_MEMORY_MAP_FAILED:
			return "VK_ERROR_MEMORY_MAP_FAILED Mapping of a memory object has failed.";
		case VK_ERROR_LAYER_NOT_PRESENT:
			return "VK_ERROR_LAYER_NOT_PRESENT A requested layer is not present or could not be loaded.";
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			return "VK_ERROR_EXTENSION_NOT_PRESENT A requested extension is not supported.";
		case VK_ERROR_FEATURE_NOT_PRESENT:
			return "VK_ERROR_FEATURE_NOT_PRESENT A requested feature is not supported.";
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			return "VK_ERROR_INCOMPATIBLE_DRIVER The requested version of Vulkan is not supported by the driver or is otherwise incompatible for implementation-specific reasons.";
		case VK_ERROR_TOO_MANY_OBJECTS:
			return "VK_ERROR_TOO_MANY_OBJECTS Too many objects of the type have already been created.";
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			return "VK_ERROR_FORMAT_NOT_SUPPORTED A requested format is not supported on this device.";
		case VK_ERROR_FRAGMENTED_POOL:
			return "VK_ERROR_FRAGMENTED_POOL A pool allocation has failed due to fragmentation of the pool’s memory. This must only be returned if no attempt to allocate host or device memory was made to accommodate the new allocation. This should be returned in preference to VK_ERROR_OUT_OF_POOL_MEMORY, but only if the implementation is certain that the pool allocation failure was due to fragmentation.";
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			return "VK_ERROR_OUT_OF_POOL_MEMORY A pool memory allocation has failed. This must only be returned if no attempt to allocate host or device memory was made to accommodate the new allocation. If the failure was definitely due to fragmentation of the pool, VK_ERROR_FRAGMENTED_POOL should be returned instead.";
		case VK_ERROR_INVALID_EXTERNAL_HANDLE:
			return "VK_ERROR_INVALID_EXTERNAL_HANDLE An external handle is not a valid handle of the specified type.";
		case VK_ERROR_SURFACE_LOST_KHR:
			return "VK_ERROR_SURFACE_LOST_KHR A surface is no longer available.";
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR The requested window is already in use by Vulkan or another API in a manner which prevents it from being used again.";
		case VK_ERROR_OUT_OF_DATE_KHR:
			return "VK_ERROR_OUT_OF_DATE_KHR A surface has changed in such a way that it is no longer compatible with the swapchain, and further presentation requests using the swapchain will fail. Applications must query the new surface properties and recreate their swapchain if they wish to continue presenting to the surface.";
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR The display used by a swapchain does not use the same presentable image layout, or is incompatible in a way that prevents sharing an image.";
		case VK_ERROR_VALIDATION_FAILED_EXT:
			return "VK_ERROR_VALIDATION_FAILED_EXT A command failed because invalid usage was detected by the implementation or a validation-layer.";
		case VK_ERROR_INVALID_SHADER_NV:
			return "VK_ERROR_INVALID_SHADER_NV One or more shaders failed to compile or link.";
		case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
			return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT ";
		case VK_ERROR_FRAGMENTATION_EXT:
			return "VK_ERROR_FRAGMENTATION A descriptor pool creation has failed due to fragmentation.";
		case VK_ERROR_NOT_PERMITTED_EXT:
			return "VK_ERROR_NOT_PERMITTED_EXT";
		case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
			return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT A buffer creation failed because the requested address is not available.";
		default:
			return std::to_string(static_cast<uint32_t>(result));
	}
}

static uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	return INT32_MAX;
}

static void CreateVKImageFromShareHandle(VkDevice& device, HANDLE handle, unsigned int width, unsigned int height, VkImage& outImage, VkDeviceMemory& outMemory)
{
	VkExternalMemoryImageCreateInfo ExternalMemoryImageCreateInfo = { VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO };
	ExternalMemoryImageCreateInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
	VkImageCreateInfo ImageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	ImageCreateInfo.pNext = &ExternalMemoryImageCreateInfo;
	ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	ImageCreateInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
	ImageCreateInfo.extent = { width, height, 1 };
	ImageCreateInfo.mipLevels = 1;
	ImageCreateInfo.arrayLayers = 1;
	ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	ImageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
	ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkResult result;
	if (result = vkCreateImage(device, &ImageCreateInfo, nullptr, &outImage); result != VK_SUCCESS)
	{
		FatalError("Failed to create a Vulkan image. VkResult: %s", ResultToString(result));
	}

	VkMemoryRequirements MemoryRequirements;
	vkGetImageMemoryRequirements(device, outImage, &MemoryRequirements);

	VkMemoryDedicatedAllocateInfo MemoryDedicatedAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO };
	MemoryDedicatedAllocateInfo.image = outImage;
	VkImportMemoryWin32HandleInfoKHR ImportMemoryWin32HandleInfo = { VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR };
	ImportMemoryWin32HandleInfo.pNext = &MemoryDedicatedAllocateInfo;
	ImportMemoryWin32HandleInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
	ImportMemoryWin32HandleInfo.handle = handle;
	VkMemoryAllocateInfo MemoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	MemoryAllocateInfo.pNext = &ImportMemoryWin32HandleInfo;
	MemoryAllocateInfo.allocationSize = MemoryRequirements.size;

	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(GetVulkanPhysicalHandle(), &memProps);

	MemoryAllocateInfo.memoryTypeIndex = FindMemoryType(GetVulkanPhysicalHandle(), MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (MemoryAllocateInfo.memoryTypeIndex == INT32_MAX)
	{
		FatalError("Failed to compatible memory type for NUI. This system may not be equipped to run RedM under Vulkan.\n");
	}

	if (result = vkAllocateMemory(device, &MemoryAllocateInfo, nullptr, &outMemory); result != VK_SUCCESS)
	{
		FatalError("Failed to allocate memory for Vulkan. VkResult: %s", ResultToString(result));
	}

	if (result = vkBindImageMemory(device, outImage, outMemory, 0); result != VK_SUCCESS)
	{
		FatalError("Failed to bind Vulkan image memory. VkResult: %s", ResultToString(result));
	}
}
#endif
