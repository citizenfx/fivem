using System.Collections.Generic;
using CitizenFX.Core.Native;

namespace CitizenFX.Core
{
	public enum PedComponents
	{
		Face,
		Head,
		Hair,
		Torso,
		Legs,
		Hands,
		Shoes,
		Special1,
		Special2,
		Special3,
		Textures,
		Torso2
	}

	public enum PedProps
	{
		Hats,
		Glasses,
		EarPieces,
		Unknown3,
		Unknown4,
		Unknown5,
		Watches,
		Wristbands,
		Unknown8,
		Unknown9,
	}
	public class Style
	{
		#region Fields
		Ped _ped;
		Dictionary<PedComponents, PedComponent> _pedComponents = new Dictionary<PedComponents, PedComponent>();
		Dictionary<PedProps, PedProp> _pedProps = new Dictionary<PedProps, PedProp>();
		#endregion

		internal Style(Ped ped)
		{
			_ped = ped;
		}

		public PedComponent this[PedComponents componentId]
		{
			get
			{
				PedComponent variation = null;
				if (!_pedComponents.TryGetValue(componentId, out variation))
				{
					variation = new PedComponent(_ped, componentId);
					_pedComponents.Add(componentId, variation);
				}
				return variation;
			}
		}

		public PedProp this[PedProps propId]
		{
			get
			{
				PedProp prop = null;
				if (!_pedProps.TryGetValue(propId, out prop))
				{
					prop = new PedProp(_ped, propId);
					_pedProps.Add(propId, prop);
				}
				return prop;
			}
		}

		public void RandomizeOutfit()
		{
			switch ((PedHash)_ped.Model)
			{
				case PedHash.Michael:
				case PedHash.Franklin:
				case PedHash.Trevor:
				case PedHash.FreemodeMale01:
				case PedHash.FreemodeFemale01:
					return; //these models freeze when randomized
			}
			API.SetPedRandomComponentVariation(_ped.Handle, false);
		}
		public void SetDefaultClothes()
		{
			API.SetPedDefaultComponentVariation(_ped.Handle);
		}

		public void RandomizeProps()
		{
			API.SetPedRandomProps(_ped.Handle);
		}
	}

	public interface IPedVariation
	{
		int Index { set; }
		bool SetVariation(int index, int textureIndex = 0);

	}
	public class PedComponent : IPedVariation
	{
		#region Fields
		readonly Ped _ped;
		readonly PedComponents _componentdId;
		#endregion

		internal PedComponent(Ped ped, PedComponents componentId)
		{
			_ped = ped;
			_componentdId = componentId;
		}


		public int Index
		{
			set { SetVariation(value); }
		}

		public bool SetVariation(int index, int textureIndex = 0)
		{
			API.SetPedComponentVariation(_ped.Handle, (int)_componentdId, index, textureIndex, 0);
			return true;
		}

		public override string ToString()
		{
			return _componentdId.ToString();
		}
	}

	public class PedProp : IPedVariation
	{
		#region Fields
		readonly Ped _ped;
		readonly PedProps _propId;
		#endregion

		internal PedProp(Ped ped, PedProps propId)
		{
			_ped = ped;
			_propId = propId;
		}

		public int Index
		{
			set { SetVariation(value); }
		}


		public bool SetVariation(int index, int textureIndex = 0)
		{
			if (index == 0)
			{
				API.ClearPedProp(_ped.Handle, (int)_propId);
				return true;
			}

			API.SetPedPropIndex(_ped.Handle, (int)_propId, index - 1, textureIndex, true);
			return true;
		}
		public override string ToString()
		{
			return _propId.ToString();
		}
	}
}