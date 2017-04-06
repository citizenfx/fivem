using System;
using System.Drawing;
using CitizenFX.Core.Native;
using System.Security;

namespace CitizenFX.Core.UI
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
			Function.Call(Hash._REMOVE_NOTIFICATION, _handle);
		}
	}

	public static class Screen
	{
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
				return Function.Call<float>(Hash._GET_SCREEN_ASPECT_RATIO, 0);
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
            [SecuritySafeCritical]
			get
			{
				return GetResolution();
			}
		}

		[SecuritySafeCritical]
		private static Size GetResolution()
		{
		    int width, height;

			unsafe
			{
				Function.Call(Hash._GET_SCREEN_ACTIVE_RESOLUTION, &width, &height);
			}

			return new Size(width, height);
		}

		/// <summary>
		/// Shows a subtitle at the bottom of the screen for a given time
		/// </summary>
		/// <param name="message">The message to display.</param>
		/// <param name="duration">The duration to display the subtitle in milliseconds.</param>
		[SecuritySafeCritical]
		public static void ShowSubtitle(string message, int duration = 2500)
		{
			Function.Call(Hash._SET_TEXT_ENTRY_2, MemoryAccess.CellEmailBcon);

			const int maxStringLength = 99;

			for (int i = 0; i < message.Length; i += maxStringLength)
			{
				Function.Call(Hash.ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME, message.Substring(i, System.Math.Min(maxStringLength, message.Length - i)));
			}

			Function.Call(Hash._DRAW_SUBTITLE_TIMED, duration, 1);
		}

		/// <summary>
		/// Displays a help message in the top corner of the screen this frame.
		/// </summary>
		/// <param name="helpText">The text to display.</param>
		[SecuritySafeCritical]
		public static void DisplayHelpTextThisFrame(string helpText)
		{
			Function.Call(Hash._SET_TEXT_COMPONENT_FORMAT, MemoryAccess.CellEmailBcon);
			const int maxStringLength = 99;

			for (int i = 0; i < helpText.Length; i += maxStringLength)
			{
				Function.Call(Hash.ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME, helpText.Substring(i, System.Math.Min(maxStringLength, helpText.Length - i)));
			}
			Function.Call(Hash._DISPLAY_HELP_TEXT_FROM_STRING_LABEL, 0, 0, 1, -1);
		}

		/// <summary>
		/// Creates a <see cref="Notification"/> above the minimap with the given message.
		/// </summary>
		/// <param name="message">The message in the notification.</param>
		/// <param name="blinking">if set to <c>true</c> the notification will blink.</param>
		/// <returns>The handle of the <see cref="Notification"/> which can be used to hide it using <see cref="Notification.Hide()"/></returns>
		public static Notification ShowNotification(string message, bool blinking = false)
		{
			Function.Call(Hash._SET_NOTIFICATION_TEXT_ENTRY, "CELL_EMAIL_BCON");

			const int maxStringLength = 99;

			for (int i = 0; i < message.Length; i += maxStringLength)
			{
				Function.Call(Hash.ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME, message.Substring(i, System.Math.Min(maxStringLength, message.Length - i)));
			}

			return new Notification(Function.Call<int>(Hash._DRAW_NOTIFICATION, blinking, true));
		}

        /// <summary>
        /// Translates a point in WorldSpace to its given Coordinates on the <see cref="Screen"/>
        /// </summary>
        /// <param name="position">The position in the World.</param>
        /// <param name="scaleWidth">if set to <c>true</c> Returns the screen position scaled by <see cref="ScaledWidth"/>; otherwise, returns the screen position scaled by <see cref="Width"/>.</param>
        /// <returns></returns>
        [SecuritySafeCritical]
        public static PointF WorldToScreen(Vector3 position, bool scaleWidth = false)
		{
			return WorldToScreen(position, scaleWidth ? ScaledWidth : Width, Height);
		}
        [SecuritySafeCritical]
		private static PointF WorldToScreen(Vector3 position, float screenWidth, float screenHeight)
		{
			float pointX, pointY;

			unsafe
			{
				if (!Function.Call<bool>(Hash._WORLD3D_TO_SCREEN2D, position.X, position.Y, position.Z, &pointX, &pointY))
				{
					return PointF.Empty;
				}
			}

			return new PointF(pointX * screenWidth, pointY * screenHeight);
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
			[SecuritySafeCritical]
			public static void Show(string loadingText = null, LoadingSpinnerType spinnerType = LoadingSpinnerType.RegularClockwise)
			{
				if(IsActive)
					Hide();
				if(loadingText == null)
				{
					Function.Call((Hash)0xABA17D7CE615ADBF, MemoryAccess.NullString);
				}
				else
				{
					Function.Call((Hash)0xABA17D7CE615ADBF, MemoryAccess.StringPtr);
					//TO DO update this to Hash._SET_LOADING_PROMPT_TEXT_ENTRY when Hash enum next gets rebuilt
					Function.Call(Hash.ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME, loadingText);
				}
				Function.Call((Hash)0xBD12F8228410D9B4, spinnerType); //TO DO update to Hash._SHOW_LOADING_PROMPT
			}

			/// <summary>
			/// Remove the loading prompt at the bottom right of the screen
			/// </summary>
			public static void Hide()
			{
				if(IsActive)
					Function.Call((Hash)0x10D373323E5B9C0D); //TO DO update to Hash._REMOVE_LOADING_PROMPT
			}

			/// <summary>
			/// Gets a value indicating whether the Loading Prompt is currently being displayed
			/// </summary>
			public static bool IsActive
			{
				get
				{
					return Function.Call<bool>((Hash)0xD422FCC5F239A915);
				}
				//TO DO update hash to Hash._IS_LOADING_PROMPT_BEING_DISPLAYED
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
				return Function.Call<bool>(Hash.IS_HUD_COMPONENT_ACTIVE, component);
			}
			/// <summary>
			/// Draws the specified <see cref="HudComponent"/> this frame.
			/// </summary>
			/// <param name="component">The <see cref="HudComponent"/></param>
			///<remarks>This will only draw the <see cref="HudComponent"/> if the <see cref="HudComponent"/> can be drawn</remarks>
			public static void ShowComponentThisFrame(HudComponent component)
			{
				Function.Call(Hash.SHOW_HUD_COMPONENT_THIS_FRAME, component);
			}
			/// <summary>
			/// Hides the specified <see cref="HudComponent"/> this frame.
			/// </summary>
			/// <param name="component">The <see cref="HudComponent"/> to hide.</param>
			public static void HideComponentThisFrame(HudComponent component)
			{
				Function.Call(Hash.HIDE_HUD_COMPONENT_THIS_FRAME, component);
			}

			/// <summary>
			/// Shows the mouse cursor this frame.
			/// </summary>
			public static void ShowCursorThisFrame()
			{
				Function.Call(Hash._SHOW_CURSOR_THIS_FRAME);
			}
			/// <summary>
			/// Gets or sets the sprite the cursor should used when drawn
			/// </summary>
			public static CursorSprite CursorSprite
			{
				get
				{
                    //return (CursorSprite)MemoryAccess.ReadCursorSprite();
                    return CursorSprite.DownArrow;
				}
				set
				{
					Function.Call(Hash._SET_CURSOR_SPRITE, value);
				}
			}

			/// <summary>
			/// Gets or sets a value indicating whether any HUD components should be rendered.
			/// </summary>
			public static bool IsVisible
			{
				get
				{
					return !Function.Call<bool>(Hash.IS_HUD_HIDDEN);
				}
				set
				{
					Function.Call(Hash.DISPLAY_HUD, value);
				}
			}

			/// <summary>
			/// Gets or sets a value indicating whether the radar is visible.
			/// </summary>
			public static bool IsRadarVisible
			{
				get { return !Function.Call<bool>(Hash.IS_RADAR_HIDDEN); }
				set { Function.Call(Hash.DISPLAY_RADAR, value); }
			}

			/// <summary>
			/// Sets how far the Minimap should be zoomed in
			/// </summary>
			/// <value>
			/// The Radar zoom, Accepts values from 0 to 200
			/// </value>
			public static int RadarZoom
			{
				set { Function.Call(Hash.SET_RADAR_ZOOM, value); }
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
				get
				{
					return Function.Call<bool>(Hash.IS_SCREEN_FADED_IN);
				}
			}

			/// <summary>
			/// Gets a value indicating whether the screen is faded out.
			/// </summary>
			/// <value>
			/// <c>true</c> if the screen is faded out; otherwise, <c>false</c>.
			/// </value>
			public static bool IsFadedOut
			{
				get
				{
					return Function.Call<bool>(Hash.IS_SCREEN_FADED_OUT);
				}
			}

			/// <summary>
			/// Gets a value indicating whether the screen is fading in.
			/// </summary>
			/// <value>
			/// <c>true</c> if the screen is fading in; otherwise, <c>false</c>.
			/// </value>
			public static bool IsFadingIn
			{
				get
				{
					return Function.Call<bool>(Hash.IS_SCREEN_FADING_IN);
				}
			}

			/// <summary>
			/// Gets a value indicating whether the screen is fading out.
			/// </summary>
			/// <value>
			/// <c>true</c> if the screen is fading out; otherwise, <c>false</c>.
			/// </value>
			public static bool IsFadingOut
			{
				get
				{
					return Function.Call<bool>(Hash.IS_SCREEN_FADING_OUT);
				}
			}

			/// <summary>
			/// Fades the screen in over a specific time, useful for transitioning
			/// </summary>
			/// <param name="time">The time for the fade in to take</param>
			public static void FadeIn(int time)
			{
				Function.Call(Hash.DO_SCREEN_FADE_IN, time);
			}

			/// <summary>
			/// Fades the screen out over a specific time, useful for transitioning
			/// </summary>
			/// <param name="time">The time for the fade out to take</param>
			public static void FadeOut(int time)
			{
				Function.Call(Hash.DO_SCREEN_FADE_OUT, time);
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
				if((int)screenEffect >= 0 && (int)screenEffect <= _effects.Length)
				{
					return _effects[(int)screenEffect];
				}
				return "INVALID";
			}

			public static void Start(ScreenEffect effectName, int duration = 0, bool looped = false)
			{
				Function.Call(Hash._START_SCREEN_EFFECT, EffectToString(effectName), duration, looped);
			}

			public static void Stop()
			{
				Function.Call(Hash._STOP_ALL_SCREEN_EFFECTS);
			}

			public static void Stop(ScreenEffect screenEffect)
			{
				Function.Call(Hash._STOP_SCREEN_EFFECT, EffectToString(screenEffect));
			}

			public static bool IsActive(ScreenEffect screenEffect)
			{
				return Function.Call<bool>(Hash._GET_SCREEN_EFFECT_IS_ACTIVE, EffectToString(screenEffect));
			}
		}
	}
}
