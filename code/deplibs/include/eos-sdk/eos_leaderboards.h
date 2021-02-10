// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "eos_leaderboards_types.h"

/**
 * The following EOS_Leaderboards_* functions allow you to query existing leaderboard definitions that have been defined for your application.
 * You can retrieve a list of scores for the top users for each Leaderboard.
 * You can also query scores for one or more users.
 */

/**
 * Query for a list of existing leaderboards definitions including their attributes.
 *
 * @param Options Structure containing information about the application whose leaderboard definitions we're retrieving.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param CompletionDelegate This function is called when the query operation completes.
 *
 * @return EOS_Success if the operation completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 */
EOS_DECLARE_FUNC(void) EOS_Leaderboards_QueryLeaderboardDefinitions(EOS_HLeaderboards Handle, const EOS_Leaderboards_QueryLeaderboardDefinitionsOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallback CompletionDelegate);

/**
 * Fetch the number of leaderboards definitions that are cached locally.
 *
 * @param Options The Options associated with retrieving the leaderboard count.
 *
 * @see EOS_Leaderboards_CopyLeaderboardDefinitionByIndex
 * @see EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId
 *
 * @return Number of leaderboards or 0 if there is an error
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Leaderboards_GetLeaderboardDefinitionCount(EOS_HLeaderboards Handle, const EOS_Leaderboards_GetLeaderboardDefinitionCountOptions* Options);

/**
 * Fetches a leaderboard definition from the cache using an index.
 *
 * @param Options Structure containing the index being accessed.
 * @param OutLeaderboardDefinition The leaderboard data for the given index, if it exists and is valid, use EOS_Leaderboards_Definition_Release when finished.
 *
 * @see EOS_Leaderboards_Definition_Release
 *
 * @return EOS_Success if the information is available and passed out in OutLeaderboardDefinition
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the leaderboard is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardDefinitionByIndex(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardDefinitionByIndexOptions* Options, EOS_Leaderboards_Definition ** OutLeaderboardDefinition);

/**
 * Fetches a leaderboard definition from the cache using a leaderboard ID.
 *
 * @param Options Structure containing the leaderboard ID being accessed.
 * @param OutLeaderboardDefinition The leaderboard definition for the given leaderboard ID, if it exists and is valid, use EOS_Leaderboards_Definition_Release when finished.
 *
 * @see EOS_Leaderboards_Definition_Release
 *
 * @return EOS_Success if the information is available and passed out in OutLeaderboardDefinition
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the leaderboard data is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardIdOptions* Options, EOS_Leaderboards_Definition ** OutLeaderboardDefinition);

/**
 * Retrieves top leaderboard records by rank in the leaderboard matching the given leaderboard ID.
 *
 * @param Options Structure containing information about the leaderboard records we're retrieving.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param CompletionDelegate This function is called when the query operation completes.
 *
 * @return EOS_Success if the operation completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 */
EOS_DECLARE_FUNC(void) EOS_Leaderboards_QueryLeaderboardRanks(EOS_HLeaderboards Handle, const EOS_Leaderboards_QueryLeaderboardRanksOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallback CompletionDelegate);

/**
 * Fetch the number of leaderboard records that are cached locally.
 *
 * @param Options The Options associated with retrieving the leaderboard record count.
 *
 * @see EOS_Leaderboards_CopyLeaderboardRecordByIndex
 * @see EOS_Leaderboards_CopyLeaderboardRecordByUserId
 *
 * @return Number of leaderboard records or 0 if there is an error
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Leaderboards_GetLeaderboardRecordCount(EOS_HLeaderboards Handle, const EOS_Leaderboards_GetLeaderboardRecordCountOptions* Options);

/**
 * Fetches a leaderboard record from a given index.
 *
 * @param Options Structure containing the index being accessed.
 * @param OutLeaderboardRecord The leaderboard record for the given index, if it exists and is valid, use EOS_Leaderboards_LeaderboardRecord_Release when finished.
 *
 * @see EOS_Leaderboards_LeaderboardRecord_Release
 *
 * @return EOS_Success if the leaderboard record is available and passed out in OutLeaderboardRecord
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the leaderboard is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardRecordByIndex(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardRecordByIndexOptions* Options, EOS_Leaderboards_LeaderboardRecord ** OutLeaderboardRecord);

/**
 * Fetches a leaderboard record from a given user ID.
 *
 * @param Options Structure containing the user ID being accessed.
 * @param OutLeaderboardRecord The leaderboard record for the given user ID, if it exists and is valid, use EOS_Leaderboards_LeaderboardRecord_Release when finished.
 *
 * @see EOS_Leaderboards_LeaderboardRecord_Release
 *
 * @return EOS_Success if the leaderboard record is available and passed out in OutLeaderboardRecord
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the leaderboard data is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardRecordByUserId(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardRecordByUserIdOptions* Options, EOS_Leaderboards_LeaderboardRecord ** OutLeaderboardRecord);

/**
 * Query for a list of scores for a given list of users.
 *
 * @param Options Structure containing information about the users whose scores we're retrieving.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param CompletionDelegate This function is called when the query operation completes.
 *
 * @return EOS_Success if the operation completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 */
EOS_DECLARE_FUNC(void) EOS_Leaderboards_QueryLeaderboardUserScores(EOS_HLeaderboards Handle, const EOS_Leaderboards_QueryLeaderboardUserScoresOptions* Options, void* ClientData, const EOS_Leaderboards_OnQueryLeaderboardUserScoresCompleteCallback CompletionDelegate);

/**
 * Fetch the number of leaderboard user scores that are cached locally.
 *
 * @param Options The Options associated with retrieving the leaderboard user scores count.
 *
 * @see EOS_Leaderboards_CopyLeaderboardUserScoreByIndex
 * @see EOS_Leaderboards_CopyLeaderboardUserScoreByUserId
 *
 * @return Number of leaderboard records or 0 if there is an error
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Leaderboards_GetLeaderboardUserScoreCount(EOS_HLeaderboards Handle, const EOS_Leaderboards_GetLeaderboardUserScoreCountOptions* Options);

/**
 * Fetches leaderboard user score from a given index.
 *
 * @param Options Structure containing the index being accessed.
 * @param OutLeaderboardUserScore The leaderboard user score for the given index, if it exists and is valid, use EOS_Leaderboards_LeaderboardUserScore_Release when finished.
 *
 * @see EOS_Leaderboards_LeaderboardUserScore_Release
 *
 * @return EOS_Success if the leaderboard scores are available and passed out in OutLeaderboardUserScore
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the leaderboard user scores are not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardUserScoreByIndex(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardUserScoreByIndexOptions* Options, EOS_Leaderboards_LeaderboardUserScore ** OutLeaderboardUserScore);

/**
 * Fetches leaderboard user score from a given user ID.
 *
 * @param Options Structure containing the user ID being accessed.
 * @param OutLeaderboardUserScore The leaderboard user score for the given user ID, if it exists and is valid, use EOS_Leaderboards_LeaderboardUserScore_Release when finished.
 *
 * @see EOS_Leaderboards_LeaderboardUserScore_Release
 *
 * @return EOS_Success if the leaderboard scores are available and passed out in OutLeaderboardUserScore
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the leaderboard user scores are not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Leaderboards_CopyLeaderboardUserScoreByUserId(EOS_HLeaderboards Handle, const EOS_Leaderboards_CopyLeaderboardUserScoreByUserIdOptions* Options, EOS_Leaderboards_LeaderboardUserScore ** OutLeaderboardUserScore);
