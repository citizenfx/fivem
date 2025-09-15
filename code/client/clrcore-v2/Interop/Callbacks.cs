#if MONO_V2
using CitizenFX.MsgPack;
using CitizenFX.Shared;
using System;

namespace CitizenFX.Core
{
	public class Callbacks
	{
#if !IS_FXSERVER
		public static async Coroutine<T> TriggerServerCallback<T>(string name, params object[] args)
		{
			if (string.IsNullOrEmpty(name))
				throw new ArgumentException("Callback name cannot be null or empty.", nameof(name));
			return await CallbacksManager.TriggerInternal<T>(Binding.Remote, null, name, args);
		}
#else
		/// <summary>
		/// Triggers a callback call to the specified client and awaits for a reply.
		/// </summary>
		/// <typeparam name="T"/>
		/// <param name="name">The callback name.</param>
		/// <param name="client">The client.</param>
		/// <param name="args">Optional arguments.</param>
		/// <exception cref="ArgumentException"></exception>
		/// <returns><![CDATA[Coroutine<T>]]></returns>
		public static async Coroutine<T> TriggerClientCallback<T>(string name, Player client, params object[] args)
		{
			if (string.IsNullOrEmpty(name))
				throw new ArgumentException("Callback name cannot be null or empty.", nameof(name));
			return await CallbacksManager.TriggerInternal<T>(Binding.Remote, client.m_handle, name, args);

		}
#endif

		/// <summary>
		/// Triggers a callback call on the same side (server - server / client - client).
		/// </summary>
		/// <typeparam name="T"/>
		/// <param name="name">The callback name.</param>
		/// <param name="args">Optional arguments.</param>
		/// <exception cref="ArgumentException"></exception>
		/// <returns><![CDATA[Coroutine<T>]]></returns>
		public static async Coroutine<T> TriggerLocalCallback<T>(string name, params object[] args)
		{
			if (string.IsNullOrEmpty(name))
				throw new ArgumentException("Callback name cannot be null or empty.", nameof(name));
			return await CallbacksManager.TriggerInternal<T>(Binding.Local, null, name, args);
		}

		/// <summary>
		/// Registers the event callback, specifying the binding and the unique name.
		/// </summary>
		/// <typeparam name="T"/>
		/// <param name="name">The callback name.</param>
		/// <param name="binding">The binding.</param>
		/// <param name="handler">The handler.</param>
		/// <exception cref="ArgumentException"></exception>
		public static void AddEventCallback(string name, Binding binding, Delegate handler)
		{
			if (string.IsNullOrEmpty(name))
				throw new ArgumentException("Callback name cannot be null or empty.", nameof(name));
			var func = MsgPackDeserializer.CreateDelegate(handler);
			CallbacksManager.Register(name, binding, func);
		}

		/// <summary>
		/// Removes the specified callback.
		/// </summary>
		/// <param name="name">The name.</param>
		/// <exception cref="ArgumentException"></exception>
		public static void RemoveCallback(string name)
		{
			if (string.IsNullOrEmpty(name))
				throw new ArgumentException("Callback name cannot be null or empty.", nameof(name));
			CallbacksManager.Unregister(name);
		}
	}
}
#endif // MONO_V2
