#pragma once

class IMumbleAudioSink : public fwRefCountable
{
public:
	virtual void SetPosition(float position[3], float distance, float overrideVolume) = 0;

	virtual void PushAudio(int16_t* pcm, int len) = 0;
};

extern
#ifndef COMPILING_VOIP_MUMBLE
DLL_IMPORT
#else
DLL_EXPORT
#endif
fwEvent<const std::wstring&, fwRefContainer<IMumbleAudioSink>*>
OnGetMumbleAudioSink;

extern
#ifndef COMPILING_VOIP_MUMBLE
DLL_IMPORT
#else
DLL_EXPORT
#endif
fwEvent<float>
OnSetMumbleVolume;
