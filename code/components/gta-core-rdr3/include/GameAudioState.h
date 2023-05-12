#pragma once

// Returns whether or not any game-related audio should be muted as a result of 'mute on focus loss'
bool
#ifdef COMPILING_GTA_CORE_RDR3
DLL_EXPORT
#else
DLL_IMPORT
#endif
	ShouldMuteGameAudio();
