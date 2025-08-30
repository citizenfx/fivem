#pragma once

class IMumbleAudioSink : public fwRefCountable
{
public:
	virtual void SetResetHandler(const std::function<void()>& resetter) = 0;

	virtual void SetPollHandler(const std::function<void(int)>& poller) = 0;

	virtual void SetPosition(float position[3], float distance, float overrideVolume) = 0;

	virtual void PushAudio(int16_t* pcm, int len) = 0;

	virtual bool IsTalkingAt(float distance) = 0;
};

extern
#ifndef COMPILING_VOIP_MUMBLE
DLL_IMPORT
#else
DLL_EXPORT
#endif
fwEvent<const std::string&, fwRefContainer<IMumbleAudioSink>*>
OnGetMumbleAudioSink;

extern
#ifndef COMPILING_VOIP_MUMBLE
DLL_IMPORT
#else
DLL_EXPORT
#endif
fwEvent<float>
OnSetMumbleVolume;
