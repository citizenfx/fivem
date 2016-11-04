using System;
using CitizenFX.Core.Native;
using System.Security;

namespace CitizenFX.Core
{
	public class RelationshipGroup : INativeValue, IEquatable<RelationshipGroup>
	{
        RelationshipGroup()
        {

        }
        [SecurityCritical]

        RelationshipGroup(string name) : this()
		{
		    int hashArg;
			unsafe
			{
				Function.Call(Native.Hash.ADD_RELATIONSHIP_GROUP, name, &hashArg);
			}

			Hash = hashArg;
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
			return Function.Call<Relationship>(Native.Hash.GET_RELATIONSHIP_BETWEEN_GROUPS, Hash, targetGroup);
		}
		public void SetRelationshipBetweenGroups(RelationshipGroup targetGroup, Relationship relationship, bool bidirectionally = false)
		{
			Function.Call(Native.Hash.SET_RELATIONSHIP_BETWEEN_GROUPS, relationship, Hash, targetGroup);

			if (bidirectionally)
			{
				Function.Call(Native.Hash.SET_RELATIONSHIP_BETWEEN_GROUPS, relationship, targetGroup, Hash);
			}
		}
		public void ClearRelationshipBetweenGroups(RelationshipGroup targetGroup, Relationship relationship, bool bidirectionally = false)
		{
			Function.Call(Native.Hash.CLEAR_RELATIONSHIP_BETWEEN_GROUPS, relationship, Hash, targetGroup);

			if (bidirectionally)
			{
				Function.Call(Native.Hash.CLEAR_RELATIONSHIP_BETWEEN_GROUPS, relationship, targetGroup, Hash);
			}
		}

		public void Remove()
		{
			Function.Call(Native.Hash.REMOVE_RELATIONSHIP_GROUP, Hash);
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
