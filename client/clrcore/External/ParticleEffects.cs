using System;
using System.Drawing;
using System.Security.Policy;
using CitizenFX.Core;
using CitizenFX.Core.Native;
using Hash = CitizenFX.Core.Native.Hash;
using System.Threading.Tasks;
using CitizenFX.Core;

namespace CitizenFX.Core
{
	[Flags]
	public enum InvertAxis
	{
		None = 0,
		X = 1,
		Y = 2,
		Z = 4
	}

	public class ParticleEffectsAsset
	{
		#region Fields
		private readonly string _assetName;
		#endregion
		/// <summary>
		/// Creates a class used for loading <see cref="ParticleEffectsAsset"/>s than can be used to start <see cref="ParticleEffect"/>s from inside the Asset
		/// </summary>
		/// <param name="assetName">The name of the asset file which contains all the <see cref="ParticleEffect"/>s you are wanting to start</param>
		/// <remarks>The files have the extension *.ypt in OpenIV, use the file name withouth the extension for the <paramref name="assetName"/></remarks>
		public ParticleEffectsAsset(string assetName)
		{
			_assetName = assetName;
		}
		/// <summary>
		/// Gets the name of the this <see cref="ParticleEffectsAsset"/> file
		/// </summary>
		public string AssetName
		{
			get { return _assetName; }
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="ParticleEffectsAsset"/> is Loaded
		/// </summary>
		/// <remarks>Use <see cref="Request()"/> or <see cref="Request(int)"/> to load the asset</remarks>
		public bool IsLoaded
		{
			get { return Function.Call<bool>(Hash.HAS_NAMED_PTFX_ASSET_LOADED, _assetName); }
		}

		/// <summary>
		/// Starts a Particle Effect that runs once at a given position then is destroyed.
		/// </summary>
		/// <param name="effectName">The name of the effect.</param>
		/// <param name="pos">The World position where the effect is.</param>
		/// <param name="rot">What rotation to apply to the effect.</param>
		/// <param name="scale">How much to scale the size of the effect by.</param>
		/// <param name="invertAxis">Which axis to flip the effect in.</param>
		/// <returns><c>true</c>If the effect was able to start; otherwise, <c>false</c>.</returns>
		public bool StartNonLoopedAtCoord(string effectName, Vector3 pos, Vector3 rot = default(Vector3), float scale = 1.0f,
			InvertAxis invertAxis = InvertAxis.None)
		{
			if (!SetNextCall())
			{
				return false;
			}
			Function.Call(Hash._SET_PTFX_ASSET_NEXT_CALL, _assetName);
			return Function.Call<bool>(Hash.START_PARTICLE_FX_NON_LOOPED_AT_COORD, effectName, pos.X, pos.Y, pos.Z, rot.X, rot.Y,
				rot.Z, scale, invertAxis.HasFlag(InvertAxis.X), invertAxis.HasFlag(InvertAxis.Y), invertAxis.HasFlag(InvertAxis.Z));
		}

		/// <summary>
		/// Starts a Particle Effect on an <see cref="Entity"/> that runs once then is destroyed.
		/// </summary>
		/// <param name="effectName">the name of the effect.</param>
		/// <param name="entity">The <see cref="Entity"/> the effect is attached to.</param>
		/// <param name="off">The offset from the <paramref name="entity"/> to attach the effect.</param>
		/// <param name="rot">The rotation, relative to the <paramref name="entity"/>, the effect has.</param>
		/// <param name="scale">How much to scale the size of the effect by.</param>
		/// <param name="invertAxis">Which axis to flip the effect in. For a car side exahust you may need to flip in the Y Axis</param>
		/// <returns><c>true</c>If the effect was able to start; otherwise, <c>false</c>.</returns>
		public bool StartNonLoopedOnEntity(string effectName, Entity entity,
			Vector3 off = default(Vector3), Vector3 rot = default(Vector3), float scale = 1.0f,
			InvertAxis invertAxis = InvertAxis.None)
		{
			if (!SetNextCall())
			{
				return false;
			}
			Function.Call(Hash._SET_PTFX_ASSET_NEXT_CALL, _assetName);
			return Function.Call<bool>(Hash.START_PARTICLE_FX_NON_LOOPED_ON_PED_BONE, effectName, entity.Handle, off.X, off.Y, off.Z, rot.X,
				rot.Y, rot.Z, -1, scale, invertAxis.HasFlag(InvertAxis.X), invertAxis.HasFlag(InvertAxis.Y),
				invertAxis.HasFlag(InvertAxis.Z));
		}

		/// <summary>
		/// Starts a Particle Effect on an <see cref="EntityBone"/> that runs once then is destroyed.
		/// </summary>
		/// <param name="effectName">the name of the effect.</param>
		/// <param name="entityBone">The <see cref="EntityBone"/> the effect is attached to.</param>
		/// <param name="off">The offset from the <paramref name="entityBone"/> to attach the effect.</param>
		/// <param name="rot">The rotation, relative to the <paramref name="entityBone"/>, the effect has.</param>
		/// <param name="scale">How much to scale the size of the effect by.</param>
		/// <param name="invertAxis">Which axis to flip the effect in. For a car side exahust you may need to flip in the Y Axis</param>
		/// <returns><c>true</c>If the effect was able to start; otherwise, <c>false</c>.</returns>
		public bool StartNonLoopedOnEntity(string effectName, EntityBone entityBone,
			Vector3 off = default(Vector3), Vector3 rot = default(Vector3), float scale = 1.0f,
			InvertAxis invertAxis = InvertAxis.None)
		{
			if(!SetNextCall())
			{
				return false;
			}
			Function.Call(Hash._SET_PTFX_ASSET_NEXT_CALL, _assetName);
			return Function.Call<bool>(Hash.START_PARTICLE_FX_NON_LOOPED_ON_PED_BONE, effectName, entityBone.Owner.Handle, off.X, off.Y, off.Z, rot.X,
				rot.Y, rot.Z, entityBone, scale, invertAxis.HasFlag(InvertAxis.X), invertAxis.HasFlag(InvertAxis.Y),
				invertAxis.HasFlag(InvertAxis.Z));
		}

		/// <summary>
		/// Creates a <see cref="ParticleEffect"/> on an <see cref="Entity"/> that runs looped.
		/// </summary>
		/// <param name="effectName">The name of the Effect</param>
		/// <param name="entity">The <see cref="Entity"/> the effect is attached to.</param>
		/// <param name="off">The offset from the <paramref name="entity"/> to attach the effect.</param>
		/// <param name="rot">The rotation, relative to the <paramref name="entity"/>, the effect has.</param>
		/// <param name="scale">How much to scale the size of the effect by.</param>
		/// <param name="invertAxis">Which axis to flip the effect in. For a car side exahust you may need to flip in the Y Axis.</param>
		/// <param name="startNow">if <c>true</c> attempt to start this effect now; otherwise, the effect will start when <see cref="ParticleEffect.Start"/> is called.</param>
		/// <returns>The <see cref="EntityParticleEffect"/> represented by this that can be used to start/stop/modify this effect</returns>
		public EntityParticleEffect CreateEffectOnEntity(string effectName, Entity entity,
			Vector3 off = default(Vector3), Vector3 rot = default(Vector3), float scale = 1.0f,
			InvertAxis invertAxis = InvertAxis.None, bool startNow = false)
		{
			var result = new EntityParticleEffect(this, effectName, entity)
			{
				Offset = off,
				Rotation = rot,
				Scale = scale,
				InvertAxis = invertAxis
			};
			if (startNow)
			{
				result.Start();
			}
			return result;
		}
		/// <summary>
		/// Creates a <see cref="ParticleEffect"/> on an <see cref="EntityBone"/> that runs looped.
		/// </summary>
		/// <param name="effectName">The name of the Effect</param>
		/// <param name="entityBone">The <see cref="EntityBone"/> the effect is attached to.</param>
		/// <param name="off">The offset from the <paramref name="entityBone"/> to attach the effect.</param>
		/// <param name="rot">The rotation, relative to the <paramref name="entityBone"/>, the effect has.</param>
		/// <param name="scale">How much to scale the size of the effect by.</param>
		/// <param name="invertAxis">Which axis to flip the effect in. For a car side exahust you may need to flip in the Y Axis.</param>
		/// <param name="startNow">if <c>true</c> attempt to start this effect now; otherwise, the effect will start when <see cref="ParticleEffect.Start"/> is called.</param>
		/// <returns>The <see cref="EntityParticleEffect"/> represented by this that can be used to start/stop/modify this effect</returns>
		public EntityParticleEffect CreateEffectOnEntity(string effectName, EntityBone entityBone,
			Vector3 off = default(Vector3), Vector3 rot = default(Vector3), float scale = 1.0f,
			InvertAxis invertAxis = InvertAxis.None, bool startNow = false)
		{
			var result = new EntityParticleEffect(this, effectName, entityBone)
			{
				Offset = off,
				Rotation = rot,
				Scale = scale,
				InvertAxis = invertAxis
			};
			if(startNow)
			{
				result.Start();
			}
			return result;
		}

		/// <summary>
		/// Creates a <see cref="ParticleEffect"/> at a position that runs looped.
		/// </summary>
		/// <param name="effectName">The name of the effect.</param>
		/// <param name="pos">The World position where the effect is.</param>
		/// <param name="rot">What rotation to apply to the effect.</param>
		/// <param name="scale">How much to scale the size of the effect by.</param>
		/// <param name="invertAxis">Which axis to flip the effect in.</param>
		/// <param name="startNow">if <c>true</c> attempt to start this effect now; otherwise, the effect will start when <see cref="ParticleEffect.Start"/> is called.</param>
		/// <returns>The <see cref="CoordinateParticleEffect"/> represented by this that can be used to start/stop/modify this effect</returns>
		public CoordinateParticleEffect CreateEffectAtCoord(string effectName, Vector3 pos, Vector3 rot = default(Vector3), float scale = 1.0f,
			InvertAxis invertAxis = InvertAxis.None, bool startNow = false)
		{
			var result = new CoordinateParticleEffect(this, effectName, pos)
			{
				Rotation = rot,
				Scale = scale,
				InvertAxis = invertAxis
			};
			if (startNow)
			{
				result.Start();
			}
			return result;
		}

		/// <summary>
		/// Sets the <see cref="Color"/> for all NonLooped Particle Effects
		/// </summary>
		static Color NonLoopedColor
		{
			set
			{
				Function.Call(Hash.SET_PARTICLE_FX_NON_LOOPED_COLOUR, value.R/255f, value.G/255f, value.B/255f);
				Function.Call(Hash.SET_PARTICLE_FX_NON_LOOPED_ALPHA, value.A / 255f);
			}
		}

		/// <summary>
		/// Attempts to load this <see cref="ParticleEffectsAsset"/> into memory so it can be used for starting <see cref="ParticleEffect"/>s.
		/// </summary>
		public void Request()
		{
			Function.Call(Hash.REQUEST_NAMED_PTFX_ASSET, _assetName);
		}

		/// <summary>
		/// Attempts to load this <see cref="ParticleEffectsAsset"/> into memory so it can be used for starting <see cref="ParticleEffect"/>s.
		/// </summary>
		/// <param name="timeout">How long in milli-seconds should the game wait while the model hasnt been loaded before giving up</param>
		/// <returns><c>true</c> if the <see cref="ParticleEffectsAsset"/> is Loaded; otherwise, <c>false</c></returns>
		public async Task<bool> Request(int timeout)
		{
			Request();

			DateTime endtime = timeout >= 0 ? DateTime.UtcNow + new TimeSpan(0, 0, 0, 0, timeout) : DateTime.MaxValue;

			while (!IsLoaded)
			{
                await BaseScript.Delay(0);

                if (DateTime.UtcNow >= endtime)
				{
					return false;
				}
				Request();
			}

			return true;
		}

		internal bool SetNextCall()
		{
			Request();
			if (IsLoaded)
			{
				Function.Call(Hash._SET_PTFX_ASSET_NEXT_CALL, _assetName);
				return true;
			}
			return false;
		}

		/// <summary>
		/// Tells the game we have finished using this <see cref="ParticleEffectsAsset"/> and it can be freed from memory
		/// </summary>
		public void MarkAsNoLongerNeeded()
		{
			Function.Call(Hash._REMOVE_NAMED_PTFX_ASSET, _assetName);
		}

		public override string ToString()
		{
			return _assetName;
		}

		public override int GetHashCode()
		{
			return _assetName.GetHashCode();
		}

		public static implicit operator InputArgument(ParticleEffectsAsset asset)
		{
			return new InputArgument(asset._assetName);
		}
	}

	public abstract class ParticleEffect
	{
		#region Fields
		protected readonly ParticleEffectsAsset _asset;
		protected readonly string _effectName;
		protected Vector3 _offset;
		protected Vector3 _rotation;
		protected Color _color;
		protected float _scale;
		protected float _range;
		protected InvertAxis _InvertAxis;
		#endregion
		internal ParticleEffect(ParticleEffectsAsset asset, string effectName)
		{
			Handle = -1;
			_asset = asset;
			_effectName = effectName;
		}

		/// <summary>
		/// Gets the Handle of this <see cref="ParticleEffect"/>
		/// </summary>
		/// <value>
		/// The handle, will return -1 when the this <see cref="ParticleEffect"/> is not active
		/// </value>
		public int Handle { get; protected set; }


		/// <summary>
		/// Gets a value indicating whether this <see cref="ParticleEffect"/> is active.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="ParticleEffect"/> is active; otherwise, <c>false</c>.
		/// </value>
		public bool IsActive
		{
			get { return Handle != -1 && Function.Call<bool>(Hash.DOES_PARTICLE_FX_LOOPED_EXIST, Handle); }
		}

		public abstract bool Start();

		/// <summary>
		/// Deletes this <see cref="ParticleEffect"/>.
		/// </summary>
		public void Stop()
		{
			if (IsActive)
			{
				Function.Call(Hash.REMOVE_PARTICLE_FX, Handle, false);
			}
			Handle = -1;
		}

		/// <summary>
		/// Gets the memory address where this <see cref="ParticleEffect"/> is located in game memory.
		/// </summary>
		public IntPtr MemoryAddress
		{
            // CFX-TODO
			get { return IntPtr.Zero; } //return IsActive ? MemoryAccess.GetPtfxAddress(Handle) : IntPtr.Zero; }
		}

		/// <summary>
		/// Gets or sets the offset.
		/// If this <see cref="ParticleEffect"/> is attached to an <see cref="Entity"/>, this refers to the offset from the <see cref="Entity"/>; 
		/// otherwise, this refers to its position in World coords
		/// </summary>
		public Vector3 Offset
		{
			get
			{
				IntPtr address = MemoryAddress;
				if (address != IntPtr.Zero)
				{
					address = MemoryAccess.ReadPtr(address + 32);
					if (address != IntPtr.Zero)
					{
						return _offset = MemoryAccess.ReadVector3(address + 144);
					}
				}
				return _offset;
			}
			set
			{
				_offset = value;
				IntPtr address = MemoryAddress;
				if (address != IntPtr.Zero)
				{
					address = MemoryAccess.ReadPtr(address + 32);
					if (address != IntPtr.Zero)
					{
						MemoryAccess.WriteVector3(address + 144, value);
					}
				}
			}
		}

		/// <summary>
		/// Gets or Sets the rotation of this <see cref="ParticleEffect"/>
		/// </summary>
		public Vector3 Rotation
		{
			get
			{
				return _rotation;
			}
			set
			{
				_rotation = value;
				if (IsActive)
				{
					//rotation information is stored in a matrix
					Vector3 off = Offset;
					Function.Call(Hash.SET_PARTICLE_FX_LOOPED_OFFSETS, Handle, off.X, off.Y, off.Z, value.X, value.Y, value.Z);
				}
			}
		}

		/// <summary>
		/// Gets or sets the <see cref="Color"/> of this <see cref="ParticleEffect"/>.
		/// </summary>
		public Color Color
		{
			get
			{
				IntPtr address = MemoryAddress;
				if (address != IntPtr.Zero)
				{
					address = MemoryAccess.ReadPtr(address + 32) + 320;
					byte r = Convert.ToByte(MemoryAccess.ReadFloat(address)*255f);
					byte g = Convert.ToByte(MemoryAccess.ReadFloat(address+4)*255f);
					byte b = Convert.ToByte(MemoryAccess.ReadFloat(address+8) * 255f);
					byte a = Convert.ToByte(MemoryAccess.ReadFloat(address+12) * 255f);
					return _color = Color.FromArgb(a, r, g, b);
				}
				return _color;
			}
			set
			{
				_color = value;
				IntPtr address = MemoryAddress;
				if (address != IntPtr.Zero)
				{
					address = MemoryAccess.ReadPtr(address + 32) + 320;
					MemoryAccess.WriteFloat(address, value.R / 255f);
					MemoryAccess.WriteFloat(address + 4, value.G / 255f);
					MemoryAccess.WriteFloat(address + 8, value.B / 255f);
					MemoryAccess.WriteFloat(address + 12, value.A / 255f);
				}
			}
		}

		/// <summary>
		/// Gets or sets the size scaling factor of this <see cref="ParticleEffect"/>
		/// </summary>
		/// <value>
		/// The scale, default = 1.0f; 
		/// To Decrease the size use a value less than 1.0f;
		/// To Increase the size use a value greater than 1.0f;
		/// </value>
		public float Scale
		{
			get
			{
				IntPtr address = MemoryAddress;
				if (address != IntPtr.Zero)
				{
					return _scale = MemoryAccess.ReadFloat(MemoryAccess.ReadPtr(address + 32) + 336);
				}
				return _scale;
			}
			set
			{
				_scale = value;
				IntPtr address = MemoryAddress;
				if (address != IntPtr.Zero)
				{
					MemoryAccess.WriteFloat(MemoryAccess.ReadPtr(address + 32) + 336, value);
				}
			}
		}

		public float Range
		{
			get
			{
				IntPtr address = MemoryAddress;
				if (address != IntPtr.Zero)
				{
					return _range = MemoryAccess.ReadFloat(MemoryAccess.ReadPtr(address + 32) + 384);
				}
				return _range;
			}
			set
			{
				_range = value;
				Function.Call(Hash._SET_PARTICLE_FX_LOOPED_RANGE, Handle, value);
			}
		}

		/// <summary>
		/// Gets or sets which axis of this <see cref="ParticleEffect"/> should be inverted.
		/// </summary>
		public InvertAxis InvertAxis
		{
			get
			{
				return _InvertAxis;
			}
			set
			{
				_InvertAxis = value;
				if (IsActive)
				{
					Stop();
					Start();
				}
			}
		}

		/// <summary>
		/// Modifys parameters of this <see cref="ParticleEffect"/>.
		/// </summary>
		/// <param name="parameterName">Name of the parameter you want to modify, these are stored inside the effect files.</param>
		/// <param name="value">The new value for the parameter.</param>
		public void SetParameter(string parameterName, float value)
		{
			if (IsActive)
				Function.Call(Hash.SET_PARTICLE_FX_LOOPED_EVOLUTION, parameterName, value, 0);
		}

		/// <summary>
		/// Gets the name of the asset this effect is stored in.
		/// </summary>
		public string AssetName
		{
			get { return _asset.AssetName; }
		}

		/// <summary>
		/// Gets the name of this effect.
		/// </summary>
		public string EffectName
		{
			get { return _effectName; }
		}

		public override string ToString()
		{
			return string.Format("{0}\\{1}", AssetName, EffectName);
		}

		public static implicit operator InputArgument(ParticleEffect effect)
		{
			//we only need to worry about supplying a particle effect to a native, never returning one
			return new InputArgument(effect.Handle);
		}
	}

	public class EntityParticleEffect : ParticleEffect
	{
		#region Fields
		private EntityBone _entityBone;
		#endregion
		internal EntityParticleEffect(ParticleEffectsAsset asset, string effectName, Entity entity)
			: base(asset, effectName)
		{
			_entityBone = entity.Bones.Core;
		}
		internal EntityParticleEffect(ParticleEffectsAsset asset, string effectName, EntityBone entitybone)
			: base(asset, effectName)
		{
			_entityBone = entitybone;
		}

		/// <summary>
		/// Gets or sets the <see cref="GTA.Entity"/> this <see cref="EntityParticleEffect"/> is attached to.
		/// </summary>
		public Entity Entity
		{
			get { return _entityBone.Owner; }
			set
			{
				_entityBone = value.Bones.Core;
				if (IsActive)
				{
					Stop();
					Start();
				}
			}
		}

		/// <summary>
		/// Gets or sets the <see cref="EntityBone"/> that this <see cref="EntityParticleEffect"/> is attached to.
		/// </summary>
		public EntityBone Bone
		{
			get { return _entityBone;}
			set
			{
				_entityBone = value;
				if (IsActive)
				{
					Stop();
					Start();
				}
			}
		}

		/// <summary>
		/// Starts this <see cref="EntityParticleEffect"/>.
		/// </summary>
		/// <returns><c>true</c> if this <see cref="EntityParticleEffect"/> was sucessfully started; otherwise, <c>false</c>.</returns>
		public override bool Start()
		{
			Stop();
			if (!_asset.SetNextCall())
				return false;

			Hash hash = _entityBone.Owner is Ped ? Hash.START_PARTICLE_FX_LOOPED_ON_PED_BONE : Hash._START_PARTICLE_FX_LOOPED_ON_ENTITY_BONE;

			Handle = Function.Call<int>(hash, _effectName, _entityBone.Owner.Handle, Offset.X, Offset.Y, Offset.Z, Rotation.X,
				Rotation.Y, Rotation.Z, _entityBone.Index, _scale, InvertAxis.HasFlag(InvertAxis.X), InvertAxis.HasFlag(InvertAxis.Y),
				InvertAxis.HasFlag(InvertAxis.Z));

			if (IsActive)
				return true;

			Handle = -1;
			return false;
		}

		/// <summary>
		/// Creates a copy of this <see cref="EntityParticleEffect"/> to another <see cref="GTA.Entity"/> to simplify applying the same effect to many Entities.
		/// </summary>
		/// <param name="entity">The <see cref="GTA.Entity"/> to copy to.</param>
		/// <returns>An <see cref="EntityParticleEffect"/> that has all the same properties as this instance, but for a different <see cref="GTA.Entity"/>.</returns>
		public EntityParticleEffect CopyTo(Entity entity)
		{
			var result = new EntityParticleEffect(_asset, _effectName, entity)
			{
				Bone = entity.Bones[_entityBone.Index],
				Offset = _offset,
				Rotation = _rotation,
				Color = _color,
				Scale = _scale,
				Range = _range,
				InvertAxis = _InvertAxis
			};
			if (IsActive)
			{
				result.Start();
			}
			return result;
		}
		/// <summary>
		/// Creates a copy of this <see cref="EntityParticleEffect"/> to another <see cref="GTA.EntityBone"/> to simplify applying the same effect to many Entities.
		/// </summary>
		/// <param name="entityBone">The <see cref="GTA.EntityBone"/> to copy to.</param>
		/// <returns>An <see cref="EntityParticleEffect"/> that has all the same properties as this instance, but for a different <see cref="GTA.EntityBone"/>.</returns>
		public EntityParticleEffect CopyTo(EntityBone entityBone)
		{
			var result = new EntityParticleEffect(_asset, _effectName, entityBone)
			{
				Offset = _offset,
				Rotation = _rotation,
				Color = _color,
				Scale = _scale,
				Range = _range,
				InvertAxis = _InvertAxis
			};
			if(IsActive)
			{
				result.Start();
			}
			return result;
		}
	}

	public class CoordinateParticleEffect : ParticleEffect
	{
		public CoordinateParticleEffect(ParticleEffectsAsset asset, string effectName, Vector3 location) : base(asset, effectName)
		{
			Offset = location;
		}

		/// <summary>
		/// Starts this <see cref="CoordinateParticleEffect"/>.
		/// </summary>
		/// <returns><c>true</c> if this <see cref="CoordinateParticleEffect"/> was sucessfully started; otherwise, <c>false</c>.</returns>
		public override bool Start()
		{
			Stop();
			if (!_asset.SetNextCall())
				return false;

			Handle = Function.Call<int>(Hash.START_PARTICLE_FX_LOOPED_AT_COORD, _effectName, Offset.X, Offset.Y, Offset.Z, Rotation.X,
				Rotation.Y, Rotation.Z, Scale, InvertAxis.HasFlag(InvertAxis.X), InvertAxis.HasFlag(InvertAxis.Y),
				InvertAxis.HasFlag(InvertAxis.Z), false);

			if (IsActive)
				return true;

			Handle = -1;
			return false;
		}

		/// <summary>
		/// Creates a copy of this <see cref="CoordinateParticleEffect"/> to another position to simplify applying the same effect to many positions.
		/// </summary>
		/// <param name="position">The position to copy to.</param>
		/// <returns>A <see cref="CoordinateParticleEffect"/> that has all the same properties as this instance, but for a different position.</returns>
		public CoordinateParticleEffect CopyTo(Vector3 position)
		{
			var result = new CoordinateParticleEffect(_asset, _effectName, position)
			{
				Rotation = _rotation,
				Color = _color,
				Scale = _scale,
				Range = _range,
				InvertAxis = _InvertAxis
			};
			if (IsActive)
			{
				result.Start();
			}
			return result;
		}
	}
}