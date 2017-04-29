using System;
using System.Collections;
using System.Collections.Generic;
using CitizenFX.Core.Native;
using System.Security;

namespace CitizenFX.Core
{
	public enum FormationType
	{
		Default,
		Circle1,
		Circle2,
		Line
	}

	public class PedGroup : PoolObject, IEquatable<PedGroup>, IEnumerable<Ped>, IDisposable
	{
		public class Enumerator : IEnumerator<Ped>
		{
			public Enumerator(PedGroup group)
			{
				_group = group;
			}

			PedGroup _group;
			Ped _current;
			int _currentIndex = -2;

			Ped IEnumerator<Ped>.Current
			{
				get
				{
					return _current;
				}
			}
			object IEnumerator.Current
			{
				get
				{
					return _current;
				}
			}

			public virtual bool MoveNext()
			{
				if (_currentIndex < (_group.MemberCount - 1))
				{
					_currentIndex++;
					_current = _currentIndex < 0 ? _group.Leader : _group.GetMember(_currentIndex);

					if (Ped.Exists(_current))
					{
						return true;
					}

					return MoveNext();
				}

				return false;
			}
			public virtual void Reset()
			{
			}

			public void Dispose()
			{
			}
		}

		public PedGroup() : base(Function.Call<int>(Hash.CREATE_GROUP, 0))
		{
		}
		public PedGroup(int handle) : base(handle)
		{
		}

		public void Dispose()
		{
			Dispose(true);
			GC.SuppressFinalize(this);
		}
		protected virtual void Dispose(bool disposing)
		{
			if (disposing)
			{
				Function.Call(Hash.REMOVE_GROUP, Handle);
			}
		}

		public int MemberCount
		{
            [SecurityCritical]
            get
			{
			    long unknBool;
			    int count;
				unsafe
				{
					Function.Call(Hash.GET_GROUP_SIZE, Handle, &unknBool, &count);
				}

				return count;
			}
		}

		public float SeparationRange
		{
			set
			{
				Function.Call(Hash.SET_GROUP_SEPARATION_RANGE, Handle, value);
			}
		}
		public FormationType FormationType
		{
			set
			{
				Function.Call(Hash.SET_GROUP_FORMATION, Handle, value);
			}
		}

		public Ped Leader
		{
			get
			{
				return new Ped(Function.Call<int>(Hash._GET_PED_AS_GROUP_LEADER, Handle));
			}
		}

		public void Add(Ped ped, bool leader)
		{
			if (leader)
			{
				Function.Call(Hash.SET_PED_AS_GROUP_LEADER, ped.Handle, Handle);
			}
			else
			{
				Function.Call(Hash.SET_PED_AS_GROUP_MEMBER, ped.Handle, Handle);
			}
		}
		public void Remove(Ped ped)
		{
			Function.Call(Hash.REMOVE_PED_FROM_GROUP, ped.Handle);
		}
		public Ped GetMember(int index)
		{
			return new Ped(Function.Call<int>(Hash.GET_PED_AS_GROUP_MEMBER, Handle, index));
		}
		public bool Contains(Ped ped)
		{
			return Function.Call<bool>(Hash.IS_PED_GROUP_MEMBER, ped.Handle, Handle);
		}

		public Ped[] ToArray(bool includingLeader)
		{
			return ToList(includingLeader).ToArray();
		}
		public List<Ped> ToList(bool includingLeader)
		{
			var result = new List<Ped>();

			if (includingLeader)
			{
				Ped leader = Leader;

				if (leader != null && leader.Exists())
				{
					result.Add(leader);
				}
			}

			for (int i = 0; i < MemberCount; i++)
			{
				Ped member = GetMember(i);

				if (member != null && member.Exists())
				{
					result.Add(member);
				}
			}

			return result;
		}

		public override void Delete()
		{
			Function.Call(Hash.REMOVE_GROUP, Handle);
		}

		public override bool Exists()
		{
			return Function.Call<bool>(Hash.DOES_GROUP_EXIST, Handle);
		}
		public static bool Exists(PedGroup pedGroup)
		{
			return !ReferenceEquals(pedGroup, null) && pedGroup.Exists();
		}

		public bool Equals(PedGroup pedGroup)
		{
			return !ReferenceEquals(pedGroup, null) && Handle == pedGroup.Handle;
		}
		public override bool Equals(object obj)
		{
			return !ReferenceEquals(obj, null) && obj.GetType() == GetType() && Equals((PedGroup)obj);
		}

		public override int GetHashCode()
		{
			return Handle;
		}

		public static bool operator ==(PedGroup left, PedGroup right)
		{
			return ReferenceEquals(left, null) ? ReferenceEquals(right, null) : left.Equals(right);
		}
		public static bool operator !=(PedGroup left, PedGroup right)
		{
			return !(left == right);
		}

		IEnumerator IEnumerable.GetEnumerator()
		{
			return new Enumerator(this);
		}
		public IEnumerator<Ped> GetEnumerator()
		{
			return new Enumerator(this);
		}
	}
}
