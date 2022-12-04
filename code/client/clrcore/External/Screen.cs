#if MONO_V2
using CitizenFX.Core;
using API = CitizenFX.FiveM.Native.Natives;
using PointF = CitizenFX.Core.Vector2;
using Size = CitizenFX.Core.Size2;

namespace CitizenFX.FiveM.UI
#else
using CitizenFX.Core.Native;
using System.Drawing;

namespace CitizenFX.Core.UI
#endif
{
	public enum HudComponent
	{
		WantedStars = 1,
		WeaponIcon,
		Cash,
		MpCash,
		MpMessage,
		VehicleName,
		AreaName,
		Unused,
		StreetName,
		HelpText,
		FloatingHelpText1,
		FloatingHelpText2,
		CashChange,
		Reticle,
		SubtitleText,
		RadioStationsWheel,
		Saving,
		GamingStreamUnusde,
		WeaponWheel,
		WeaponWheelStats,
		DrugsPurse01,
		DrugsPurse02,
		DrugsPurse03,
		DrugsPurse04,
		MpTagCashFromBank,
		MpTagPackages,
		MpTagCuffKeys,
		MpTagDownloadData,
		MpTagIfPedFollowing,
		MpTagKeyCard,
		MpTagRandomObject,
		MpTagRemoteControl,
		MpTagCashFromSafe,
		MpTagWeaponsPackage,
		MpTagKeys,
		MpVehicle,
		MpVehicleHeli,
		MpVehiclePlane,
		PlayerSwitchAlert,
		MpRankBar,
		DirectorMode,
		ReplayController,
		ReplayMouse,
		ReplayHeader,
		ReplayOptions,
		ReplayHelpText,
		ReplayMiscText,
		ReplayTopLine,
		ReplayBottomLine,
		ReplayLeftBar,
		ReplayTimer
	}

	public enum LoadingSpinnerType
	{
		Clockwise1 = 1,
		Clockwise2,
		Clockwise3,
		SocialClubSaving,
		RegularClockwise
	}

	public enum CursorSprite
	{
		Normal = 1,
		LightArrow,
		OpenHand,
		GrabHand,
		MiddleFinger,
		LeftArrow,
		RightArrow,
		UpArrow,
		DownArrow,
		HorizontalDoubleArrow,
		NormalWithPlus,
		NormalWithMinus
	}

	public enum ScreenEffect
	{
		SwitchHudIn,
		SwitchHudOut,
		FocusIn,
		FocusOut,
		MinigameEndNeutral,
		MinigameEndTrevor,
		MinigameEndFranklin,
		MinigameEndMichael,
		MinigameTransitionOut,
		MinigameTransitionIn,
		SwitchShortNeutralIn,
		SwitchShortFranklinIn,
		SwitchShortTrevorIn,
		SwitchShortMichaelIn,
		SwitchOpenMichaelIn,
		SwitchOpenFranklinIn,
		SwitchOpenTrevorIn,
		SwitchHudMichaelOut,
		SwitchHudFranklinOut,
		SwitchHudTrevorOut,
		SwitchShortFranklinMid,
		SwitchShortMichaelMid,
		SwitchShortTrevorMid,
		DeathFailOut,
		CamPushInNeutral,
		CamPushInFranklin,
		CamPushInMichael,
		CamPushInTrevor,
		SwitchSceneFranklin,
		SwitchSceneTrevor,
		SwitchSceneMichael,
		SwitchSceneNeutral,
		MpCelebWin,
		MpCelebWinOut,
		MpCelebLose,
		MpCelebLoseOut,
		DeathFailNeutralIn,
		DeathFailMpDark,
		DeathFailMpIn,
		MpCelebPreloadFade,
		PeyoteEndOut,
		PeyoteEndIn,
		PeyoteIn,
		PeyoteOut,
		MpRaceCrash,
		SuccessFranklin,
		SuccessTrevor,
		SuccessMichael,
		DrugsMichaelAliensFightIn,
		DrugsMichaelAliensFight,
		DrugsMichaelAliensFightOut,
		DrugsTrevorClownsFightIn,
		DrugsTrevorClownsFight,
		DrugsTrevorClownsFightOut,
		HeistCelebPass,
		HeistCelebPassBw,
		HeistCelebEnd,
		HeistCelebToast,
		MenuMgHeistIn,
		MenuMgTournamentIn,
		MenuMgSelectionIn,
		ChopVision,
		DmtFlightIntro,
		DmtFlight,
		DrugsDrivingIn,
		DrugsDrivingOut,
		SwitchOpenNeutralFib5,
		HeistLocate,
		MpJobLoad,
		RaceTurbo,
		MpIntroLogo,
		HeistTripSkipFade,
		MenuMgHeistOut,
		MpCoronaSwitch,
		MenuMgSelectionTint,
		SuccessNeutral,
		ExplosionJosh3,
		SniperOverlay,
		RampageOut,
		Rampage,
		DontTazemeBro,
	}

	public sealed class Notification
	{
		#region Fields
		int _handle;
		#endregion

		internal Notification(int handle)
		{
			_handle = handle;
		}

		/// <summary>
		/// Hides this <see cref="Notification"/> instantly
		/// </summary>
		public void Hide()
		{
			API.RemoveNotification(_handle);
		}
	}

	public static class Screen
	{
		/// <summary>
		/// Converts the inputString into a string[] (array) containing strings each 99 or less characters long.
		/// </summary>
		/// <param name="inputString">The string to convert.</param>
		/// <returns>string[] containing strings each 99 or less characters long.</returns>
		public static string[] StringToArray(string inputString)
		{
			int stringsNeeded = (inputString.Length % 99 == 0) ? (inputString.Length / 99) : ((inputString.Length / 99) + 1);

			string[] outputString = new string[stringsNeeded];
			for (int i = 0; i < stringsNeeded; i++)
			{
				outputString[i] = inputString.Substring(i * 99, MathUtil.Clamp(inputString.Substring(i * 99).Length, 0, 99));
			}

			return outputString;
		}

		/// <summary>
		/// The base width of the screen used for all UI Calculations, unless ScaledDraw is used
		/// </summary>
		public const float Width = 1280f;
		/// <summary>
		/// The base height of the screen used for all UI Calculations
		/// </summary>
		public const float Height = 720f;
		/// <summary>
		/// Gets the current Screen Aspect Ratio
		/// </summary>		   
		public static float AspectRatio
		{
			get
			{
				return API.GetScreenAspectRatio(false);
			}
		}
		/// <summary>
		/// Gets the width of the scaled against a 720pixel height base.
		/// </summary>
		public static float ScaledWidth
		{
			get
			{
				return Height * AspectRatio;
			}
		}
		/// <summary>
		/// Gets the actual Screen resolution the game is being rendered at
		/// </summary>
		public static Size Resolution
		{
			get
			{
				int height = 0, width = 0;
				API.GetScreenActiveResolution(ref width, ref height);
				return new Size(width, height);
			}
		}

		/// <summary>
		/// Shows a subtitle at the bottom of the screen for a given time
		/// </summary>
		/// <param name="message">The message to display.</param>
		/// <param name="duration">The duration to display the subtitle in milliseconds.</param>
		public static void ShowSubtitle(string message, int duration = 2500)
		{
			string[] strings = StringToArray(message);

			API.BeginTextCommandPrint("CELL_EMAIL_BCON");

			foreach (string s in strings)
			{
				API.AddTextComponentSubstringPlayerName(s);
			}

			API.EndTextCommandPrint(duration, true);
		}

		/// <summary>
		/// Displays a help message in the top corner of the screen this frame.
		/// </summary>
		/// <param name="helpText">The text to display.</param>
		public static void DisplayHelpTextThisFrame(string helpText)
		{
			string[] strings = StringToArray(helpText);

			API.BeginTextCommandDisplayHelp("CELL_EMAIL_BCON");

			foreach (string s in strings)
			{
				API.AddTextComponentSubstringPlayerName(s);
			}

			API.EndTextCommandDisplayHelp(0, false, false, -1);
		}

		/// <summary>
		/// Creates a <see cref="Notification"/> above the minimap with the given message.
		/// </summary>
		/// <param name="message">The message in the notification.</param>
		/// <param name="blinking">if set to <c>true</c> the notification will blink.</param>
		/// <returns>The handle of the <see cref="Notification"/> which can be used to hide it using <see cref="Notification.Hide()"/></returns>
		public static Notification ShowNotification(string message, bool blinking = false)
		{
			string[] strings = StringToArray(message);


			API.SetNotificationTextEntry("CELL_EMAIL_BCON");

			foreach (string s in strings)
			{
				API.AddTextComponentSubstringPlayerName(s);
			}

			return new Notification(API.DrawNotification(blinking, true));
		}

		/// <summary>
		/// Translates a point in WorldSpace to its given Coordinates on the <see cref="Screen"/>
		/// </summary>
		/// <param name="position">The position in the World.</param>
		/// <param name="scaleWidth">if set to <c>true</c> Returns the screen position scaled by <see cref="ScaledWidth"/>; otherwise, returns the screen position scaled by <see cref="Width"/>.</param>
		/// <returns></returns>
		public static PointF WorldToScreen(Vector3 position, bool scaleWidth = false)
		{
			float pointX = 0f, pointY = 0f;

			if (!API.GetScreenCoordFromWorldCoord(position.X, position.Y, position.Z, ref pointX, ref pointY))
			{
				return PointF.Empty;
			}

			return new PointF(pointX * (scaleWidth ? ScaledWidth : Width), pointY * Height);
		}

		public static class LoadingPrompt
		{
			/// <summary>
			/// Creates a loading prompt at the bottom right of the screen with the given text and spinner type
			/// </summary>
			/// <param name="loadingText">The text to display next to the spinner</param>
			/// <param name="spinnerType">The style of spinner to draw</param>
			/// <remarks>
			/// <see cref="LoadingSpinnerType.Clockwise1"/>, <see cref="LoadingSpinnerType.Clockwise2"/>, <see cref="LoadingSpinnerType.Clockwise3"/> and <see cref="LoadingSpinnerType.RegularClockwise"/> all see to be the same. 
			/// But Rockstar always seem to use the <see cref="LoadingSpinnerType.RegularClockwise"/> in the scripts
			/// </remarks>
			public static void Show(string loadingText = null, LoadingSpinnerType spinnerType = LoadingSpinnerType.RegularClockwise)
			{
				if (IsActive)
					Hide();

				if (loadingText == null)
				{
					API.BeginTextCommandBusyString(null);
				}
				else
				{
					API.BeginTextCommandBusyString("STRING");
					API.AddTextComponentSubstringPlayerName(loadingText);
				}
				API.EndTextCommandBusyString((int)spinnerType);
			}

			/// <summary>
			/// Remove the loading prompt at the bottom right of the screen
			/// </summary>
			public static void Hide()
			{
				if (IsActive)
					API.RemoveLoadingPrompt();
			}

			/// <summary>
			/// Gets a value indicating whether the Loading Prompt is currently being displayed
			/// </summary>
			public static bool IsActive
			{
				get
				{
					return API.IsLoadingPromptBeingDisplayed();
				}
			}

		}

		public static class Hud
		{
			/// <summary>
			/// Determines whether a given <see cref="HudComponent"/> is Active.
			/// </summary>
			/// <param name="component">The <see cref="HudComponent"/> to check</param>
			/// <returns><c>true</c> if the <see cref="HudComponent"/> is Active; otherwise, <c>false</c></returns>
			public static bool IsComponentActive(HudComponent component)
			{
				return API.IsHudComponentActive((int)component);
			}
			/// <summary>
			/// Draws the specified <see cref="HudComponent"/> this frame.
			/// </summary>
			/// <param name="component">The <see cref="HudComponent"/></param>
			///<remarks>This will only draw the <see cref="HudComponent"/> if the <see cref="HudComponent"/> can be drawn</remarks>
			public static void ShowComponentThisFrame(HudComponent component)
			{
				API.ShowHudComponentThisFrame((int)component);
			}
			/// <summary>
			/// Hides the specified <see cref="HudComponent"/> this frame.
			/// </summary>
			/// <param name="component">The <see cref="HudComponent"/> to hide.</param>
			public static void HideComponentThisFrame(HudComponent component)
			{
				API.HideHudComponentThisFrame((int)component);
			}

			/// <summary>
			/// Shows the mouse cursor this frame.
			/// </summary>
			public static void ShowCursorThisFrame()
			{
				API.ShowCursorThisFrame();
			}
			/// <summary>
			/// Gets or sets the sprite the cursor should used when drawn
			/// </summary>
			public static CursorSprite CursorSprite
			{
				get { /*return (CursorSprite)MemoryAccess.ReadCursorSprite();*/ return CursorSprite.DownArrow; }
				set { API.SetCursorSprite((int)value); }
			}

			/// <summary>
			/// Gets or sets a value indicating whether any HUD components should be rendered.
			/// </summary>
			public static bool IsVisible
			{
				get { return !(API.IsHudHidden() || !API.IsHudPreferenceSwitchedOn()); }
				set { API.DisplayHud(value); }
			}

			/// <summary>
			/// Gets or sets a value indicating whether the radar is visible.
			/// </summary>
			public static bool IsRadarVisible
			{
				get { return !(API.IsRadarHidden() || !API.IsRadarPreferenceSwitchedOn()); }
				set { API.DisplayRadar(value); }
			}

			/// <summary>
			/// Sets how far the Minimap should be zoomed in
			/// </summary>
			/// <value>
			/// The Radar zoom, Accepts values from 0 to 200
			/// </value>
			public static int RadarZoom
			{
				set { API.SetRadarZoom(value); }
			}
		}

		public static class Fading
		{
			/// <summary>
			/// Gets a value indicating whether the screen is faded in.
			/// </summary>
			/// <value>
			/// <c>true</c> if the screen is faded in; otherwise, <c>false</c>.
			/// </value>
			public static bool IsFadedIn
			{
				get { return API.IsScreenFadedIn(); }
			}

			/// <summary>
			/// Gets a value indicating whether the screen is faded out.
			/// </summary>
			/// <value>
			/// <c>true</c> if the screen is faded out; otherwise, <c>false</c>.
			/// </value>
			public static bool IsFadedOut
			{
				get { return API.IsScreenFadedOut(); }
			}

			/// <summary>
			/// Gets a value indicating whether the screen is fading in.
			/// </summary>
			/// <value>
			/// <c>true</c> if the screen is fading in; otherwise, <c>false</c>.
			/// </value>
			public static bool IsFadingIn
			{
				get { return API.IsScreenFadingIn(); }
			}

			/// <summary>
			/// Gets a value indicating whether the screen is fading out.
			/// </summary>
			/// <value>
			/// <c>true</c> if the screen is fading out; otherwise, <c>false</c>.
			/// </value>
			public static bool IsFadingOut
			{
				get { return API.IsScreenFadingOut(); }
			}

			/// <summary>
			/// Fades the screen in over a specific time, useful for transitioning
			/// </summary>
			/// <param name="time">The time for the fade in to take</param>
			public static void FadeIn(int time)
			{
				API.DoScreenFadeIn(time);
			}

			/// <summary>
			/// Fades the screen out over a specific time, useful for transitioning
			/// </summary>
			/// <param name="time">The time for the fade out to take</param>
			public static void FadeOut(int time)
			{
				API.DoScreenFadeOut(time);
			}
		}

		public static class Effects
		{
			private static readonly string[] _effects = new string[]
			{
				"SwitchHUDIn",
				"SwitchHUDOut",
				"FocusIn",
				"FocusOut",
				"MinigameEndNeutral",
				"MinigameEndTrevor",
				"MinigameEndFranklin",
				"MinigameEndMichael",
				"MinigameTransitionOut",
				"MinigameTransitionIn",
				"SwitchShortNeutralIn",
				"SwitchShortFranklinIn",
				"SwitchShortTrevorIn",
				"SwitchShortMichaelIn",
				"SwitchOpenMichaelIn",
				"SwitchOpenFranklinIn",
				"SwitchOpenTrevorIn",
				"SwitchHUDMichaelOut",
				"SwitchHUDFranklinOut",
				"SwitchHUDTrevorOut",
				"SwitchShortFranklinMid",
				"SwitchShortMichaelMid",
				"SwitchShortTrevorMid",
				"DeathFailOut",
				"CamPushInNeutral",
				"CamPushInFranklin",
				"CamPushInMichael",
				"CamPushInTrevor",
				"SwitchSceneFranklin",
				"SwitchSceneTrevor",
				"SwitchSceneMichael",
				"SwitchSceneNeutral",
				"MP_Celeb_Win",
				"MP_Celeb_Win_Out",
				"MP_Celeb_Lose",
				"MP_Celeb_Lose_Out",
				"DeathFailNeutralIn",
				"DeathFailMPDark",
				"DeathFailMPIn",
				"MP_Celeb_Preload_Fade",
				"PeyoteEndOut",
				"PeyoteEndIn",
				"PeyoteIn",
				"PeyoteOut",
				"MP_race_crash",
				"SuccessFranklin",
				"SuccessTrevor",
				"SuccessMichael",
				"DrugsMichaelAliensFightIn",
				"DrugsMichaelAliensFight",
				"DrugsMichaelAliensFightOut",
				"DrugsTrevorClownsFightIn",
				"DrugsTrevorClownsFight",
				"DrugsTrevorClownsFightOut",
				"HeistCelebPass",
				"HeistCelebPassBW",
				"HeistCelebEnd",
				"HeistCelebToast",
				"MenuMGHeistIn",
				"MenuMGTournamentIn",
				"MenuMGSelectionIn",
				"ChopVision",
				"DMT_flight_intro",
				"DMT_flight",
				"DrugsDrivingIn",
				"DrugsDrivingOut",
				"SwitchOpenNeutralFIB5",
				"HeistLocate",
				"MP_job_load",
				"RaceTurbo",
				"MP_intro_logo",
				"HeistTripSkipFade",
				"MenuMGHeistOut",
				"MP_corona_switch",
				"MenuMGSelectionTint",
				"SuccessNeutral",
				"ExplosionJosh3",
				"SniperOverlay",
				"RampageOut",
				"Rampage",
				"Dont_tazeme_bro"
			};

			private static string EffectToString(ScreenEffect screenEffect)
			{
				if ((int)screenEffect >= 0 && (int)screenEffect <= _effects.Length)
				{
					return _effects[(int)screenEffect];
				}
				return "INVALID";
			}

			public static void Start(ScreenEffect effectName, int duration = 0, bool looped = false)
			{
				API.StartScreenEffect(EffectToString(effectName), duration, looped);
			}

			public static void Stop()
			{
				API.StopAllScreenEffects();
			}

			public static void Stop(ScreenEffect screenEffect)
			{
				API.StopScreenEffect(EffectToString(screenEffect));
			}

			public static bool IsActive(ScreenEffect screenEffect)
			{
				return API.GetScreenEffectIsActive(EffectToString(screenEffect));
			}
		}
	}
}
