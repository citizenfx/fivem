using CitizenFX.Core.Native;
using System;
using System.Collections.Generic;
using System.Linq;

#if MONO_V2
using CitizenFX.Core;
using API = CitizenFX.FiveM.Native.Natives;
using Prop = CitizenFX.FiveM.Object;
using TaskPed = CitizenFX.Core.Coroutine<CitizenFX.FiveM.Ped>;
using TaskPickup = CitizenFX.Core.Coroutine<CitizenFX.FiveM.Pickup>;
using TaskProp = CitizenFX.Core.Coroutine<CitizenFX.FiveM.Object>;
using TaskVehicle = CitizenFX.Core.Coroutine<CitizenFX.FiveM.Vehicle>;
using compat_i32_u32 = System.UInt32;
using compat_i32_i64 = System.Int64;

namespace CitizenFX.FiveM
#else
using System.Drawing;
using TaskPed = System.Threading.Tasks.Task<CitizenFX.Core.Ped>;
using TaskPickup = System.Threading.Tasks.Task<CitizenFX.Core.Pickup>;
using TaskProp = System.Threading.Tasks.Task<CitizenFX.Core.Prop>;
using TaskVehicle = System.Threading.Tasks.Task<CitizenFX.Core.Vehicle>;
using compat_i32_u32 = System.Int32;
using compat_i32_i64 = System.Int32;

namespace CitizenFX.Core
#endif
{
	class GTACalender : System.Globalization.GregorianCalendar
	{
		public override int GetDaysInYear(int year, int era)
		{
			return 31 * 12;
		}
		public override int GetDaysInMonth(int year, int month, int era)
		{
			return 31;
		}
	}
	enum ZoneID
	{
		AIRP,
		ALAMO,
		ALTA,
		ARMYB,
		BANHAMC,
		BANNING,
		BEACH,
		BHAMCA,
		BRADP,
		BRADT,
		BURTON,
		CALAFB,
		CANNY,
		CCREAK,
		CHAMH,
		CHIL,
		CHU,
		CMSW,
		CYPRE,
		DAVIS,
		DELBE,
		DELPE,
		DELSOL,
		DESRT,
		DOWNT,
		DTVINE,
		EAST_V,
		EBURO,
		ELGORL,
		ELYSIAN,
		GALFISH,
		golf,
		GRAPES,
		GREATC,
		HARMO,
		HAWICK,
		HORS,
		HUMLAB,
		JAIL,
		KOREAT,
		LACT,
		LAGO,
		LDAM,
		LEGSQU,
		LMESA,
		LOSPUER,
		MIRR,
		MORN,
		MOVIE,
		MTCHIL,
		MTGORDO,
		MTJOSE,
		MURRI,
		NCHU,
		NOOSE,
		OCEANA,
		PALCOV,
		PALETO,
		PALFOR,
		PALHIGH,
		PALMPOW,
		PBLUFF,
		PBOX,
		PROCOB,
		RANCHO,
		RGLEN,
		RICHM,
		ROCKF,
		RTRAK,
		SanAnd,
		SANCHIA,
		SANDY,
		SKID,
		SLAB,
		STAD,
		STRAW,
		TATAMO,
		TERMINA,
		TEXTI,
		TONGVAH,
		TONGVAV,
		VCANA,
		VESP,
		VINE,
		WINDF,
		WVINE,
		ZANCUDO,
		ZP_ORT,
		ZQ_UAR
	}

	public enum Weather
	{
		Unknown = -1,
		ExtraSunny,
		Clear,
		Clouds,
		Smog,
		Foggy,
		Overcast,
		Raining,
		ThunderStorm,
		Clearing,
		Neutral,
		Snowing,
		Blizzard,
		Snowlight,
		Christmas,
		Halloween
	}
	public enum WeatherTypeHash
	{
		Unknown = -1,
		ExtraSunny = -1750463879,
		Clear = 916995460,
		Neutral = -1530260698,
		Smog = 282916021,
		Foggy = -1368164796,
		Clouds = 821931868,
		Overcast = -1148613331,
		Clearing = 1840358669,
		Raining = 1420204096,
		ThunderStorm = -1233681761,
		Blizzard = 669657108,
		Snowing = -273223690,
		Snowlight = 603685163,
		Christmas = -1429616491,
		Halloween = -921030142
	}

	public enum CloudHat
	{
		Unknown = 1,
		Altostratus,
		Cirrus,
		Cirrocumulus,
		Clear,
		Cloudy,
		Contrails,
		Horizon,
		HorizonBand1,
		HorizonBand2,
		HorizonBand3,
		Horsey,
		Nimbus,
		Puffs,
		Rain,
		Snowy,
		Stormy,
		Stratoscumulus,
		Stripey,
		Shower,
		Wispy
	}

	[Flags]
	public enum IntersectOptions
	{
		Everything = -1,
		Map = 1,
		MissionEntities,
		Peds1 = 12,
		Objects = 16,
		Unk1 = 32,
		Unk2 = 64,
		Unk3 = 128,
		Vegetation = 256,
		Unk4 = 512
	}

	public enum MarkerType
	{
		UpsideDownCone,
		VerticalCylinder,
		ThickChevronUp,
		ThinChevronUp,
		CheckeredFlagRect,
		CheckeredFlagCircle,
		VerticleCircle,
		PlaneModel,
		LostMCDark,
		LostMCLight,
		Number0,
		Number1,
		Number2,
		Number3,
		Number4,
		Number5,
		Number6,
		Number7,
		Number8,
		Number9,
		ChevronUpx1,
		ChevronUpx2,
		ChevronUpx3,
		HorizontalCircleFat,
		ReplayIcon,
		HorizontalCircleSkinny,
		HorizontalCircleSkinnyArrow,
		HorizontalSplitArrowCircle,
		DebugSphere,
		DollarSign,
		HorizontalBars,
		WolfHead,
		QuestionMark,
		PlaneSymbol,
		HelicopterSymbol,
		BoatSymbol,
		CarSymbol,
		MotorcycleSymbol,
		BikeSymbol,
		TruckSymbol,
		ParachuteSymbol,
		SawbladeSymbol
	}
	public enum ExplosionType
	{
		Grenade,
		GrenadeL,
		StickyBomb,
		Molotov1,
		Rocket,
		TankShell,
		HiOctane,
		Car,
		Plane,
		PetrolPump,
		Bike,
		Steam,
		Flame,
		WaterHydrant,
		GasCanister,
		Boat,
		ShipDestroy,
		Truck,
		Bullet,
		SmokeGL,
		SmokeG,
		BZGas,
		Flare,
		GasCanister2,
		Extinguisher,
		ProgramAR,
		Train,
		Barrel,
		Propane,
		Blimp,
		FlameExplode,
		Tanker,
		PlaneRocket,
		VehicleBullet,
		GasTank,
		FireWork,
		SnowBall,
		ProxMine,
		Valkyrie
	}

	public static class World
	{
		#region Fields
		internal static readonly string[] _weatherNames = {
			"EXTRASUNNY",
			"CLEAR",
			"CLOUDS",
			"SMOG",
			"FOGGY",
			"OVERCAST",
			"RAIN",
			"THUNDER",
			"CLEARING",
			"NEUTRAL",
			"SNOW",
			"BLIZZARD",
			"SNOWLIGHT",
			"XMAS"
		};

		internal static readonly Dictionary<CloudHat, string> CloudHatDict = new Dictionary<CloudHat, string> {
			{CloudHat.Unknown, "Unknown"},
			{CloudHat.Altostratus, "altostratus"},
			{CloudHat.Cirrus, "Cirrus"},
			{CloudHat.Cirrocumulus, "cirrocumulus"},
			{CloudHat.Clear, "Clear 01"},
			{CloudHat.Cloudy, "Cloudy 01"},
			{CloudHat.Contrails, "Contrails"},
			{CloudHat.Horizon, "Horizon"},
			{CloudHat.HorizonBand1, "horizonband1"},
			{CloudHat.HorizonBand2, "horizonband2"},
			{CloudHat.HorizonBand3, "horizonband3"},
			{CloudHat.Horsey, "horsey"},
			{CloudHat.Nimbus, "Nimbus"},
			{CloudHat.Puffs, "Puffs"},
			{CloudHat.Rain, "RAIN"},
			{CloudHat.Snowy, "Snowy 01"},
			{CloudHat.Stormy, "Stormy 01"},
			{CloudHat.Stratoscumulus, "stratoscumulus"},
			{CloudHat.Stripey, "Stripey"},
			{CloudHat.Shower, "shower"},
			{CloudHat.Wispy, "Wispy"}
		};
		#endregion

		/// <summary>
		/// Gets or sets the current date and time in the GTA World.
		/// </summary>
		/// <value>
		/// The current date and time.
		/// </value>
		public static DateTime CurrentDate
		{
			get
			{
				int year = API.GetClockYear();
				int month = API.GetClockMonth();
				int day = API.GetClockDayOfMonth();
				int hour = API.GetClockHours();
				int minute = API.GetClockMinutes();
				int second = API.GetClockSeconds();

				return new DateTime(year, month, day, hour, minute, second, new GTACalender());
			}
			set
			{
				API.SetClockDate(value.Day, value.Month, value.Year);
				API.SetClockTime(value.Hour, value.Minute, value.Second);
			}
		}
		/// <summary>
		/// Gets or sets the current time of day in the GTA World.
		/// </summary>
		/// <value>
		/// The current time of day
		/// </value>
		public static TimeSpan CurrentDayTime
		{
			get
			{
				int hours = API.GetClockHours();
				int minutes = API.GetClockMinutes();
				int seconds = API.GetClockSeconds();

				return new TimeSpan(hours, minutes, seconds);
			}
			set
			{
				API.SetClockTime(value.Hours, value.Minutes, value.Seconds);
			}
		}

		/// <summary>
		/// Sets a value indicating whether lights in the <see cref="World"/> should be rendered.
		/// </summary>
		/// <value>
		///   <c>true</c> if blackout; otherwise, <c>false</c>.
		/// </value>
		public static bool Blackout
		{
			set
			{
				API.SetBlackout(value);
			}
		}

		private static CloudHat _currentCloudHat = CloudHat.Clear;
		/// <summary>
		/// Gets or sets the current Cloud Hat.
		/// </summary>
		public static CloudHat CloudHat
		{
			get => _currentCloudHat;
			set
			{
				_currentCloudHat = value;
				if (_currentCloudHat == CloudHat.Unknown)
				{
					_currentCloudHat = CloudHat.Clear;
					API.ClearCloudHat();
					return;
				}
				API.SetCloudHatTransition(CloudHatDict.ContainsKey(_currentCloudHat) ? CloudHatDict[_currentCloudHat] : "", 3f);
			}
		}

		/// <summary>
		/// Gets or sets the current Cloud Hat opacity. On a scale of 0.0 to 1.0.
		/// </summary>
		public static float CloudHatOpacity
		{
			get => API.GetCloudHatOpacity();
			set => API.SetCloudHatOpacity(MathUtil.Clamp(value, 0f, 1f));
		}

		/// <summary>
		/// Gets or sets the weather.
		/// </summary>
		/// <value>
		/// The weather.
		/// </value>
		public static Weather Weather
		{
			get
			{
				switch (unchecked((int)API.GetPrevWeatherTypeHashName()))
				{
					case -1750463879:
						return Weather.ExtraSunny;
					case 916995460:
						return Weather.Clear;
					case -1530260698:
						return Weather.Neutral;
					case 282916021:
						return Weather.Smog;
					case -1368164796:
						return Weather.Foggy;
					case 821931868:
						return Weather.Clouds;
					case -1148613331:
						return Weather.Overcast;
					case 1840358669:
						return Weather.Clearing;
					case 1420204096:
						return Weather.Raining;
					case -1233681761:
						return Weather.ThunderStorm;
					case 669657108:
						return Weather.Blizzard;
					case -273223690:
						return Weather.Snowing;
					case 603685163:
						return Weather.Snowlight;
					case -1429616491:
						return Weather.Christmas;
					case -921030142:
						return Weather.Halloween;
					default:
						return Weather.Unknown;
				}
			}
			set
			{
				if (Enum.IsDefined(typeof(Weather), value) && value != Weather.Unknown)
				{
					API.SetWeatherTypeNow(_weatherNames[(int)value]);
				}
			}
		}
		/// <summary>
		/// Gets or sets the next weather.
		/// </summary>
		/// <value>
		/// The next weather.
		/// </value>
		public static Weather NextWeather
		{
			get
			{
				switch (unchecked((int)API.GetNextWeatherTypeHashName()))
				{
					case -1750463879:
						return Weather.ExtraSunny;
					case 916995460:
						return Weather.Clear;
					case -1530260698:
						return Weather.Neutral;
					case 282916021:
						return Weather.Smog;
					case -1368164796:
						return Weather.Foggy;
					case 821931868:
						return Weather.Clouds;
					case -1148613331:
						return Weather.Overcast;
					case 1840358669:
						return Weather.Clearing;
					case 1420204096:
						return Weather.Raining;
					case -1233681761:
						return Weather.ThunderStorm;
					case 669657108:
						return Weather.Blizzard;
					case -273223690:
						return Weather.Snowing;
					case 603685163:
						return Weather.Snowlight;
					case -1429616491:
						return Weather.Christmas;
					case -921030142:
						return Weather.Halloween;
					default:
						return Weather.Unknown;
				}
			}
			set
			{
				if (Enum.IsDefined(typeof(Weather), value) && value != Weather.Unknown)
				{
					API.SetWeatherTypeOverTime(value.ToString(), 0f); // rockstar uses 45 seconds transition time, use TransitionToWeather for that.
				}
			}
		}
		/// <summary>
		/// Gets or sets the weather transition.
		/// </summary>
		/// <value>
		/// The weather transition.
		/// </value>
		public static float WeatherTransition
		{
			get
			{
				// CFX-TODO: Find a way to get this native to work, currently it doesn't seem to work.
				// API.GetWeatherTypeTransition()
				return 0.0f;
			}
			set
			{
				// doesn't seem to work.
				API.SetWeatherTypeTransition(0, 0, value);
			}
		}
		/// <summary>
		/// Transitions to weather. Duration is 45f in most scripts.
		/// </summary>
		/// <param name="weather">The weather.</param>
		/// <param name="duration">The duration.</param>
		public static void TransitionToWeather(Weather weather, float duration)
		{
			if (Enum.IsDefined(typeof(Weather), weather) && weather != Weather.Unknown)
			{
				API.SetWeatherTypeOverTime(_weatherNames[(int)weather], duration);
			}
		}

		/// <summary>
		/// Sets the gravity level for all <see cref="World"/> objects.
		/// </summary>
		/// <value>
		/// The gravity level: 
		/// 9.8f - Default gravity.
		/// 2.4f - Moon gravity.
		/// 0.1f - Very low gravity.
		/// 0.0f - No gravity.
		/// </value>
		public static float GravityLevel
		{
			get { return MemoryAccess.ReadWorldGravity(); }
			set
			{
				//write the value you want to the first item in the array where the native reads the gravity level choices from
				MemoryAccess.WriteWorldGravity(value);
				//call set_gravity_level normally using 0 as gravity type
				//the native will then set the gravity level to what we just wrote
				API.SetGravityLevel(0);
				//reset the array item back to 9.8 so as to restore behaviour of the native
				MemoryAccess.WriteWorldGravity(9.800000f);
			}
		}

		/// <summary>
		/// Gets or sets the rendering camera.
		/// </summary>
		/// <value>
		/// The rendering <see cref="Camera"/>.
		/// </value>
		/// <remarks>
		/// Setting to <c>null</c> sets the rendering <see cref="Camera"/> to <see cref="GameplayCamera"/>.
		/// </remarks>
		public static Camera RenderingCamera
		{
			get
			{
				return new Camera(API.GetRenderingCam());
			}
			set
			{
				if (value == null)
				{
					API.RenderScriptCams(false, false, 3000, true, false);
				}
				else
				{
					value.IsActive = true;
					API.RenderScriptCams(true, false, 3000, true, false);
				}
			}
		}
		/// <summary>
		/// Destroys all user created <see cref="Camera"/>s.
		/// </summary>
		public static void DestroyAllCameras()
		{
			API.DestroyAllCams(false);
		}

		/// <summary>
		/// Gets or sets the waypoint position.
		/// </summary>
		/// <returns>The <see cref="Vector3"/> coordinates of the Waypoint <see cref="Blip"/></returns>
		/// <remarks>
		/// Returns an empty <see cref="Vector3"/> if a waypoint <see cref="Blip"/> hasn't been set
		/// If the game engine cant extract height information the Z component will be 0.0f
		/// </remarks>
		public static Vector3 WaypointPosition
		{
			get
			{
				Blip waypointBlip = GetWaypointBlip();

				if (waypointBlip == null)
				{
					return Vector3.Zero;
				}

				Vector3 position = waypointBlip.Position;
				position.Z = GetGroundHeight(position);

				return position;
			}
			set
			{
				API.SetNewWaypoint(value.X, value.Y);
			}
		}

		/// <summary>
		/// Gets the waypoint blip.
		/// </summary>
		/// <returns>The <see cref="Vector3"/> coordinates of the Waypoint <see cref="Blip"/></returns>
		/// <remarks>
		/// Returns <c>null</c> if a waypoint <see cref="Blip"/> hasn't been set
		/// </remarks>
		public static Blip GetWaypointBlip()
		{
			if (!Game.IsWaypointActive)
			{
				return null;
			}

			for (int it = API.GetBlipInfoIdIterator(), blip = API.GetFirstBlipInfoId(it); API.DoesBlipExist(blip); blip = API.GetNextBlipInfoId(it))
			{
				if (API.GetBlipInfoIdType(blip) == 4)
				{
					return new Blip(blip);
				}
			}

			return null;
		}

		/// <summary>
		/// Removes the waypoint.
		/// </summary>
		public static void RemoveWaypoint()
		{
			API.SetWaypointOff();
		}

		/// <summary>
		/// Gets the straight line distance between 2 positions.
		/// </summary>
		/// <param name="origin">The origin.</param>
		/// <param name="destination">The destination.</param>
		/// <returns>The distance</returns>
		public static float GetDistance(Vector3 origin, Vector3 destination)
		{
			return (float)Math.Sqrt(destination.DistanceToSquared(origin));
		}
		/// <summary>
		/// Calculates the travel distance using roads and paths between 2 positions.
		/// </summary>
		/// <param name="origin">The origin.</param>
		/// <param name="destination">The destination.</param>
		/// <returns>The travel distance</returns>
		public static float CalculateTravelDistance(Vector3 origin, Vector3 destination)
		{
			return API.CalculateTravelDistanceBetweenPoints(origin.X, origin.Y, origin.Z, destination.X, destination.Y, destination.Z);
		}
		/// <summary>
		/// Gets the height of the ground at a given position.
		/// </summary>
		/// <param name="position">The position.</param>
		/// <returns>The height measured in meters</returns>
		public static float GetGroundHeight(Vector3 position)
		{
			return GetGroundHeight(new Vector2(position.X, position.Y));
		}

		/// <summary>
		/// Gets the height of the ground at a given position.
		/// </summary>
		/// <param name="position">The position.</param>
		/// <returns>The height measured in meters</returns>
		public static float GetGroundHeight(Vector2 position)
		{
			float resultArg = 0f;
			API.RequestCollisionAtCoord(position.X, position.Y, 1000f);
			API.GetGroundZFor_3dCoord(position.X, position.Y, 1000f, ref resultArg, false);
			return resultArg;
		}

		/// <summary>
		/// Gets an <c>array</c> of all the <see cref="Blip"/>s on the map with a given <see cref="BlipSprite"/>.
		/// </summary>
		/// <param name="blipTypes">The blip types to include, leave blank to get all <see cref="Blip"/>s.</param>
		public static Blip[] GetAllBlips(params BlipSprite[] blipTypes)
		{
			var res = new List<Blip>();
			if (blipTypes.Length == 0)
			{
				blipTypes = Enum.GetValues(typeof(BlipSprite)).Cast<BlipSprite>().ToArray();
			}
			foreach (BlipSprite sprite in blipTypes)
			{
				int handle = API.GetFirstBlipInfoId((int)sprite);

				while (API.DoesBlipExist(handle))
				{
					res.Add(new Blip(handle));

					handle = API.GetNextBlipInfoId((int)sprite);
				}
			}
			return res.ToArray();
		}

		/// <summary>
		/// Gets an <c>array</c> of all the <see cref="Prop"/>s on the map.
		/// </summary>
		public static Prop[] GetAllProps()
		{
			List<Prop> props = new List<Prop>();

			int entHandle = -1;
			int handle = API.FindFirstObject(ref entHandle);

			Prop prop = (Prop)Entity.FromHandle(entHandle);
			if (prop != null && prop.Exists())
				props.Add(prop);

			entHandle = -1;
			while (API.FindNextObject(handle, ref entHandle))
			{
				prop = (Prop)Entity.FromHandle(entHandle);
				if (prop != null && prop.Exists())
					props.Add(prop);

				entHandle = -1;
			}

			API.EndFindObject(handle);
			return props.ToArray();
		}

		/// <summary>
		/// Gets an <c>array</c> of all the <see cref="Ped"/>s on the map.
		/// </summary>
		public static Ped[] GetAllPeds()
		{
			List<Ped> peds = new List<Ped>();

			int entHandle = -1;
			int handle = API.FindFirstPed(ref entHandle);
			Ped ped = (Ped)Entity.FromHandle(entHandle);
			if (ped != null && ped.Exists())
				peds.Add(ped);

			entHandle = -1;
			while (API.FindNextPed(handle, ref entHandle))
			{
				ped = (Ped)Entity.FromHandle(entHandle);
				if (ped != null && ped.Exists())
					peds.Add(ped);
				entHandle = -1;
			}
			API.EndFindPed(handle);
			return peds.ToArray();
		}

		/// <summary>
		/// Gets an <c>array</c> of all the <see cref="Vehicle"/>s on the map.
		/// </summary>
		public static Vehicle[] GetAllVehicles()
		{
			List<Vehicle> vehicles = new List<Vehicle>();

			int entHandle = -1;
			int handle = API.FindFirstVehicle(ref entHandle);
			Vehicle veh = (Vehicle)Entity.FromHandle(entHandle);
			if (veh != null && veh.Exists())
				vehicles.Add(veh);

			entHandle = -1;
			while (API.FindNextVehicle(handle, ref entHandle))
			{
				veh = (Vehicle)Entity.FromHandle(entHandle);
				if (veh != null && veh.Exists())
					vehicles.Add(veh);
				entHandle = -1;
			}
			API.EndFindVehicle(handle);
			return vehicles.ToArray();
		}

		/// <summary>
		/// Gets an <c>array</c> of all the <see cref="Pickup"/>s on the map.
		/// </summary>
		public static Pickup[] GetAllPickups()
		{
			List<Pickup> pickups = new List<Pickup>();

			int entHandle = -1;
			int handle = API.FindFirstPickup(ref entHandle);
			Pickup pickup = new Pickup(entHandle);
			if (pickup != null && pickup.Exists())
				pickups.Add(pickup);

			entHandle = -1;
			while (API.FindNextPickup(handle, ref entHandle))
			{
				pickup = new Pickup(entHandle);
				if (pickup != null && pickup.Exists())
					pickups.Add(pickup);
				entHandle = -1;
			}
			API.EndFindPickup(handle);
			return pickups.ToArray();
		}

		// CFX-TODO
		/*
		/// <summary>
		/// Gets an <c>array</c> of all the <see cref="Checkpoint"/>s.
		/// </summary>
		public static Checkpoint[] GetAllCheckpoints()
		{					   
			return Array.ConvertAll<int, Checkpoint>(MemoryAccess.GetCheckpointHandles(), element => new Checkpoint(element));
		}

		static int[] ModelListToHashList(Model[] models)
		{
			return Array.ConvertAll<Model, int>(models, model => model.Hash);
		}

		/// <summary>
		/// Gets an <c>array</c>of all <see cref="Ped"/>s in the World.
		/// </summary>
		/// <param name="models">The <see cref="Model"/> of <see cref="Ped"/>s to get, leave blank for all <see cref="Ped"/> <see cref="Model"/>s.</param>
		public static Ped[] GetAllPeds(params Model[] models)
		{
			return Array.ConvertAll<int, Ped>(MemoryAccess.GetPedHandles(ModelListToHashList(models)), handle => new Ped(handle));
		}

		/// <summary>
		/// Gets an <c>array</c> of all <see cref="Ped"/>s in a given region in the World.
		/// </summary>
		/// <param name="position">The position to check the <see cref="Ped"/> against.</param>
		/// <param name="radius">The maximun distance from the <paramref name="position"/> to detect <see cref="Ped"/>s.</param>
		/// <param name="models">The <see cref="Model"/> of <see cref="Ped"/>s to get, leave blank for all <see cref="Ped"/> <see cref="Model"/>s.</param>
		public static Ped[] GetNearbyPeds(Vector3 position, float radius, params Model[] models)
		{
			return Array.ConvertAll<int, Ped>(MemoryAccess.GetPedHandles(position, radius, ModelListToHashList(models)), handle => new Ped(handle));
		}
		/// <summary>
		/// Gets an <c>array</c> of all <see cref="Ped"/>s near a given <see cref="Ped"/> in the world
		/// </summary>
		/// <param name="ped">The ped to check.</param>
		/// <param name="radius">The maximun distance from the <paramref name="ped"/> to detect <see cref="Ped"/>s.</param>
		/// <param name="models">The <see cref="Model"/> of <see cref="Ped"/>s to get, leave blank for all <see cref="Ped"/> <see cref="Model"/>s.</param>
		/// <remarks>Doesnt include the <paramref name="ped"/> in the result</remarks>
		public static Ped[] GetNearbyPeds(Ped ped, float radius, params Model[] models)
		{
			int[] handles = MemoryAccess.GetPedHandles(ped.Position, radius, ModelListToHashList(models));

			var result = new List<Ped>();

			foreach (int handle in handles)
			{
				if (handle == ped.Handle)
				{
					continue;
				}

				result.Add(new Ped(handle));
			}

			return result.ToArray();
		}
		/// <summary>
		/// Gets the closest <see cref="Ped"/> to a given position in the World.
		/// </summary>
		/// <param name="position">The position to find the nearest <see cref="Ped"/>.</param>
		/// <param name="radius">The maximun distance from the <paramref name="position"/> to detect <see cref="Ped"/>s.</param>
		/// <param name="models">The <see cref="Model"/> of <see cref="Ped"/>s to get, leave blank for all <see cref="Ped"/> <see cref="Model"/>s.</param>
		/// <remarks>Returns <c>null</c> if no <see cref="Ped"/> was in the given region.</remarks>
		public static Ped GetClosestPed(Vector3 position, float radius, params Model[] models)
		{
			Ped[] peds =
				Array.ConvertAll<int, Ped>(MemoryAccess.GetPedHandles(position, radius, ModelListToHashList(models)),
					handle => new Ped(handle));
			return GetClosest<Ped>(position, peds);
		}


		/// <summary>
		/// A fast way to get the total number of vehicles spawned in the world.
		/// </summary>
        /// // CFX-TODO
		public static int VehicleCount { get { return 0; } }// return MemoryAccess.GetNumberOfVehicles(); } }
		/// <summary>
		/// Gets an <c>array</c> of all <see cref="Vehicle"/>s in the World.
		/// </summary>
		/// <param name="models">The <see cref="Model"/> of <see cref="Vehicle"/>s to get, leave blank for all <see cref="Vehicle"/> <see cref="Model"/>s.</param>
		public static Vehicle[] GetAllVehicles(params Model[] models)
		{
			return Array.ConvertAll<int, Vehicle>(MemoryAccess.GetVehicleHandles(ModelListToHashList(models)), handle => new Vehicle(handle));
		}
		/// <summary>
		/// Gets an <c>array</c> of all <see cref="Vehicle"/>s in a given region in the World.
		/// </summary>
		/// <param name="position">The position to check the <see cref="Vehicle"/> against.</param>
		/// <param name="radius">The maximun distance from the <paramref name="position"/> to detect <see cref="Vehicle"/>s.</param>
		/// <param name="models">The <see cref="Model"/> of <see cref="Vehicle"/>s to get, leave blank for all <see cref="Vehicle"/> <see cref="Model"/>s.</param>
		public static Vehicle[] GetNearbyVehicles(Vector3 position, float radius, params Model[] models)
		{
			return Array.ConvertAll<int, Vehicle>(MemoryAccess.GetVehicleHandles(position, radius, ModelListToHashList(models)), handle => new Vehicle(handle));
		}
		/// <summary>
		/// Gets an <c>array</c> of all <see cref="Vehicle"/>s near a given <see cref="Ped"/> in the world
		/// </summary>
		/// <param name="ped">The ped to check.</param>
		/// <param name="radius">The maximun distance from the <paramref name="ped"/> to detect <see cref="Vehicle"/>s.</param>
		/// <param name="models">The <see cref="Model"/> of <see cref="Vehicle"/>s to get, leave blank for all <see cref="Vehicle"/> <see cref="Model"/>s.</param>
		/// <remarks>Doesnt include the <see cref="Vehicle"/> the <paramref name="ped"/> is using in the result</remarks>
		public static Vehicle[] GetNearbyVehicles(Ped ped, float radius, params Model[] models)
		{
			int[] handles = MemoryAccess.GetVehicleHandles(ped.Position, radius, ModelListToHashList(models));

			var result = new List<Vehicle>();
			Vehicle ignore = ped.CurrentVehicle;
			int ignoreHandle = Vehicle.Exists(ignore) ? ignore.Handle : 0;

			foreach (int handle in handles)
			{
				if (handle == ignoreHandle)
				{
					continue;
				}

				result.Add(new Vehicle(handle));
			}

			return result.ToArray();
		}
		/// <summary>
		/// Gets the closest <see cref="Vehicle"/> to a given position in the World.
		/// </summary>
		/// <param name="position">The position to find the nearest <see cref="Vehicle"/>.</param>
		/// <param name="radius">The maximun distance from the <paramref name="position"/> to detect <see cref="Vehicle"/>s.</param>
		/// <param name="models">The <see cref="Model"/> of <see cref="Vehicle"/>s to get, leave blank for all <see cref="Vehicle"/> <see cref="Model"/>s.</param>
		/// <remarks>Returns <c>null</c> if no <see cref="Vehicle"/> was in the given region.</remarks>
		public static Vehicle GetClosestVehicle(Vector3 position, float radius, params Model[] models)
		{
			Vehicle[] vehicles = 
				Array.ConvertAll<int, Vehicle>(MemoryAccess.GetVehicleHandles(position, radius, ModelListToHashList(models)),
					handle => new Vehicle(handle));
			return GetClosest<Vehicle>(position, vehicles);

		}

		/// <summary>
		/// Gets an <c>array</c> of all <see cref="Prop"/>s in the World.
		/// </summary>
		/// <param name="models">The <see cref="Model"/> of <see cref="Prop"/>s to get, leave blank for all <see cref="Prop"/> <see cref="Model"/>s.</param>
		public static Prop[] GetAllProps(params Model[] models)
		{						
			return Array.ConvertAll<int, Prop>(MemoryAccess.GetPropHandles(ModelListToHashList(models)), handle => new Prop(handle));
		}
		/// <summary>
		/// Gets an <c>array</c> of all <see cref="Prop"/>s in a given region in the World.
		/// </summary>
		/// <param name="position">The position to check the <see cref="Prop"/> against.</param>
		/// <param name="radius">The maximun distance from the <paramref name="position"/> to detect <see cref="Prop"/>s.</param>
		/// <param name="models">The <see cref="Model"/> of <see cref="Prop"/>s to get, leave blank for all <see cref="Prop"/> <see cref="Model"/>s.</param>
		public static Prop[] GetNearbyProps(Vector3 position, float radius, params Model[] models)
		{
			return Array.ConvertAll<int, Prop>(MemoryAccess.GetPropHandles(position, radius, ModelListToHashList(models)), handle => new Prop(handle));
		}
		/// <summary>
		/// Gets the closest <see cref="Prop"/> to a given position in the World.
		/// </summary>
		/// <param name="position">The position to find the nearest <see cref="Prop"/>.</param>
		/// <param name="radius">The maximun distance from the <paramref name="position"/> to detect <see cref="Prop"/>s.</param>
		/// <param name="models">The <see cref="Model"/> of <see cref="Prop"/>s to get, leave blank for all <see cref="Prop"/> <see cref="Model"/>s.</param>
		/// <remarks>Returns <c>null</c> if no <see cref="Prop"/> was in the given region.</remarks>
		public static Prop GetClosestProp(Vector3 position, float radius, params Model[] models)
		{
			Prop[] props =
				Array.ConvertAll<int, Prop>(MemoryAccess.GetPropHandles(position, radius, ModelListToHashList(models)),
					handle => new Prop(handle));
			return GetClosest<Prop>(position, props);

		}
		/// <summary>
		/// Gets an <c>array</c> of all <see cref="Entity"/>s in the World.
		/// </summary>
		public static Entity[] GetAllEntities()
		{									   
			return Array.ConvertAll<int, Entity>(MemoryAccess.GetEntityHandles(), Entity.FromHandle);
		}
		/// <summary>
		/// Gets an <c>array</c> of all <see cref="Entity"/>s in a given region in the World.
		/// </summary>
		/// <param name="position">The position to check the <see cref="Entity"/> against.</param>
		/// <param name="radius">The maximun distance from the <paramref name="position"/> to detect <see cref="Entity"/>s.</param>
		public static Entity[] GetNearbyEntities(Vector3 position, float radius)
		{
			return Array.ConvertAll<int, Entity>(MemoryAccess.GetEntityHandles(position, radius), Entity.FromHandle);
		}

		/// <summary>
		/// Gets an <c>array</c> of all <see cref="Prop"/>s in the World associated with a <see cref="Pickup"/>.
		/// </summary>
		public static Prop[] GetAllPickupObjects()
		{
			return Array.ConvertAll<int, Prop>(MemoryAccess.GetPickupObjectHandles(), handle => new Prop(handle));
		}
		/// <summary>
		/// Gets an <c>array</c> of all <see cref="Prop"/>s in a given region in the World associated with a <see cref="Pickup"/>.
		/// </summary>
		/// <param name="position">The position to check the <see cref="Entity"/> against.</param>
		/// <param name="radius">The maximun distance from the <paramref name="position"/> to detect <see cref="Prop"/>s.</param>
		public static Prop[] GetNearbyPickupObjects(Vector3 position, float radius)
		{
			return Array.ConvertAll<int, Prop>(MemoryAccess.GetPickupObjectHandles(position, radius), handle => new Prop(handle));
		}

		/// <summary>
		/// Gets the closest <see cref="Prop"/> to a given position in the World associated with a <see cref="Pickup"/>.
		/// </summary>
		/// <param name="position">The position to find the nearest <see cref="Prop"/>.</param>
		/// <param name="radius">The maximun distance from the <paramref name="position"/> to detect <see cref="Prop"/>s.</param>
		/// <remarks>Returns <c>null</c> if no <see cref="Prop"/> was in the given region.</remarks>
		public static Prop GetClosestPickupObject(Vector3 position, float radius)
		{
			Prop[] props =
				Array.ConvertAll<int, Prop>(MemoryAccess.GetPickupObjectHandles(position, radius),
					handle => new Prop(handle));
			return GetClosest<Prop>(position, props);
		}
        */

		/// <summary>
		/// Gets the closest <see cref="ISpatial"/> to a given position in the World.
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="position">The position to check against.</param>
		/// <param name="spatials">The spatials to check.</param>
		/// <returns>The closest <see cref="ISpatial"/> to the <paramref name="position"/></returns>
		public static T GetClosest<T>(Vector3 position, params T[] spatials) where T : ISpatial
		{
			ISpatial closest = null;
			float closestDistance = 3e38f;

			foreach (var spatial in spatials)
			{
				float distance = position.DistanceToSquared(spatial.Position);

				if (distance <= closestDistance)
				{
					closest = spatial;
					closestDistance = distance;
				}
			}
			return (T)closest;
		}
		/// <summary>
		/// Gets the closest <see cref="ISpatial"/> to a given position in the World ignoring height.
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="position">The position to check against.</param>
		/// <param name="spatials">The spatials to check.</param>
		/// <returns>The closest <see cref="ISpatial"/> to the <paramref name="position"/></returns>
		public static T GetClosest<T>(Vector2 position, params T[] spatials) where T : ISpatial
		{
			ISpatial closest = null;
			float closestDistance = 3e38f;
			Vector3 pos = new Vector3(position.X, position.Y, 0.0f);
			foreach (var spatial in spatials)
			{
				float distance = pos.DistanceToSquared2D(spatial.Position);

				if (distance <= closestDistance)
				{
					closest = spatial;
					closestDistance = distance;
				}
			}
			return (T)closest;
		}

		/// <summary>
		/// Gets the nearest safe coordinate to position a <see cref="Ped"/>.
		/// </summary>
		/// <param name="position">The position to check around.</param>
		/// <param name="sidewalk">if set to <c>true</c> Only find positions on the sidewalk.</param>
		/// <param name="flags">The flags.</param>
		public static Vector3 GetSafeCoordForPed(Vector3 position, bool sidewalk = true, int flags = 0)
		{
			Vector3 outPos = new Vector3();
			if (API.GetSafeCoordForPed(position.X, position.Y, position.Z, sidewalk, ref outPos, flags))
			{
				return outPos;
			}
			return Vector3.Zero;
		}

		/// <summary>
		/// Gets the next position on the street where a <see cref="Vehicle"/> can be placed.
		/// </summary>
		/// <param name="position">The position to check around.</param>
		/// <param name="unoccupied">if set to <c>true</c> only find positions that dont already have a vehicle in them.</param>
		public static Vector3 GetNextPositionOnStreet(Vector2 position, bool unoccupied = false)
		{
			return GetNextPositionOnStreet(new Vector3(position.X, position.Y, 0f), unoccupied);
		}
		/// <summary>
		/// Gets the next position on the street where a <see cref="Vehicle"/> can be placed.
		/// </summary>
		/// <param name="position">The position to check around.</param>
		/// <param name="unoccupied">if set to <c>true</c> only find positions that dont already have a vehicle in them.</param>
		public static Vector3 GetNextPositionOnStreet(Vector3 position, bool unoccupied = false)
		{
			Vector3 outPos = new Vector3();
			if (unoccupied)
			{
				for (int i = 1; i < 40; i++)
				{
					API.GetNthClosestVehicleNode(position.X, position.Y, position.Z, i, ref outPos, 1, 3, 0);
					if (!outPos.IsZero && !API.IsPointObscuredByAMissionEntity(outPos.X, outPos.Y, outPos.Z, 5f, 5f, 5f, 0))
					{
						return outPos;
					}
				}
			}
			else if (API.GetNthClosestVehicleNode(position.X, position.Y, position.Z, 1, ref outPos, 1, 3, 0))
			{
				return outPos;
			}
			return Vector3.Zero;
		}

		/// <summary>
		/// Gets the next position on the street where a <see cref="Ped"/> can be placed.
		/// </summary>
		/// <param name="position">The position to check around.</param>
		public static Vector3 GetNextPositionOnSidewalk(Vector2 position)
		{
			return GetNextPositionOnSidewalk(new Vector3(position.X, position.Y, 0f));
		}
		/// <summary>
		/// Gets the next position on the street where a <see cref="Ped"/> can be placed.
		/// </summary>
		/// <param name="position">The position to check around.</param>
		public static Vector3 GetNextPositionOnSidewalk(Vector3 position)
		{
			Vector3 outPos = new Vector3();
			if (API.GetSafeCoordForPed(position.X, position.Y, position.Z, true, ref outPos, 0))
			{
				return outPos;
			}
			else if (API.GetSafeCoordForPed(position.X, position.Y, position.Z, false, ref outPos, 0))
			{
				return outPos;
			}
			return Vector3.Zero;
		}

		/// <summary>
		/// Gets the localized name of the a zone in the map.
		/// </summary>
		/// <param name="position">The position on the map.</param>
		public static string GetZoneLocalizedName(Vector2 position)
		{
			return GetZoneLocalizedName(new Vector3(position.X, position.Y, 0f));
		}
		/// <summary>
		/// Gets the localized name of the a zone in the map.
		/// </summary>
		/// <param name="position">The position on the map.</param>
		public static string GetZoneLocalizedName(Vector3 position)
		{
			return Game.GetGXTEntry(API.GetNameOfZone(position.X, position.Y, position.Z));
		}
		/// <summary>
		/// Gets the display name of the a zone in the map.
		/// Use <see cref="Game.GetGXTEntry(string)"/> to convert to the localized name.
		/// </summary>
		/// <param name="position">The position on the map.</param>
		public static string GetZoneDisplayName(Vector2 position)
		{
			return GetZoneDisplayName(new Vector3(position.X, position.Y, 0f));
		}
		/// <summary>
		/// Gets the display name of the a zone in the map.
		/// Use <see cref="Game.GetGXTEntry(string)"/> to convert to the localized name.
		/// </summary>
		/// <param name="position">The position on the map.</param>
		public static string GetZoneDisplayName(Vector3 position)
		{
			return API.GetNameOfZone(position.X, position.Y, position.Z);
		}

		public static string GetStreetName(Vector2 position)
		{
			return GetStreetName(new Vector3(position.X, position.Y, 0f));
		}
		public static string GetStreetName(Vector3 position)
		{
			uint streetHash = 0u, crossingHash = 0u;
			API.GetStreetNameAtCoord(position.X, position.Y, position.Z, ref streetHash, ref crossingHash);

			return API.GetStreetNameFromHashKey(streetHash);
		}


		/// <summary>
		/// Creates a <see cref="Blip"/> at the given position on the map.
		/// </summary>
		/// <param name="position">The position of the blip on the map.</param>
		public static Blip CreateBlip(Vector3 position)
		{
			return new Blip(API.AddBlipForCoord(position.X, position.Y, position.Z));
		}
		/// <summary>
		/// Creates a <see cref="Blip"/> for a circular area at the given position on the map.
		/// </summary>
		/// <param name="position">The position of the blip on the map.</param>
		/// <param name="radius">The radius of the area on the map.</param>
		public static Blip CreateBlip(Vector3 position, float radius)
		{
			return new Blip(API.AddBlipForRadius(position.X, position.Y, position.Z, radius));
		}

		/// <summary>
		/// Creates a <see cref="Camera"/>, use <see cref="World.RenderingCamera"/> to switch to this camera
		/// </summary>
		/// <param name="position">The position of the camera.</param>
		/// <param name="rotation">The rotation of the camera.</param>
		/// <param name="fov">The field of view of the camera.</param>
		public static Camera CreateCamera(Vector3 position, Vector3 rotation, float fov)
		{
			return new Camera(API.CreateCamWithParams("DEFAULT_SCRIPTED_CAMERA", position.X, position.Y, position.Z, rotation.X, rotation.Y, rotation.Z, fov, true, 2));
		}

		/// <summary>
		/// Spawns a <see cref="Ped"/> of the given <see cref="Model"/> at the position and heading specified.
		/// </summary>
		/// <param name="model">The <see cref="Model"/> of the <see cref="Ped"/>.</param>
		/// <param name="position">The position to spawn the <see cref="Ped"/> at.</param>
		/// <param name="heading">The heading of the <see cref="Ped"/>.</param>
		/// <remarks>returns <c>null</c> if the <see cref="Ped"/> could not be spawned</remarks>
		public static async TaskPed CreatePed(Model model, Vector3 position, float heading = 0f)
		{
			if (!model.IsPed || !await model.Request(1000))
			{
				return null;
			}

			return new Ped(API.CreatePed(26, (uint)model.Hash, position.X, position.Y, position.Z, heading, true, false));
		}
		/// <summary>
		/// Spawns a <see cref="Ped"/> of a random <see cref="Model"/> at the position specified.
		/// </summary>
		/// <param name="position">The position to spawn the <see cref="Ped"/> at.</param>
		public static Ped CreateRandomPed(Vector3 position)
		{
			return new Ped(API.CreateRandomPed(position.X, position.Y, position.Z));
		}

		/// <summary>
		/// Spawns a <see cref="Vehicle"/> of the given <see cref="Model"/> at the position and heading specified.
		/// </summary>
		/// <param name="model">The <see cref="Model"/> of the <see cref="Vehicle"/>.</param>
		/// <param name="position">The position to spawn the <see cref="Vehicle"/> at.</param>
		/// <param name="heading">The heading of the <see cref="Vehicle"/>.</param>
		/// <remarks>returns <c>null</c> if the <see cref="Vehicle"/> could not be spawned</remarks>
		public static async TaskVehicle CreateVehicle(Model model, Vector3 position, float heading = 0f)
		{
			if (!model.IsVehicle || !await model.Request(1000))
			{
				return null;
			}

			return new Vehicle(API.CreateVehicle((uint)model.Hash, position.X, position.Y, position.Z, heading, true, false));
		}

		/// <summary>
		/// Spawns a <see cref="Vehicle"/> of a random <see cref="Model"/> at the position specified.
		/// </summary>
		/// <param name="position">The position to spawn the <see cref="Vehicle"/> at.</param>
		/// <param name="heading">The heading of the <see cref="Vehicle"/>.</param>
		/// <remarks>returns <c>null</c> if the <see cref="Vehicle"/> could not be spawned</remarks>
		public static TaskVehicle CreateRandomVehicle(Vector3 position, float heading = 0f)
		{
			Array vehicleHashes = Enum.GetValues(typeof(VehicleHash));
			Random random = new Random();
			Model model = null;
			while (model is null || !model.IsValid || !model.IsInCdImage || !model.IsVehicle)
			{
				model = (VehicleHash)vehicleHashes.GetValue(random.Next(vehicleHashes.Length));
			}
			return CreateVehicle(model, position, heading);
		}

		//[SecurityCritical]
		//private static Vehicle _CreateRandomVehicle(Vector3 position, float heading = 0f)
		//{
		//	int outModel, outInt;
		//	unsafe
		//	{
		//		Function.Call(Hash.GET_RANDOM_VEHICLE_MODEL_IN_MEMORY, 1, &outModel, &outInt);
		//	}
		//	Model model = outModel;
		//	if (model.IsVehicle && model.IsLoaded)
		//	{
		//		return
		//			new Vehicle(Function.Call<int>(Hash.CREATE_VEHICLE, model.Hash, position.X, position.Y, position.Z, heading, false, false));
		//	}
		//	return null;
		//}

		/// <summary>
		/// Spawns a <see cref="Prop"/> of the given <see cref="Model"/> at the position specified.
		/// </summary>
		/// <param name="model">The <see cref="Model"/> of the <see cref="Prop"/>.</param>
		/// <param name="position">The position to spawn the <see cref="Prop"/> at.</param>
		/// <param name="dynamic">if set to <c>true</c> the <see cref="Prop"/> will have physics; otherwise, it will be static.</param>
		/// <param name="placeOnGround">if set to <c>true</c> place the prop on the ground nearest to the <paramref name="position"/>.</param>
		/// <remarks>returns <c>null</c> if the <see cref="Prop"/> could not be spawned</remarks>
		public static async TaskProp CreateProp(Model model, Vector3 position, bool dynamic, bool placeOnGround)
		{
			if (!await model.Request(1000))
			{
				return null;
			}

			if (placeOnGround)
			{
				position.Z = GetGroundHeight(position);
			}

			return new Prop(API.CreateObject((compat_i32_u32)model.Hash, position.X, position.Y, position.Z, true, true, dynamic));
		}
		/// <summary>
		/// Spawns a <see cref="Prop"/> of the given <see cref="Model"/> at the position specified.
		/// </summary>
		/// <param name="model">The <see cref="Model"/> of the <see cref="Prop"/>.</param>
		/// <param name="position">The position to spawn the <see cref="Prop"/> at.</param>
		/// <param name="rotation">The rotation of the <see cref="Prop"/>.</param>
		/// <param name="dynamic">if set to <c>true</c> the <see cref="Prop"/> will have physics; otherwise, it will be static.</param>
		/// <param name="placeOnGround">if set to <c>true</c> place the prop on the ground nearest to the <paramref name="position"/>.</param>
		/// <remarks>returns <c>null</c> if the <see cref="Prop"/> could not be spawned</remarks>
		public static async TaskProp CreateProp(Model model, Vector3 position, Vector3 rotation, bool dynamic, bool placeOnGround)
		{
			Prop prop = await CreateProp(model, position, dynamic, placeOnGround);

			if (prop != null)
			{
				prop.Rotation = rotation;
			}

			return prop;
		}
		/// <summary>
		/// Spawns a <see cref="Prop"/> of the given <see cref="Model"/> at the position specified without any offset.
		/// </summary>
		/// <param name="model">The <see cref="Model"/> of the <see cref="Prop"/>.</param>
		/// <param name="position">The position to spawn the <see cref="Prop"/> at.</param>
		/// <param name="dynamic">if set to <c>true</c> the <see cref="Prop"/> will have physics; otherwise, it will be static.</param>
		/// <remarks>returns <c>null</c> if the <see cref="Prop"/> could not be spawned</remarks>
		public static async TaskProp CreatePropNoOffset(Model model, Vector3 position, bool dynamic)
		{
			if (!await model.Request(1000))
			{
				return null;
			}

			return new Prop(API.CreateObjectNoOffset((uint)model.Hash, position.X, position.Y, position.Z, true, true, dynamic));
		}
		/// <summary>
		/// Spawns a <see cref="Prop"/> of the given <see cref="Model"/> at the position specified without any offset.
		/// </summary>
		/// <param name="model">The <see cref="Model"/> of the <see cref="Prop"/>.</param>
		/// <param name="position">The position to spawn the <see cref="Prop"/> at.</param>
		/// <param name="rotation">The rotation of the <see cref="Prop"/>.</param>
		/// <param name="dynamic">if set to <c>true</c> the <see cref="Prop"/> will have physics; otherwise, it will be static.</param>
		/// <remarks>returns <c>null</c> if the <see cref="Prop"/> could not be spawned</remarks>
		public static async TaskProp CreatePropNoOffset(Model model, Vector3 position, Vector3 rotation, bool dynamic)
		{
			Prop prop = await CreatePropNoOffset(model, position, dynamic);

			if (prop != null)
			{
				prop.Rotation = rotation;
			}

			return prop;
		}

		public static async TaskPickup CreatePickup(PickupType type, Vector3 position, Model model, int value)
		{
			if (!await model.Request(1000))
			{
				return null;
			}

			int handle = API.CreatePickup((uint)type, position.X, position.Y, position.Z, 0, value, true, (uint)model.Hash);

			if (handle == 0)
			{
				return null;
			}

			return new Pickup(handle);
		}
		public static async TaskPickup CreatePickup(PickupType type, Vector3 position, Vector3 rotation, Model model, int value)
		{
			if (!await model.Request(1000))
			{
				return null;
			}

			int handle = API.CreatePickupRotate((uint)type, position.X, position.Y, position.Z, rotation.X, rotation.Y, rotation.Z, 0, value, 2, true, (uint)model.Hash);

			if (handle == 0)
			{
				return null;
			}

			return new Pickup(handle);
		}
		public static async TaskProp CreateAmbientPickup(PickupType type, Vector3 position, Model model, int value)
		{
			if (!await model.Request(1000))
			{
				return null;
			}

			int handle = API.CreateAmbientPickup((uint)type, position.X, position.Y, position.Z, 0, value, (uint)model.Hash, false, true);

			if (handle == 0)
			{
				return null;
			}

			return new Prop(handle);
		}

		/// <summary>
		/// Creates a <see cref="Checkpoint"/> in the world.
		/// </summary>
		/// <param name="icon">The <see cref="CheckpointIcon"/> to display inside the <see cref="Checkpoint"/>.</param>
		/// <param name="position">The position in the World.</param>
		/// <param name="pointTo">The position in the world where this <see cref="Checkpoint"/> should point.</param>
		/// <param name="radius">The radius of the <see cref="Checkpoint"/>.</param>
		/// <param name="color">The color of the <see cref="Checkpoint"/>.</param>
		/// <remarks>returns <c>null</c> if the <see cref="Checkpoint"/> could not be created</remarks>
		public static Checkpoint CreateCheckpoint(CheckpointIcon icon, Vector3 position, Vector3 pointTo, float radius, Color color)
		{
			int handle = API.CreateCheckpoint((int)icon, position.X, position.Y, position.Z, pointTo.X, pointTo.Y, pointTo.Z, radius, color.R, color.G, color.B, color.A, 0);

			if (handle == 0)
			{
				return null;
			}

			return new Checkpoint(handle);
		}
		/// <summary>
		/// Creates a <see cref="Checkpoint"/> in the world.
		/// </summary>
		/// <param name="icon">The <see cref="CheckpointCustomIcon"/> to display inside the <see cref="Checkpoint"/>.</param>
		/// <param name="position">The position in the World.</param>
		/// <param name="pointTo">The position in the world where this <see cref="Checkpoint"/> should point.</param>
		/// <param name="radius">The radius of the <see cref="Checkpoint"/>.</param>
		/// <param name="color">The color of the <see cref="Checkpoint"/>.</param>
		/// <remarks>returns <c>null</c> if the <see cref="Checkpoint"/> could not be created</remarks>
		public static Checkpoint CreateCheckpoint(CheckpointCustomIcon icon, Vector3 position, Vector3 pointTo, float radius, Color color)
		{
			int handle = API.CreateCheckpoint(42, position.X, position.Y, position.Z, pointTo.X, pointTo.Y, pointTo.Z, radius, color.R, color.G, color.B, color.A, icon);

			if (handle == 0)
			{
				return null;
			}

			return new Checkpoint(handle);
		}

		/// <summary>
		/// Spawns a <see cref="Rope"/>.
		/// </summary>
		/// <param name="type">The type of <see cref="Rope"/>.</param>
		/// <param name="position">The position of the <see cref="Rope"/>.</param>
		/// <param name="rotation">The rotation of the <see cref="Rope"/>.</param>
		/// <param name="length">The length of the <see cref="Rope"/>.</param>
		/// <param name="minLength">The minimum length of the <see cref="Rope"/>.</param>
		/// <param name="breakable">if set to <c>true</c> the <see cref="Rope"/> will break if shot.</param>
		public static Rope AddRope(RopeType type, Vector3 position, Vector3 rotation, float length, float minLength, bool breakable)
		{
			API.RopeLoadTextures();
			compat_i32_i64 unkPntr = 0;
			return new Rope(API.AddRope(position.X, position.Y, position.Z, rotation.X, rotation.Y, rotation.Z, length, (int)type, length, minLength, 0.5f, false, false, true, 1.0f, breakable, ref unkPntr));
		}


		/// <summary>
		/// Fires a single bullet in the world
		/// </summary>
		/// <param name="sourcePosition">Where the bullet is fired from.</param>
		/// <param name="targetPosition">Where the bullet is fired to.</param>
		/// <param name="owner">The <see cref="Ped"/> who fired the bullet, leave <c>null</c> for no one.</param>
		/// <param name="weaponAsset">The weapon that the bullet is fired from.</param>
		/// <param name="damage">The damage the bullet will cause.</param>
		/// <param name="speed">The speed, only affects projectile weapons, leave -1 for default.</param>
		public static void ShootBullet(Vector3 sourcePosition, Vector3 targetPosition, Ped owner, WeaponAsset weaponAsset, int damage, float speed = -1f)
		{
			API.ShootSingleBulletBetweenCoords(sourcePosition.X, sourcePosition.Y, sourcePosition.Z, targetPosition.X, targetPosition.Y, targetPosition.Z, damage, true, (uint)weaponAsset.Hash, (owner == null ? 0 : owner.Handle), true, false, speed);
		}

		/// <summary>
		/// Creates an explosion in the world
		/// </summary>
		/// <param name="position">The position of the explosion.</param>
		/// <param name="type">The type of explosion.</param>
		/// <param name="radius">The radius of the explosion.</param>
		/// <param name="cameraShake">The amount of camera shake to apply to nearby cameras.</param>
		/// <param name="owner">The <see cref="Ped"/> who caused the explosion, leave null if no one caused the explosion.</param>
		/// <param name="aubidble">if set to <c>true</c> explosion can be heard.</param>
		/// <param name="invisible">if set to <c>true</c> explosion is invisible.</param>
		public static void AddExplosion(Vector3 position, ExplosionType type, float radius, float cameraShake, Ped owner = null, bool aubidble = true, bool invisible = false)
		{
			if (Entity.Exists(owner))
			{
				API.AddOwnedExplosion(owner.Handle, position.X, position.Y, position.Z, (int)type, radius, aubidble, invisible, cameraShake);
			}
			else
			{
				API.AddExplosion(position.X, position.Y, position.Z, (int)type, radius, aubidble, invisible, cameraShake);
			}
		}

		/// <summary>
		/// Creates a <see cref="RelationshipGroup"/> with the given name.
		/// </summary>
		/// <param name="name">The name of the relationship group.</param>
		public static RelationshipGroup AddRelationshipGroup(string name)
		{
			uint resultArg = 0u;
			API.AddRelationshipGroup(name, ref resultArg);
			return new RelationshipGroup(resultArg);
		}

		/// <summary>
		/// Creates a raycast between 2 points.
		/// </summary>
		/// <param name="source">The source of the raycast.</param>
		/// <param name="target">The target of the raycast.</param>
		/// <param name="options">What type of objects the raycast should intersect with.</param>
		/// <param name="ignoreEntity">Specify an <see cref="Entity"/> that the raycast should ignore, leave null for no entities ignored.</param>
		public static RaycastResult Raycast(Vector3 source, Vector3 target, IntersectOptions options, Entity ignoreEntity = null)
		{
			return new RaycastResult(API.StartShapeTestRay(source.X, source.Y, source.Z, target.X, target.Y, target.Z, (int)options, ignoreEntity == null ? 0 : ignoreEntity.Handle, 7));
		}
		/// <summary>
		/// Creates a raycast between 2 points.
		/// </summary>
		/// <param name="source">The source of the raycast.</param>
		/// <param name="direction">The direction of the raycast.</param>
		/// <param name="maxDistance">How far the raycast should go out to.</param>
		/// <param name="options">What type of objects the raycast should intersect with.</param>
		/// <param name="ignoreEntity">Specify an <see cref="Entity"/> that the raycast should ignore, leave null for no entities ignored.</param>
		public static RaycastResult Raycast(Vector3 source, Vector3 direction, float maxDistance, IntersectOptions options, Entity ignoreEntity = null)
		{
			Vector3 target = source + direction * maxDistance;

			return new RaycastResult(API.StartShapeTestRay(source.X, source.Y, source.Z, target.X, target.Y, target.Z, (int)options, ignoreEntity == null ? 0 : ignoreEntity.Handle, 7));
		}

		/// <summary>
		/// Creates a 3D raycast between 2 points.
		/// </summary>
		/// <param name="source">The source of the raycast.</param>
		/// <param name="target">The target of the raycast.</param>
		/// <param name="radius">The radius of the raycast.</param>
		/// <param name="options">What type of objects the raycast should intersect with.</param>
		/// <param name="ignoreEntity">Specify an <see cref="Entity"/> that the raycast should ignore, leave null for no entities ignored.</param>
		public static RaycastResult RaycastCapsule(Vector3 source, Vector3 target, float radius, IntersectOptions options, Entity ignoreEntity = null)
		{
			return new RaycastResult(API.StartShapeTestCapsule(source.X, source.Y, source.Z, target.X, target.Y, target.Z, radius, (int)options, ignoreEntity == null ? 0 : ignoreEntity.Handle, 7));
		}
		/// <summary>
		/// Creates a 3D raycast between 2 points.
		/// </summary>
		/// <param name="source">The source of the raycast.</param>
		/// <param name="direction">The direction of the raycast.</param>
		/// <param name="radius">The radius of the raycast.</param>
		/// <param name="maxDistance">How far the raycast should go out to.</param>
		/// <param name="options">What type of objects the raycast should intersect with.</param>
		/// <param name="ignoreEntity">Specify an <see cref="Entity"/> that the raycast should ignore, leave null for no entities ignored.</param>
		public static RaycastResult RaycastCapsule(Vector3 source, Vector3 direction, float maxDistance, float radius, IntersectOptions options, Entity ignoreEntity = null)
		{
			Vector3 target = source + direction * maxDistance;

			return new RaycastResult(API.StartShapeTestCapsule(source.X, source.Y, source.Z, target.X, target.Y, target.Z, radius, (int)options, ignoreEntity == null ? 0 : ignoreEntity.Handle, 7));
		}
		/// <summary>
		/// Determines where the crosshair intersects with the world.
		/// </summary>
		/// <returns>A <see cref="RaycastResult"/> containing information about where the crosshair intersects with the world.</returns>
		public static RaycastResult GetCrosshairCoordinates()
		{
			return Raycast(GameplayCamera.Position, GameplayCamera.GetOffsetPosition(new Vector3(0f, 1000f, 0f)), IntersectOptions.Everything, null);
		}

		/// <summary>
		/// Determines where the crosshair intersects with the world.
		/// </summary>
		/// <param name="ignoreEntity">Prevent the raycast detecting a specific <see cref="Entity"/>.</param>
		/// <returns>A <see cref="RaycastResult"/> containing information about where the crosshair intersects with the world.</returns>
		public static RaycastResult GetCrosshairCoordinates(Entity ignoreEntity)
		{
			return Raycast(GameplayCamera.Position, GameplayCamera.GetOffsetPosition(new Vector3(0f, 1000f, 0f)), IntersectOptions.Everything, ignoreEntity);
		}

		/// <summary>
		/// Draws a marker in the world, this needs to be done on a per frame basis
		/// </summary>
		/// <param name="type">The type of marker.</param>
		/// <param name="pos">The position of the marker.</param>
		/// <param name="dir">The direction the marker points in.</param>
		/// <param name="rot">The rotation of the marker.</param>
		/// <param name="scale">The amount to scale the marker by.</param>
		/// <param name="color">The color of the marker.</param>
		/// <param name="bobUpAndDown">if set to <c>true</c> the marker will bob up and down.</param>
		/// <param name="faceCamera">if set to <c>true</c> the marker will always face the camera, regardless of its rotation.</param>
		/// <param name="rotateY">if set to <c>true</c> rotates only on the y axis(heading).</param>
		/// <param name="textueDict">Name of texture dictionary to load the texture from, leave null for no texture in the marker.</param>
		/// <param name="textureName">Name of texture inside the dictionary to load the texture from, leave null for no texture in the marker.</param>
		/// <param name="drawOnEntity">if set to <c>true</c> draw on any <see cref="Entity"/> that intersects the marker.</param>
		public static void DrawMarker(MarkerType type, Vector3 pos, Vector3 dir, Vector3 rot, Vector3 scale, Color color,
			bool bobUpAndDown = false, bool faceCamera = false, bool rotateY = false, string textueDict = null, string textureName = null, bool drawOnEntity = false)
		{
			if (!string.IsNullOrEmpty(textueDict) && !string.IsNullOrEmpty(textureName))
			{
				API.DrawMarker((int)type, pos.X, pos.Y, pos.Z, dir.X, dir.Y, dir.Z, rot.X, rot.Y, rot.Z, scale.X,
					scale.Y, scale.Z, color.R, color.G, color.B, color.A, bobUpAndDown, faceCamera, 2, rotateY, textueDict,
					textureName, drawOnEntity);
			}
			else
			{
				API.DrawMarker((int)type, pos.X, pos.Y, pos.Z, dir.X, dir.Y, dir.Z, rot.X, rot.Y, rot.Z, scale.X,
					scale.Y, scale.Z, color.R, color.G, color.B, color.A, bobUpAndDown, faceCamera, 2, rotateY, null, null, drawOnEntity);
			}
		}

		/// <summary>
		/// Draws light around a region.
		/// </summary>
		/// <param name="position">The position to center the light around.</param>
		/// <param name="color">The color of the light.</param>
		/// <param name="range">How far the light should extend to.</param>
		/// <param name="intensity">The intensity: <c>0.0f</c> being no intensity, <c>1.0f</c> being full intensity.</param>
		public static void DrawLightWithRange(Vector3 position, Color color, float range, float intensity)
		{
			API.DrawLightWithRange(position.X, position.Y, position.Z, color.R, color.G, color.B, range, intensity);
		}

		public static void DrawSpotLight(Vector3 pos, Vector3 dir, Color color, float distance, float brightness,
			float roundness, float radius, float fadeout)
		{
			API.DrawSpotLight(pos.X, pos.Y, pos.Z, dir.X, dir.Y, dir.Z, color.R, color.G, color.B, distance,
				brightness, roundness, radius, fadeout);
		}

		public static void DrawSpotLightWithShadow(Vector3 pos, Vector3 dir, Color color, float distance, float brightness,
			float roundness, float radius, float fadeout)
		{
			API.DrawSpotLightWithShadow(pos.X, pos.Y, pos.Z, dir.X, dir.Y, dir.Z, color.R, color.G, color.B,
				distance, brightness, roundness, radius, fadeout, 0);
		}

		public static void DrawLine(Vector3 start, Vector3 end, Color color)
		{
			API.DrawLine(start.X, start.Y, start.Z, end.X, end.Y, end.Z, color.R, color.G, color.B, color.A);
		}

		public static void DrawPoly(Vector3 vertexA, Vector3 vertexB, Vector3 vertexC, Color color)
		{
			API.DrawPoly(vertexA.X, vertexA.Y, vertexA.Z, vertexB.X, vertexB.Y, vertexB.Z, vertexC.X, vertexC.Y,
				vertexC.Z, color.R, color.G, color.B, color.A);
		}

		/// <summary>
		/// Stops all particle effects in a range.
		/// </summary>
		/// <param name="pos">The position in the world to stop particle effects.</param>
		/// <param name="range">The maximum distance from the <paramref name="pos"/> to stop particle effects.</param>
		public static void RemoveAllParticleEffectsInRange(Vector3 pos, float range)
		{
			API.RemoveParticleFxInRange(pos.X, pos.Y, pos.Z, range);
		}
	}
}
