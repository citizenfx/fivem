#include <StdInc.h>
#include <CefOverlay.h>

#include <gameSkeleton.h>

#include <Hooking.h>

#include <sysAllocator.h>

namespace rage
{
	class audSound
	{
	public:
		virtual ~audSound() = 0;

		virtual void m_8() = 0;

		virtual void m_10() = 0;

		virtual void m_18() = 0;

		virtual void Init() = 0;

		virtual void m_28() = 0;

		void PrepareAndPlay(void* a1, bool a2, int a3, bool a4);

		void StopAndForget(void* a1);
	};

	static hook::cdecl_stub<void(audSound*, void*, bool, int, bool)> _audSound_PrepareAndPlay([]()
	{
		return hook::get_pattern("0F 85 D3 00 00 00 41 83 CB FF", -0x35);
	});

	static hook::cdecl_stub<void(audSound*, void*)> _audSound_StopAndForget([]()
	{
		return hook::get_pattern("74 24 45 0F B6 41 62", -0x10);
	});

	void audSound::PrepareAndPlay(void* a1, bool a2, int a3, bool a4)
	{
		_audSound_PrepareAndPlay(this, a1, a2, a3, a4);
	}

	void audSound::StopAndForget(void* a1)
	{
		_audSound_StopAndForget(this, a1);
	}

	class audReferencedRingBuffer : public sysUseAllocator
	{
	public:
		audReferencedRingBuffer();

		~audReferencedRingBuffer();

		inline void SetBuffer(void* buffer, uint32_t size)
		{
			m_data = buffer;
			m_size = size;
			m_initialized = true;
		}

		uint32_t PushAudio(const void* data, uint32_t size);

	private:
		void* m_data; // +0
		uint32_t m_size; // +8
		uint32_t m_fC; // +12
		uint32_t m_f10_0; // +16
		uint32_t m_f14_0; // +20
		uint32_t m_f18_0; // +24
		uint32_t m_pad_f1C; // +28
		CRITICAL_SECTION m_lock; // +32
		bool m_f48_0; // +72
		bool m_initialized; // +73
		uint8_t m_pad_f4A[6]; // +74
		uint32_t m_usageCount; // +80
		uint32_t m_f54; // +84
	};

	audReferencedRingBuffer::audReferencedRingBuffer()
	{
		m_data = nullptr;
		m_size = 0;

		m_f10_0 = 0;
		m_f14_0 = 0;
		m_f18_0 = 0;
		m_f48_0 = false;
		m_initialized = false;

		m_usageCount = 1;

		InitializeCriticalSectionAndSpinCount(&m_lock, 1000);
	}

	audReferencedRingBuffer::~audReferencedRingBuffer()
	{
		DeleteCriticalSection(&m_lock);
	}

	static hook::cdecl_stub<uint32_t(audReferencedRingBuffer*, const void*, uint32_t)> _audReferencedRingBuffer_PushAudio([]()
	{
		return hook::get_pattern("8B 71 08 48 8B 29 2B F0 48 8B D9 41 8B", -0x1B);
	});

	uint32_t audReferencedRingBuffer::PushAudio(const void* data, uint32_t size)
	{
		return _audReferencedRingBuffer_PushAudio(this, data, size);
	}

	class audExternalStreamSound : public rage::audSound
	{
	public:
		bool InitStreamPlayer(rage::audReferencedRingBuffer* buffer, int channels, int frequency);
	};

	static hook::cdecl_stub<bool(rage::audExternalStreamSound*, rage::audReferencedRingBuffer*, int, int)> _audExternalStreamSound_InitStreamPlayer([]()
	{
		return hook::get_pattern("F0 FF 42 50 48 89 11 44 89 41", -0x2A);
	});

	bool audExternalStreamSound::InitStreamPlayer(rage::audReferencedRingBuffer* buffer, int channels, int frequency)
	{
		return _audExternalStreamSound_InitStreamPlayer(this, buffer, channels, frequency);
	}

	class audCategory
	{

	};

	class audCategoryManager
	{
	public:
		rage::audCategory* GetCategoryPtr(uint32_t category);
	};

	static hook::cdecl_stub<rage::audCategory*(rage::audCategoryManager*, uint32_t)> _audCategoryManager_GetCategoryPtr([]()
	{
		return hook::get_call(hook::get_pattern("48 8B CB BA EA 75 96 D5 E8", 8));
	});

	rage::audCategory* audCategoryManager::GetCategoryPtr(uint32_t category)
	{
		return _audCategoryManager_GetCategoryPtr(this, category);
	}

	class audSoundInitParams
	{
	public:
		audSoundInitParams();

		void SetCategory(rage::audCategory* category);

	private:
		uint8_t m_pad[0xB0];
	};

	void audSoundInitParams::SetCategory(rage::audCategory* category)
	{
		*(audCategory**)(&m_pad[88]) = category;
	}

	static hook::cdecl_stub<void(rage::audSoundInitParams*)> _audSoundInitParams_ctor([]()
	{
		return hook::get_pattern("0F 57 C0 48 8D 41 10 BA 03 00 00 00");
	});

	audSoundInitParams::audSoundInitParams()
	{
		_audSoundInitParams_ctor(this);

		// 1604
		m_pad[0x9A] = *(uint32_t*)hook::get_adjusted(0x141C5F998);
	}

	class audEntity
	{
	public:
		virtual ~audEntity() = 0;

		void CreateSound_PersistentReference(const char* name, audSound** outSound, const audSoundInitParams& params);

		void CreateSound_PersistentReference(uint32_t nameHash, audSound** outSound, const audSoundInitParams& params);
	};

	static hook::cdecl_stub<void(audEntity*, const char*, audSound**, const audSoundInitParams&)> _audEntity_CreateSound_PersistentReference_char([]()
	{
		return hook::get_call(hook::get_pattern("4C 8D 4C 24 50 4C 8D 43 08 48 8D 0D", 0x14));
	});

	static hook::cdecl_stub<void(audEntity*, uint32_t, audSound**, const audSoundInitParams&)> _audEntity_CreateSound_PersistentReference_uint([]()
	{
		return (void*)hook::get_call(hook::get_call(hook::get_pattern<char>("4C 8D 4C 24 50 4C 8D 43 08 48 8D 0D", 0x14)) + 0x3F);
	});

	void audEntity::CreateSound_PersistentReference(const char* name, audSound** outSound, const audSoundInitParams& params)
	{
		return _audEntity_CreateSound_PersistentReference_char(this, name, outSound, params);
	}

	void audEntity::CreateSound_PersistentReference(uint32_t nameHash, audSound** outSound, const audSoundInitParams& params)
	{
		return _audEntity_CreateSound_PersistentReference_uint(this, nameHash, outSound, params);
	}

	audEntity* g_frontendAudioEntity;

	audCategoryManager* g_categoryMgr;

	static HookFunction hookFunction([]()
	{
		g_frontendAudioEntity = hook::get_address<audEntity*>(hook::get_pattern("4C 8D 4C 24 50 4C 8D 43 08 48 8D 0D", 0xC));

		g_categoryMgr = hook::get_address<audCategoryManager*>(hook::get_pattern("48 8B CB BA EA 75 96 D5 E8", -4));
	});
}

static hook::cdecl_stub<void()> _updateAudioThread([]()
{
	return hook::get_pattern("40 0F 95 C7 40 84 FF 74 05", -0x14);
});

extern "C"
{
#include <libswresample/swresample.h>
};

class RageAudioStream : public nui::IAudioStream
{
public:
	RageAudioStream(const nui::AudioStreamParams& params);

	virtual ~RageAudioStream();

	virtual void ProcessPacket(const float** data, int frames, int64 pts) override;

private:
	bool TryEnsureInitialized();

private:
	rage::audExternalStreamSound* m_sound;

	rage::audReferencedRingBuffer* m_buffer;

	uint8_t* m_bufferData;

	nui::AudioStreamParams m_initParams;

	SwrContext* m_avr;
};

class RageAudioSink : public nui::IAudioSink
{
public:
	virtual std::shared_ptr<nui::IAudioStream> CreateAudioStream(const nui::AudioStreamParams& params) override;
};

RageAudioStream::RageAudioStream(const nui::AudioStreamParams& params)
	: m_initParams(params)
{
	m_sound = nullptr;

	m_avr = swr_alloc();

	// channel layout
	int channelLayout = AV_CH_LAYOUT_MONO;

	if (params.channelLayout != nui::CefChannelLayout::CEF_CHANNEL_LAYOUT_MONO)
	{
		channelLayout = AV_CH_LAYOUT_STEREO;
	}

	// sample format
	AVSampleFormat sampleFormat = AV_SAMPLE_FMT_S16;
	int sampleRate = params.sampleRate;

	m_avr = swr_alloc_set_opts(m_avr, channelLayout, AV_SAMPLE_FMT_S16, 44100, channelLayout, sampleFormat, sampleRate, 0, NULL);

	swr_init(m_avr);
}

RageAudioStream::~RageAudioStream()
{
	if (m_sound)
	{
		m_sound->StopAndForget(nullptr);
	}

	swr_free(&m_avr);
}

void RageAudioStream::ProcessPacket(const float** data, int frames, int64 pts)
{
	if (TryEnsureInitialized())
	{
		// interleave and normalize the data
		std::vector<int16_t> buffer(frames * m_initParams.channels);
		auto cursor = buffer.data();

		for (int f = 0; f < frames; f++)
		{
			for (int c = 0; c < m_initParams.channels; c++)
			{
				*cursor++ = (int16_t)(data[c][f] * INT16_MAX);
			}
		}

		// resample to 44.1 kHz
		int outSamples = av_rescale_rnd(swr_get_delay(m_avr, m_initParams.sampleRate) + frames, (int)44100, m_initParams.sampleRate, AV_ROUND_UP);

		auto dataRef = buffer.data();

		uint8_t* resampledBytes;
		av_samples_alloc(&resampledBytes, NULL, m_initParams.channels, outSamples, AV_SAMPLE_FMT_S16, 0);
		outSamples = swr_convert(m_avr, &resampledBytes, outSamples, (const uint8_t**)&dataRef, frames);

		// push audio to the buffer
		m_buffer->PushAudio(resampledBytes, outSamples * sizeof(int16_t) * m_initParams.channels);

		// free
		av_freep(&resampledBytes);
	}
}

static bool audioRunning;

bool RageAudioStream::TryEnsureInitialized()
{
	if (m_sound)
	{
		return true;
	}

	if (audioRunning)
	{
		rage::audSoundInitParams initValues;
		
		// set the audio category
		if (!m_initParams.categoryName.empty())
		{
			auto category = rage::g_categoryMgr->GetCategoryPtr(HashString(m_initParams.categoryName.c_str()));

			if (category)
			{
				initValues.SetCategory(category);
			}
		}

		rage::g_frontendAudioEntity->CreateSound_PersistentReference(0xD8CE9439, (rage::audSound**)&m_sound, initValues);

		if (m_sound)
		{
			if (m_initParams.channelLayout == nui::CefChannelLayout::CEF_CHANNEL_LAYOUT_MONO)
			{
				m_initParams.channels = 1;
			}
			else
			{
				m_initParams.channels = 2;
			}

			// have ~1 second of audio buffer room
			auto size = 48000 * sizeof(int16_t) * m_initParams.channels;
			m_bufferData = (uint8_t*)rage::GetAllocator()->Allocate(size, 16, 0);

			m_buffer = new rage::audReferencedRingBuffer();
			m_buffer->SetBuffer(m_bufferData, size);

			m_sound->InitStreamPlayer(m_buffer, m_initParams.channels, 44100);
			m_sound->PrepareAndPlay(nullptr, true, -1, false);

			_updateAudioThread();

			return true;
		}
	}

	return false;
}

std::shared_ptr<nui::IAudioStream> RageAudioSink::CreateAudioStream(const nui::AudioStreamParams& params)
{
	return std::make_shared<RageAudioStream>(params);
}

static RageAudioSink g_audioSink;

static InitFunction initFunction([]()
{
	rage::OnInitFunctionInvoked.Connect([](rage::InitFunctionType type, const rage::InitFunctionData& data)
	{
		if (type == rage::InitFunctionType::INIT_CORE && data.funcHash == 0xE6D408DF)
		{
			audioRunning = true;
		}
	});

	//nui::SetAudioSink(&g_audioSink);
});
