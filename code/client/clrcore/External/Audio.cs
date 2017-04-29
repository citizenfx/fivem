using CitizenFX.Core.Native;

namespace CitizenFX.Core
{
	public enum AudioFlag
	{
		ActivateSwitchWheelAudio,
		AllowCutsceneOverScreenFade,
		AllowForceRadioAfterRetune,
		AllowPainAndAmbientSpeechToPlayDuringCutscene,
		AllowPlayerAIOnMission,
		AllowPoliceScannerWhenPlayerHasNoControl,
		AllowRadioDuringSwitch,
		AllowRadioOverScreenFade,
		AllowScoreAndRadio,
		AllowScriptedSpeechInSlowMo,
		AvoidMissionCompleteDelay,
		DisableAbortConversationForDeathAndInjury,
		DisableAbortConversationForRagdoll,
		DisableBarks,
		DisableFlightMusic,
		DisableReplayScriptStreamRecording,
		EnableHeadsetBeep,
		ForceConversationInterrupt,
		ForceSeamlessRadioSwitch,
		ForceSniperAudio,
		FrontendRadioDisabled,
		HoldMissionCompleteWhenPrepared,
		IsDirectorModeActive,
		IsPlayerOnMissionForSpeech,
		ListenerReverbDisabled,
		LoadMPData,
		MobileRadioInGame,
		OnlyAllowScriptTriggerPoliceScanner,
		PlayMenuMusic,
		PoliceScannerDisabled,
		ScriptedConvListenerMaySpeak,
		SpeechDucksScore,
		SuppressPlayerScubaBreathing,
		WantedMusicDisabled,
		WantedMusicOnMission
	}

	public static class Audio
	{
		#region Fields
		internal static readonly string[] _audioFlags = {
			"ActivateSwitchWheelAudio",
			"AllowCutsceneOverScreenFade",
			"AllowForceRadioAfterRetune",
			"AllowPainAndAmbientSpeechToPlayDuringCutscene",
			"AllowPlayerAIOnMission",
			"AllowPoliceScannerWhenPlayerHasNoControl",
			"AllowRadioDuringSwitch",
			"AllowRadioOverScreenFade",
			"AllowScoreAndRadio",
			"AllowScriptedSpeechInSlowMo",
			"AvoidMissionCompleteDelay",
			"DisableAbortConversationForDeathAndInjury",
			"DisableAbortConversationForRagdoll",
			"DisableBarks",
			"DisableFlightMusic",
			"DisableReplayScriptStreamRecording",
			"EnableHeadsetBeep",
			"ForceConversationInterrupt",
			"ForceSeamlessRadioSwitch",
			"ForceSniperAudio",
			"FrontendRadioDisabled",
			"HoldMissionCompleteWhenPrepared",
			"IsDirectorModeActive",
			"IsPlayerOnMissionForSpeech",
			"ListenerReverbDisabled",
			"LoadMPData",
			"MobileRadioInGame",
			"OnlyAllowScriptTriggerPoliceScanner",
			"PlayMenuMusic",
			"PoliceScannerDisabled",
			"ScriptedConvListenerMaySpeak",
			"SpeechDucksScore",
			"SuppressPlayerScubaBreathing",
			"WantedMusicDisabled",
			"WantedMusicOnMission"
		};
		#endregion

		public static int PlaySoundAt(Vector3 position, string sound, string set)
		{
			Function.Call(Hash.PLAY_SOUND_FROM_COORD, -1, sound, position.X, position.Y, position.Z, set, 0, 0, 0);
			return Function.Call<int>(Hash.GET_SOUND_ID);
		}
		public static int PlaySoundAt(Vector3 position, string sound)
		{
			Function.Call(Hash.PLAY_SOUND_FROM_COORD, -1, sound, position.X, position.Y, position.Z, 0, 0, 0, 0);
			return Function.Call<int>(Hash.GET_SOUND_ID);
		}
		public static int PlaySoundFromEntity(Entity entity, string sound, string set)
		{
			Function.Call(Hash.PLAY_SOUND_FROM_ENTITY, -1, sound, entity.Handle, set, 0, 0);
			return Function.Call<int>(Hash.GET_SOUND_ID);
		}
		public static int PlaySoundFromEntity(Entity entity, string sound)
		{
			Function.Call(Hash.PLAY_SOUND_FROM_ENTITY, -1, sound, entity.Handle, 0, 0, 0);
			return Function.Call<int>(Hash.GET_SOUND_ID);
		}
		public static int PlaySoundFrontend(string sound, string set)
		{
			Function.Call(Hash.PLAY_SOUND_FRONTEND, -1, sound, set, 0);
			return Function.Call<int>(Hash.GET_SOUND_ID);
		}
		public static int PlaySoundFrontend(string sound)
		{
			Function.Call(Hash.PLAY_SOUND_FRONTEND, -1, sound, 0, 0);
			return Function.Call<int>(Hash.GET_SOUND_ID);
		}

		public static void StopSound(int id)
		{
			Function.Call(Hash.STOP_SOUND, id);
		}
		public static void ReleaseSound(int id)
		{
			Function.Call(Hash.RELEASE_SOUND_ID, id);
		}

		public static bool HasSoundFinished(int id)
		{
			return Function.Call<bool>(Hash.HAS_SOUND_FINISHED, id);
		}

		public static void SetAudioFlag(string flag, bool toggle)
		{
			Function.Call(Hash.SET_AUDIO_FLAG, flag, toggle);
		}
		public static void SetAudioFlag(AudioFlag flag, bool toggle)
		{
			SetAudioFlag(_audioFlags[(int)flag], toggle);
		}
	}
}
