#include <StdInc.h>
#include <Hooking.h>

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
	auto vramLocation = hook::get_address<uint64_t*>(hook::get_pattern("4C 63 C0 48 8D 05 ? ? ? ? 48 8D 14", 6));

	OnGrcCreateDevice.Connect([vramLocation, totalPhys]()
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
							auto changeBudget = [vramLocation, totalPhys](uint64_t budget)
							{
								auto maxBudget = std::max(std::min(budget, (totalPhys / 2)) - (2 * GB), uint64_t(0xBBA00000));

								trace("VRAM budget change: patching game to use %d byte budget (clamped to %d due to system RAM)\n", budget, maxBudget);

								for (int i = 0; i < 80; i++)
								{
									vramLocation[i] = maxBudget;
								}
							};

							changeBudget(vmi.Budget);

							std::thread([adapter3, changeBudget]()
							{
								SetThreadName(-1, "[Cfx] VRAM Budgeting");

								HANDLE hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);

								if (!hEvent)
								{
									return;
								}

								DWORD cookie;

								if (SUCCEEDED(adapter3->RegisterVideoMemoryBudgetChangeNotificationEvent(hEvent, &cookie)))
								{
									while (true)
									{
										WaitForSingleObject(hEvent, INFINITE);

										DXGI_QUERY_VIDEO_MEMORY_INFO vmi;

										if (SUCCEEDED(adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &vmi)))
										{
											changeBudget(vmi.Budget);
										}
									}
								}
							}).detach();
						}
					}
				}
			}
		}
	});
});
