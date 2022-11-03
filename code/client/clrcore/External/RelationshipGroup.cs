using System;

#if MONO_V2
using API = CitizenFX.FiveM.Native.Natives;
using INativeValue = CitizenFX.Core.Native.Input.Primitive;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;

namespace CitizenFX.Core
#endif
{
	public class RelationshipGroup : INativeValue, IEquatable<RelationshipGroup>
	{
		RelationshipGroup()
		{

		}

		RelationshipGroup(string name) : this()
		{
			uint hashArg = 0u;

			API.AddRelationshipGroup(name, ref hashArg);

			Hash = (int)hashArg;
		}
		public RelationshipGroup(int hash) : this()
		{
			Hash = hash;
		}
		public RelationshipGroup(uint hash) : this((int)hash)
		{
		}

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

		public Relationship GetRelationshipBetweenGroups(RelationshipGroup targetGroup)
		{
			return (Relationship)API.GetRelationshipBetweenGroups((uint)Hash, (uint)targetGroup.Hash);
		}
		public void SetRelationshipBetweenGroups(RelationshipGroup targetGroup, Relationship relationship, bool bidirectionally = false)
		{
			API.SetRelationshipBetweenGroups((int)relationship, (uint)Hash, (uint)targetGroup.Hash);

			if (bidirectionally)
			{
				API.SetRelationshipBetweenGroups((int)relationship, (uint)targetGroup.Hash, (uint)Hash);
			}
		}
		public void ClearRelationshipBetweenGroups(RelationshipGroup targetGroup, Relationship relationship, bool bidirectionally = false)
		{
			API.ClearRelationshipBetweenGroups((int)relationship, (uint)Hash, (uint)targetGroup.Hash);

			if (bidirectionally)
			{
				API.ClearRelationshipBetweenGroups((int)relationship, (uint)targetGroup.Hash, (uint)Hash);
			}
		}

		public void Remove()
		{
			API.RemoveRelationshipGroup((uint)Hash);
		}

		public bool Equals(RelationshipGroup obj)
		{
			return Hash == obj.Hash;
		}
		public override bool Equals(object obj)
		{
			return obj != null && Equals((RelationshipGroup)obj);
		}

		public override int GetHashCode()
		{
			return Hash;
		}
		public override string ToString()
		{
			return "0x" + Hash.ToString("X");
		}

		public static implicit operator RelationshipGroup(int source)
		{
			return new RelationshipGroup(source);
		}
		public static implicit operator RelationshipGroup(uint source)
		{
			return new RelationshipGroup(source);
		}
		public static implicit operator RelationshipGroup(string source)
		{
			return new RelationshipGroup(source);
		}

		public static bool operator ==(RelationshipGroup left, RelationshipGroup right)
		{
			return left.Equals(right);
		}
		public static bool operator !=(RelationshipGroup left, RelationshipGroup right)
		{
			return !(left == right);
		}
	}
}
