// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "eos_stats_types.h"

/**
 * The Stats Interface manages user stats like number of items collected, fastest completion time for a level, number of wins/losses, number of times that a user has performed a certain action, and so on.
 * You can use stats to determine when to unlock achievements and how to use rank users in leaderboards.
 * All Stats Interface calls take a handle of type EOS_HStats as the first parameter.
 * This handle can be retrieved from an EOS_HPlatform handle by using the EOS_Platform_GetStatsInterface function.
 * 
 * @see EOS_Platform_GetStatsInterface
 */

/**
 * Ingest a stat by the amount specified in Options.
 * When the operation is complete and the delegate is triggered the stat will be uploaded to the backend to be processed.
 * The stat may not be updated immediately and an achievement using the stat may take a while to be unlocked once the stat has been uploaded.
 *
 * @param Options Structure containing information about the stat we're ingesting.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param CompletionDelegate This function is called when the ingest stat operation completes.
 *
 * @return EOS_Success if the operation completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_InvalidUser if target user ID is missing or incorrect
 */
EOS_DECLARE_FUNC(void) EOS_Stats_IngestStat(EOS_HStats Handle, const EOS_Stats_IngestStatOptions* Options, void* ClientData, const EOS_Stats_OnIngestStatCompleteCallback CompletionDelegate);

/**
 * Query for a list of stats for a specific player.
 *
 * @param Options Structure containing information about the player whose stats we're retrieving.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate This function is called when the query player stats operation completes.
 *
 * @return EOS_Success if the operation completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_InvalidUser if target user ID is missing or incorrect
 */
EOS_DECLARE_FUNC(void) EOS_Stats_QueryStats(EOS_HStats Handle, const EOS_Stats_QueryStatsOptions* Options, void* ClientData, const EOS_Stats_OnQueryStatsCompleteCallback CompletionDelegate);

/**
 * Fetch the number of stats that are cached locally.
 *
 * @param Options The Options associated with retrieving the stat count
 *
 * @see EOS_Stats_CopyStatByIndex
 *
 * @return Number of stats or 0 if there is an error
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Stats_GetStatsCount(EOS_HStats Handle, const EOS_Stats_GetStatCountOptions* Options);

/**
 * Fetches a stat from a given index. Use EOS_Stats_Stat_Release when finished with the data.
 *
 * @param Options Structure containing the Epic Online Services Account ID and index being accessed
 * @param OutStat The stat data for the given index, if it exists and is valid
 *
 * @see EOS_Stats_Stat_Release
 *
 * @return EOS_Success if the information is available and passed out in OutStat
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the stat is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Stats_CopyStatByIndex(EOS_HStats Handle, const EOS_Stats_CopyStatByIndexOptions* Options, EOS_Stats_Stat ** OutStat);

/**
 * Fetches a stat from cached stats by name. Use EOS_Stats_Stat_Release when finished with the data.
 *
 * @param Options Structure containing the Epic Online Services Account ID and name being accessed
 * @param OutStat The stat data for the given name, if it exists and is valid
 *
 * @see EOS_Stats_Stat_Release
 *
 * @return EOS_Success if the information is available and passed out in OutStat
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the stat is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Stats_CopyStatByName(EOS_HStats Handle, const EOS_Stats_CopyStatByNameOptions* Options, EOS_Stats_Stat ** OutStat);
