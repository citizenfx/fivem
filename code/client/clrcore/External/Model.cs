using System;
using CitizenFX.Core.Native;
using System.Security;
using System.Threading.Tasks;
using CitizenFX.Core;

namespace CitizenFX.Core
{
	public class Model : INativeValue, IEquatable<Model>
	{
        public Model()
        {

        }
		public Model(int hash) : this()
		{
			Hash = hash;
		}
		public Model(string name) : this(Game.GenerateHash(name))
		{
		}
		public Model(PedHash hash) : this((int)hash)
		{
		}
		public Model(VehicleHash hash) : this((int)hash)
		{
		}
		public Model(WeaponHash hash) : this((int)hash)
		{
		}

		/// <summary>
		/// Gets the hash for this <see cref="Model"/>.
		/// </summary>
		public int Hash { get; private set; }

		public override ulong NativeValue
		{
			get
			{
				return (ulong)Hash;
			}
			set
			{
				Hash = unchecked((int)value);
			}
		}

		/// <summary>
		/// Returns true if this <see cref="Model"/> is valid.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Model"/> is valid; otherwise, <c>false</c>.
		/// </value>
		public bool IsValid
		{
			get
			{
				return Function.Call<bool>(Native.Hash.IS_MODEL_VALID, Hash);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is in the cd image.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Model"/> is in the cd image; otherwise, <c>false</c>.
		/// </value>
		public bool IsInCdImage
		{
			get
			{
				return Function.Call<bool>(Native.Hash.IS_MODEL_IN_CDIMAGE, Hash);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is loaded so it can be spawned.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Model"/> is loaded; otherwise, <c>false</c>.
		/// </value>
		public bool IsLoaded
		{
			get
			{
				return Function.Call<bool>(Native.Hash.HAS_MODEL_LOADED, Hash);
			}
		}
		/// <summary>
		/// Gets a value indicating whether the collision for this <see cref="Model"/> is loaded.
		/// </summary>
		/// <value>
		/// <c>true</c> if the collision is loaded; otherwise, <c>false</c>.
		/// </value>
		public bool IsCollisionLoaded
		{
			get
			{
				return Function.Call<bool>(Native.Hash.HAS_COLLISION_FOR_MODEL_LOADED, Hash);
			}
		}

		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is a bicycle.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Model"/> is a bicycle; otherwise, <c>false</c>.
		/// </value>
		public bool IsBicycle
		{
			get
			{
				return Function.Call<bool>(Native.Hash.IS_THIS_MODEL_A_BICYCLE, Hash);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is a motorbike.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Model"/> is a motorbike; otherwise, <c>false</c>.
		/// </value>
		public bool IsBike
		{
			get
			{
				return Function.Call<bool>(Native.Hash.IS_THIS_MODEL_A_BIKE, Hash);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is a boat.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Model"/> is a boat; otherwise, <c>false</c>.
		/// </value>
		public bool IsBoat
		{
			get
			{
				return Function.Call<bool>(Native.Hash.IS_THIS_MODEL_A_BOAT, Hash);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is a car.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Model"/> is a car; otherwise, <c>false</c>.
		/// </value>
		public bool IsCar
		{
			get
			{
				return Function.Call<bool>(Native.Hash.IS_THIS_MODEL_A_CAR, Hash);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is a cargobob.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Model"/> is a cargobob; otherwise, <c>false</c>.
		/// </value>
		public bool IsCargobob
		{
			get
			{
				VehicleHash hash = (VehicleHash)Hash;
				return hash == VehicleHash.Cargobob || hash == VehicleHash.Cargobob2 || hash == VehicleHash.Cargobob3 || hash == VehicleHash.Cargobob4;
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is a helicopter.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Model"/> is a helicopter; otherwise, <c>false</c>.
		/// </value>
		public bool IsHelicopter
		{
			get
			{
				return Function.Call<bool>(Native.Hash.IS_THIS_MODEL_A_HELI, Hash);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is a ped.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Model"/> is a ped; otherwise, <c>false</c>.
		/// </value>
		public bool IsPed
		{
			get
			{
                // CFX-TODO
                return false;
				//return MemoryAccess.IsModelAPed(Hash);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is a plane.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Model"/> is a plane; otherwise, <c>false</c>.
		/// </value>
		public bool IsPlane
		{
			get
			{
				return Function.Call<bool>(Native.Hash.IS_THIS_MODEL_A_PLANE, Hash);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is a prop.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Model"/> is a prop; otherwise, <c>false</c>.
		/// </value>
		public bool IsProp
		{
			get
			{
				return IsValid && !IsPed && !IsVehicle;
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is a quadbike.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Model"/> is a quadbike; otherwise, <c>false</c>.
		/// </value>
		public bool IsQuadbike
		{
			get
			{
				return Function.Call<bool>(Native.Hash.IS_THIS_MODEL_A_QUADBIKE, Hash);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is a train.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Model"/> is a train; otherwise, <c>false</c>.
		/// </value>
		public bool IsTrain
		{
			get
			{
				return Function.Call<bool>(Native.Hash.IS_THIS_MODEL_A_TRAIN, Hash);
			}
		}
		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is a vehicle.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Model"/> is a vehicle; otherwise, <c>false</c>.
		/// </value>
		public bool IsVehicle
		{
			get
			{
				return Function.Call<bool>(Native.Hash.IS_MODEL_A_VEHICLE, Hash);
			}
		}

		/// <summary>
		/// Gets the dimensions of this <see cref="Model"/>.
		/// </summary>
		/// <returns></returns>
		public Vector3 GetDimensions()
		{
			Vector3 right, left;
			GetDimensions(out right, out left);

			return Vector3.Subtract(left, right);
		}
        /// <summary>
        /// Gets the dimensions of this <see cref="Model"/>.
        /// </summary>
        /// <param name="minimum">The minimum dimensions.</param>
        /// <param name="maximum">The maximum dimensions.</param>
        [SecuritySafeCritical]
        public void GetDimensions(out Vector3 minimum, out Vector3 maximum)
		{
		    NativeVector3 min, max;
			unsafe
			{
				Function.Call(Native.Hash.GET_MODEL_DIMENSIONS, Hash, &min, &max);
			}

		    minimum = min;
		    maximum = max;
		}

		/// <summary>
		/// Attempt to load this <see cref="Model"/> into memory.
		/// </summary>
		public void Request()
		{
			Function.Call(Native.Hash.REQUEST_MODEL, Hash);
		}
		/// <summary>
		/// Attempt to load this <see cref="Model"/> into memory for a given period of time.
		/// </summary>
		/// <param name="timeout">The time (in milliseconds) before giving up trying to load this <see cref="Model"/></param>
		/// <returns><c>true</c> if this <see cref="Model"/> is loaded; otherwise, <c>false</c></returns>
		public async Task<bool> Request(int timeout)
		{
			Request();

			DateTime endtime = timeout >= 0 ? DateTime.UtcNow + new TimeSpan(0, 0, 0, 0, timeout) : DateTime.MaxValue;

			while (!IsLoaded)
			{
                await BaseScript.Delay(0);
                Request();

				if (DateTime.UtcNow >= endtime)
				{
					return false;
				}
			}

			return true;
		}
		/// <summary>
		/// Frees this <see cref="Model"/> from memory.
		/// </summary>
		public void MarkAsNoLongerNeeded()
		{
			Function.Call(Native.Hash.SET_MODEL_AS_NO_LONGER_NEEDED, Hash);
		}

		public bool Equals(Model model)
		{
			return Hash == model.Hash;
		}
		public override bool Equals(object obj)
		{
			return obj != null && Equals((Model)obj);
		}

		public override int GetHashCode()
		{
			return Hash;
		}
		public override string ToString()
		{
			return "0x" + Hash.ToString("X");
		}

		public static implicit operator Model(int source)
		{
			return new Model(source);
		}
		public static implicit operator Model(string source)
		{
			return new Model(source);
		}
		public static implicit operator Model(PedHash source)
		{
			return new Model(source);
		}
		public static implicit operator Model(VehicleHash source)
		{
			return new Model(source);
		}
		public static implicit operator Model(WeaponHash source)
		{
			return new Model(source);
		}

		public static implicit operator int(Model source)
		{
			return source.Hash;
		}
		public static implicit operator PedHash(Model source)
		{
			return (PedHash)source.Hash;
		}
		public static implicit operator VehicleHash(Model source)
		{
			return (VehicleHash)source.Hash;
		}
		public static implicit operator WeaponHash(Model source)
		{
			return (WeaponHash)source.Hash;
		}

		public static bool operator ==(Model left, Model right)
		{
			return left.Equals(right);
		}
		public static bool operator !=(Model left, Model right)
		{
			return !left.Equals(right);
		}
	}
}
