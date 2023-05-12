using System.Collections;
using System.Collections.Generic;
#if MONO_V2
using API = CitizenFX.FiveM.Native.Natives;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;

namespace CitizenFX.Core
#endif
{
	public class PedBoneCollection : EntityBoneCollection, IEnumerable<PedBone>
	{
		public new class Enumerator : IEnumerator<PedBone>
		{
			#region Fields
			private readonly PedBoneCollection _collection;
			private int currentIndex = -1;// skip the CORE bone index(-1)
			#endregion

			public Enumerator(PedBoneCollection collection)
			{
				_collection = collection;
			}
			public PedBone Current
			{
				get { return _collection[currentIndex]; }
			}

			object IEnumerator.Current
			{
				get { return _collection[currentIndex]; }
			}

			public void Dispose()
			{

			}

			public bool MoveNext()
			{
				// CFX-TODO
				return ++currentIndex < 0;// _collection.Count;
			}

			public void Reset()
			{
				currentIndex = -1;
			}
		}

		#region Fields
		private new readonly Ped _owner;
		#endregion
		internal PedBoneCollection(Ped owner) : base(owner)
		{
			_owner = owner;
		}

		/// <summary>
		/// Gets the <see cref="PedBone"/> with the specified bone name.
		/// </summary>
		/// <param name="boneName">Name of the bone.</param>
		public new PedBone this[string boneName]
		{
			get { return new PedBone(_owner, boneName); }
		}

		/// <summary>
		/// Gets the <see cref="PedBone"/> at the specified bone index.
		/// </summary>
		/// <param name="boneIndex">The bone index.</param>
		public new PedBone this[int boneIndex]
		{
			get { return new PedBone(_owner, boneIndex); }
		}

		/// <summary>
		/// Gets the <see cref="PedBone"/> with the specified boneId.
		/// </summary>
		/// <param name="boneId">The boneId.</param>
		public PedBone this[Bone boneId]
		{
			get { return new PedBone(_owner, boneId); }
		}

		public new PedBone Core
		{
			get { return new PedBone(_owner, -1); }
		}

		/// <summary>
		/// Gets the last damaged Bone for this <see cref="Ped"/>.
		/// </summary>
		public PedBone LastDamaged
		{
			get
			{
				int outBone = 0;
				if (API.GetPedLastDamageBone(_owner.Handle, ref outBone))
				{
					return this[(Bone)outBone];
				}
				return this[Bone.SKEL_ROOT];
			}
		}

		/// <summary>
		/// Clears the last damage a bone on this <see cref="Ped"/> received.
		/// </summary>
		public void ClearLastDamaged()
		{
			API.ClearPedLastDamageBone(_owner.Handle);
		}

		public new IEnumerator<PedBone> GetEnumerator()
		{
			return new Enumerator(this);
		}
		IEnumerator IEnumerable.GetEnumerator()
		{
			return GetEnumerator();
		}
	}
}
