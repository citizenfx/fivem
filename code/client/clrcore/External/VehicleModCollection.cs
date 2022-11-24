using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;

#if MONO_V2
using CitizenFX.Core;
using API = CitizenFX.FiveM.Native.Natives;
using TaskBool = CitizenFX.Core.Coroutine<bool>;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;
using System.Drawing;
using TaskBool = System.Threading.Tasks.Task<bool>;

namespace CitizenFX.Core
#endif
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
			return API.GetNumVehicleMods(_owner.Handle, (int)type) > 0;
		}

		public VehicleMod[] GetAllMods()
		{
			return
				Enum.GetValues(typeof(VehicleModType)).Cast<VehicleModType>().Where(HasVehicleMod).Select(modType => this[modType]).ToArray();
		}

		public VehicleWheelType WheelType
		{
			get { return (VehicleWheelType)API.GetVehicleWheelType(_owner.Handle); }
			set { API.SetVehicleWheelType(_owner.Handle, (int)value); }
		}

		public VehicleWheelType[] AllowedWheelTypes
		{
			get
			{
				if (_owner.Model.IsBicycle || _owner.Model.IsBike)
				{
					return new VehicleWheelType[] { VehicleWheelType.BikeWheels };
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
							res.AddRange(new VehicleWheelType[] { VehicleWheelType.BennysOriginals, VehicleWheelType.BennysBespoke });
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
			if (!API.HasThisAdditionalTextLoaded("mod_mnu", 10))
			{
				API.ClearAdditionalText(10, true);
				API.RequestAdditionalText("mod_mnu", 10);
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
			API.SetVehicleModKit(_owner.Handle, 0);
		}

		public async TaskBool RequestAdditionTextFile(int timeout = 1000)
		{
			if (!API.HasThisAdditionalTextLoaded("mod_mnu", 10))
			{
				API.ClearAdditionalText(10, true);
				API.RequestAdditionalText("mod_mnu", 10);

				int end = Game.GameTime + timeout;
				{
					while (Game.GameTime < end)
					{
						if (API.HasThisAdditionalTextLoaded("mod_mnu", 10))
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

				return API.GetVehicleLivery(_owner.Handle);
			}
			set
			{
				if (this[VehicleModType.Livery].ModCount > 0)
				{
					this[VehicleModType.Livery].Index = value;
				}
				else
				{
					API.SetVehicleLivery(_owner.Handle, value);
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

				return API.GetVehicleLiveryCount(_owner.Handle);
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
			return Game.GetGXTEntry(API.GetLiveryName(_owner.Handle, index));
		}

		public VehicleWindowTint WindowTint
		{
			get
			{
				return (VehicleWindowTint)API.GetVehicleWindowTint(_owner.Handle);
			}
			set
			{
				API.SetVehicleWindowTint(_owner.Handle, (int)value);
			}
		}

		public VehicleColor PrimaryColor
		{
			get
			{
				int color1 = 0, color2 = 0;
				API.GetVehicleColours(_owner.Handle, ref color1, ref color2);

				return (VehicleColor)color1;
			}
			set
			{
				API.SetVehicleColours(_owner.Handle, (int)value, (int)SecondaryColor);
			}
		}
		public VehicleColor SecondaryColor
		{
			get
			{
				int color1 = 0, color2 = 0;
				API.GetVehicleColours(_owner.Handle, ref color1, ref color2);

				return (VehicleColor)color2;
			}
			set
			{
				API.SetVehicleColours(_owner.Handle, (int)PrimaryColor, (int)value);
			}
		}
		public VehicleColor RimColor
		{
			get
			{
				int color1 = 0, color2 = 0;
				API.GetVehicleExtraColours(_owner.Handle, ref color1, ref color2);

				return (VehicleColor)color2;
			}
			set
			{
				API.SetVehicleExtraColours(_owner.Handle, (int)PearlescentColor, (int)value);
			}
		}
		public VehicleColor PearlescentColor
		{
			get
			{
				int color1 = 0, color2 = 0;
				API.GetVehicleExtraColours(_owner.Handle, ref color1, ref color2);

				return (VehicleColor)color1;
			}
			set
			{
				API.SetVehicleExtraColours(_owner.Handle, (int)value, (int)RimColor);
			}
		}
		public VehicleColor TrimColor
		{
			get
			{
				int color = 0;
				API.GetVehicleInteriorColour(_owner.Handle, ref color);

				return (VehicleColor)color;
			}
			set
			{
				API.SetVehicleInteriorColour(_owner.Handle, (int)value);
			}
		}
		public VehicleColor DashboardColor
		{
			get
			{
				int color = 0;
				API.GetVehicleDashboardColour(_owner.Handle, ref color);

				return (VehicleColor)color;
			}
			set
			{
				API.SetVehicleDashboardColour(_owner.Handle, (int)value);
			}
		}
		public int ColorCombination
		{
			get
			{
				return API.GetVehicleColourCombination(_owner.Handle);
			}
			set
			{
				API.SetVehicleColourCombination(_owner.Handle, value);
			}
		}
		public int ColorCombinationCount
		{
			get
			{
				return API.GetNumberOfVehicleColours(_owner.Handle);
			}
		}
		public Color TireSmokeColor
		{
			get
			{
				int red = 0, green = 0, blue = 0;
				API.GetVehicleTyreSmokeColor(_owner.Handle, ref red, ref green, ref blue);

				return Color.FromArgb(red, green, blue);
			}
			set
			{
				API.SetVehicleTyreSmokeColor(_owner.Handle, value.R, value.G, value.B);
			}
		}
		public Color NeonLightsColor
		{
			get
			{
				int red = 0, green = 0, blue = 0;
				API.GetVehicleNeonLightsColour(_owner.Handle, ref red, ref green, ref blue);

				return Color.FromArgb(red, green, blue);
			}
			set
			{
				API.SetVehicleNeonLightsColour(_owner.Handle, value.R, value.G, value.B);
			}
		}
		public bool IsNeonLightsOn(VehicleNeonLight light)
		{
			return API.IsVehicleNeonLightEnabled(_owner.Handle, (int)light);
		}
		public void SetNeonLightsOn(VehicleNeonLight light, bool on)
		{
			API.SetVehicleNeonLightEnabled(_owner.Handle, (int)light, on);
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
			get
			{
				int red = 0, green = 0, blue = 0;
				API.GetVehicleCustomPrimaryColour(_owner.Handle, ref red, ref green, ref blue);

				return Color.FromArgb(red, green, blue);
			}
			set
			{
				API.SetVehicleCustomPrimaryColour(_owner.Handle, value.R, value.G, value.B);
			}
		}
		public Color CustomSecondaryColor
		{
			get
			{
				int red = 0, green = 0, blue = 0;
				API.GetVehicleCustomSecondaryColour(_owner.Handle, ref red, ref green, ref blue);

				return Color.FromArgb(red, green, blue);
			}
			set
			{
				API.SetVehicleCustomSecondaryColour(_owner.Handle, value.R, value.G, value.B);
			}
		}
		public bool IsPrimaryColorCustom
		{
			get
			{
				return API.GetIsVehiclePrimaryColourCustom(_owner.Handle);
			}
		}
		public bool IsSecondaryColorCustom
		{
			get
			{
				return API.GetIsVehicleSecondaryColourCustom(_owner.Handle);
			}
		}
		public void ClearCustomPrimaryColor()
		{
			API.ClearVehicleCustomPrimaryColour(_owner.Handle);
		}
		public void ClearCustomSecondaryColor()
		{
			API.ClearVehicleCustomSecondaryColour(_owner.Handle);
		}

		public LicensePlateStyle LicensePlateStyle
		{
			get
			{
				return (LicensePlateStyle)API.GetVehicleNumberPlateTextIndex(_owner.Handle);
			}
			set
			{
				API.SetVehicleNumberPlateTextIndex(_owner.Handle, (int)value);
			}
		}
		public LicensePlateType LicensePlateType
		{
			get
			{
				return (LicensePlateType)API.GetVehiclePlateType(_owner.Handle);
			}
		}
		public string LicensePlate
		{
			get
			{
				return API.GetVehicleNumberPlateText(_owner.Handle);
			}
			set
			{
				API.SetVehicleNumberPlateText(_owner.Handle, value);
			}
		}
	}
}
