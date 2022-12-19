using CitizenFX.Core.Native;

namespace CitizenFX.Core
{
	public static class Events
	{
#if IS_FXSERVER
		public static readonly CString AllPlayers = "-1";
#endif
		public static void TriggerEvent(string eventName, params object[] args)
			=> CoreNatives.TriggerEventInternal(eventName, args);

		public static void TriggerEvent(CString eventName, params object[] args)
			=> CoreNatives.TriggerEventInternal(eventName, args);

#if !IS_FXSERVER
		public static void TriggerServerEvent(string eventName, params object[] args)
			=> CoreNatives.TriggerServerEventInternal(eventName, args);

		public static void TriggerLatentServerEvent(string eventName, int bytesPerSecond, params object[] args)
			=> CoreNatives.TriggerLatentServerEventInternal(eventName, args, bytesPerSecond);

		public static void TriggerServerEvent(CString eventName, params object[] args)
			=> CoreNatives.TriggerServerEventInternal(eventName, args);

		public static void TriggerLatentServerEvent(CString eventName, int bytesPerSecond, params object[] args)
			=> CoreNatives.TriggerLatentServerEventInternal(eventName, args, bytesPerSecond);
#else
		public static void TriggerClientEvent(string eventName, Player player, params object[] args)
			=> CoreNatives.TriggerClientEventInternal(eventName, player.Handle, args);

		public static void TriggerLatentClientEvent(string eventName, Player player, int bytesPerSecond, params object[] args)
			=> CoreNatives.TriggerLatentClientEventInternal(eventName, player.Handle, args, bytesPerSecond);

		public static void TriggerClientEvent(CString eventName, Player player, params object[] args)
			=> CoreNatives.TriggerClientEventInternal(eventName, player.Handle, args);

		public static void TriggerLatentClientEvent(CString eventName, Player player, int bytesPerSecond, params object[] args)
			=> CoreNatives.TriggerLatentClientEventInternal(eventName, player.Handle, args, bytesPerSecond);
		
		/// <summary>
		/// Broadcasts an event to all connected players.
		/// </summary>
		/// <param name="eventName">The name of the event.</param>
		/// <param name="args">Arguments to pass to the event.</param>
		public static void TriggerAllClientsEvent(string eventName, params object[] args)
			=> CoreNatives.TriggerClientEventInternal(eventName, AllPlayers, args);

		/// <summary>
		/// Broadcasts an event to all connected players.
		/// </summary>
		/// <param name="eventName">The name of the event.</param>
		/// <param name="bytesPerSecond">Amount of bytes send in 1 second.</param>
		/// <param name="args">Arguments to pass to the event.</param>
		public static void TriggerLatentAllClientsEvent(string eventName, int bytesPerSecond, params object[] args)
			=> CoreNatives.TriggerLatentClientEventInternal(eventName, AllPlayers, args, bytesPerSecond);
#endif
	}
}
