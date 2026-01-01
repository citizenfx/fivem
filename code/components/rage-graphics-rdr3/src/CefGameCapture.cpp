#include "StdInc.h"

#include <vulkan/vulkan.h>
#include <d3d12.h>
#include <d3d11_1.h>

#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <dxgi.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <Error.h>

#include <CoreConsole.h>
#include <HostSharedData.h>

#include <VulkanHelper.h>
#include <CrossBuildRuntime.h>

#include <D3D12Helper.h>

#include <vulkan/vulkan_win32.h>
#pragma comment(lib, "vulkan-1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")


#include <wrl.h>
namespace WRL = Microsoft::WRL;

struct GameRenderData
{
	HANDLE handle = NULL;
	int width = 0;
	int height = 0;
	bool requested = false;
};

static void* g_lastBackbufTexture;

static void RenderBufferToBuffer(ID3D12Resource* resource, int width = 0, int height = 0)
{
	static auto didCallCrashometry = ([]()
	{
		AddCrashometry("did_render_backbuf", "true");

		return true;
	})();

	ID3D12Device* device = (ID3D12Device*)GetGraphicsDriverHandle();

	if (!device)
	{
		return;
	}

	static WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	static WRL::ComPtr<ID3D12GraphicsCommandList> commandList;

	if (!commandAllocator)
	{
		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	}
	else
	{
		commandAllocator->Reset();
	}

	if (!commandList)
	{
		device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	}
	else
	{
		commandList->Reset(commandAllocator.Get(), nullptr);
	}

	rage::sga::BackBufferData* data = rage::sga::Driver_GetBackBuffer();
	if (!data || !data->m_texture)
	{
		return;
	}

	ID3D12Resource* backBuffer = ((rage::sga::TextureD3D12*)data->m_texture)->resource;
	if (!backBuffer)
	{
		return;
	}

	D3D12_RESOURCE_DESC srcDesc = backBuffer->GetDesc();
	D3D12_RESOURCE_DESC dstDesc = resource->GetDesc();

	if (srcDesc.Width != dstDesc.Width || srcDesc.Height != dstDesc.Height || srcDesc.Format != dstDesc.Format)
	{
		return;
	}

	d3d12::SetupResourceBarrier(commandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_SOURCE);
	commandList->CopyResource(resource, backBuffer);
	d3d12::SetupResourceBarrier(commandList, backBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PRESENT);

	HRESULT hr = commandList->Close();

	if (FAILED(hr))
	{
		return;
	}

	WRL::ComPtr<ID3D12CommandQueue> queue = d3d12::GetCommandQueue();

	if (!queue)
	{
		return;
	}

	static WRL::ComPtr<ID3D12Fence> fence;
	static HANDLE fenceEvent = nullptr;
	static UINT64 fenceValue = 0;

	if (!fence)
	{
		hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
		assert(SUCCEEDED(hr));
		fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		assert(fenceEvent != nullptr);
	}

	ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
	queue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	hr = queue->Signal(fence.Get(), ++fenceValue);
	assert(SUCCEEDED(hr));

	if (fence->GetCompletedValue() < fenceValue)
	{
		hr = fence->SetEventOnCompletion(fenceValue, fenceEvent);
		assert(SUCCEEDED(hr));
		WaitForSingleObject(fenceEvent, INFINITE);
	}
}

static void CaptureBufferOutputDX()
{
	static HostSharedData<GameRenderData> handleData("CfxGameRenderHandle");
	static D3D12_RESOURCE_DESC resDesc;
	static int lastWidth = 1;
	static int lastHeight = 1;

	rage::sga::BackBufferData* data = rage::sga::Driver_GetBackBuffer();
	if (!data)
	{
		return;
	}

	rage::sga::TextureD3D12* sgaTexture = ((rage::sga::TextureD3D12*)data->m_texture);
	if (!sgaTexture)
	{
		return;
	}

	ID3D12Resource* backBuf = sgaTexture->resource;
	if (!backBuf)
	{
		return;
	}

	resDesc = backBuf->GetDesc();
	handleData->width = resDesc.Width;
	handleData->height = resDesc.Height;

	bool shouldChange = false;
	static ID3D12Resource* resource = nullptr;

	if (lastWidth != handleData->width || lastHeight != handleData->height || !resource)
	{
		lastWidth = handleData->width;
		lastHeight = handleData->height;

		if (resource)
		{
			resource->Release();
			resource = nullptr;
		}

		shouldChange = true;
	}

	if (shouldChange)
	{
		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = resDesc.Width;
		texDesc.Height = resDesc.Height;
		texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		texDesc.MipLevels = 1;
		texDesc.DepthOrArraySize = 1;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		ID3D12Device* device = (ID3D12Device*)GetGraphicsDriverHandle();
		if (!device)
		{
			return;
		}

		HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_SHARED, &texDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&resource));

		if (FAILED(hr))
		{			return;
		}

		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = 1;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
		device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;

		device->CreateRenderTargetView(resource, &rtvDesc, rtvHandle);

		HANDLE sharedHandle = nullptr;
		hr = device->CreateSharedHandle(resource, nullptr, GENERIC_ALL, nullptr, &sharedHandle);

		if (FAILED(hr))
		{
			return;
		}

		handleData->handle = sharedHandle;
	}

	if (!resource)
	{
		return;
	}

	if (!handleData->requested)
	{
		return;
	}

	RenderBufferToBuffer(resource);
}

namespace WRL = Microsoft::WRL;

// These will only ever exist when VK driver is being used and exclusively for game view capture.
static ID3D11Device* g_d3d11VKDevice = nullptr;
static ID3D11DeviceContext* g_d3d11VKDeviceContext = nullptr;
static ID3D11Texture2D* g_d3d11VKStagingTexture = nullptr;
static WRL::ComPtr<ID3D11Texture2D> g_d3d11VKTexture = nullptr;

static HANDLE g_vkDX11StagingHandle = NULL;

// Experimental (and currently not fully functional). Make the VK image a share handle, write to it, open it in D3D11, copy the contents to a new texture and provide a safe handle for GLES.
static void RenderBufferToBuffer(VkImage image, VkDeviceMemory memory, uint32_t width = 0, uint32_t height = 0)
{
	static HostSharedData<GameRenderData> handleData("CfxGameRenderHandle");

	static auto didCallCrashometry = ([]()
	{
		AddCrashometry("did_render_backbufvk", "true");

		return true;
	})();

	rage::sga::BackBufferData* data = rage::sga::Driver_GetBackBuffer();
	if (!data)
	{
		return;
	}

	rage::sga::TextureVK* texture = (rage::sga::TextureVK*)data->m_texture;
	if (!texture)
	{
		return;
	}

	VkDevice device = (VkDevice)GetGraphicsDriverHandle();
	VkPhysicalDevice physicalDevice = (VkPhysicalDevice)GetVulkanPhysicalDevice();

	if (!device || !physicalDevice)
	{
		return;
	}

	VkImage backbufferImage = texture->image->image;
	VkDeviceMemory backbufferMemory = texture->image->memory;

	static bool isInitalized = false;
	static VkCommandPool commandPool;
	static VkQueue graphicsQueue;
	static VkFence copyFence = VK_NULL_HANDLE;

	if (!copyFence)
	{
		VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		vkCreateFence(device, &fenceInfo, nullptr, &copyFence);
	}
	else
	{
		vkResetFences(device, 1, &copyFence);
	}

	if (!graphicsQueue)
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		int graphicsQueueFamilyIndex = -1;

		for (uint32_t i = 0; i < queueFamilies.size(); i++)
		{
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				graphicsQueueFamilyIndex = i;
				break;
			}
		}
		vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &graphicsQueue);
	}

	if (!commandPool)
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = 0;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
	}
}

static void CaptureBufferOutputVK()
{
	static HostSharedData<GameRenderData> handleData("CfxGameRenderHandle");
	static int lastWidth = 1;
	static int lastHeight = 1;

	// OpenGL ES (GLES) doesn't play nice with vulkan texture share handles and will usuaully crash on trying to handle them. In order to handle capturing game-view for users.
	// We create a dummy D3D11 device that sole purpose is to handle copying the vulkan owned backbuffer to a d3d11 device and pass that to GLES
	if (!g_d3d11VKDevice)
	{
		// We should be safe to assume that the users GPU (that is currently running at minimum vulkan 1.2) supports a basic feature level of DX11.
		D3D_FEATURE_LEVEL featureLevels[] = {
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};

		IDXGIAdapter* ppAdapter = nullptr;
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

					adapter.CopyTo(&ppAdapter);
					break;
				}
			}
		}

		if (FAILED(D3D11CreateDevice(ppAdapter,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			D3D11_CREATE_DEVICE_BGRA_SUPPORT,
			featureLevels,
			ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION,
			&g_d3d11VKDevice,
			nullptr,
			&g_d3d11VKDeviceContext)))
		{
			trace("Unable to create D3D11 Device for game capture\n");
			return;
		}
	}

	rage::sga::BackBufferData* data = rage::sga::Driver_GetBackBuffer();
	if (!data)
	{
		return;
	}

	rage::sga::TextureVK* texture = (rage::sga::TextureVK*)data->m_texture;

	if (!texture)
	{
		return;
	}

	VkDevice device = (VkDevice)GetGraphicsDriverHandle();
	VkPhysicalDevice physicalDevice = (VkPhysicalDevice)GetVulkanPhysicalDevice();

	handleData->width = texture->width;
	handleData->height = texture->height;

	static VkImage vkImage = nullptr;
	static VkDeviceMemory vkMemory = nullptr;

	if (!g_d3d11VKTexture || (handleData->width != lastWidth || handleData->height != lastHeight))
	{
		if (g_d3d11VKTexture)
		{
			g_d3d11VKTexture->Release();
			g_d3d11VKTexture = nullptr;
		}

		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = handleData->width;
		texDesc.Height = handleData->height;
		texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

		g_d3d11VKDevice->CreateTexture2D(&texDesc, nullptr, &g_d3d11VKTexture);

		WRL::ComPtr<IDXGIResource> dxgiResource;
		HANDLE sharedHandle;
		HRESULT hr = g_d3d11VKTexture.As(&dxgiResource);
		if (FAILED(hr))
		{
			return;
		}

		hr = dxgiResource->GetSharedHandle(&sharedHandle);
		if (FAILED(hr))
		{
			return;
		}
		handleData->handle = sharedHandle;
	}

	if (handleData->width != lastWidth || handleData->height != lastHeight)
	{
		lastWidth = handleData->width;
		lastHeight = handleData->height;

		VkExternalMemoryImageCreateInfo externalInfo{};
		externalInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
		externalInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_KMT_BIT;

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
		imageInfo.extent = { (unsigned int)handleData->width, (unsigned int)handleData->height, 1 };
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.pNext = &externalInfo;
		vkCreateImage(device, &imageInfo, nullptr, &vkImage);

        VkExportMemoryAllocateInfo exportAlloc{};
		exportAlloc.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO;
		exportAlloc.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_KMT_BIT;

		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(device, vkImage, &memReq);

		uint32_t index = FindMemoryType(physicalDevice, memReq.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReq.size;
		allocInfo.memoryTypeIndex = index;
		allocInfo.pNext = &exportAlloc;

		vkAllocateMemory(device, &allocInfo, nullptr, &vkMemory);
		vkBindImageMemory(device, vkImage, vkMemory, 0);

		VkMemoryGetWin32HandleInfoKHR handleInfo{};
		handleInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR;
		handleInfo.memory = vkMemory;
		handleInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_KMT_BIT;

		static PFN_vkGetMemoryWin32HandleKHR vkGetMemoryWin32HandleKHR = (PFN_vkGetMemoryWin32HandleKHR)vkGetDeviceProcAddr(device, "vkGetMemoryWin32HandleKHR");
		VkResult res = vkGetMemoryWin32HandleKHR(device, &handleInfo, &g_vkDX11StagingHandle);

		if (res != VK_SUCCESS)
		{
			return;
		}
	}

	if (!vkImage || !vkMemory)
	{
		return;
	}

	if (!handleData->requested)
	{
		return;
	}

	RenderBufferToBuffer(vkImage, vkMemory, lastWidth, lastHeight);
}

static HookFunction hookFunction([]()
{
	OnPostFrontendRender.Connect([]()
	{
		uintptr_t a1 = 0;
		uintptr_t a2 = 0;

		EnqueueGenericDrawCommand([](uintptr_t, uintptr_t)
		{
			if (GetCurrentGraphicsAPI() == GraphicsAPI::D3D12)
			{
				CaptureBufferOutputDX();
			}
			else
			{
				CaptureBufferOutputVK();
			}
		},
		&a1, &a2);
	},
	INT32_MAX);
});
