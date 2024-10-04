using System;
using System.Collections.Generic;
using System.Linq;


#if MONO_V2
#if IS_RDR3
using CitizenFX.RedM.Native;
namespace CitizenFX.RedM
#elif IS_FXSERVER
using CitizenFX.Server.Native;
namespace CitizenFX.Server
#else // IS_GTA
using CitizenFX.FiveM.Native;
namespace CitizenFX.FiveM
#endif // END IS_RDR
#else // !MONO_V2
using Natives = CitizenFX.Core.Native.API;
namespace CitizenFX.Core
#endif // END !MONO_V2
{
	public static partial class Game
	{

		/// <inheritdoc cref="GenerateHash"/>
		/// <remarks>Using a non-ASCII string has undefined behavior.</remarks>
		public static uint GenerateHashASCII(string input)
		{
			uint hash = 0;

			if (input != null)
			{
				var len = input.Length;
				for (var i = 0; i < len; i++)
				{
					uint c = input[i];
					if ((c - 'A') <= 26)
					{
						c += ('a' - 'A');
					}

					hash += c;
					hash += (hash << 10);
					hash ^= (hash >> 6);
				}

				hash += (hash << 3);
				hash ^= (hash >> 11);
				hash += (hash << 15);
			}

			return hash;
		}

		private static readonly Dictionary<Type, string> s_entityTypeMap = new Dictionary<Type, string>
		{
			{ typeof(Ped), "CPed" },
			{ typeof(Prop), "CObject" },
#if !IS_FXSERVER
			{ typeof(Pickup), "CPickup" },
#endif
			{ typeof(Vehicle), "CVehicle" }
		};

		public static IEnumerable<T> GetGamePool<T>()
			where T : PoolObject
		{

			if (!s_entityTypeMap.ContainsKey(typeof(T)))
			{
				throw new ArgumentException($"Unsupported entity type: {typeof(T).Name}");
			}

			string entType = s_entityTypeMap[typeof(T)];


#if MONO_V2
			var pool = Natives.GetGamePool(entType) as object[];
#else
			var pool = Natives.GetGamePool(entType) as List<object>;
#endif

			if (pool != null)
			{
				// Trying to use .Cast here won't work, so we use opt to use the converter
				return pool.Select(entObj =>
				{
					int entId = Convert.ToInt32(entObj);
					var data = Activator.CreateInstance(typeof(T), entId);
					return data as T;
				});
			}

			return Array.Empty<T>();
		}
	}
}
