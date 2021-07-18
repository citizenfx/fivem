using CitizenFX.Core.Native;
using System;
using System.Collections.Generic;

namespace CitizenFX.Core
{
	public static class World
	{
		/// <summary>
		/// Gets the straight line distance between 2 positions.
		/// </summary>
		/// <param name="origin">The origin.</param>
		/// <param name="destination">The destination.</param>
		/// <returns>The distance</returns>
		public static float GetDistance(Vector3 origin, Vector3 destination)
		{
			return (float)Math.Sqrt(destination.DistanceToSquared(origin));
		}

		/// <summary>
		/// Gets an <c>array</c> of all the <see cref="Prop"/>s on the map.
		/// </summary>
		public static Prop[] GetAllProps()
		{
			List<Prop> props = new List<Prop>();

			foreach (int entHandle in API.GetAllObjects())
			{
				Prop prop = (Prop)Entity.FromHandle(entHandle);
				if (prop != null && prop.Exists())
				{
					props.Add(prop);
				}
			}
			
			return props.ToArray();
		}

		/// <summary>
		/// Gets an <c>array</c> of all the <see cref="Ped"/>s on the map.
		/// </summary>
		public static Ped[] GetAllPeds()
		{
			List<Ped> peds = new List<Ped>();

			foreach (int entHandle in API.GetAllPeds())
			{
				Ped ped = (Ped)Entity.FromHandle(entHandle);
				if (ped != null && ped.Exists())
				{
					peds.Add(ped);
				}
			}

			return peds.ToArray();
		}

		/// <summary>
		/// Gets an <c>array</c> of all the <see cref="Vehicle"/>s on the map.
		/// </summary>
		public static Vehicle[] GetAllVehicles()
		{
			List<Vehicle> vehicles = new List<Vehicle>();

			foreach (int entHandle in API.GetAllVehicles())
			{
				Vehicle vehicle = (Vehicle)Entity.FromHandle(entHandle);
				if (vehicle != null && vehicle.Exists())
				{
					vehicles.Add(vehicle);
				}
			}

			return vehicles.ToArray();
		}

		/// <summary>
		/// Gets the closest <see cref="ISpatial"/> to a given position in the World.
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="position">The position to check against.</param>
		/// <param name="spatials">The spatials to check.</param>
		/// <returns>The closest <see cref="ISpatial"/> to the <paramref name="position"/></returns>
		public static T GetClosest<T>(Vector3 position, params T[] spatials) where T : ISpatial
		{
			ISpatial closest = null;
			float closestDistance = 3e38f;

			foreach (var spatial in spatials)
			{
				float distance = position.DistanceToSquared(spatial.Position);

				if (distance <= closestDistance)
				{
					closest = spatial;
					closestDistance = distance;
				}
			}
			return (T)closest;
		}
		
		/// <summary>
		/// Gets the closest <see cref="ISpatial"/> to a given position in the World ignoring height.
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="position">The position to check against.</param>
		/// <param name="spatials">The spatials to check.</param>
		/// <returns>The closest <see cref="ISpatial"/> to the <paramref name="position"/></returns>
		public static T GetClosest<T>(Vector2 position, params T[] spatials) where T : ISpatial
		{
			ISpatial closest = null;
			float closestDistance = 3e38f;
			Vector3 pos = new Vector3(position.X, position.Y, 0.0f);
			foreach (var spatial in spatials)
			{
				float distance = pos.DistanceToSquared2D(spatial.Position);

				if (distance <= closestDistance)
				{
					closest = spatial;
					closestDistance = distance;
				}
			}
			return (T)closest;
		}

		/// <summary>
		/// Creates a <see cref="Blip"/> at the given position on the map.
		/// </summary>
		/// <param name="position">The position of the blip on the map.</param>
		public static Blip CreateBlip(Vector3 position)
		{
			return new Blip(API.AddBlipForCoord(position.X, position.Y, position.Z));
		}
		
		/// <summary>
		/// Creates a <see cref="Blip"/> for a circular area at the given position on the map.
		/// </summary>
		/// <param name="position">The position of the blip on the map.</param>
		/// <param name="radius">The radius of the area on the map.</param>
		public static Blip CreateBlip(Vector3 position, float radius)
		{
			return new Blip(API.AddBlipForRadius(position.X, position.Y, position.Z, radius));
		}

		/// <summary>
		/// Spawns a <see cref="Ped"/> of the given model name as <see cref="string"/> at the position and heading specified.
		/// </summary>
		/// <param name="model">The <see cref="Model"/> of the <see cref="Ped"/>.</param>
		/// <param name="position">The position to spawn the <see cref="Ped"/> at.</param>
		/// <param name="heading">The heading of the <see cref="Ped"/>.</param>
		/// <remarks>returns <c>null</c> if the <see cref="Ped"/> could not be spawned</remarks>
		public static Ped CreatePed(string model, Vector3 position, float heading = 0f)
		{
			return new Ped(API.CreatePed(26, (uint)API.GetHashKey(model), position.X, position.Y, position.Z, heading, true, false));
		}

		/// <summary>
		/// Spawns a <see cref="Vehicle"/> of the given model name as <see cref="string"/> at the position and heading specified.
		/// </summary>
		/// <param name="model">The model name as <see cref="string"/> of the <see cref="Vehicle"/>.</param>
		/// <param name="position">The position to spawn the <see cref="Vehicle"/> at.</param>
		/// <param name="heading">The heading of the <see cref="Vehicle"/>.</param>
		/// <remarks>returns <c>null</c> if the <see cref="Vehicle"/> could not be spawned</remarks>
		public static Vehicle CreateVehicle(string model, Vector3 position, float heading = 0f)
		{
			return new Vehicle(API.CreateVehicle((uint)API.GetHashKey(model), position.X, position.Y, position.Z, heading, true, false));
		}

		/// <summary>
		/// Spawns a <see cref="Prop"/> of the given model name as <see cref="string"/> at the position specified.
		/// </summary>
		/// <param name="model">The model name as <see cref="string"/> of the <see cref="Prop"/>.</param>
		/// <param name="position">The position to spawn the <see cref="Prop"/> at.</param>
		/// <param name="dynamic">if set to <c>true</c> the <see cref="Prop"/> will have physics; otherwise, it will be static.</param>
		/// <remarks>returns <c>null</c> if the <see cref="Prop"/> could not be spawned</remarks>
		public static Prop CreateProp(string model, Vector3 position, bool dynamic)
		{
			return new Prop(API.CreateObject(API.GetHashKey(model), position.X, position.Y, position.Z, true, true, dynamic));
		}

		/// <summary>
		/// Spawns a <see cref="Prop"/> of the given model name as <see cref="string"/> at the position specified.
		/// </summary>
		/// <param name="model">The model name as <see cref="string"/> of the <see cref="Prop"/>.</param>
		/// <param name="position">The position to spawn the <see cref="Prop"/> at.</param>
		/// <param name="rotation">The rotation of the <see cref="Prop"/>.</param>
		/// <param name="dynamic">if set to <c>true</c> the <see cref="Prop"/> will have physics; otherwise, it will be static.</param>
		/// <remarks>returns <c>null</c> if the <see cref="Prop"/> could not be spawned</remarks>
		public static Prop CreateProp(string model, Vector3 position, Vector3 rotation, bool dynamic)
		{
			Prop prop = CreateProp(model, position, dynamic);

			if (prop != null)
			{
				prop.Rotation = rotation;
			}

			return prop;
		}

		/// <summary>
		/// Spawns a <see cref="Prop"/> of the given model name as <see cref="string"/> at the position specified without any offset.
		/// </summary>
		/// <param name="model">The model name as <see cref="string"/> of the <see cref="Prop"/>.</param>
		/// <param name="position">The position to spawn the <see cref="Prop"/> at.</param>
		/// <param name="dynamic">if set to <c>true</c> the <see cref="Prop"/> will have physics; otherwise, it will be static.</param>
		/// <remarks>returns <c>null</c> if the <see cref="Prop"/> could not be spawned</remarks>
		public static Prop CreatePropNoOffset(string model, Vector3 position, bool dynamic)
		{
			return new Prop(API.CreateObjectNoOffset((uint)API.GetHashKey(model), position.X, position.Y, position.Z, true, true, dynamic));
		}

		/// <summary>
		/// Spawns a <see cref="Prop"/> of the given model name as <see cref="string"/> at the position specified without any offset.
		/// </summary>
		/// <param name="model">The model name as <see cref="string"/> of the <see cref="Prop"/>.</param>
		/// <param name="position">The position to spawn the <see cref="Prop"/> at.</param>
		/// <param name="rotation">The rotation of the <see cref="Prop"/>.</param>
		/// <param name="dynamic">if set to <c>true</c> the <see cref="Prop"/> will have physics; otherwise, it will be static.</param>
		/// <remarks>returns <c>null</c> if the <see cref="Prop"/> could not be spawned</remarks>
		public static Prop CreatePropNoOffset(string model, Vector3 position, Vector3 rotation, bool dynamic)
		{
			Prop prop = CreatePropNoOffset(model, position, dynamic);

			if (prop != null)
			{
				prop.Rotation = rotation;
			}

			return prop;
		}
	}
}