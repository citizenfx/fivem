#include <StdInc.h>
#include <Hooking.h>

#include <CoreConsole.h>
#include <DrawCommands.h>

#include <dxgi1_4.h>
#include <wrl.h>

namespace WRL = Microsoft::WRL;

// use 1000 for one so we catch 'hardware reserved' memory as well
constexpr uint64_t GB = 1000 * 1024 * 1024;

static int extRamMode = 0;

static HookFunction hookFunction([]()
{
	MEMORYSTATUSEX msex = { 0 };
	msex.dwLength = sizeof(msex);

	GlobalMemoryStatusEx(&msex);

	uint32_t allocatorReservation = 0;

	if (msex.ullTotalPhys >= 16 * GB)
	{
		trace(">16 GiB of system RAM, increasing streaming allocator size to 2 GiB\n");

		allocatorReservation = 0x7FFFFFFF;
		extRamMode = 2;
	}
	else if (msex.ullTotalPhys >= 12 * GB)
	{
		trace(">12 GiB of system RAM, increasing streaming allocator size to 1.5 GiB\n");

		allocatorReservation = 0x60000000;
		extRamMode = 1;
	}

	auto vramLocation = hook::get_address<uint64_t*>(hook::get_pattern("4C 63 C0 48 8D 05 ? ? ? ? 48 8D 14", 6));

	// the full code will 100% break 4/4GB systems
	if (extRamMode == 0)
	{
		// but we can lower the 'normal' and 'low' quality setting a bit so it doesn't ruin 8/8 systems entirely
		//
		// externally, there are 4 texture settings:
		// 0: 'normal'
		// 1: 'high'
		// 2: 'very high'
		// 3: no override
		//
		// however, these are implemented by means of an internal flag which is the amount of mips to cut off, as follows:
		// 0: 'low' (non-existent setting, cut off 2 mips)
		// 1: 'normal' (cut off 1 mip)
		// 2: 'high' (cut off no mips)
		// x: 'very high' (cut off no mips, and allow +hi TXDs)
		//
		// the issue is R* seems to have thought in a few cases that the internal texture setting flag mapped to normal, high, very high,
		// and not unused, normal, high/very high, where 'very high' is just 'high' with the +hi loading flag enabled.
		//
		// instead, we'll just keep high and very high the same, but move the 'low' flag to 'normal' and lower the 'low' size some more in case
		// anyone feels like enabling the real 'low'
		for (int i = 0; i < 80; i += 4)
		{
			vramLocation[i + 1] = vramLocation[i] * 0.9; // bit of extra fiddling to help <12 GB systems
			vramLocation[i] *= 0.75;
		}

		return;
	}

	// extend grcResourceCache pool a bit
	{
		auto location = hook::get_pattern<char>("BA 00 00 05 00 48 8B C8 44 88");
		hook::put<uint32_t>(location + 1, 0xA0000);
		hook::put<uint32_t>(location + 23, 0xA001B);
	}

	// increase allocator amount
	auto location = hook::get_pattern("41 B8 00 00 00 40 48 8B D5 89", 2);

	if (allocatorReservation)
	{
		hook::put<uint32_t>(location, allocatorReservation);
	}

	auto totalPhys = msex.ullTotalPhys;

	auto changeBudget = [vramLocation, totalPhys](uint64_t budget)
	{
		auto maxBudget = std::max(std::min(int64_t(budget), int64_t(totalPhys / 2)), int64_t(0xBBA00000));

		console::DPrintf("graphics", "VRAM budget change: patching game to use %d byte budget (clamped to %d due to system RAM)\n", budget, maxBudget);

		for (int i = 0; i < 80; i += 4)
		{
			// fix R* vision of the texture quality weirdness
			vramLocation[i + 1] = vramLocation[i];
			vramLocation[i] *= 0.75;
			vramLocation[i + 2] = maxBudget;
			vramLocation[i + 3] = maxBudget;
		}
	};

	OnGrcCreateDevice.Connect([changeBudget]()
	{
		auto device = GetD3D11Device();
		
		WRL::ComPtr<IDXGIDevice> dxgiDevice;

		if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&dxgiDevice))))
		{
			WRL::ComPtr<IDXGIAdapter> adapter;
			
			if (SUCCEEDED(dxgiDevice->GetAdapter(&adapter)))
			{
				WRL::ComPtr<IDXGIAdapter3> adapter3;

				if (SUCCEEDED(adapter.As(&adapter3)))
				{
					DXGI_QUERY_VIDEO_MEMORY_INFO vmi;

					if (SUCCEEDED(adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &vmi)))
					{
						if (vmi.Budget >= (3 * GB))
						{
							changeBudget(4 * GB);
						}
					}
				}
			}
		}
	});
});
