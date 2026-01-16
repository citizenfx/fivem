#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <DrawCommands.h>

namespace d3d12
{
	// This commandQueue is created by us in order to not interfere with the games command queue when possible.
	static Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue()
	{
		ID3D12Device* device = (ID3D12Device*)GetGraphicsDriverHandle();

		static Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;

		if (!device)
		{
			return nullptr;
		}

		if (commandQueue)
		{
			return commandQueue;
		}

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.NodeMask = 0;

		HRESULT hr = ((ID3D12Device*)GetGraphicsDriverHandle())->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
		if (FAILED(hr))
		{
			trace("Unable to create CommandQueue 0x%x\n", hr);
			return nullptr;
		}

		return commandQueue;
	}

	static void SetupResourceBarrier(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = resource;
		barrier.Transition.StateBefore = stateBefore;
		barrier.Transition.StateAfter = stateAfter;
		barrier.Transition.Subresource = subresource;
		commandList->ResourceBarrier(1, &barrier);
	}
}
