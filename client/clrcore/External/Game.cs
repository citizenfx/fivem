using System;
using CitizenFX.Core.Native;
using System.Threading.Tasks;
using CitizenFX.Core;
using System.Security;

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
		v1_0_877_1_NoSteam
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
			"RADIO_OFF"
		};

		static Player _cachedPlayer;
		#endregion

		static Game()
		{
            Version = GameVersion.v1_0_505_2_NoSteam;
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
				return (Language)Function.Call<int>(Hash._GET_UI_LANGUAGE_ID);
			}
		}

		/// <summary>
		/// Gets how many milliseconds the game has been open in this session
		/// </summary> 
		public static int GameTime
		{
			get
			{
				return Function.Call<int>(Hash.GET_GAME_TIMER);
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
				Function.Call(Hash.SET_TIME_SCALE, value);
			}
		}
		/// <summary>
		/// Gets the total number of frames that's been rendered in this session.
		/// </summary>
		public static int FrameCount
		{
			get
			{
				return Function.Call<int>(Hash.GET_FRAME_COUNT);
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
				return Function.Call<float>(Hash.GET_FRAME_TIME);
			}
		}

		/// <summary>
		/// Gets or sets the maximum wanted level a <see cref="GTA.Player"/> can receive.
		/// </summary>
		/// <value>
		/// The maximum wanted level, only accepts values 0 to 5
		/// </value>
		public static int MaxWantedLevel
		{
			get
			{
				return Function.Call<int>(Hash.GET_MAX_WANTED_LEVEL);
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

				Function.Call(Hash.SET_MAX_WANTED_LEVEL, value);
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
				Function.Call(Hash.SET_WANTED_LEVEL_MULTIPLIER, value);
			}
		}

		/// <summary>
		/// Sets a value indicating whether Police <see cref="Blip"/>s should be visible on the Minimap
		/// </summary>
		public static bool ShowsPoliceBlipsOnRadar
		{
			set
			{
				Function.Call(Hash.SET_POLICE_RADAR_BLIPS, value);
			}
		}

		/// <summary>
		/// Gets or sets the radio station.
		/// </summary>
		public static RadioStation RadioStation
		{
			get
			{
				string radioName = Function.Call<string>(Hash.GET_PLAYER_RADIO_STATION_NAME);

				if (String.IsNullOrEmpty(radioName))
				{
					return RadioStation.RadioOff;
				}

				return (RadioStation)Array.IndexOf(_radioNames, radioName);
			}
			[SecuritySafeCritical]
			set
			{
				if (Enum.IsDefined(typeof(RadioStation), value) && value != RadioStation.RadioOff)
				{
					Function.Call(Hash.SET_RADIO_TO_STATION_NAME, _radioNames[(int)value]);
				}
				else
				{
					Function.Call(Hash.SET_RADIO_TO_STATION_NAME, MemoryAccess.NullString);
				}
			}
		}

		/// <summary>
		/// Gets the <see cref="GTA.Player"/> that you are controling
		/// </summary>
		public static Player Player
		{
			get
			{
				int handle = Function.Call<int>(Hash.PLAYER_ID);

				if (ReferenceEquals(_cachedPlayer, null) || handle != _cachedPlayer.Handle)
				{
					_cachedPlayer = new Player(handle);
				}

				return _cachedPlayer;
			}
		}

		/// <summary>
		/// Gets the <see cref="GTA.Ped"/> that you are controling
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
				return !Function.Call<bool>(Hash._IS_NIGHTVISION_INACTIVE);
			}
			set
			{
				Function.Call(Hash.SET_NIGHTVISION, value);
			}
		}
		/// <summary>
		/// Gets or sets a value indicating whether to render the world with a thermal vision filter
		/// </summary>
		public static bool ThermalVision
		{
			get
			{
				return Function.Call<bool>(Hash._IS_SEETHROUGH_ACTIVE);
			}
			set
			{
				Function.Call(Hash.SET_SEETHROUGH, value);
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
				return Function.Call<bool>(Hash.GET_MISSION_FLAG);
			}
			set
			{
				Function.Call(Hash.SET_MISSION_FLAG, value);
			}
		}
		/// <summary>
		/// Gets or sets a value informing the Game Engine if a random event is in progress
		/// </summary>
		/// <value>
		/// if <c>true</c> a random event is currently active; otherwise, <c>false</c>
		/// </value>
		public static bool IsRandomEventActive
		{
			get
			{
				return Function.Call<bool>(Hash.GET_RANDOM_EVENT_FLAG);
			}
			set
			{
				Function.Call(Hash.SET_RANDOM_EVENT_FLAG, value);
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
				return Function.Call<bool>(Hash.IS_CUTSCENE_ACTIVE);
			}
		}
		/// <summary>
		/// Gets a value indicating whether there is a Waypoint set
		/// </summary>
		public static bool IsWaypointActive
		{
			get
			{
				return Function.Call<bool>(Hash.IS_WAYPOINT_ACTIVE);
			}
		}

		/// <summary>
		/// Gets or sets a value indicating whether the game is paused
		/// </summary>
		public static bool IsPaused
		{
			get
			{
				return Function.Call<bool>(Hash.IS_PAUSE_MENU_ACTIVE);
			}
			set
			{
				Function.Call(Hash.SET_PAUSE_MENU_ACTIVE, value);
			}
		}
		/// <summary>
		/// Gets a value indicating whether there is a loading screen being displayed
		/// </summary>
		public static bool IsLoading
		{
			get
			{
				return Function.Call<bool>(Hash.GET_IS_LOADING_SCREEN_ACTIVE);
			}
		}

		/// <summary>
		/// Gets whether the last input was made with a GamePad or Keyboard and Mouse
		/// </summary>
		public static InputMode CurrentInputMode
		{
			get
			{
				return Function.Call<bool>(Hash._IS_INPUT_DISABLED, 2) ? InputMode.MouseAndKeyboard : InputMode.GamePad;
			}
		}

		/// <summary>
		/// Gets whether a <see cref="Keys"/> is currently held down
		/// </summary>
		/// <param name="key">The key.</param>
		/// <returns></returns>
		/*public static bool IsKeyPressed(Keys key)
		{
			return ScriptDomain.CurrentDomain.IsKeyPressed(key);
		}*/

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
			return Function.Call<bool>(Hash._HAS_BUTTON_COMBINATION_JUST_BEEN_ENTERED, combination.Hash, combination.Length);
		}
		/// <summary>
		/// Gets whether a cheat code was entered into the cheat text box
		/// </summary>
		/// <param name="cheat">The name of the cheat to check.</param>
		/// <returns><c>true</c> if the cheat was just entered; otherwise, <c>false</c></returns>
		public static bool WasCheatStringJustEntered(string cheat)
		{
			return Function.Call<bool>((Hash) 0x557E43C447E700A8, GenerateHash(cheat));
		}

		/// <summary>
		/// Gets whether a <see cref="Control"/> is currently pressed
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns><c>true</c> if the <see cref="Control"/> is pressed; otherwise, <c>false</c></returns>
		public static bool IsControlPressed(int index, Control control)
		{
			return Function.Call<bool>(Hash.IS_DISABLED_CONTROL_PRESSED, index, control);
		}
		/// <summary>
		/// Gets whether a <see cref="Control"/> was just pressed this frame
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns><c>true</c> if the <see cref="Control"/> was just pressed this frame; otherwise, <c>false</c></returns>
		public static bool IsControlJustPressed(int index, Control control)
		{
			return Function.Call<bool>(Hash.IS_DISABLED_CONTROL_JUST_PRESSED, index, control);
		}
		/// <summary>
		/// Gets whether a <see cref="Control"/> was just released this frame
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns><c>true</c> if the <see cref="Control"/> was just released this frame; otherwise, <c>false</c></returns>
		public static bool IsControlJustReleased(int index, Control control)
		{
			return Function.Call<bool>(Hash.IS_DISABLED_CONTROL_JUST_RELEASED, index, control);
		}
		/// <summary>
		/// Gets whether a <see cref="Control"/> is Enabled this frame and is currently pressed
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns><c>true</c> if the <see cref="Control"/> is pressed; otherwise, <c>false</c></returns>
		public static bool IsEnabledControlPressed(int index, Control control)
		{
			return Function.Call<bool>(Hash.IS_CONTROL_PRESSED, index, control);
		}
		/// <summary>
		/// Gets whether a <see cref="Control"/> is Enabled and was just pressed this frame
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns><c>true</c> if the <see cref="Control"/> was just pressed this frame; otherwise, <c>false</c></returns>
		public static bool IsEnabledControlJustPressed(int index, Control control)
		{
			return Function.Call<bool>(Hash.IS_CONTROL_JUST_PRESSED, index, control);
		}
		/// <summary>
		/// Gets whether a <see cref="Control"/> is Enabled and was just released this frame
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns><c>true</c> if the <see cref="Control"/> was just released this frame; otherwise, <c>false</c></returns>
		public static bool IsEnabledControlJustReleased(int index, Control control)
		{
			return Function.Call<bool>(Hash.IS_CONTROL_JUST_RELEASED, index, control);
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
			return Function.Call<bool>(Hash.IS_CONTROL_ENABLED, index, control);
		}

		/// <summary>
		/// Makes the Game Engine respond to the given Control this frame
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		public static void EnableControlThisFrame(int index, Control control)
		{
			Function.Call(Hash.ENABLE_CONTROL_ACTION, index, control, true);
		}
		/// <summary>
		/// Makes the Game Engine ignore to the given Control this frame
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		public static void DisableControlThisFrame(int index, Control control)
		{
			Function.Call(Hash.DISABLE_CONTROL_ACTION, index, control, true);
		}
		/// <summary>
		/// Disables all <see cref="Control"/>s this frame.
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		public static void DisableAllControlsThisFrame(int index)
		{
			Function.Call(Hash.DISABLE_ALL_CONTROL_ACTIONS, index);
		}
		/// <summary>
		/// Enables all <see cref="Control"/>s this frame.
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		public static void EnableAllControlsThisFrame(int index)
		{
			Function.Call(Hash.ENABLE_ALL_CONTROL_ACTIONS, index);
		}

		/// <summary>
		/// Gets an Analog value of a <see cref="Control"/> input between -1.0f and 1.0f
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns>The normalised <see cref="Control"/> value</returns>
		public static float GetControlNormal(int index, Control control)
		{
			return Function.Call<float>(Hash.GET_CONTROL_NORMAL, index, control);
		}
		/// <summary>
		/// Gets an Analog value of a Disabled <see cref="Control"/> input between -1.0f and 1.0f
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns>The normalised <see cref="Control"/> value</returns>
		public static float GetDisabledControlNormal(int index, Control control)
		{
			return Function.Call<float>(Hash.GET_DISABLED_CONTROL_NORMAL, index, control);
		}
		/// <summary>
		/// Gets an value of a <see cref="Control"/> input.
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <returns>The <see cref="Control"/> value</returns>
		public static int GetControlValue(int index, Control control)
		{
			return Function.Call<int>(Hash.GET_CONTROL_VALUE, index, control);
		}
		/// <summary>
		/// Override a <see cref="Control"/> by giving it a user defined value this frame.
		/// </summary>
		/// <param name="index">The Input Method (0 = Mouse and Keyboard, 2 = GamePad).</param>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <param name="value">the value to set the control to.</param>
		public static void SetControlNormal(int index, Control control, float value)
		{
			Function.Call(Hash._SET_CONTROL_NORMAL, index, control, value);
		}

		/// <summary>
		/// Pauses or Resumes the game
		/// </summary>
		/// <param name="value">if set to <c>true</c> Pause the game; otherwise, resume the game.</param>
		public static void Pause(bool value)
		{
			Function.Call(Hash.SET_GAME_PAUSED, value);
		}
		/// <summary>
		/// Pauses or Resumes the game clock
		/// </summary>
		/// <param name="value">if set to <c>true</c> Pause the game clock; otherwise, resume the game clock.</param>
		public static void PauseClock(bool value)
		{
			Function.Call(Hash.PAUSE_CLOCK, value);
		}

		/// <summary>
		/// Performs and automative game save
		/// </summary>
		public static void DoAutoSave()
		{
			Function.Call(Hash.DO_AUTO_SAVE);
		}
		/// <summary>
		/// Shows the save menu enabling the user to perform a manual game save.
		/// </summary>
		public static void ShowSaveMenu()
		{
			Function.Call(Hash.SET_SAVE_MENU_ACTIVE, true);
		}

		/// <summary>
		/// Determines the game language files contain a entry for the specified GXT key
		/// </summary>
		/// <param name="entry">The GXT key.</param>
		/// <returns><c>true</c> if GXT entry exists; otherwise, <c>false</c></returns>
		public static bool DoesGXTEntryExist(string entry)
		{
			return Function.Call<bool>(Hash.DOES_TEXT_LABEL_EXIST, entry);
		}

		/// <summary>
		/// Returns a localised <see cref="string"/> from the games language files with a specified GXT key
		/// </summary>
		/// <param name="entry">The GXT key.</param>
		/// <returns>The localised <see cref="string"/> if the key exists; otherwise, <see cref="string.Empty"/></returns>
		public static string GetGXTEntry(string entry)
		{
			return DoesGXTEntryExist(entry) ? Function.Call<string>(Hash._GET_LABEL_TEXT, entry) : String.Empty;
		}
		internal static bool DoesGXTEntryExist(ulong entry)
		{
			return Function.Call<bool>(Hash.DOES_TEXT_LABEL_EXIST, entry);
		}

		internal static string GetGXTEntry(ulong entry)
		{
			return DoesGXTEntryExist(entry) ? Function.Call<string>(Hash._GET_LABEL_TEXT, entry) : String.Empty;
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
			return unchecked((int) MemoryAccess.GetHashKey(input));
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
			Function.Call(Hash.TRIGGER_MUSIC_EVENT, musicFile);
		}
		/// <summary>
		/// Stops playing a music file
		/// </summary>
		/// <param name="musicFile">The music file to stop.</param>
		public static void StopMusic(string musicFile)
		{
			Function.Call(Hash.CANCEL_MUSIC_EVENT, musicFile);
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

			while (Function.Call<int>(Hash.UPDATE_ONSCREEN_KEYBOARD) == 0)
			{
                await BaseScript.Delay(0);
			}

			//ScriptDomain.CurrentDomain.PauseKeyboardEvents(false);

			return Function.Call<string>(Hash.GET_ONSCREEN_KEYBOARD_RESULT);
		}
		[SecuritySafeCritical]
		private static void ClearKeyboard(WindowTitle windowTitle, string defaultText, int maxLength)
		{
			Function.Call(Hash.DISPLAY_ONSCREEN_KEYBOARD, true, windowTitle.ToString(), MemoryAccess.NullString, defaultText, MemoryAccess.NullString, MemoryAccess.NullString, MemoryAccess.NullString, maxLength + 1);
		}
	}
}
