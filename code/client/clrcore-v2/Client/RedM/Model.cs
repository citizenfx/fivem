using System;
using CitizenFX.Core;
using CitizenFX.RedM.Native;
using TaskBool = CitizenFX.Core.Coroutine<bool>;

namespace CitizenFX.RedM
{
	public class Model : IEquatable<Model>
	{
		private uint m_hash;
		public Model()
		{

		}
		public Model(uint hash)
		{
			Hash = hash;
		}
		public Model(string name)
		{
			Hash = Game.GenerateHash(name);
		}

		/// <summary>
		/// Gets the hash for this <see cref="Model"/>.
		/// </summary>
		public uint Hash
		{
			get => m_hash;
			private set => m_hash = value;
		}

		/// <summary>
		/// Returns true if this <see cref="Model"/> is valid.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Model"/> is valid; otherwise, <c>false</c>.
		/// </value>
		public bool IsValid => Natives.IsModelValid(Hash);
		
		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is in the cd image.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Model"/> is in the cd image; otherwise, <c>false</c>.
		/// </value>
		public bool IsInCdImage => Natives.IsModelInCdimage(Hash);
		
		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is loaded so it can be spawned.
		/// </summary>
		/// <value>
		///   <c>true</c> if this <see cref="Model"/> is loaded; otherwise, <c>false</c>.
		/// </value>
		public bool IsLoaded => Natives.HasModelLoaded(Hash);
		
		/// <summary>
		/// Gets a value indicating whether the collision for this <see cref="Model"/> is loaded.
		/// </summary>
		/// <value>
		/// <c>true</c> if the collision is loaded; otherwise, <c>false</c>.
		/// </value>
		public bool IsCollisionLoaded => Natives.HasCollisionForModelLoaded(Hash);

		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is a boat.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Model"/> is a boat; otherwise, <c>false</c>.
		/// </value>
		public bool IsBoat => Natives.IsThisModelABoat(Hash);
		
		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is a ped.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Model"/> is a ped; otherwise, <c>false</c>.
		/// </value>
		public bool IsPed => Natives.IsModelAPed(Hash);
		
		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is a prop.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Model"/> is a prop; otherwise, <c>false</c>.
		/// </value>
		public bool IsProp => IsValid && !IsPed && !IsVehicle && !Natives.IsWeaponValid(Hash);
		
		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is a train.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Model"/> is a train; otherwise, <c>false</c>.
		/// </value>
		public bool IsTrain => Natives.IsThisModelATrain(Hash);
		
		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is a vehicle.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Model"/> is a vehicle; otherwise, <c>false</c>.
		/// </value>
		public bool IsVehicle => Natives.IsModelAVehicle(Hash);

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
		/// <param name="minimum">The minimum dimensions output <see cref="Vector3"/>.</param>
		/// <param name="maximum">The maximum dimensions output <see cref="Vector3"/>.</param>
		public void GetDimensions(out Vector3 minimum, out Vector3 maximum)
		{
			Vector3 min = new Vector3(0f, 0f, 0f);
			Vector3 max = new Vector3(0f, 0f, 0f);

			Natives.GetModelDimensions(Hash, ref min, ref max);

			minimum = min;
			maximum = max;
		}

		/// <summary>
		/// Attempt to load this <see cref="Model"/> into memory.
		/// </summary>
		public void Request()
		{
			Natives.RequestModel(Hash, false);
		}
		/// <summary>
		/// Attempt to load this <see cref="Model"/> into memory for a given period of time.
		/// </summary>
		/// <param name="timeout">The time (in milliseconds) before giving up trying to load this <see cref="Model"/></param>
		/// <returns><c>true</c> if this <see cref="Model"/> is loaded; otherwise, <c>false</c></returns>
		public async TaskBool Request(int timeout)
		{
			// Only request the model if it's not yet loaded.
			if (!IsLoaded)
			{
				// Make sure the model is valid (needs || checks because weapon models don't return true when IsModelValid or IsModelInCdimage is called).
				if (Natives.IsModelInCdimage(Hash) || Natives.IsModelValid(Hash) || Natives.IsWeaponValid(Hash))
				{
					Request();

					int timer = Natives.GetGameTimer();

					while (!IsLoaded)
					{
						await Coroutine.Yield();

						if (Natives.GetGameTimer() - timer >= timeout)
						{
							return false;
						}
					}
				}
				else
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
			Natives.SetModelAsNoLongerNeeded(Hash);
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
			return (int)Hash;
		}
		public override string ToString()
		{
			return "0x" + Hash.ToString("X");
		}

		public static implicit operator Model(string source)
		{
			return new Model(source);
		}
		
		public static implicit operator uint(Model source)
		{
			return source.Hash;
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
