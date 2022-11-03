using System;
using System.Collections;
using System.Collections.Generic;

#if MONO_V2
using API = CitizenFX.FiveM.Native.Natives;
using compat_i32_i64 = System.Int64;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;
using compat_i32_i64 = System.Int32;

namespace CitizenFX.Core
#endif
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

		public PedGroup() : base(API.CreateGroup(0))
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
				API.RemoveGroup(Handle);
			}
		}

		public int MemberCount
		{
			get
			{
				compat_i32_i64 unknBool = 0;
				int count = 0;

				API.GetGroupSize(Handle, ref unknBool, ref count);

				return count;
			}
		}

		public float SeparationRange
		{
			set
			{
				API.SetGroupSeparationRange(Handle, value);
			}
		}
		public FormationType FormationType
		{
			set
			{
				API.SetGroupFormation(Handle, (int)value);
			}
		}

		public Ped Leader
		{
			get
			{
				return new Ped(API.GetPedAsGroupLeader(Handle));
			}
		}

		public void Add(Ped ped, bool leader)
		{
			if (leader)
			{
				API.SetPedAsGroupLeader(ped.Handle, Handle);
			}
			else
			{
				API.SetPedAsGroupMember(ped.Handle, Handle);
			}
		}
		public void Remove(Ped ped)
		{
			API.RemovePedFromGroup(ped.Handle);
		}
		public Ped GetMember(int index)
		{
			return new Ped(API.GetPedAsGroupMember(Handle, index));
		}
		public bool Contains(Ped ped)
		{
			return API.IsPedGroupMember(ped.Handle, Handle);
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
			API.RemoveGroup(Handle);
		}

		public override bool Exists()
		{
			return API.DoesGroupExist(Handle);
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
