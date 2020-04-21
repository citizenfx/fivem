#include <StdInc.h>

// TODO: decouple for specific D3D11 features
#ifndef IS_RDR3
#include <dxgi.h>
#include <d3d11.h>
#include <wrl.h>

#include <chrono>

#include <CefOverlay.h>

extern nui::GameInterface* g_nuiGi;
fwEvent<std::chrono::microseconds, std::chrono::microseconds> OnVSync;

namespace gl {
	namespace {
		Microsoft::WRL::ComPtr<IDXGIOutput> DXGIOutputFromMonitor(
			HMONITOR monitor,
			const Microsoft::WRL::ComPtr<ID3D11Device>& d3d11_device) {
			Microsoft::WRL::ComPtr<IDXGIDevice> dxgi_device;
			if (FAILED(d3d11_device.As(&dxgi_device))) {
				return nullptr;
			}

			Microsoft::WRL::ComPtr<IDXGIAdapter> dxgi_adapter;
			if (FAILED(dxgi_device->GetAdapter(&dxgi_adapter))) {
				return nullptr;
			}

			size_t i = 0;
			while (true) {
				Microsoft::WRL::ComPtr<IDXGIOutput> output;
				if (FAILED(dxgi_adapter->EnumOutputs(i++, &output)))
					break;

				DXGI_OUTPUT_DESC desc = {};
				if (FAILED(output->GetDesc(&desc))) {
					return nullptr;
				}

				if (desc.Monitor == monitor)
					return output;
			}

			return nullptr;
		}
	}  // namespace

	   
	static HookFunction postInitFunction([]()
	{
		std::thread([]()
		{
			SetThreadName(-1, "GPUVsync");
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

			static HMONITOR primary_monitor_;
			static Microsoft::WRL::ComPtr<IDXGIOutput> primary_output_;

			while (true)
			{
				if (!g_nuiGi || !g_nuiGi->GetD3D11Device())
				{
					Sleep(50);
					continue;
				}

				// From Raymond Chen's blog "How do I get a handle to the primary monitor?"
				// https://devblogs.microsoft.com/oldnewthing/20141106-00/?p=43683
				HMONITOR monitor = MonitorFromWindow(nullptr, MONITOR_DEFAULTTOPRIMARY);
				if (primary_monitor_ != monitor) {
					primary_monitor_ = monitor;
					primary_output_ = DXGIOutputFromMonitor(monitor, g_nuiGi->GetD3D11Device());
				}

				auto interval = std::chrono::microseconds(int64_t(1000000.0 / 60));

				MONITORINFOEX monitor_info = {};
				monitor_info.cbSize = sizeof(MONITORINFOEX);
				if (monitor && GetMonitorInfo(monitor, &monitor_info)) {
					DEVMODE display_info = {};
					display_info.dmSize = sizeof(DEVMODE);
					display_info.dmDriverExtra = 0;
					if (EnumDisplaySettings(monitor_info.szDevice, ENUM_CURRENT_SETTINGS,
						&display_info) &&
						display_info.dmDisplayFrequency > 1) {
						interval =
							std::chrono::microseconds(int64_t(1000000.0 / display_info.dmDisplayFrequency));
					}
				}

				// according to https://crbug.com/953970#c30:
				// "there is a process wide lock in Win7 there prevents other DX from running in the process if you call WaitForVBlank."
				// stick to the approximate sleep code path on Win7 therefore
				static auto win8 = IsWindows8OrGreater();

				std::chrono::steady_clock::duration wait_for_vblank_start_time = std::chrono::high_resolution_clock::now().time_since_epoch();
				bool wait_for_vblank_succeeded =
					primary_output_ && win8 && SUCCEEDED(primary_output_->WaitForVBlank());

				// WaitForVBlank returns very early instead of waiting until vblank when the
				// monitor goes to sleep.  We use 1ms as a threshold for the duration of
				// WaitForVBlank and fallback to Sleep() if it returns before that.  This
				// could happen during normal operation for the first call after the vsync
				// thread becomes non-idle, but it shouldn't happen often.
				const auto kVBlankIntervalThreshold = std::chrono::milliseconds(1);
				std::chrono::steady_clock::duration wait_for_vblank_elapsed_time =
					std::chrono::high_resolution_clock::now().time_since_epoch() - wait_for_vblank_start_time;

				if (!wait_for_vblank_succeeded ||
					wait_for_vblank_elapsed_time < kVBlankIntervalThreshold) {
					Sleep(static_cast<DWORD>(std::chrono::duration_cast<std::chrono::milliseconds>(interval).count()));
				}

				auto vsync_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
				OnVSync(vsync_time, std::chrono::duration_cast<std::chrono::microseconds>(interval));
			}
		}).detach();
	});

}  // namespace gl
#endif
