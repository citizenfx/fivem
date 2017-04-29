using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Runtime.Versioning;
using System.Collections.ObjectModel;
using CitizenFX.Core.Native;

namespace CitizenFX.Core
{

	public sealed class VehicleMod
	{
		#region Fields

		Vehicle _owner;

		private static readonly ReadOnlyDictionary<int, Tuple<string, string>> _hornNames = new ReadOnlyDictionary
			<int, Tuple<string, string>>(
			new Dictionary<int, Tuple<string, string>>
			{
				{-1,  new Tuple<string, string>("CMOD_HRN_0", "Stock Horn")},
				{0,  new Tuple<string, string>("CMOD_HRN_TRK","Truck Horn")},
				{1,  new Tuple<string, string>("CMOD_HRN_COP", "Cop Horn")},
				{2,  new Tuple<string, string>("CMOD_HRN_CLO", "Clown Horn")},
				{3,  new Tuple<string, string>("CMOD_HRN_MUS1", "Musical Horn 1")},
				{4,  new Tuple<string, string>("CMOD_HRN_MUS2", "Musical Horn 2")},
				{5,  new Tuple<string, string>("CMOD_HRN_MUS3", "Musical Horn 3")},
				{6,  new Tuple<string, string>("CMOD_HRN_MUS4", "Musical Horn 4")},
				{7,  new Tuple<string, string>("CMOD_HRN_MUS5", "Musical Horn 5")},
				{8,  new Tuple<string, string>("CMOD_HRN_SAD", "Sad Trombone")},
				{9,  new Tuple<string, string>("HORN_CLAS1", "Classical Horn 1")},
				{10,  new Tuple<string, string>("HORN_CLAS2", "Classical Horn 2")},
				{11,  new Tuple<string, string>("HORN_CLAS3", "Classical Horn 3")},
				{12,  new Tuple<string, string>("HORN_CLAS4", "Classical Horn 4")},
				{13,  new Tuple<string, string>("HORN_CLAS5", "Classical Horn 5")},
				{14,  new Tuple<string, string>("HORN_CLAS6", "Classical Horn 6")},
				{15,  new Tuple<string, string>("HORN_CLAS7", "Classical Horn 7")},
				{16,  new Tuple<string, string>("HORN_CNOTE_C0", "Scale Do")},
				{17,  new Tuple<string, string>("HORN_CNOTE_D0", "Scale Re")},
				{18,  new Tuple<string, string>("HORN_CNOTE_E0", "Scale Mi")},
				{19,  new Tuple<string, string>("HORN_CNOTE_F0", "Scale Fa")},
				{20,  new Tuple<string, string>("HORN_CNOTE_G0", "Scale Sol")},
				{21,  new Tuple<string, string>("HORN_CNOTE_A0", "Scale La")},
				{22,  new Tuple<string, string>("HORN_CNOTE_B0", "Scale Ti")},
				{23,  new Tuple<string, string>("HORN_CNOTE_C1", "Scale Do (High)")},
				{24,  new Tuple<string, string>("HORN_HIPS1", "Jazz Horn 1")},
				{25,  new Tuple<string, string>("HORN_HIPS2", "Jazz Horn 2")},
				{26,  new Tuple<string, string>("HORN_HIPS3", "Jazz Horn 3")},
				{27,  new Tuple<string, string>("HORN_HIPS4", "Jazz Horn Loop")},
				{28,  new Tuple<string, string>("HORN_INDI_1", "Star Spangled Banner 1")},
				{29,  new Tuple<string, string>("HORN_INDI_2", "Star Spangled Banner 2")},
				{30,  new Tuple<string, string>("HORN_INDI_3", "Star Spangled Banner 3")},
				{31,  new Tuple<string, string>("HORN_INDI_4", "Star Spangled Banner 4")},
				{32,  new Tuple<string, string>("HORN_LUXE2", "Classical Horn Loop 1")},
				{33,  new Tuple<string, string>("HORN_LUXE1", "Classical Horn 8")},
				{34,  new Tuple<string, string>("HORN_LUXE3", "Classical Horn Loop 2")},
				{35,  new Tuple<string, string>("HORN_LUXE2", "Classical Horn Loop 1")},
				{36,  new Tuple<string, string>("HORN_LUXE1", "Classical Horn 8")},
				{37,  new Tuple<string, string>("HORN_LUXE3", "Classical Horn Loop 2")},
				{38,  new Tuple<string, string>("HORN_HWEEN1", "Halloween Loop 1")},
				{39,  new Tuple<string, string>("HORN_HWEEN1", "Halloween Loop 1")},
				{40,  new Tuple<string, string>("HORN_HWEEN2", "Halloween Loop 2")},
				{41,  new Tuple<string, string>("HORN_HWEEN2", "Halloween Loop 2")},
				{42,  new Tuple<string, string>("HORN_LOWRDER1", "San Andreas Loop")},
				{43,  new Tuple<string, string>("HORN_LOWRDER1", "San Andreas Loop")},
				{44,  new Tuple<string, string>("HORN_LOWRDER2", "Liberty City Loop")},
				{45,  new Tuple<string, string>("HORN_LOWRDER2", "Liberty City Loop")},
				{46,  new Tuple<string, string>("HORN_XM15_1", "Festive Loop 1")},
				{47,  new Tuple<string, string>("HORN_XM15_2", "Festive Loop 2")},
				{48,  new Tuple<string, string>("HORN_XM15_3", "Festive Loop 3")}
			});
		#endregion

		internal VehicleMod(Vehicle owner, VehicleModType modType)
		{
			_owner = owner;
			ModType = modType;
		}

		public VehicleModType ModType { get; private set; }

		public int Index
		{
			get
			{
				return Function.Call<int>(Hash.GET_VEHICLE_MOD, _owner.Handle, ModType);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_MOD, _owner.Handle, ModType, value, Variation);
			}
		}

		public bool Variation
		{
			get
			{				  
				return Function.Call<bool>(Hash.GET_VEHICLE_MOD_VARIATION, _owner.Handle, ModType);
			}
			set
			{
				Function.Call(Hash.SET_VEHICLE_MOD, _owner.Handle, ModType, Index, value);
			}	 

		}

		public string LocalizedModTypeName
		{
			get
			{
				if (!Function.Call<bool>(Hash.HAS_THIS_ADDITIONAL_TEXT_LOADED, "mod_mnu", 10))
				{
					Function.Call(Hash.CLEAR_ADDITIONAL_TEXT, 10, true);
					Function.Call(Hash.REQUEST_ADDITIONAL_TEXT, "mod_mnu", 10);
				}
				string cur = "";
				switch (ModType)
				{
					case VehicleModType.Armor:
						cur = Game.GetGXTEntry("CMOD_MOD_ARM");
						break;
					case VehicleModType.Brakes:
						cur = Game.GetGXTEntry("CMOD_MOD_BRA");
						break;
					case VehicleModType.Engine:
						cur = Game.GetGXTEntry("CMOD_MOD_ENG");
						break;
					case VehicleModType.Suspension:
						cur = Game.GetGXTEntry("CMOD_MOD_SUS");
						break;
					case VehicleModType.Transmission:
						cur = Game.GetGXTEntry("CMOD_MOD_TRN");
						break;
					case VehicleModType.Horns:
						cur = Game.GetGXTEntry("CMOD_MOD_HRN");
						break;
					case VehicleModType.FrontWheel:
						if (!_owner.Model.IsBike && _owner.Model.IsBicycle)
						{
							cur = Game.GetGXTEntry("CMOD_MOD_WHEM");
							if (cur == "")
								return "Wheels";
						}
						else
							cur = Game.GetGXTEntry("CMOD_WHE0_0");
						break;
					case VehicleModType.RearWheel:
						cur = Game.GetGXTEntry("CMOD_WHE0_1");
						break;

					  //Bennys
					case VehicleModType.PlateHolder:
						cur = Game.GetGXTEntry("CMM_MOD_S0");
						break;
					case VehicleModType.VanityPlates:
						cur = Game.GetGXTEntry("CMM_MOD_S1");
						break;
					case VehicleModType.TrimDesign:
						if (_owner.Model == VehicleHash.SultanRS)
						{
							cur = Game.GetGXTEntry("CMM_MOD_S2b");
						}
						else
						{
							cur = Game.GetGXTEntry("CMM_MOD_S2");
						}
						break;
					case VehicleModType.Ornaments:
						cur = Game.GetGXTEntry("CMM_MOD_S3");
						break;
					case VehicleModType.Dashboard:
						cur = Game.GetGXTEntry("CMM_MOD_S4");
						break;
					case VehicleModType.DialDesign:
						cur = Game.GetGXTEntry("CMM_MOD_S5"); 
						break;
					case VehicleModType.DoorSpeakers:
						cur = Game.GetGXTEntry("CMM_MOD_S6");
						break;
					case VehicleModType.Seats:
						cur = Game.GetGXTEntry("CMM_MOD_S7");
						break;
					case VehicleModType.SteeringWheels:
						cur = Game.GetGXTEntry("CMM_MOD_S8");
						break;
					case VehicleModType.ColumnShifterLevers:
						cur = Game.GetGXTEntry("CMM_MOD_S9");
						break;
					case VehicleModType.Plaques:
						cur = Game.GetGXTEntry("CMM_MOD_S10");
						break;
					case VehicleModType.Speakers:
						cur = Game.GetGXTEntry("CMM_MOD_S11");
						break;
					case VehicleModType.Trunk:
						cur = Game.GetGXTEntry("CMM_MOD_S12");
						break;
					case VehicleModType.Hydraulics:
						cur = Game.GetGXTEntry("CMM_MOD_S13");
						break;
					case VehicleModType.EngineBlock:
						cur = Game.GetGXTEntry("CMM_MOD_S14");
						break;
					case VehicleModType.AirFilter:
						if (_owner.Model == VehicleHash.SultanRS)
						{
							cur = Game.GetGXTEntry("CMM_MOD_S15b");
						}
						else
						{
							cur = Game.GetGXTEntry("CMM_MOD_S15");
						}
						break;
					case VehicleModType.Struts:
						if (_owner.Model == VehicleHash.SultanRS || _owner.Model == VehicleHash.Banshee2)
						{
							cur = Game.GetGXTEntry("CMM_MOD_S16b");
						}
						else
							cur = Game.GetGXTEntry("CMM_MOD_S16");
						break;
					case VehicleModType.ArchCover:
						if (_owner.Model == VehicleHash.SultanRS)
						{
							cur = Game.GetGXTEntry("CMM_MOD_S17b");
						}
						else
						{
							cur = Game.GetGXTEntry("CMM_MOD_S17");
						}
						break;
					case VehicleModType.Aerials:
						if (_owner.Model == VehicleHash.SultanRS)
						{
							cur = Game.GetGXTEntry("CMM_MOD_S18b");
						}
						else if (_owner.Model == VehicleHash.BType3)
						{
							cur = Game.GetGXTEntry("CMM_MOD_S18c");
						}
						else
						{
							cur = Game.GetGXTEntry("CMM_MOD_S18");
						}
						break;
					case VehicleModType.Trim:
						if (_owner.Model == VehicleHash.SultanRS)
						{
							cur = Game.GetGXTEntry("CMM_MOD_S19b");
						}
						else if (_owner.Model == VehicleHash.BType3)
						{
							cur = Game.GetGXTEntry("CMM_MOD_S19c");
						}
						else if (_owner.Model == VehicleHash.Virgo2)
						{
							cur = Game.GetGXTEntry("CMM_MOD_S19d");
						}
						else
						{
							cur = Game.GetGXTEntry("CMM_MOD_S19");
						}
						break;
					case VehicleModType.Tank:
						if (_owner.Model == VehicleHash.SlamVan3)
						{
							cur = Game.GetGXTEntry("CMM_MOD_S27");
						}
						else
						{
							cur = Game.GetGXTEntry("CMM_MOD_S20");
						}
						break;

					case VehicleModType.Windows:
						if (_owner.Model == VehicleHash.BType3)
						{
							cur = Game.GetGXTEntry("CMM_MOD_S21b");
						}
						else
						{
							cur = Game.GetGXTEntry("CMM_MOD_S21");
						}
						break;
					case (VehicleModType)47:
						if (_owner.Model == VehicleHash.SlamVan3)
						{
							cur = Game.GetGXTEntry("SLVAN3_RDOOR");
						}
						else
						{
							cur = Game.GetGXTEntry("CMM_MOD_S22");
						}
						break;
					case VehicleModType.Livery:
						cur = Game.GetGXTEntry("CMM_MOD_S23");
						break;

					default:
						cur = Function.Call<string>(Hash.GET_MOD_SLOT_NAME, _owner.Handle, ModType);
						if (Game.DoesGXTEntryExist(cur))
						{
							cur = Game.GetGXTEntry(cur);
						}
						break;
				}
				if (cur == "")
				{
					cur = ModType.ToString();  //would only happen if the text isnt loaded
				}
				return cur;

			}
		}
		public string LocalizedModName
		{
			get { return GetLocalizedModName(Index); }
		}

		public string GetLocalizedModName(int index)
		{
			//this still needs a little more work, but its better than what it used to be
			if (ModCount == 0)
				return "";
			if (index < -1 || index >= ModCount)
				return "";
			if (!Function.Call<bool>(Hash.HAS_THIS_ADDITIONAL_TEXT_LOADED, "mod_mnu", 10))
			{
				Function.Call(Hash.CLEAR_ADDITIONAL_TEXT, 10, true);
				Function.Call(Hash.REQUEST_ADDITIONAL_TEXT, "mod_mnu", 10);
			}
			string cur;
			if (ModType == VehicleModType.Horns)
			{
				if (_hornNames.ContainsKey(index))
				{
					if (Game.DoesGXTEntryExist(_hornNames[index].Item1))
					{
						return Game.GetGXTEntry(_hornNames[index].Item1);
					}
					return _hornNames[index].Item2;
				}
				return "";
			}
			if (ModType == VehicleModType.FrontWheel || ModType == VehicleModType.RearWheel)
			{
				if (index == -1)
				{
					if (!_owner.Model.IsBike && _owner.Model.IsBicycle)
					{
						return Game.GetGXTEntry("CMOD_WHE_0");
					}
					else
					{
						return Game.GetGXTEntry("CMOD_WHE_B_0");
					}
				}
				if (index >= ModCount / 2)
				{
					return Game.GetGXTEntry("CHROME") + " " +
						   Game.GetGXTEntry(Function.Call<ulong>(Hash.GET_MOD_TEXT_LABEL, _owner.Handle, ModType, index));
				}
				else
				{
					return Game.GetGXTEntry(Function.Call<ulong>(Hash.GET_MOD_TEXT_LABEL, _owner.Handle, ModType, index));
				}
			}

			switch (ModType)
			{
				case VehicleModType.Armor:
					return Game.GetGXTEntry("CMOD_ARM_" + (index + 1).ToString());
				case VehicleModType.Brakes:
					return Game.GetGXTEntry("CMOD_BRA_" + (index + 1).ToString());
				case VehicleModType.Engine:
					if (index == -1)
					{
						//Engine doesn't list anything in LSC for no parts, but there is a setting with no part. so just use armours none
						return Game.GetGXTEntry("CMOD_ARM_0");
					}
					return Game.GetGXTEntry("CMOD_ENG_" + (index + 2).ToString());
				case VehicleModType.Suspension:
					return Game.GetGXTEntry("CMOD_SUS_" + (index + 1).ToString());
				case VehicleModType.Transmission:
					return Game.GetGXTEntry("CMOD_GBX_" + (index + 1).ToString());
			}
			if (index > -1)
			{
				cur = Function.Call<string>(Hash.GET_MOD_TEXT_LABEL, _owner.Handle, ModType, index);
				if (Game.DoesGXTEntryExist(cur))
				{
					cur = Game.GetGXTEntry(cur);
					if (cur == "" || cur == "NULL")
					{
						return LocalizedModTypeName + " " + (index + 1).ToString();
					}
					return cur;
				}
				return LocalizedModTypeName + " " + (index + 1).ToString();
			}
			else
			{
				switch (ModType)
				{
					case VehicleModType.AirFilter:
						if (_owner.Model == VehicleHash.Tornado)
						{
						}
						break;
					case VehicleModType.Struts:
						switch ((VehicleHash) _owner.Model)
						{
							case VehicleHash.Banshee:
							case VehicleHash.Banshee2:
							case VehicleHash.SultanRS:
								return Game.GetGXTEntry("CMOD_COL5_41");
						}
						break;

				}
				return Game.GetGXTEntry("CMOD_DEF_0");
			}			
		}
		public int ModCount
		{
			get
			{
				return Function.Call<int>(Hash.GET_NUM_VEHICLE_MODS, _owner.Handle, ModType);
			}
		}
		public Vehicle Vehicle
		{
			get { return _owner; }
		}

		public void Remove()
		{
			Function.Call(Hash.REMOVE_VEHICLE_MOD, _owner.Handle, ModType);
		}
	}
}
