using CitizenFX.Core;
using CitizenFX.Core.Native;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Drawing;
using System.Linq;
using System.Security;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
	public enum VehicleModType
	{
		Spoilers,
		FrontBumper,
		RearBumper,
		SideSkirt,
		Exhaust,
		Frame,
		Grille,
		Hood,
		Fender,
		RightFender,
		Roof,
		Engine,
		Brakes,
		Transmission,
		Horns,
		Suspension,
		Armor,
		FrontWheel = 23,
		RearWheel,
		PlateHolder,
		VanityPlates,
		TrimDesign,
		Ornaments,
		Dashboard,
		DialDesign,
		DoorSpeakers,
		Seats,
		SteeringWheels,
		ColumnShifterLevers,
		Plaques,
		Speakers,
		Trunk,
		Hydraulics,
		EngineBlock,
		AirFilter,
		Struts,
		ArchCover,
		Aerials,
		Trim,
		Tank,
		Windows,
		Livery = 48
	}

	public enum VehicleToggleModType
	{
		Turbo = 18,
		TireSmoke = 20,
		XenonHeadlights = 22
	}

	public enum VehicleWheelType
	{
		Sport,
		Muscle,
		Lowrider,
		SUV,
		Offroad,
		Tuner,
		BikeWheels,
		HighEnd,
		BennysOriginals,
		BennysBespoke
	}

	public sealed class VehicleModCollection
	{
		#region Fields

		Vehicle _owner;
		readonly Dictionary<VehicleModType, VehicleMod> _vehicleMods = new Dictionary<VehicleModType, VehicleMod>();
		readonly Dictionary<VehicleToggleModType, VehicleToggleMod> _vehicleToggleMods = new Dictionary<VehicleToggleModType, VehicleToggleMod>();

		private static readonly ReadOnlyDictionary<VehicleWheelType, Tuple<string, string>> _wheelNames = new ReadOnlyDictionary
			<VehicleWheelType, Tuple<string, string>>(
			new Dictionary<VehicleWheelType, Tuple<string, string>>
			{
				{VehicleWheelType.BikeWheels, new Tuple<string, string>("CMOD_WHE1_0", "Bike")},
				{VehicleWheelType.HighEnd, new Tuple<string, string>("CMOD_WHE1_1", "High End")},
				{VehicleWheelType.Lowrider, new Tuple<string, string>("CMOD_WHE1_2", "Lowrider")},
				{VehicleWheelType.Muscle, new Tuple<string, string>("CMOD_WHE1_3", "Muscle")},
				{VehicleWheelType.Offroad, new Tuple<string, string>("CMOD_WHE1_4", "Offroad")},
				{VehicleWheelType.Sport, new Tuple<string, string>("CMOD_WHE1_5", "Sport")},
				{VehicleWheelType.SUV, new Tuple<string, string>("CMOD_WHE1_6", "SUV")},
				{VehicleWheelType.Tuner, new Tuple<string, string>("CMOD_WHE1_7", "Tuner")},
				{VehicleWheelType.BennysOriginals, new Tuple<string, string>("CMOD_WHE1_8", "Benny's Originals")},
				{VehicleWheelType.BennysBespoke, new Tuple<string, string>("CMOD_WHE1_9", "Benny's Bespoke")}
			});
		#endregion

		internal VehicleModCollection(Vehicle owner)
		{
			_owner = owner;
		}


		public VehicleMod this[VehicleModType modType]
		{
			get
			{
				VehicleMod vehicleMod = null;

				if (!_vehicleMods.TryGetValue(modType, out vehicleMod))
				{
					vehicleMod = new VehicleMod(_owner, modType);
					_vehicleMods.Add(modType, vehicleMod);
				}

				return vehicleMod;
			}
		}

		public VehicleToggleMod this[VehicleToggleModType modType]
		{
			get
			{
				VehicleToggleMod vehicleToggleMod = null;

				if (!_vehicleToggleMods.TryGetValue(modType, out vehicleToggleMod))
				{
					vehicleToggleMod = new VehicleToggleMod(_owner, modType);
					_vehicleToggleMods.Add(modType, vehicleToggleMod);
				}

				return vehicleToggleMod;
			}
		}

		public bool HasVehicleMod(VehicleModType type)
		{
			return Function.Call<int>(Hash.GET_NUM_VEHICLE_MODS, _owner.Handle, type) > 0;
		}

		public VehicleMod[] GetAllMods()
		{
			return
				Enum.GetValues(typeof(VehicleModType)).Cast<VehicleModType>().Where(HasVehicleMod).Select(modType => this[modType]).ToArray();
		}

		public VehicleWheelType WheelType
		{
			get { return Function.Call<VehicleWheelType>(Hash.GET_VEHICLE_WHEEL_TYPE, _owner.Handle); }
			set { Function.Call(Hash.SET_VEHICLE_WHEEL_TYPE, _owner.Handle, value); }
		}

		public VehicleWheelType[] AllowedWheelTypes
		{
			get
			{
				if (_owner.Model.IsBicycle || _owner.Model.IsBike)
				{
					return new VehicleWheelType[] {VehicleWheelType.BikeWheels};
				}
				if (_owner.Model.IsCar)
				{
					var res = new List<VehicleWheelType>()
					{
						VehicleWheelType.Sport,
						VehicleWheelType.Muscle,
						VehicleWheelType.Lowrider,
						VehicleWheelType.SUV,
						VehicleWheelType.Offroad,
						VehicleWheelType.Tuner,
						VehicleWheelType.HighEnd
					};
					switch ((VehicleHash)_owner.Model)
					{
						case VehicleHash.Faction2:
						case VehicleHash.Buccaneer2:
						case VehicleHash.Chino2:
						case VehicleHash.Moonbeam2:
						case VehicleHash.Primo2:
						case VehicleHash.Voodoo2:
						case VehicleHash.SabreGT2:
						case VehicleHash.Tornado5:
						case VehicleHash.Virgo2:
						case VehicleHash.Minivan2:
						case VehicleHash.SlamVan3:
						case VehicleHash.Faction3:
							res.AddRange(new VehicleWheelType[] {VehicleWheelType.BennysOriginals, VehicleWheelType.BennysBespoke});
							break;
						case VehicleHash.SultanRS:
						case VehicleHash.Banshee2:
							res.Add(VehicleWheelType.BennysOriginals);
							break;
					}
					return res.ToArray();
				}
				return new VehicleWheelType[0];
			}
		}

		public string LocalizedWheelTypeName
		{
			get { return GetLocalizedWheelTypeName(WheelType); }
		}

		public string GetLocalizedWheelTypeName(VehicleWheelType wheelType)
		{
			if (!Function.Call<bool>(Hash.HAS_THIS_ADDITIONAL_TEXT_LOADED, "mod_mnu", 10))
			{
				Function.Call(Hash.CLEAR_ADDITIONAL_TEXT, 10, true);
				Function.Call(Hash.REQUEST_ADDITIONAL_TEXT, "mod_mnu", 10);
			}
			if (_wheelNames.ContainsKey(wheelType))
			{
				if (Game.DoesGXTEntryExist(_wheelNames[wheelType].Item1))
				{
					return Game.GetGXTEntry(_wheelNames[wheelType].Item1);
				}
				return _wheelNames[wheelType].Item2;
			}
			throw new ArgumentException("Wheel Type is undefined", "wheelType");
		}

		public void InstallModKit()
		{
			Function.Call(Hash.SET_VEHICLE_MOD_KIT, _owner.Handle, 0);
		}

		public async Task<bool> RequestAdditionTextFile(int timeout = 1000)
		{
			if (!Function.Call<bool>(Hash.HAS_THIS_ADDITIONAL_TEXT_LOADED, "mod_mnu", 10))
			{
				Function.Call(Hash.CLEAR_ADDITIONAL_TEXT, 10, true);
				Function.Call(Hash.REQUEST_ADDITIONAL_TEXT, "mod_mnu", 10);
				int end = Game.GameTime + timeout;
				{
					while (Game.GameTime < end)
					{
						if (Function.Call<bool>(Hash.HAS_THIS_ADDITIONAL_TEXT_LOADED, "mod_mnu", 10))
							return true;
                        await BaseScript.Delay(0);
                    }
					return false;
				}
			}
			return true;
		}

		public int Livery
		{
			get
			{
				int modCount = this[VehicleModType.Livery].ModCount;

				if (modCount > 0)
				{
					return modCount;
				}

				return Function.Call<int>(Hash.GET_VEHICLE_LIVERY, _owner.Handle);
			}
			set
			{
				if (this[VehicleModType.Livery].ModCount > 0)
				{
					this[VehicleModType.Livery].Index = value;
				}
				else
				{
					Function.Call(Hash.SET_VEHICLE_LIVERY, _owner.Handle, value);
				}
			}
		}
		public int LiveryCount
		{
			get
			{
				int modCount = this[VehicleModType.Livery].ModCount;

				if (modCount > 0)
				{
					return modCount;
				}

				return Function.Call<int>(Hash.GET_VEHICLE_LIVERY_COUNT, _owner.Handle);
			}
		}

		public string LocalizedLiveryName
		{
			get
			{
				return GetLocalizedLiveryName(Livery);
			}
		}

		public string GetLocalizedLiveryName(int index)
		{
			int modCount = this[VehicleModType.Livery].ModCount;

			if (modCount > 0)
			{
				return this[VehicleModType.Livery].GetLocalizedModName(index);
			}
			return Game.GetGXTEntry(Function.Call<ulong>(Hash.GET_LIVERY_NAME, _owner.Handle, index));
		}

		public VehicleWindowTint WindowTint
		{
			get
			{
				return Function.Call<VehicleWindowTint>(Hash.GET_VEHICLE_WINDOW_TINT,_owner.Handle);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_WINDOW_TINT,_owner.Handle, value);
			}
		}

		public VehicleColor PrimaryColor
		{
            [SecuritySafeCritical]
            get
			{
				int color1, color2;
				unsafe
				{
					Function.Call(Hash.GET_VEHICLE_COLOURS,_owner.Handle, &color1, &color2);
				}

				return (VehicleColor)color1;
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_COLOURS,_owner.Handle, value, SecondaryColor);
			}
		}
		public VehicleColor SecondaryColor
		{
            [SecuritySafeCritical]
            get
			{
				int color1, color2;
				unsafe
				{
					Function.Call(Hash.GET_VEHICLE_COLOURS, _owner.Handle, &color1, &color2);
				}

				return (VehicleColor)color2;
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_COLOURS,_owner.Handle, PrimaryColor, value);
			}
		}
		public VehicleColor RimColor
		{
            [SecuritySafeCritical]
            get
			{
				int color1, color2;
				unsafe
				{
					Function.Call(Hash.GET_VEHICLE_EXTRA_COLOURS,_owner.Handle, &color1, &color2);
				}

				return (VehicleColor)color2;
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_EXTRA_COLOURS,_owner.Handle, PearlescentColor, value);
			}
		}
		public VehicleColor PearlescentColor
		{
            [SecuritySafeCritical]
            get
			{
				int color1, color2;
				unsafe
				{
					Function.Call(Hash.GET_VEHICLE_EXTRA_COLOURS,_owner.Handle, &color1, &color2);
				}

				return (VehicleColor)color1;
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_EXTRA_COLOURS,_owner.Handle, value, RimColor);
			}
		}
		public VehicleColor TrimColor
		{
            [SecuritySafeCritical]
            get
			{
				int color;
				unsafe
				{
					Function.Call((Hash)9012939617897488694uL,_owner.Handle, &color);
				}

				return (VehicleColor)color;
			}
			set
			{
				Function.Call((Hash)17585947422526242585uL,_owner.Handle, value);
			}
		}
		public VehicleColor DashboardColor
		{
			get
			{
				int color;
				unsafe
				{
					Function.Call((Hash)13214509638265019391uL,_owner.Handle, &color);
				}

				return (VehicleColor)color;
			}
			set
			{
				Function.Call((Hash)6956317558672667244uL,_owner.Handle, value);
			}
		}
		public int ColorCombination
		{
			get
			{
				return Function.Call<int>(Hash.GET_VEHICLE_COLOUR_COMBINATION,_owner.Handle);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_COLOUR_COMBINATION,_owner.Handle, value);
			}
		}
		public int ColorCombinationCount
		{
			get
			{
				return Function.Call<int>(Hash.GET_NUMBER_OF_VEHICLE_COLOURS,_owner.Handle);
			}
		}
		public Color TireSmokeColor
		{
            [SecuritySafeCritical]
            get
			{
				int red, green, blue;
				unsafe
				{
					Function.Call(Hash.GET_VEHICLE_TYRE_SMOKE_COLOR,_owner.Handle, &red, &green, &blue);
				}

				return Color.FromArgb(red, green, blue);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_TYRE_SMOKE_COLOR,_owner.Handle, value.R, value.G, value.B);
			}
		}
		public Color NeonLightsColor
		{
            [SecuritySafeCritical]
            get
			{
				int red, green, blue;
				unsafe
				{
					Function.Call(Hash._GET_VEHICLE_NEON_LIGHTS_COLOUR, _owner.Handle, &red, &green, &blue);
				}

				return Color.FromArgb(red, green, blue);
			}
			set
			{
				Function.Call(Hash._SET_VEHICLE_NEON_LIGHTS_COLOUR,_owner.Handle, value.R, value.G, value.B);
			}
		}
		public bool IsNeonLightsOn(VehicleNeonLight light)
		{
			return Function.Call<bool>(Hash._IS_VEHICLE_NEON_LIGHT_ENABLED, _owner.Handle, light);
		}
		public void SetNeonLightsOn(VehicleNeonLight light, bool on)
		{
			Function.Call(Hash._SET_VEHICLE_NEON_LIGHT_ENABLED, _owner.Handle, light, on);
		}
		public bool HasNeonLights
		{
			get
			{ return Enum.GetValues(typeof(VehicleNeonLight)).Cast<VehicleNeonLight>().Any(HasNeonLight); }
		}
		public bool HasNeonLight(VehicleNeonLight neonLight)
		{
			switch (neonLight)
			{
				case VehicleNeonLight.Left:
					return _owner.Bones.HasBone("neon_l");
				case VehicleNeonLight.Right:
					return _owner.Bones.HasBone("neon_r");
				case VehicleNeonLight.Front:
					return _owner.Bones.HasBone("neon_f");
				case VehicleNeonLight.Back:
					return _owner.Bones.HasBone("neon_b");
				default:
					return false;
			}
		}

		public Color CustomPrimaryColor
		{
            [SecuritySafeCritical]
            get
			{
				int red, green, blue;
				unsafe
				{
					Function.Call(Hash.GET_VEHICLE_CUSTOM_PRIMARY_COLOUR, _owner.Handle, &red, &green, &blue);
				}

				return Color.FromArgb(red, green, blue);

			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_CUSTOM_PRIMARY_COLOUR,_owner.Handle, value.R, value.G, value.B);
			}
		}
		public Color CustomSecondaryColor
		{
            [SecuritySafeCritical]
			get
			{
				int red, green, blue;
				unsafe
				{
					Function.Call(Hash.GET_VEHICLE_CUSTOM_SECONDARY_COLOUR, _owner.Handle, &red, &green, &blue);
				}

				return Color.FromArgb(red, green, blue);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_CUSTOM_SECONDARY_COLOUR,_owner.Handle, value.R, value.G, value.B);
			}
		}
		public bool IsPrimaryColorCustom
		{
			get
			{
				return Function.Call<bool>(Hash.GET_IS_VEHICLE_PRIMARY_COLOUR_CUSTOM,_owner.Handle);
			}
		}
		public bool IsSecondaryColorCustom
		{
			get
			{
				return Function.Call<bool>(Hash.GET_IS_VEHICLE_SECONDARY_COLOUR_CUSTOM,_owner.Handle);
			}
		}
		public void ClearCustomPrimaryColor()
		{
			Function.Call(Hash.CLEAR_VEHICLE_CUSTOM_PRIMARY_COLOUR,_owner.Handle);
		}
		public void ClearCustomSecondaryColor()
		{
			Function.Call(Hash.CLEAR_VEHICLE_CUSTOM_SECONDARY_COLOUR,_owner.Handle);
		}

		public LicensePlateStyle LicensePlateStyle
		{
			get
			{
				return Function.Call<LicensePlateStyle>(Hash.GET_VEHICLE_NUMBER_PLATE_TEXT_INDEX,_owner.Handle);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX,_owner.Handle, value);
			}
		}
		public LicensePlateType LicensePlateType
		{
			get
			{
				return Function.Call<LicensePlateType>(Hash.GET_VEHICLE_PLATE_TYPE,_owner.Handle);
			}
		}
		public string LicensePlate
		{
			get
			{
				return Function.Call<string>(Hash.GET_VEHICLE_NUMBER_PLATE_TEXT,_owner.Handle);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_NUMBER_PLATE_TEXT,_owner.Handle, value);
			}
		}
	}
}
