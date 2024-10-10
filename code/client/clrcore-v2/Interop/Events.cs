using CitizenFX.Core.Native;
using System;
using System.ComponentModel;

namespace CitizenFX.Core
{
	public static class Events
	{
#if IS_FXSERVER
		public static readonly CString AllPlayers = "-1";
#endif

		/// <summary>
		/// Register an event handler
		/// </summary>
		/// <remarks>Be aware this will keep <paramref name="handler"/>.Target object alive once registered, unless it's of type <see cref="BaseScript"/></remarks>
		/// <param name="eventName">name to listen for</param>
		/// <param name="handler">delegate to call once triggered</param>
		/// <param name="binding">limit calls to certain sources, e.g.: server only, client only</param>
		public static void RegisterEventHandler(string eventName, DynFunc handler, Binding binding = Binding.Local)
		{
			if (handler.Target is BaseScript script)
			{
				// these should stay with their owner
				script.RegisterEventHandler(eventName, handler, binding);
			}
			else
			{
				EventsManager.AddEventHandler(eventName, handler, binding);
			}
		}

		/// <summary>
		/// Unregister an event handler
		/// </summary>
		/// <param name="eventName">name to remove event for</param>
		/// <param name="handler">delegate to remove</param>
		public static void UnregisterEventHandler(string eventName, DynFunc handler)
		{
			if (handler.Target is BaseScript script)
			{
				// these should stay with their owner
				script.UnregisterEventHandler(eventName, handler);
			}
			else
			{
				EventsManager.RemoveEventHandler(eventName, handler);
			}
		}

		public static void TriggerEvent(string eventName, params object[] args)
			=> CoreNatives.TriggerEventInternal(eventName, args);

		public static void TriggerEvent(CString eventName, params object[] args)
			=> CoreNatives.TriggerEventInternal(eventName, args);

#if !IS_FXSERVER
		public static void TriggerServerEvent(string eventName, params object[] args)
			=> CoreNatives.TriggerServerEventInternal(eventName, args);

		public static void TriggerUnreliableServerEvent(string eventName, params object[] args)
			=> CoreNatives.TriggerUnreliableServerEventInternal(eventName, args);

		public static void TriggerLatentServerEvent(string eventName, int bytesPerSecond, params object[] args)
			=> CoreNatives.TriggerLatentServerEventInternal(eventName, args, bytesPerSecond);

		public static void TriggerServerEvent(CString eventName, params object[] args)
			=> CoreNatives.TriggerServerEventInternal(eventName, args);

		public static void TriggerUnreliableServerEvent(CString eventName, params object[] args)
			=> CoreNatives.TriggerUnreliableServerEventInternal(eventName, args);

		public static void TriggerLatentServerEvent(CString eventName, int bytesPerSecond, params object[] args)
			=> CoreNatives.TriggerLatentServerEventInternal(eventName, args, bytesPerSecond);
#else
		public static void TriggerClientEvent(string eventName, Shared.Player player, params object[] args)
			=> CoreNatives.TriggerClientEventInternal(eventName, player.m_handle, args);

		public static void TriggerUnreliableClientEvent(string eventName, Shared.Player player, params object[] args)
			=> CoreNatives.TriggerUnreliableClientEventInternal(eventName, player.m_handle, args);

		public static void TriggerLatentClientEvent(string eventName, Shared.Player player, int bytesPerSecond, params object[] args)
			=> CoreNatives.TriggerLatentClientEventInternal(eventName, player.m_handle, args, bytesPerSecond);

		public static void TriggerClientEvent(CString eventName, Shared.Player player, params object[] args)
			=> CoreNatives.TriggerClientEventInternal(eventName, player.m_handle, args);

		public static void TriggerUnreliableClientEvent(CString eventName, Shared.Player player, params object[] args)
			=> CoreNatives.TriggerUnreliableClientEventInternal(eventName, player.m_handle, args);

		public static void TriggerLatentClientEvent(CString eventName, Shared.Player player, int bytesPerSecond, params object[] args)
			=> CoreNatives.TriggerLatentClientEventInternal(eventName, player.m_handle, args, bytesPerSecond);
		
		/// <summary>
		/// Broadcasts an event to all connected players.
		/// </summary>
		/// <param name="eventName">The name of the event.</param>
		/// <param name="args">Arguments to pass to the event.</param>
		public static void TriggerAllClientsEvent(string eventName, params object[] args)
			=> CoreNatives.TriggerClientEventInternal(eventName, AllPlayers, args);

		/// <summary>
		/// Broadcasts an unreliable event to all connected players.
		/// </summary>
		/// <param name="eventName">The name of the event.</param>
		/// <param name="args">Arguments to pass to the event.</param>
		public static void TriggerUnreliableAllClientsEvent(string eventName, params object[] args)
			=> CoreNatives.TriggerUnreliableClientEventInternal(eventName, AllPlayers, args);

		/// <summary>
		/// Broadcasts an event to all connected players.
		/// </summary>
		/// <param name="eventName">The name of the event.</param>
		/// <param name="bytesPerSecond">Amount of bytes send in 1 second.</param>
		/// <param name="args">Arguments to pass to the event.</param>
		public static void TriggerLatentAllClientsEvent(string eventName, int bytesPerSecond, params object[] args)
			=> CoreNatives.TriggerLatentClientEventInternal(eventName, AllPlayers, args, bytesPerSecond);
#endif

		#region Shared library support
#if IS_FXSERVER
		// Unsupported but reroute as there's only 1 server
		public static void TriggerServerEvent(string eventName, params object[] args)
			=> CoreNatives.TriggerEventInternal(eventName, args);

		public static void TriggerUnreliableServerEvent(string eventName, params object[] args)
			=> CoreNatives.TriggerEventInternal(eventName, args);

		public static void TriggerLatentServerEvent(string eventName, int bytesPerSecond, params object[] args)
			=> CoreNatives.TriggerEventInternal(eventName, args);

		public static void TriggerServerEvent(CString eventName, params object[] args)
			=> CoreNatives.TriggerEventInternal(eventName, args);

		public static void TriggerUnreliableServerEvent(CString eventName, params object[] args)
			=> CoreNatives.TriggerEventInternal(eventName, args);

		public static void TriggerLatentServerEvent(CString eventName, int bytesPerSecond, params object[] args)
			=> CoreNatives.TriggerEventInternal(eventName, args);
#else
		// Unsupported functions on client. No logical fallback unless player is us, let's throw instead
		[EditorBrowsable(EditorBrowsableState.Never)]
		public static void TriggerClientEvent(string eventName, Shared.Player player, params object[] args)
			=> throw new NotSupportedException();

		[EditorBrowsable(EditorBrowsableState.Never)]
		public static void TriggerUnreliableClientEvent(string eventName, Shared.Player player, params object[] args)
			=> throw new NotSupportedException();

		[EditorBrowsable(EditorBrowsableState.Never)]
		public static void TriggerLatentClientEvent(string eventName, Shared.Player player, int bytesPerSecond, params object[] args)
			=> throw new NotSupportedException();

		[EditorBrowsable(EditorBrowsableState.Never)]
		public static void TriggerClientEvent(CString eventName, Shared.Player player, params object[] args)
			=> throw new NotSupportedException();

		[EditorBrowsable(EditorBrowsableState.Never)]
		public static void TriggerUnreliableClientEvent(CString eventName, Shared.Player player, params object[] args)
			=> throw new NotSupportedException();

		[EditorBrowsable(EditorBrowsableState.Never)]
		public static void TriggerLatentClientEvent(CString eventName, Shared.Player player, int bytesPerSecond, params object[] args)
			=> throw new NotSupportedException();

		[EditorBrowsable(EditorBrowsableState.Never)]
		public static void TriggerAllClientsEvent(string eventName, params object[] args)
			=> throw new NotSupportedException();

		[EditorBrowsable(EditorBrowsableState.Never)]
		public static void TriggerUnreliableAllClientsEvent(string eventName, params object[] args)
			=> throw new NotSupportedException();

		[EditorBrowsable(EditorBrowsableState.Never)]
		public static void TriggerLatentAllClientsEvent(string eventName, int bytesPerSecond, params object[] args)
			=> throw new NotSupportedException();
#endif
		#endregion
	}
}
