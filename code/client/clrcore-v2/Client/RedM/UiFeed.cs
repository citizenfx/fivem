using System.Security;
using CitizenFX.RedM.Native;

namespace CitizenFX.RedM
{
	public static class UiFeed
	{
		// a lot of feed natives reuse the same general structure with just different argument count so we just define
		// these two and reuse them
		[SecuritySafeCritical]
		private static int GenerateFeed4Args(Hash nativeHash, UiFeedInfo feedInfo, UiFeedData feedData, bool bUnk1 = true, bool bUnk2 = true)
		{
			int retVal;
			unsafe
			{
				UnsafeUiInfo dur = new UnsafeUiInfo(feedInfo);
				UnsafeUiFeedNotification notification = new UnsafeUiFeedNotification(feedData);
				retVal = Natives.Call<int>(nativeHash, &dur, &notification, bUnk1, bUnk2);
			}
			return retVal;
		}

		[SecuritySafeCritical]
		private static int GenerateFeed3Args(Hash nativeHash, UiFeedInfo feedInfo, UiFeedData feedData, bool bUnk1 = true)
		{
			int retVal;
			unsafe
			{
				UnsafeUiInfo dur = new UnsafeUiInfo(feedInfo);
				UnsafeUiFeedNotification notification = new UnsafeUiFeedNotification(feedData);
				retVal = Natives.Call<int>(nativeHash, &dur, &notification, bUnk1);
			}
			return retVal;
		}

		/// <summary>
		/// Places the first <paramref name="feedData" /> varString onto the kill feed.
		/// </summary>
		/// <returns>Returns a feedId that can be used by <see cref="UiFeed.ClearFeedByFeedId"/> to remove the feed</returns>
		public static int FeedTicker(UiFeedInfo feedInfo, UiFeedData feedData)
		{
			return GenerateFeed3Args(Hash._UI_FEED_POST_FEED_TICKER, feedInfo, feedData);
		}

		/// <summary>
		/// </summary>
		/// <returns>Returns a feedId that can be used by <see cref="UiFeed.ClearFeedByFeedId"/> to remove the feed</returns>
		public static int GameUpdateShard(UiFeedInfo feedInfo, UiFeedData feedData)
		{
			return GenerateFeed3Args(Hash._UI_FEED_POST_GAME_UPDATE_SHARD, feedInfo, feedData);
		}

		/// <summary>
		/// Places the first <paramref name="feedData" /> varString as a simple notification on the top left of the screen.
		/// </summary>
		/// <returns>Returns a feedId that can be used by <see cref="UiFeed.ClearFeedByFeedId"/> to remove the feed</returns>
		public static int HelpText(UiFeedInfo feedInfo, UiFeedData feedData)
		{
			return GenerateFeed3Args(Hash._UI_FEED_POST_HELP_TEXT, feedInfo, feedData);
		}

		/// <summary>
		/// Displays the first <paramref name="feedData" /> varString as a location in the bottom left of the screen
		/// </summary>
		/// <returns>Returns a feedId that can be used by <see cref="UiFeed.ClearFeedByFeedId"/> to remove the feed</returns>
		public static int MissionName(UiFeedInfo feedInfo, UiFeedData feedData)
		{
			return GenerateFeed3Args(Hash._UI_FEED_POST_MISSION_NAME, feedInfo, feedData);
		}

		/// <summary>
		/// Displays the first <paramref name="feedData" /> varString as a subtitle in the bottom of the screen
		/// </summary>
		/// <returns>Returns a feedId that can be used by <see cref="UiFeed.ClearFeedByFeedId"/> to remove the feed</returns>
		public static int Objective(UiFeedInfo feedInfo, UiFeedData feedData)
		{
			return GenerateFeed3Args(Hash._UI_FEED_POST_OBJECTIVE, feedInfo, feedData);
		}

		/// <summary>
		/// </summary>
		/// <returns>Returns a feedId that can be used by <see cref="UiFeed.ClearFeedByFeedId"/> to remove the feed</returns>
		public static int OneTextShard(UiFeedInfo feedInfo, UiFeedData feedData)
		{
			return GenerateFeed3Args(Hash._UI_FEED_POST_ONE_TEXT_SHARD, feedInfo, feedData);
		}

		/// <summary>
		/// Displays the <paramref name="feedData" /> as a <see cref="UiFeed.SampleToast" />
		/// </summary>
		/// <returns>Returns a feedId that can be used by <see cref="UiFeed.ClearFeedByFeedId"/> to remove the feed</returns>
		public static int RankupToast(UiFeedInfo feedInfo, UiFeedData feedData)
		{
			return GenerateFeed3Args(Hash._UI_FEED_POST_RANKUP_TOAST, feedInfo, feedData);
		}

		/// <summary>
		/// Displays the first <paramref name="feedData" /> as a message at the center of the screen.
		/// </summary>
		/// <returns>Returns a feedId that can be used by <see cref="UiFeed.ClearFeedByFeedId"/> to remove the feed</returns>
		public static int ReticleMessage(UiFeedInfo feedInfo, UiFeedData feedData)
		{
			return GenerateFeed3Args(Hash._UI_FEED_POST_RETICLE_MESSAGE, feedInfo, feedData);
		}

		/// <summary>
		/// Displays the <paramref name="feedInfo"/> as a notification in the top left.
		/// This notification is similar to <see cref="UiFeed.SampleToast" /> but it will not display the color, and
		/// varString2 will disappear before the notification finishes.
		/// </summary>
		/// <returns>Returns a feedId that can be used by <see cref="UiFeed.ClearFeedByFeedId"/> to remove the feed</returns>
		public static int SampleNotification(UiFeedInfo feedInfo, UiFeedData feedData)
		{
			return GenerateFeed4Args(Hash._UI_FEED_POST_SAMPLE_NOTIFICATION, feedInfo, feedData);
		}


		/// <summary>
		/// Displays the <paramref name="feedInfo" /> as a notification in the top left with the specified icon and color.
		/// </summary>
		/// <returns>Returns a feedId that can be used by <see cref="UiFeed.ClearFeedByFeedId"/> to remove the feed</returns>
		public static int SampleToast(UiFeedInfo feedInfo, UiFeedData feedData)
		{
			return GenerateFeed4Args(Hash._UI_FEED_POST_SAMPLE_TOAST, feedInfo, feedData);
		}

		/// <summary>
		/// Displays the <paramref name="feedInfo" /> as a simple notification on the right above the kill feed.
		/// This will use whatever is in <see cref="UiFeed" />s icon field as the color
		/// </summary>
		/// <returns>Returns a feedId that can be used by <see cref="UiFeed.ClearFeedByFeedId"/> to remove the feed</returns>
		public static int SampleToastRight(UiFeedInfo feedInfo, UiFeedData feedData)
		{
			return GenerateFeed3Args(Hash._UI_FEED_POST_SAMPLE_TOAST_RIGHT, feedInfo, feedData);
		}

		/// <summary>
		/// </summary>
		/// <returns>Returns a feedId that can be used by <see cref="UiFeed.ClearFeedByFeedId"/> to remove the feed</returns>
		public static int ToastWithAppLink(UiFeedInfo feedInfo, UiFeedData feedData)
		{
			return GenerateFeed3Args(Hash._UI_FEED_POST_SAMPLE_TOAST_WITH_APP_LINK, feedInfo, feedData);
		}

		/// <summary>
		/// </summary>
		/// <returns>Returns a feedId that can be used by <see cref="UiFeed.ClearFeedByFeedId"/> to remove the feed</returns>
		public static int ThreeTextShard(UiFeedInfo feedInfo, UiFeedData feedData)
		{
			return GenerateFeed4Args(Hash._UI_FEED_POST_THREE_TEXT_SHARD, feedInfo, feedData);
		}


		/// <summary>
		/// </summary>
		/// <returns>Returns a feedId that can be used by <see cref="UiFeed.ClearFeedByFeedId"/> to remove the feed</returns>
		public static int TwoTextShard(UiFeedInfo feedInfo, UiFeedData feedData)
		{
			return GenerateFeed4Args(Hash._UI_FEED_POST_TWO_TEXT_SHARD, feedInfo, feedData);
		}

		/// <summary>
		/// Displays the <paramref name="feedInfo" /> as a microphone notification on the right side of the screen
		/// </summary>
		/// <returns>Returns a feedId that can be used by <see cref="UiFeed.ClearFeedByFeedId"/> to remove the feed</returns>
		public static int VoiceChatFeed(UiFeedInfo feedInfo, UiFeedData feedData)
		{
			return GenerateFeed3Args(Hash._UI_FEED_POST_VOICE_CHAT_FEED, feedInfo, feedData);
		}
		
		
		/// <summary>
		/// Displays the <paramref name="feedInfo" /> as a microphone notification on the right side of the screen
		/// </summary>
		/// <returns>Returns a feedId that can be used by <see cref="UiFeed.ClearFeedByFeedId"/> to remove the feed</returns>
		public static int LocationShard(UiFeedInfo feedInfo, UiFeedData feedData)
		{
			return GenerateFeed4Args(Hash._UI_FEED_POST_LOCATION_SHARD, feedInfo, feedData, false, true);
		}

		/// <summary>
		/// Removes the specified <paramref name="feedId" /> from the feed.
		/// </summary>
		public static void ClearFeedByFeedId(int feedId)
		{
			Natives.UiFeedClearHelpTextFeed(feedId, false);
		}
	}
}
