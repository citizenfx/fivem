#if MONO_V2
using CitizenFX.Core;
using API = CitizenFX.FiveM.Native.Natives;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;

namespace CitizenFX.Core
#endif
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
			API.PlaySoundFromCoord(-1, sound, position.X, position.Y, position.Z, set, false, 0, false);
			return API.GetSoundId();
		}
		public static int PlaySoundAt(Vector3 position, string sound)
		{
			API.PlaySoundFromCoord(-1, sound, position.X, position.Y, position.Z, null, false, 0, false);
			return API.GetSoundId();
		}
		public static int PlaySoundFromEntity(Entity entity, string sound, string set)
		{
			API.PlaySoundFromEntity(-1, sound, entity.Handle, set, false, 0);
			return API.GetSoundId();
		}
		public static int PlaySoundFromEntity(Entity entity, string sound)
		{
			API.PlaySoundFromEntity(-1, sound, entity.Handle, null, false, 0);
			return API.GetSoundId();
		}
		public static int PlaySoundFrontend(string sound, string set)
		{
			API.PlaySoundFrontend(-1, sound, set, false);
			return API.GetSoundId();
		}
		public static int PlaySoundFrontend(string sound)
		{
			API.PlaySoundFrontend(-1, sound, null, false);
			return API.GetSoundId();
		}

		public static void StopSound(int id)
		{
			API.StopSound(id);
		}
		public static void ReleaseSound(int id)
		{
			API.ReleaseSoundId(id);
		}

		public static bool HasSoundFinished(int id)
		{
			return API.HasSoundFinished(id);
		}

		public static void SetAudioFlag(string flag, bool toggle)
		{
			API.SetAudioFlag(flag, toggle);
		}
		public static void SetAudioFlag(AudioFlag flag, bool toggle)
		{
			SetAudioFlag(_audioFlags[(int)flag], toggle);
		}
	}
}
