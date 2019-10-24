using System;
using CitizenFX.Core.Native;
using System.Threading.Tasks;
using CitizenFX.Core;
using System.Security;
using System.Runtime.InteropServices;

namespace CitizenFX.Core
{
	public enum GameVersion
	{
		Unknown = -1,
		v1_0_335_2_Steam,
		v1_0_335_2_NoSteam,
		v1_0_350_1_Steam,
		v1_0_350_2_NoSteam,
		v1_0_372_2_Steam,
		v1_0_372_2_NoSteam,
		v1_0_393_2_Steam,
		v1_0_393_2_NoSteam,
		v1_0_393_4_Steam,
		v1_0_393_4_NoSteam,
		v1_0_463_1_Steam,
		v1_0_463_1_NoSteam,
		v1_0_505_2_Steam,
		v1_0_505_2_NoSteam,
		v1_0_573_1_Steam,
		v1_0_573_1_NoSteam,
		v1_0_617_1_Steam,
		v1_0_617_1_NoSteam,
		v1_0_678_1_Steam,
		v1_0_678_1_NoSteam,
		v1_0_757_2_Steam,
		v1_0_757_2_NoSteam,
		v1_0_757_3_Steam,
		v1_0_757_4_NoSteam,
		v1_0_791_2_Steam,
		v1_0_791_2_NoSteam,
		v1_0_877_1_Steam,
		v1_0_877_1_NoSteam,
		v1_0_944_2_Steam,
		v1_0_944_2_NoSteam,
		v1_0_1011_1_Steam,
		v1_0_1011_1_NoSteam,
		v1_0_1032_1_Steam,
		v1_0_1032_1_NoSteam,
		v1_0_1103_2_Steam,
		v1_0_1103_2_NoSteam
	}
	public enum Language
	{
		American,
		French,
		German,
		Italian,
		Spanish,
		Portuguese,
		Polish,
		Russian,
		Korean,
		Chinese,
		Japanese,
		Mexican
	}
	public enum InputMode
	{
		MouseAndKeyboard,
		GamePad
	}
	public enum WindowTitle
	{
		CELL_EMAIL_BOD,
		CELL_EMAIL_BODE,
		CELL_EMAIL_BODF,
		CELL_EMAIL_SOD,
		CELL_EMAIL_SODE,
		CELL_EMAIL_SODF,
		CELL_EMASH_BOD,
		CELL_EMASH_BODE,
		CELL_EMASH_BODF,
		CELL_EMASH_SOD,
		CELL_EMASH_SODE,
		CELL_EMASH_SODF,
		FMMC_KEY_TIP10,
		FMMC_KEY_TIP12,
		FMMC_KEY_TIP12F,
		FMMC_KEY_TIP12N,
		FMMC_KEY_TIP8,
		FMMC_KEY_TIP8F,
		FMMC_KEY_TIP8FS,
		FMMC_KEY_TIP8S,
		FMMC_KEY_TIP9,
		FMMC_KEY_TIP9F,
		FMMC_KEY_TIP9N,
		PM_NAME_CHALL
	}

	public static class Game
	{
		#region Fields
		internal static readonly string[] _radioNames = {
			"RADIO_01_CLASS_ROCK",
			"RADIO_02_POP",
			"RADIO_03_HIPHOP_NEW",
			"RADIO_04_PUNK",
			"RADIO_05_TALK_01",
			"RADIO_06_COUNTRY",
			"RADIO_07_DANCE_01",
			"RADIO_08_MEXICAN",
			"RADIO_09_HIPHOP_OLD",
			"RADIO_11_TALK_02",
			"RADIO_12_REGGAE",
			"RADIO_13_JAZZ",
			"RADIO_14_DANCE_02",
			"RADIO_15_MOTOWN",
			"RADIO_16_SILVERLAKE",
			"RADIO_17_FUNK",
			"RADIO_18_90S_ROCK",
			"RADIO_19_USER",
			"RADIO_20_THELAB",
			"RADIO_21_DLC_XM17",
			"RADIO_22_DLC_BATTLE_MIX1_RADIO",
			"RADIO_OFF"
		};

		static Player _cachedPlayer;
		#endregion

		static Game()
		{
			Version = GameVersion.v1_0_1103_2_NoSteam;
		}

		/// <summary>
		/// Gets the current GameVersion.
		/// </summary>	
		public static GameVersion Version { get; private set; }

		/// <summary>
		/// Gets the game Language.
		/// </summary>		
		public static Language Language
		{
			get
			{
				return (Language)API.GetUiLanguageId();
			}
		}

		/// <summary>
		/// Gets how many milliseconds the game has been open in this session
		/// </summary> 
		public static int GameTime
		{
			get
			{
				return API.GetGameTimer();
			}
		}
		/// <summary>
		/// Sets the time scale of the Game.
		/// </summary>
		/// <value>
		/// The Time Scale, only accepts values in range 0.0f to 1.0f
		/// </value>
		public static float TimeScale
		{
			set
			{
				API.SetTimeScale(value);
			}
		}
		/// <summary>
		/// Gets the total number of frames that's been rendered in this session.
		/// </summary>
		public static int FrameCount
		{
			get
			{
				return API.GetFrameCount();
			}
		}
		/// <summary>
		/// Gets the current frame rate per second.
		/// </summary>
		public static float FPS
		{
			get
			{
				return 1.0f / LastFrameTime;
			}
		}
		/// <summary>
		/// Gets the time it currently takes to render a frame, in seconds.
		/// </summary>
		public static float LastFrameTime
		{
			get
			{
				return API.GetFrameTime();
			}
		}

		/// <summary>
		/// Gets or sets the maximum wanted level a <see cref="Player"/> can receive.
		/// </summary>
		/// <value>
		/// The maximum wanted level, only accepts values 0 to 5
		/// </value>
		public static int MaxWantedLevel
		{
			get
			{
				return API.GetMaxWantedLevel();
			}
			set
			{
				if (value < 0)
				{
					value = 0;
				}
				else if (value > 5)
				{
					value = 5;
				}

				API.SetMaxWantedLevel(value);
			}
		}

		/// <summary>
		/// Sets the wanted level multiplier
		/// </summary>
		/// <value>
		/// The multiplier to apply to a players wanted level
		/// </value>
		public static float WantedMultiplier
		{
			set
			{
				API.SetWantedLevelMultiplier(value);
			}
		}

		/// <summary>
		/// Sets a value indicating whether Police <see cref="Blip"/>s should be visible on the Minimap
		/// </summary>
		public static bool ShowsPoliceBlipsOnRadar
		{
			set
			{
				API.SetPoliceRadarBlips(value);
			}
		}

		/// <summary>
		/// Gets or sets the radio station.
		/// </summary>
		public static RadioStation RadioStation
		{
			get
			{
				string radioName = API.GetPlayerRadioStationName();

				if (string.IsNullOrEmpty(radioName))
				{
					return RadioStation.RadioOff;
				}

				return (RadioStation)Array.IndexOf(_radioNames, radioName);
			}
			set
			{
				if (Enum.IsDefined(typeof(RadioStation), value) && value != RadioStation.RadioOff)
				{
					API.SetRadioToStationName(_radioNames[(int)value]);
				}
				else
				{
					API.SetRadioToStationName(null);
				}
			}
		}

		/// <summary>
		/// Gets the <see cref="Player"/> that you are controling
		/// </summary>
		public static Player Player
		{
			get
			{
				int handle = API.PlayerId();

				if (ReferenceEquals(_cachedPlayer, null) || handle != _cachedPlayer.Handle)
				{
					_cachedPlayer = new Player(handle);
				}

				return _cachedPlayer;
			}
		}

		/// <summary>
		/// Gets the <see cref="Ped"/> that you are controling
		/// </summary>
		public static Ped PlayerPed
		{
			get
			{
				return Player.Character;
			}
		}

		/// <summary>
		/// Gets or sets a value indicating whether to render the world with a night vision filter
		/// </summary>
		public static bool Nightvision
		{
			get
			{
				return API.IsNightvisionActive();
			}
			set
			{
				API.SetNightvision(value);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether to render the world with a thermal vision filter
		/// </summary>
		public static bool ThermalVision
		{
			get
			{
				return API.IsSeethroughActive();
			}
			set
			{
				API.SetSeethrough(value);
			}
		}

		/// <summary>
		/// Gets or sets a value informing the Game Engine if a mission is in progress
		/// </summary>
		/// <value>
		/// if <c>true</c> a mission is currently active; otherwise, <c>false</c>
		/// </value>
		public static bool IsMissionActive
		{
			get
			{
				return API.GetMissionFlag();
			}
			set
			{
				API.SetMissionFlag(value);
			}
		}
		/// <summary>
		/// Gets or sets a value informing the Game Engine if a random event is in progress.
		/// </summary>
		/// <value>
		/// if <c>true</c> a random event is currently active; otherwise, <c>false</c>
		/// </value>
		public static bool IsRandomEventActive
		{
			get
			{
				return API.GetRandomEventFlag() == 1;
			}
			set
			{
				API.SetRandomEventFlag(value ? 1 : 0);
			}
		}

		/// <summary>
		/// Gets or a value indicating whether the cutscene is active.
		/// </summary>
		/// <value>
		/// if <c>true</c> a cutscene is currently active; otherwise, <c>false</c>
		/// </value>
		public static bool IsCutsceneActive
		{
			get
			{
				return API.IsCutsceneActive();
			}
		}
		/// <summary>
		/// Gets a value indicating whether there is a Waypoint set
		/// </summary>
		public static bool IsWaypointActive
		{
			get
			{
				return API.IsWaypointActive();
			}
		}

		/// <summary>
		/// Gets or sets a value indicating whether the game is paused
		/// </summary>
		public static bool IsPaused
		{
			get
			{
				return API.IsPauseMenuActive();
			}
			set
			{
				API.SetPauseMenuActive(value);
			}
		}
		/// <summary>
		/// Gets a value indicating whether there is a loading screen being displayed
		/// </summary>
		public static bool IsLoading
		{
			get
			{
				return API.GetIsLoadingScreenActive();
			}
		}

		/// <summary>
		/// Gets whether the last input was made with a GamePad or Keyboard and Mouse
		/// </summary>
		public static InputMode CurrentInputMode
		{
			get
			{
				return API.IsInputDisabled(2) ? InputMode.MouseAndKeyboard : InputMode.GamePad;
			}
		}
		/*
		/// <summary>
		/// Gets whether a <see cref="Keys"/> is currently held down
		/// </summary>
		/// <param name="key">The key.</param>
		/// <returns></returns>
		public static bool IsKeyPressed(Keys key)
		{
			return ScriptDomain.CurrentDomain.IsKeyPressed(key);
		}
		*/

		/// <summary>
		/// Gets whether a <see cref="ButtonCombination"/> was entered.
		/// </summary>
		/// <param name="combination">The <see cref="ButtonCombination"/> to check against.</param>
		/// <returns><c>true</c> if the <see cref="ButtonCombination"/> was just entered; otherwise, <c>false</c></returns>
		/// <remarks>
		/// Only works for Gamepad inputs
		/// Cheat combinations use the same system
		/// </remarks>
		public static bool WasButtonCombinationJustEntered(ButtonCombination combination)
		{
			return API.HasButtonCombinationJustBeenEntered((uint)combination.Hash, combination.Length);
		}
		/// <summary>
		/// Gets whether a cheat code was entered into the cheat text box
		/// </summary>
		/// <param name="cheat">The name of the cheat to check.</param>
		/// <returns><c>true</c> if the cheat was just entered; otherwise, <c>false</c></returns>
		public static bool WasCheatStringJustEntered(string cheat)
		{
			return API.HasCheatStringJustBeenEntered((uint)GenerateHash(cheat));
		}

		/// <summary>
		/// Gets whether a <see cref="Control"/> is currently pressed
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns><c>true</c> if the <see cref="Control"/> is pressed; otherwise, <c>false</c></returns>
		public static bool IsControlPressed(int index, Control control)
		{
			return API.IsDisabledControlPressed(index, (int)control);
		}
		/// <summary>
		/// Gets whether a <see cref="Control"/> was just pressed this frame
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns><c>true</c> if the <see cref="Control"/> was just pressed this frame; otherwise, <c>false</c></returns>
		public static bool IsControlJustPressed(int index, Control control)
		{
			return API.IsDisabledControlJustPressed(index, (int)control);
		}
		/// <summary>
		/// Gets whether a <see cref="Control"/> was just released this frame
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns><c>true</c> if the <see cref="Control"/> was just released this frame; otherwise, <c>false</c></returns>
		public static bool IsControlJustReleased(int index, Control control)
		{
			return API.IsDisabledControlJustReleased(index, (int)control);
		}
		/// <summary>
		/// Gets whether a <see cref="Control"/> is Enabled this frame and is currently pressed
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns><c>true</c> if the <see cref="Control"/> is pressed; otherwise, <c>false</c></returns>
		public static bool IsEnabledControlPressed(int index, Control control)
		{
			return API.IsControlPressed(index, (int)control);
		}
		/// <summary>
		/// Gets whether a <see cref="Control"/> is Enabled and was just pressed this frame
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns><c>true</c> if the <see cref="Control"/> was just pressed this frame; otherwise, <c>false</c></returns>
		public static bool IsEnabledControlJustPressed(int index, Control control)
		{
			return API.IsControlJustPressed(index, (int)control);
		}
		/// <summary>
		/// Gets whether a <see cref="Control"/> is Enabled and was just released this frame
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns><c>true</c> if the <see cref="Control"/> was just released this frame; otherwise, <c>false</c></returns>
		public static bool IsEnabledControlJustReleased(int index, Control control)
		{
			return API.IsControlJustReleased(index, (int)control);
		}
		/// <summary>
		/// Gets whether a <see cref="Control"/> is Disabled this frame and is currently pressed
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns><c>true</c> if the <see cref="Control"/> is pressed; otherwise, <c>false</c></returns>
		public static bool IsDisabledControlPressed(int index, Control control)
		{
			return IsControlPressed(index, control) && !IsControlEnabled(index, control);
		}
		/// <summary>
		/// Gets whether a <see cref="Control"/> is Disabled and was just pressed this frame
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns><c>true</c> if the <see cref="Control"/> was just pressed this frame; otherwise, <c>false</c></returns>
		public static bool IsDisabledControlJustPressed(int index, Control control)
		{
			return IsControlJustPressed(index, control) && !IsControlEnabled(index, control);
		}
		/// <summary>
		/// Gets whether a <see cref="Control"/> is Disabled and was just released this frame
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns><c>true</c> if the <see cref="Control"/> was just released this frame; otherwise, <c>false</c></returns>
		public static bool IsDisabledControlJustReleased(int index, Control control)
		{
			return IsControlJustReleased(index, control) && !IsControlEnabled(index, control);
		}
		/// <summary>
		/// Gets whether a <see cref="Control"/> is Enabled ot Disabled this frame
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns><c>true</c> if the <see cref="Control"/> is Enabled; otherwise, <c>false</c></returns>
		public static bool IsControlEnabled(int index, Control control)
		{
			return API.IsControlEnabled(index, (int)control);
		}

		/// <summary>
		/// Makes the Game Engine respond to the given Control this frame
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		public static void EnableControlThisFrame(int index, Control control)
		{
			API.EnableControlAction(index, (int)control, true);
		}
		/// <summary>
		/// Makes the Game Engine ignore to the given Control this frame
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		public static void DisableControlThisFrame(int index, Control control)
		{
			API.DisableControlAction(index, (int)control, true);
		}
		/// <summary>
		/// Disables all <see cref="Control"/>s this frame.
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		public static void DisableAllControlsThisFrame(int index)
		{
			API.DisableAllControlActions(index);
		}
		/// <summary>
		/// Enables all <see cref="Control"/>s this frame.
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		public static void EnableAllControlsThisFrame(int index)
		{
			API.EnableAllControlActions(index);
		}

		/// <summary>
		/// Gets an Analog value of a <see cref="Control"/> input between -1.0f and 1.0f
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns>The normalised <see cref="Control"/> value</returns>
		public static float GetControlNormal(int index, Control control)
		{
			return API.GetControlNormal(index, (int)control);
		}
		/// <summary>
		/// Gets an Analog value of a Disabled <see cref="Control"/> input between -1.0f and 1.0f
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns>The normalised <see cref="Control"/> value</returns>
		public static float GetDisabledControlNormal(int index, Control control)
		{
			return API.GetDisabledControlNormal(index, (int)control);
		}
		/// <summary>
		/// Gets an value of a <see cref="Control"/> input.
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns>The <see cref="Control"/> value</returns>
		public static int GetControlValue(int index, Control control)
		{
			return API.GetControlValue(index, (int)control);
		}
		/// <summary>
		/// Override a <see cref="Control"/> by giving it a user defined value this frame.
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <param name="value">the value to set the control to.</param>
		public static void SetControlNormal(int index, Control control, float value)
		{
			API.SetControlNormal(index, (int)control, value);
		}

		/// <summary>
		/// Pauses or Resumes the game
		/// </summary>
		/// <param name="value">if set to <c>true</c> Pause the game; otherwise, resume the game.</param>
		public static void Pause(bool value)
		{
			API.SetGamePaused(value);
		}
		/// <summary>
		/// Pauses or Resumes the game clock
		/// </summary>
		/// <param name="value">if set to <c>true</c> Pause the game clock; otherwise, resume the game clock.</param>
		public static void PauseClock(bool value)
		{
			API.PauseClock(value);
		}

		/// <summary>
		/// Performs and automative game save
		/// </summary>
		public static void DoAutoSave()
		{
			API.DoAutoSave();
		}
		/// <summary>
		/// Shows the save menu enabling the user to perform a manual game save.
		/// </summary>
		public static void ShowSaveMenu()
		{
			API.SetSaveMenuActive(true);
		}

		/// <summary>
		/// Determines the game language files contain a entry for the specified GXT key
		/// </summary>
		/// <param name="entry">The GXT key.</param>
		/// <returns><c>true</c> if GXT entry exists; otherwise, <c>false</c></returns>
		public static bool DoesGXTEntryExist(string entry)
		{
			return API.DoesTextLabelExist(entry);
		}

		/// <summary>
		/// Returns a localised <see cref="string"/> from the games language files with a specified GXT key
		/// </summary>
		/// <param name="entry">The GXT key.</param>
		/// <returns>The localised <see cref="string"/> if the key exists; otherwise, <see cref="string.Empty"/></returns>
		public static string GetGXTEntry(string entry)
		{
			return DoesGXTEntryExist(entry) ? API.GetLabelText(entry) : string.Empty;
		}
		internal static bool DoesGXTEntryExist(ulong entry)
		{
			return API.DoesTextLabelExist(entry.ToString());
		}

		internal static string GetGXTEntry(ulong entry)
		{
			return DoesGXTEntryExist(entry) ? API.GetLabelText(entry.ToString()) : string.Empty;
		}

		/// <summary>
		/// Calculates a Jenkins One At A Time hash from the given <see cref="string"/> which can then be used by any native function that takes a hash
		/// </summary>
		/// <param name="input">The input <see cref="string"/> to hash.</param>
		/// <returns>The Jenkins hash of the <see cref="string"/></returns>
		public static int GenerateHash(string input)
		{
			if (string.IsNullOrEmpty(input))
			{
				return 0;
			}
			return unchecked((int)MemoryAccess.GetHashKey(input));
		}

		/// <summary>
		/// Plays a sound from the games sound files
		/// </summary>
		/// <param name="soundFile">The file the sound is stored in.</param>
		/// <param name="soundSet">The name of the sound inside the file.</param>
		public static void PlaySound(string soundFile, string soundSet)
		{
			Audio.ReleaseSound(Audio.PlaySoundFrontend(soundFile, soundSet));
		}

		/// <summary>
		/// Plays music from the games music files
		/// </summary>
		/// <param name="musicFile">The music file to play.</param>
		public static void PlayMusic(string musicFile)
		{
			API.TriggerMusicEvent(musicFile);
		}
		/// <summary>
		/// Stops playing a music file
		/// </summary>
		/// <param name="musicFile">The music file to stop.</param>
		public static void StopMusic(string musicFile)
		{
			API.CancelMusicEvent(musicFile);
		}
		/// <summary>
		/// Creates an input box for enabling a user to input text using the keyboard
		/// </summary>
		/// <param name="maxLength">The maximum length of input allowed.</param>
		/// <returns>The <see cref="string"/> of what the user entered, If the user cancelled <see cref="string.Empty"/> is returned</returns>
		public static async Task<string> GetUserInput(int maxLength)
		{
			return await GetUserInput(WindowTitle.FMMC_KEY_TIP8, string.Empty, maxLength);
		}
		/// <summary>
		/// Creates an input box for enabling a user to input text using the keyboard
		/// </summary>
		/// <param name="defaultText">The default text.</param>
		/// <param name="maxLength">The maximum length of input allowed.</param>
		/// <returns>The <see cref="string"/> of what the user entered, If the user cancelled <see cref="string.Empty"/> is returned</returns>
		public static async Task<string> GetUserInput(string defaultText, int maxLength)
		{
			return await GetUserInput(WindowTitle.FMMC_KEY_TIP8, defaultText, maxLength);
		}
		/// <summary>
		/// Creates an input box for enabling a user to input text using the keyboard
		/// </summary>
		/// <param name="windowTitle">The Title of the Window.</param>
		/// <param name="maxLength">The maximum length of input allowed.</param>
		/// <returns>The <see cref="string"/> of what the user entered, If the user cancelled <see cref="string.Empty"/> is returned</returns>
		public static async Task<string> GetUserInput(WindowTitle windowTitle, int maxLength)
		{
			return await GetUserInput(windowTitle, string.Empty, maxLength);
		}
		/// <summary>
		/// Creates an input box for enabling a user to input text using the keyboard
		/// </summary>
		/// <param name="windowTitle">The Title of the Window.</param>
		/// <param name="defaultText">The default text.</param>
		/// <param name="maxLength">The maximum length of input allowed.</param>
		/// <returns>The <see cref="string"/> of what the user entered, If the user cancelled <see cref="string.Empty"/> is returned</returns>
		public static async Task<string> GetUserInput(WindowTitle windowTitle, string defaultText, int maxLength)
		{
			//ScriptDomain.CurrentDomain.PauseKeyboardEvents(true);

			ClearKeyboard(windowTitle, defaultText, maxLength);

			while (API.UpdateOnscreenKeyboard() == 0)
			{
				await BaseScript.Delay(0);
			}

			//ScriptDomain.CurrentDomain.PauseKeyboardEvents(false);

			return API.GetOnscreenKeyboardResult();
		}

		private static void ClearKeyboard(WindowTitle windowTitle, string defaultText, int maxLength)
		{
			API.DisplayOnscreenKeyboard(1, windowTitle.ToString(), null, defaultText, null, null, null, maxLength + 1);
		}



		/// <summary>
		/// Private unsafe version of <see cref="GetTattooCollectionData(int, int)"/>
		/// </summary>
		/// <param name="characterType"></param>
		/// <param name="decorationIndex"></param>
		/// <returns></returns>
		[SecuritySafeCritical]
		private static TattooCollectionData _GetTattooCollectionData(int characterType, int decorationIndex)
		{
			UnsafeTattooCollectionData data;
			unsafe
			{
				Function.Call((Hash)0xFF56381874F82086, characterType, decorationIndex, &data);
			}
			return data.GetData();
		}

		/// <summary>
		/// Returns a <see cref="TattooCollectionData"/> struct containing information about a specific tattoo.
		/// Currently only the <see cref="TattooCollectionData.TattooCollectionHash"/>, <see cref="TattooCollectionData.TattooNameHash"/>
		/// and <see cref="TattooCollectionData.TattooZone"/> are known. It's still unkown what the other values are used for or if 
		/// they're even correctly offset in the byte array.
		/// </summary>
		/// <param name="characterType">Character types 0 = Michael, 1 = Franklin, 2 = Trevor, 3 = MPMale, 4 = MPFemale</param>
		/// <param name="decorationIndex">Tattoo index, value between 0 and <see cref="API.GetNumDecorations(int)"/></param>
		/// <returns></returns>
		public static TattooCollectionData GetTattooCollectionData(int characterType, int decorationIndex)
		{
			if (characterType > -1 && characterType < 5)
			{
				return _GetTattooCollectionData(characterType, decorationIndex);
			}
			return new TattooCollectionData();
		}



		/// <summary>
		/// Private (unsafe) version of <see cref="GetAltPropVariationData(int, int)"/>
		/// </summary>
		/// <param name="ped"></param>
		/// <param name="propIndex"></param>
		/// <returns></returns>
		[SecuritySafeCritical]
		private static AltPropVariationData[] _GetAltPropVariationData(int ped, int propIndex)
		{
			int prop = API.GetPedPropIndex(ped, propIndex);
			int propTexture = API.GetPedPropTextureIndex(ped, propIndex);
			uint propHashName = (uint)API.GetHashNameForProp(ped, propIndex, prop, propTexture);

			uint someHash = 0;
			int unk1 = 0;
			int unk2 = 0;

			int maxVariations = API.N_0xd40aac51e8e4c663(propHashName);

			AltPropVariationData[] items = new AltPropVariationData[maxVariations];
			if (maxVariations > 0)
			{
				items = new AltPropVariationData[maxVariations];
				for (var i = 0; i < maxVariations; i++)
				{
					UnsafeAltPropVariationData data = new UnsafeAltPropVariationData();
					unsafe
					{
						Function.Call((Hash)0xD81B7F27BC773E66, propHashName, i, &someHash, &unk1, &unk2);
					}
					unsafe
					{
						Function.Call((Hash)0x5D5CAFF661DDF6FC, someHash, &data);
					}
					items[i] = data.GetData(someHash, unk1, unk2);
				}
				return items;
			}
			return null;
		}

		/// <summary>
		/// Gets the alternate prop index data for a specific prop on a specific ped.
		/// This is used to check for the 'alternate' version of a helmet with a visor for example (open/closed visor variants).
		/// </summary>
		/// <param name="ped"></param>
		/// <param name="propIndex"></param>
		/// <returns></returns>
		public static AltPropVariationData[] GetAltPropVariationData(int ped, int propIndex)
		{
			return _GetAltPropVariationData(ped, propIndex);
		}


		[StructLayout(LayoutKind.Explicit, Size = 0x28)]
		[SecuritySafeCritical]
		internal struct UnsafeWeaponHudStats
		{
			[FieldOffset(0x00)] private int hudDamage;

			[FieldOffset(0x08)] private int hudSpeed;

			[FieldOffset(0x10)] private int hudCapacity;

			[FieldOffset(0x18)] private int hudAccuracy;

			[FieldOffset(0x20)] private int hudRange;


			public WeaponHudStats GetSafeStats()
			{
				return new WeaponHudStats(hudDamage, hudSpeed, hudCapacity, hudAccuracy, hudRange);
			}
		}

		public struct WeaponHudStats
		{
			public int hudDamage;
			public int hudSpeed;
			public int hudCapacity;
			public int hudAccuracy;
			public int hudRange;

			public WeaponHudStats(int hudDamage, int hudSpeed, int hudCapacity, int hudAccuracy, int hudRange)
			{
				this.hudDamage = hudDamage;
				this.hudSpeed = hudSpeed;
				this.hudCapacity = hudCapacity;
				this.hudAccuracy = hudAccuracy;
				this.hudRange = hudRange;
			}

			public override bool Equals(object obj)
			{
				if (obj is WeaponHudStats stat)
				{
					return stat.hudDamage == hudDamage &&
						stat.hudSpeed == hudSpeed &&
						stat.hudCapacity == hudCapacity &&
						stat.hudAccuracy == hudAccuracy &&
						stat.hudRange == hudRange;
				}
				return false;
			}

			public override int GetHashCode()
			{
				unchecked
				{
					int hash = 13;
					hash = (hash * 7) + hudDamage.GetHashCode();
					hash = (hash * 7) + hudSpeed.GetHashCode();
					hash = (hash * 7) + hudCapacity.GetHashCode();
					hash = (hash * 7) + hudAccuracy.GetHashCode();
					hash = (hash * 7) + hudRange.GetHashCode();
					return hash;
				}
			}

			public static bool operator ==(WeaponHudStats left, WeaponHudStats right)
			{
				return left.Equals(right);
			}

			public static bool operator !=(WeaponHudStats left, WeaponHudStats right)
			{
				return !(left == right);
			}
		}

		[StructLayout(LayoutKind.Explicit, Size = 0x28)]
		[SecuritySafeCritical]
		internal struct UnsafeWeaponComponentHudStats
		{
			[FieldOffset(0x00)] private int hudDamage;

			[FieldOffset(0x08)] private int hudSpeed;

			[FieldOffset(0x10)] private int hudCapacity;

			[FieldOffset(0x18)] private int hudAccuracy;

			[FieldOffset(0x20)] private int hudRange;


			public WeaponComponentHudStats GetSafeStats()
			{
				return new WeaponComponentHudStats(hudDamage, hudSpeed, hudCapacity, hudAccuracy, hudRange);
			}
		}

		public struct WeaponComponentHudStats
		{
			public int hudDamage;
			public int hudSpeed;
			public int hudCapacity;
			public int hudAccuracy;
			public int hudRange;

			public WeaponComponentHudStats(int hudDamage, int hudSpeed, int hudCapacity, int hudAccuracy, int hudRange)
			{
				this.hudDamage = hudDamage;
				this.hudSpeed = hudSpeed;
				this.hudCapacity = hudCapacity;
				this.hudAccuracy = hudAccuracy;
				this.hudRange = hudRange;
			}

			public override bool Equals(object obj)
			{
				if (obj is WeaponComponentHudStats stat)
				{
					return stat.hudDamage == hudDamage &&
						stat.hudSpeed == hudSpeed &&
						stat.hudCapacity == hudCapacity &&
						stat.hudAccuracy == hudAccuracy &&
						stat.hudRange == hudRange;
				}
				return false;
			}

			public override int GetHashCode()
			{
				unchecked
				{
					int hash = 13;
					hash = (hash * 7) + hudDamage.GetHashCode();
					hash = (hash * 7) + hudSpeed.GetHashCode();
					hash = (hash * 7) + hudCapacity.GetHashCode();
					hash = (hash * 7) + hudAccuracy.GetHashCode();
					hash = (hash * 7) + hudRange.GetHashCode();
					return hash;
				}
			}

			public static bool operator ==(WeaponComponentHudStats left, WeaponComponentHudStats right)
			{
				return left.Equals(right);
			}

			public static bool operator !=(WeaponComponentHudStats left, WeaponComponentHudStats right)
			{
				return !(left == right);
			}
		}

		[SecuritySafeCritical]
		private static WeaponHudStats _GetWeaponHudStats(uint weaponHash)
		{
			UnsafeWeaponHudStats unsafeStats = new UnsafeWeaponHudStats();
			unsafe
			{
				Function.Call(Hash.GET_WEAPON_HUD_STATS, weaponHash, &unsafeStats);
			}
			return unsafeStats.GetSafeStats();
		}

		[SecuritySafeCritical]
		private static WeaponComponentHudStats _GetWeaponComponentHudStats(uint weaponComponentHash)
		{
			UnsafeWeaponComponentHudStats unsafeStats = new UnsafeWeaponComponentHudStats();
			unsafe
			{
				Function.Call(Hash.GET_WEAPON_COMPONENT_HUD_STATS, weaponComponentHash, &unsafeStats);
			}
			return unsafeStats.GetSafeStats();
		}

		/// <summary>
		/// Get the hud stats for this weapon.
		/// </summary>
		/// <param name="weaponHash"></param>
		/// <param name="weaponStats"></param>
		/// <returns></returns>
		public static bool GetWeaponHudStats(uint weaponHash, ref WeaponHudStats weaponStats)
		{
			if (!API.IsWeaponValid(weaponHash))
				return false;
			weaponStats = _GetWeaponHudStats(weaponHash);
			return true;
		}

		/// <summary>
		/// Get the hud stats for this weapon component.
		/// </summary>
		/// <param name="weaponHash"></param>
		/// <param name="weaponComponentStats"></param>
		/// <returns></returns>
		public static bool GetWeaponComponentHudStats(uint weaponHash, ref WeaponComponentHudStats weaponComponentStats)
		{
			weaponComponentStats = _GetWeaponComponentHudStats(weaponHash);
			return true;
		}
	}
}
