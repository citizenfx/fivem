using System;
using CitizenFX.Core;
using CitizenFX.RedM.Native;
using NativeHash = CitizenFX.RedM.Native.Hash;

namespace CitizenFX.RedM
{
	public class Model : IEquatable<Model>, IDisposable
	{
		private readonly uint m_modelHash;

		public uint Hash => m_modelHash;

		// We shouldn't ever get this man requests to start with, if someone
		// manages to hit this limit they should look into their code.
		private byte m_requestCount = 0;

		public Model(string modelName)
		{
			m_modelHash = Game.GenerateHash(modelName);
		}

		public Model(uint modelHash)
		{
			m_modelHash = modelHash;
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
        
        // TODO: Use native call when this returns the right type
		/// <summary>
		/// Gets a value indicating whether this <see cref="Model"/> is a draft vehicle.
		/// </summary>
		/// <value>
		/// <c>true</c> if this <see cref="Model"/> is a vehicle; otherwise, <c>false</c>.
		/// </value>
		public bool IsDraftVehicle => Natives.Call<bool>(NativeHash._IS_THIS_MODEL_A_DRAFT_VEHICLE, Hash);

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
			Vector3 min = Vector3.Zero;
			Vector3 max = Vector3.Zero;

			Natives.GetModelDimensions(Hash, ref min, ref max);

			minimum = min;
			maximum = max;
		}

		public async Coroutine<bool> Request(uint timeout = 1000)
		{
			Natives.RequestModel(m_modelHash, false);
            
			int maxTime = Natives.GetGameTimer() + (int)timeout;
			m_requestCount++;
			
			while (!Natives.HasModelLoaded(m_modelHash))
			{
				if (maxTime < Natives.GetGameTimer())
				{
					Remove();
					return false;
				}
                
				await Coroutine.Delay(0);
			}
            
			return true;
		}

		public void Remove()
		{
			// If we're being cleaned up invoking natives can cause a game crash
			if (AppDomain.CurrentDomain.IsFinalizingForUnload()) return;

			if (m_requestCount > 0) {
				m_requestCount--;
				Natives.SetModelAsNoLongerNeeded(m_modelHash);
			}
		}

		public void Dispose() => Remove();

		// allow models to go into u32 variables so we can just pass into anything
		// requesting it.
		public static implicit operator uint(Model m) => m.m_modelHash;

		public static implicit operator Model(string modelString) => new Model(modelString);

		// when the class gets destroyed we want to set the model as no longer
		// needed so we don't keep unnecessary models in the streaming memory
		// cache
		~Model() => Dispose();
        
		public bool Equals(Model model)
		{
			return Hash == model?.Hash;
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
        
		public static bool operator ==(Model left, Model right)
		{
			return left?.Equals(right) ?? right is null;
		}
		public static bool operator !=(Model left, Model right)
		{
			return !(left == right);
		}
	}
}
