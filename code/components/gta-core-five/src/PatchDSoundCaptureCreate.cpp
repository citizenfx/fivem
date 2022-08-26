#include <StdInc.h>
#include <Hooking.h> // for HookFunction and hook::iat

#include "gameSkeleton.h" // for INIT_SESSION end
#include <ICoreGameInit.h> // for OneSyncEnabled

#include <unordered_set>

#include <mmsystem.h> // for WAVEFORMATEX, required before dsound.h

// dsound.h doesn't include these, and WRL::RuntimeClass needs this defined
struct __declspec(uuid("b0210781-89cd-11d0-af08-00a0c925cd16")) IDirectSoundCapture;
struct __declspec(uuid("b0210782-89cd-11d0-af08-00a0c925cd16")) IDirectSoundCaptureBuffer;

#include <dsound.h>
#include <wrl.h>

namespace WRL = Microsoft::WRL;

class DirectSoundCaptureBuffer : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IDirectSoundCaptureBuffer>
{
public:
	DirectSoundCaptureBuffer(LPCDSCBUFFERDESC bufferDesc)
	{
		// we don't handle effects
		assert(bufferDesc->dwFXCount == 0);

		m_bufferDesc = *bufferDesc;

		// wave format is an external buffer to application-managed memory
		// in Five this points to a static-lifetime struct, but this may not always be the case
		m_waveFormat = *m_bufferDesc.lpwfxFormat;
		m_bufferDesc.lpwfxFormat = &m_waveFormat;
	}

	// *must* be called with the original device
	bool Materialize(IDirectSoundCapture* device)
	{
		// safeguard to not materialize twice
		if (m_underlyingBuffer)
		{
			return true;
		}

		// create original buffer
		if (SUCCEEDED(device->CreateCaptureBuffer(&m_bufferDesc, &m_underlyingBuffer, NULL)))
		{
			// start, if a start was pending
			if (m_started)
			{
				m_underlyingBuffer->Start(m_startFlags);
			}

			return true;
		}

		return false;
	}

	virtual HRESULT __stdcall GetCaps(LPDSCBCAPS pDSCBCaps) override
	{
		// doesn't actually get called, but for good measure
		if (m_underlyingBuffer)
		{
			return m_underlyingBuffer->GetCaps(pDSCBCaps);
		}

		return E_NOTIMPL;
	}

	virtual HRESULT __stdcall GetCurrentPosition(LPDWORD pdwCapturePosition, LPDWORD pdwReadPosition) override
	{
		if (m_underlyingBuffer)
		{
			return m_underlyingBuffer->GetCurrentPosition(pdwCapturePosition, pdwReadPosition);
		}

		// if not materialized, we just pretend to not have any data
		if (pdwCapturePosition)
		{
			*pdwCapturePosition = 0;
		}

		if (pdwReadPosition)
		{
			*pdwReadPosition = 0;
		}

		return S_OK;
	}

	virtual HRESULT __stdcall GetFormat(LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten) override
	{
		// doesn't actually get called, but for good measure
		if (m_underlyingBuffer)
		{
			return m_underlyingBuffer->GetFormat(pwfxFormat, dwSizeAllocated, pdwSizeWritten);
		}

		return E_NOTIMPL;
	}

	virtual HRESULT __stdcall GetStatus(LPDWORD pdwStatus) override
	{
		// doesn't actually get called, but for good measure
		if (m_underlyingBuffer)
		{
			return m_underlyingBuffer->GetStatus(pdwStatus);
		}

		return E_NOTIMPL;
	}

	virtual HRESULT __stdcall Initialize(LPDIRECTSOUNDCAPTURE pDirectSoundCapture, LPCDSCBUFFERDESC pcDSCBufferDesc) override
	{
		// not used by applications
		return E_NOTIMPL;
	}

	virtual HRESULT __stdcall Lock(DWORD dwOffset, DWORD dwBytes, LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags) override
	{
		if (m_underlyingBuffer)
		{
			return m_underlyingBuffer->Lock(dwOffset, dwBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2, dwFlags);
		}

		// shouldn't be called if not materialized, so this is fine
		return E_NOTIMPL;
	}

	virtual HRESULT __stdcall Start(DWORD dwFlags) override
	{
		if (m_underlyingBuffer)
		{
			return m_underlyingBuffer->Start(dwFlags);
		}

		// save start flags and to-start state
		m_startFlags = dwFlags;
		m_started = true;

		return S_OK;
	}

	virtual HRESULT __stdcall Stop(void) override
	{
		if (m_underlyingBuffer)
		{
			return m_underlyingBuffer->Stop();
		}

		// note down that we *aren't* started anymore
		m_started = false;

		return S_OK;
	}

	virtual HRESULT __stdcall Unlock(LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2) override
	{
		if (m_underlyingBuffer)
		{
			return m_underlyingBuffer->Unlock(pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
		}

		// shouldn't be called if not materialized
		return E_NOTIMPL;
	}

private:
	// saved buffer desc for materialize
	DSCBUFFERDESC m_bufferDesc;

	// wave format for materialize
	WAVEFORMATEX m_waveFormat;

	// the real buffer
	WRL::ComPtr<IDirectSoundCaptureBuffer> m_underlyingBuffer;

	// whether to call Start() on materialize
	bool m_started = false;

	// flags to pass to Start()
	DWORD m_startFlags = 0;
};

// list of capture objects to materialize
static std::mutex g_capturesMutex;
static std::unordered_set<IDirectSoundCapture*> g_captures;

// global toggle to 'bypass' materialization
static bool g_captureEnabled = false;

static HRESULT(WINAPI* g_origDSoundCaptureCreate)(LPCGUID pcGuidDevice, LPDIRECTSOUNDCAPTURE* ppDSC, LPUNKNOWN pUnkOuter);

class DirectSoundCapture : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IDirectSoundCapture>
{
public:
	DirectSoundCapture(LPCGUID deviceGuid)
		: m_deviceGuid(*deviceGuid)
	{
		// keep track of us
		std::unique_lock _(g_capturesMutex);
		g_captures.insert(this);
	}

	virtual ~DirectSoundCapture()
	{
		std::unique_lock _(g_capturesMutex);
		g_captures.erase(this);
	}

	bool Materialize()
	{
		// don't materialize twice
		if (m_underlyingDevice)
		{
			return true;
		}

		// create a capture
		if (FAILED(g_origDSoundCaptureCreate(&m_deviceGuid, &m_underlyingDevice, NULL)))
		{
			return false;
		}

		// materialize any pending buffers
		for (const auto& buffer : m_captureBuffers)
		{
			if (!buffer->Materialize(m_underlyingDevice.Get()))
			{
				return false;
			}
		}

		return true;
	}

	virtual HRESULT WINAPI CreateCaptureBuffer(LPCDSCBUFFERDESC pcDSCBufferDesc, LPDIRECTSOUNDCAPTUREBUFFER* ppDSCBuffer, LPUNKNOWN pUnkOuter) override
	{
		auto captureBuffer = WRL::Make<DirectSoundCaptureBuffer>(pcDSCBufferDesc);

		if (m_underlyingDevice)
		{
			// materialize right away if we have a device
			if (!captureBuffer->Materialize(m_underlyingDevice.Get()))
			{
				return E_FAIL;
			}
		}

		// store the buffer to materialize later
		m_captureBuffers.push_back(captureBuffer);

		return captureBuffer.CopyTo(ppDSCBuffer);
	}

	virtual HRESULT WINAPI GetCaps(LPDSCCAPS pDSCCaps) override
	{
		if (m_underlyingDevice)
		{
			return m_underlyingDevice->GetCaps(pDSCCaps);
		}

		return E_NOTIMPL;
	}

	virtual HRESULT WINAPI Initialize(LPCGUID pcGuidDevice) override
	{
		// not used by apps
		return E_NOTIMPL;
	}

private:
	// the device GUID to create
	GUID m_deviceGuid;

	// capture buffers to materialize when we get materialized
	std::vector<WRL::ComPtr<DirectSoundCaptureBuffer>> m_captureBuffers;

	// real device
	WRL::ComPtr<IDirectSoundCapture> m_underlyingDevice;
};

static HRESULT WINAPI DirectSoundCaptureCreateWrap(LPCGUID pcGuidDevice, LPDIRECTSOUNDCAPTURE *ppDSC, LPUNKNOWN pUnkOuter)
{
	auto dsoundCapture = WRL::Make<DirectSoundCapture>(pcGuidDevice);

	// if capture was enabled, start materializing
	if (g_captureEnabled)
	{
		if (!dsoundCapture->Materialize())
		{
			return E_FAIL;
		}
	}

	return dsoundCapture.CopyTo(ppDSC);
}

static void MaterializeDevices()
{
	// mark capture as enabled (so any further devices get immediately materialized)
	g_captureEnabled = true;

	// materialize any stored devices
	std::unique_lock _(g_capturesMutex);
	for (auto capture : g_captures)
	{
		static_cast<DirectSoundCapture*>(capture)->Materialize();
	}
}

static HookFunction hookFunction([]
{
	rage::OnInitFunctionEnd.Connect([](rage::InitFunctionType type)
	{
		// on session end...
		if (type == rage::INIT_SESSION)
		{
			// if joining a non-1s server (aka: needs built-in voice)
			if (!Instance<ICoreGameInit>::Get()->OneSyncEnabled)
			{
				// materialize devices
				MaterializeDevices();
			}
		}
	});

	g_origDSoundCaptureCreate = hook::iat("dsound.dll", DirectSoundCaptureCreateWrap, 6);
});
