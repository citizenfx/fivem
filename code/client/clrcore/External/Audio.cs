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

		/// <remarks>This method allocates a sound ID. It is imperative you release the sound ID when you are done using it through <see cref="ReleaseSound"/></remarks>
		public static int PlaySoundAt(Vector3 position, string sound, string set)
		{
			var soundId = API.GetSoundId();
			API.PlaySoundFromCoord(soundId, sound, position.X, position.Y, position.Z, set, false, 0, false);
			return soundId;
		}

		/// <inheritdoc cref="PlaySoundAt(Vector3, string, string)"/>
		public static int PlaySoundAt(Vector3 position, string sound)
		{
			var soundId = API.GetSoundId();
			API.PlaySoundFromCoord(soundId, sound, position.X, position.Y, position.Z, null, false, 0, false);
			return soundId;
		}

		/// <inheritdoc cref="PlaySoundAt(Vector3, string, string)"/>
		public static int PlaySoundFromEntity(Entity entity, string sound, string set)
		{
			var soundId = API.GetSoundId();
			API.PlaySoundFromEntity(soundId, sound, entity.Handle, set, false, 0);
			return soundId;
		}

		/// <inheritdoc cref="PlaySoundAt(Vector3, string, string)"/>
		public static int PlaySoundFromEntity(Entity entity, string sound)
		{
			var soundId = API.GetSoundId();
			API.PlaySoundFromEntity(soundId, sound, entity.Handle, null, false, 0);
			return soundId;
		}

		/// <inheritdoc cref="PlaySoundAt(Vector3, string, string)"/>
		public static int PlaySoundFrontend(string sound, string set)
		{
			var soundId = API.GetSoundId();
			API.PlaySoundFrontend(soundId, sound, set, false);
			return soundId;
		}

		/// <inheritdoc cref="PlaySoundAt(Vector3, string, string)"/>
		public static int PlaySoundFrontend(string sound)
		{
			var soundId = API.GetSoundId();
			API.PlaySoundFrontend(soundId, sound, null, false);
			return soundId;
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
