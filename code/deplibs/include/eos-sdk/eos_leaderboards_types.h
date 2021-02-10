// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "eos_common.h"

#pragma pack(push, 8)

EXTERN_C typedef struct EOS_LeaderboardsHandle* EOS_HLeaderboards;

/** Timestamp value representing an undefined time for EOS_HLeaderboards. */
#define EOS_LEADERBOARDS_TIME_UNDEFINED -1

/** The most recent version of the EOS_Leaderboards_QueryLeaderboardDefinitions struct. */
#define EOS_LEADERBOARDS_QUERYLEADERBOARDDEFINITIONS_API_LATEST 2

/**
 * Input parameters for the EOS_Leaderboards_QueryLeaderboardDefinitions function.
 * StartTime and EndTime are optional parameters, they can be used to limit the list of definitions
 * to overlap the time window specified.
 */
EOS_STRUCT(EOS_Leaderboards_QueryLeaderboardDefinitionsOptions, (
	/** API Version: Set this to EOS_LEADERBOARDS_QUERYLEADERBOARDDEFINITIONS_API_LATEST. */
	int32_t ApiVersion;
	/** An optional POSIX timestamp for the leaderboard's start time, or EOS_LEADERBOARDS_TIME_UNDEFINED */
	int64_t StartTime;
	/** An optional POSIX timestamp for the leaderboard's end time, or EOS_LEADERBOARDS_TIME_UNDEFINED */
	int64_t EndTime;
	/**
	 * Product User ID for user who is querying definitions.
	 * Must be set when using a client policy that requires a valid logged in user.
	 * Not used for Dedicated Server where no user is available.
	 */
	EOS_ProductUserId LocalUserId;
));

/**
 * An enumeration of the different leaderboard aggregation types.
 */
EOS_ENUM(EOS_ELeaderboardAggregation,
	/** Minimum */
	EOS_LA_Min = 0,
	/** Maximum */
	EOS_LA_Max = 1,
	/** Sum */
	EOS_LA_Sum = 2,
	/** Latest */
	EOS_LA_Latest = 3
);

/** The most recent version of the EOS_Leaderboards_Definition struct. */
#define EOS_LEADERBOARDS_DEFINITION_API_LATEST 1

/**
 * Contains information about a single leaderboard definition
 */
EOS_STRUCT(EOS_Leaderboards_Definition, (
	/** API Version: Set this to EOS_LEADERBOARDS_DEFINITION_API_LATEST. */
	int32_t ApiVersion;
	/** Unique ID to identify leaderboard. */
	const char* LeaderboardId;
	/** Name of stat used to rank leaderboard. */
	const char* StatName;
	/** Aggregation used to sort leaderboard. */
	EOS_ELeaderboardAggregation Aggregation;
	/** The POSIX timestamp for the start time, or EOS_LEADERBOARDS_TIME_UNDEFINED. */
	int64_t StartTime;
	/** The POSIX timestamp for the end time, or EOS_LEADERBOARDS_TIME_UNDEFINED. */
	int64_t EndTime;
));

/** The most recent version of the EOS_Leaderboards_GetLeaderboardDefinitionCount API. */
#define EOS_LEADERBOARDS_GETLEADERBOARDDEFINITIONCOUNT_API_LATEST 1

/**
 * Input parameters for the EOS_Leaderboards_GetLeaderboardDefinitionCount function.
 */
EOS_STRUCT(EOS_Leaderboards_GetLeaderboardDefinitionCountOptions, (
	/** API Version: Set this to EOS_LEADERBOARDS_GETLEADERBOARDDEFINITIONCOUNT_API_LATEST. */
	int32_t ApiVersion;
));

/** The most recent version of the EOS_Leaderboards_CopyLeaderboardDefinitionByIndexOptions struct. */
#define EOS_LEADERBOARDS_COPYLEADERBOARDDEFINITIONBYINDEX_API_LATEST 1

/**
 * Input parameters for the EOS_Leaderboards_CopyLeaderboardDefinitionByIndex function.
 */
EOS_STRUCT(EOS_Leaderboards_CopyLeaderboardDefinitionByIndexOptions, (
	/** API Version: Set this to EOS_LEADERBOARDS_COPYLEADERBOARDDEFINITIONBYINDEX_API_LATEST. */
	int32_t ApiVersion;
	/** Index of the leaderboard definition to retrieve from the cache */
	uint32_t LeaderboardIndex;
));

/** The most recent version of the EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardIdOptions struct. */
#define EOS_LEADERBOARDS_COPYLEADERBOARDDEFINITIONBYLEADERBOARDID_API_LATEST 1

/**
 * Input parameters for the EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId function.
 */
EOS_STRUCT(EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardIdOptions, (
	/** API Version: Set this to EOS_LEADERBOARDS_COPYLEADERBOARDDEFINITIONBYLEADERBOARDID_API_LATEST. */
	int32_t ApiVersion;
	/** The ID of the leaderboard whose definition you want to copy from the cache */
	const char* LeaderboardId;
));

/**
 * Release the memory associated with a leaderboard definition. This must be called on data retrieved from
 * EOS_Leaderboards_CopyLeaderboardDefinitionByIndex or EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId.
 *
 * @param LeaderboardDefinition - The Leaderboard definition to release.
 *
 * @see EOS_Leaderboards_Definition
 * @see EOS_Leaderboards_CopyLeaderboardDefinitionByIndex
 * @see EOS_Leaderboards_CopyLeaderboardDefinitionByLeaderboardId
 */
EOS_DECLARE_FUNC(void) EOS_Leaderboards_Definition_Release(EOS_Leaderboards_Definition* LeaderboardDefinition);

/**
 * Data containing the result information for a query leaderboard definitions request.
 */
EOS_STRUCT(EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallbackInfo, (
	/** The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Leaderboards_QueryLeaderboardDefinitions. */
	void* ClientData;
));

/**
 * Function prototype definition for callbacks passed to EOS_Leaderboards_QueryLeaderboardDefinitions
 * @param Data A EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallback, const EOS_Leaderboards_OnQueryLeaderboardDefinitionsCompleteCallbackInfo* Data);


/** The most recent version of the EOS_Leaderboards_UserScoresQueryStatInfo struct. */
#define EOS_LEADERBOARDS_USERSCORESQUERYSTATINFO_API_LATEST 1

/**
 * Contains information about a single stat to query with user scores.
 */
EOS_STRUCT(EOS_Leaderboards_UserScoresQueryStatInfo, (
	/** API Version: Set this to EOS_LEADERBOARDS_USERSCORESQUERYSTATINFO_API_LATEST. */
	int32_t ApiVersion;
	/** The name of the stat to query. */
	const char* StatName;
	/** Aggregation used to sort the cached user scores. */
	EOS_ELeaderboardAggregation Aggregation;
));

/** The most recent version of the EOS_Leaderboards_QueryLeaderboardUserScores struct. */
#define EOS_LEADERBOARDS_QUERYLEADERBOARDUSERSCORES_API_LATEST 2

/**
 * Input parameters for the EOS_Leaderboards_QueryLeaderboardUserScores function.
 */
EOS_STRUCT(EOS_Leaderboards_QueryLeaderboardUserScoresOptions, (
	/** API Version: Set this to EOS_LEADERBOARDS_QUERYLEADERBOARDUSERSCORES_API_LATEST. */
	int32_t ApiVersion;
	/** An array of Product User IDs indicating the users whose scores you want to retrieve */
	const EOS_ProductUserId* UserIds;
	/** The number of users included in query */
	uint32_t UserIdsCount;
	/** The stats to be collected, along with the sorting method to use when determining rank order for each stat */
	const EOS_Leaderboards_UserScoresQueryStatInfo* StatInfo;
	/** The number of stats to query */
	uint32_t StatInfoCount;
	/** An optional POSIX timestamp, or EOS_LEADERBOARDS_TIME_UNDEFINED; results will only include scores made after this time */
	int64_t StartTime;
	/** An optional POSIX timestamp, or EOS_LEADERBOARDS_TIME_UNDEFINED; results will only include scores made before this time */
	int64_t EndTime;
	/**
	 * Product User ID for user who is querying user scores.
	 * Must be set when using a client policy that requires a valid logged in user.
	 * Not used for Dedicated Server where no user is available.
	 */
	EOS_ProductUserId LocalUserId;
));

/** The most recent version of the EOS_Leaderboards_LeaderboardUserScore struct. */
#define EOS_LEADERBOARDS_LEADERBOARDUSERSCORE_API_LATEST 1

/**
 * Contains information about a single leaderboard user score
 */
EOS_STRUCT(EOS_Leaderboards_LeaderboardUserScore, (
	/** API Version: Set this to EOS_LEADERBOARDS_LEADERBOARDUSERSCORE_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the user who got this score */
	EOS_ProductUserId UserId;
	/** Leaderboard score */
	int32_t Score;
));

/** The most recent version of the EOS_Leaderboards_GetLeaderboardUserScoreCount API. */
#define EOS_LEADERBOARDS_GETLEADERBOARDUSERSCORECOUNT_API_LATEST 1

/**
 * Input parameters for the EOS_Leaderboards_GetLeaderboardUserScoreCount function.
 */
EOS_STRUCT(EOS_Leaderboards_GetLeaderboardUserScoreCountOptions, (
	/** API Version: Set this to EOS_LEADERBOARDS_GETLEADERBOARDUSERSCORECOUNT_API_LATEST. */
	int32_t ApiVersion;
	/** Name of stat used to rank leaderboard. */
	const char* StatName;
));

/** The most recent version of the EOS_Leaderboards_CopyLeaderboardUserScoreByIndexOptions struct. */
#define EOS_LEADERBOARDS_COPYLEADERBOARDUSERSCOREBYINDEX_API_LATEST 1

/**
 * Input parameters for the EOS_Leaderboards_CopyLeaderboardUserScoreByIndex function.
 */
EOS_STRUCT(EOS_Leaderboards_CopyLeaderboardUserScoreByIndexOptions, (
	/** API Version: Set this to EOS_LEADERBOARDS_COPYLEADERBOARDUSERSCOREBYINDEX_API_LATEST. */
	int32_t ApiVersion;
	/** Index of the sorted leaderboard user score to retrieve from the cache. */
	uint32_t LeaderboardUserScoreIndex;
	/** Name of the stat used to rank the leaderboard. */
	const char* StatName;
));

/** The most recent version of the EOS_Leaderboards_CopyLeaderboardUserScoreByUserIdOptions struct. */
#define EOS_LEADERBOARDS_COPYLEADERBOARDUSERSCOREBYUSERID_API_LATEST 1

/**
 * Input parameters for the EOS_Leaderboards_CopyLeaderboardUserScoreByUserId function.
 */
EOS_STRUCT(EOS_Leaderboards_CopyLeaderboardUserScoreByUserIdOptions, (
	/** API Version: Set this to EOS_LEADERBOARDS_COPYLEADERBOARDUSERSCOREBYUSERID_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID to look for when copying leaderboard score data from the cache */
	EOS_ProductUserId UserId;
	/** The name of the stat that is used to rank this leaderboard */
	const char* StatName;
));

/**
 * Release the memory associated with leaderboard user score. This must be called on data retrieved from
 * EOS_Leaderboards_CopyLeaderboardUserScoreByIndex or EOS_Leaderboards_CopyLeaderboardUserScoreByUserId.
 *
 * @param LeaderboardUserScore - The Leaderboard user score to release.
 *
 * @see EOS_Leaderboards_LeaderboardUserScore
 * @see EOS_Leaderboards_CopyLeaderboardUserScoreByIndex
 * @see EOS_Leaderboards_CopyLeaderboardUserScoreByUserId
 */
EOS_DECLARE_FUNC(void) EOS_Leaderboards_LeaderboardUserScore_Release(EOS_Leaderboards_LeaderboardUserScore* LeaderboardUserScore);

/**
 * Data containing the result information for a query leaderboard user scores request.
 */
EOS_STRUCT(EOS_Leaderboards_OnQueryLeaderboardUserScoresCompleteCallbackInfo, (
	/** The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Leaderboards_QueryLeaderboardUserScores. */
	void* ClientData;
));

/**
 * Function prototype definition for callbacks passed to EOS_Leaderboards_QueryLeaderboardUserScores
 * @param Data A EOS_Leaderboards_OnQueryLeaderboardUserScoresCompleteCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Leaderboards_OnQueryLeaderboardUserScoresCompleteCallback, const EOS_Leaderboards_OnQueryLeaderboardUserScoresCompleteCallbackInfo* Data);


/** The most recent version of the EOS_Leaderboards_QueryLeaderboardRanks struct. */
#define EOS_LEADERBOARDS_QUERYLEADERBOARDRANKS_API_LATEST 2

/**
 * Input parameters for the EOS_Leaderboards_QueryLeaderboardRanks function.
 *
 * @see EOS_Leaderboards_Definition
 */
EOS_STRUCT(EOS_Leaderboards_QueryLeaderboardRanksOptions, (
	/** API Version: Set this to EOS_LEADERBOARDS_QUERYLEADERBOARDRANKS_API_LATEST. */
	int32_t ApiVersion;
	/** The ID of the leaderboard whose information you want to retrieve. */
	const char* LeaderboardId;
	/**
	 * Product User ID for user who is querying ranks.
	 * Must be set when using a client policy that requires a valid logged in user.
	 * Not used for Dedicated Server where no user is available.
	 */
	EOS_ProductUserId LocalUserId;
));

/** The most recent version of the EOS_Leaderboards_LeaderboardRecord struct. */
#define EOS_LEADERBOARDS_LEADERBOARDRECORD_API_LATEST 2

/**
 * Contains information about a single leaderboard record
 */
EOS_STRUCT(EOS_Leaderboards_LeaderboardRecord, (
	/** API Version: Set this to EOS_LEADERBOARDS_LEADERBOARDRECORD_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID assoicated with this record */
	EOS_ProductUserId UserId;
	/** Sorted position on leaderboard */
	uint32_t Rank;
	/** Leaderboard score */
	int32_t Score;
	/** The latest display name seen for the user since they last time logged in. This is empty if the user does not have a display name set. */
	const char* UserDisplayName;
));

/** The most recent version of the EOS_Leaderboards_GetLeaderboardRecordCount API. */
#define EOS_LEADERBOARDS_GETLEADERBOARDRECORDCOUNT_API_LATEST 1

/**
 * Input parameters for the EOS_Leaderboards_GetLeaderboardRecordCount function.
 */
EOS_STRUCT(EOS_Leaderboards_GetLeaderboardRecordCountOptions, (
	/** API Version: Set this to EOS_LEADERBOARDS_GETLEADERBOARDRECORDCOUNT_API_LATEST. */
	int32_t ApiVersion;
));

/** The most recent version of the EOS_Leaderboards_CopyLeaderboardRecordByIndexOptions struct. */
#define EOS_LEADERBOARDS_COPYLEADERBOARDRECORDBYINDEX_API_LATEST 2

/**
 * Input parameters for the EOS_Leaderboards_CopyLeaderboardRecordByIndex function.
 */
EOS_STRUCT(EOS_Leaderboards_CopyLeaderboardRecordByIndexOptions, (
	/** API Version: Set this to EOS_LEADERBOARDS_COPYLEADERBOARDRECORDBYINDEX_API_LATEST. */
	int32_t ApiVersion;
	/** Index of the leaderboard record to retrieve from the cache */
	uint32_t LeaderboardRecordIndex;
));

/** The most recent version of the EOS_Leaderboards_CopyLeaderboardRecordByUserIdOptions struct. */
#define EOS_LEADERBOARDS_COPYLEADERBOARDRECORDBYUSERID_API_LATEST 2

/**
 * Input parameters for the EOS_Leaderboards_CopyLeaderboardRecordByUserId function.
 */
EOS_STRUCT(EOS_Leaderboards_CopyLeaderboardRecordByUserIdOptions, (
	/** API Version: Set this to EOS_LEADERBOARDS_COPYLEADERBOARDRECORDBYUSERID_API_LATEST. */
	int32_t ApiVersion;
	/** Leaderboard data will be copied from the cache if it relates to the user matching this Product User ID */
	EOS_ProductUserId UserId;
));

/**
 * Release the memory associated with leaderboard record. This must be called on data retrieved from
 * EOS_Leaderboards_CopyLeaderboardRecordByIndex or EOS_Leaderboards_CopyLeaderboardRecordByUserId.
 *
 * @param LeaderboardRecord - The Leaderboard record to release.
 *
 * @see EOS_Leaderboards_LeaderboardRecord
 * @see EOS_Leaderboards_CopyLeaderboardRecordByIndex
 * @see EOS_Leaderboards_CopyLeaderboardRecordByUserId
 */
EOS_DECLARE_FUNC(void) EOS_Leaderboards_LeaderboardRecord_Release(EOS_Leaderboards_LeaderboardRecord* LeaderboardRecord);

/**
 * Data containing the result information for a query leaderboard ranks request.
 */
EOS_STRUCT(EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallbackInfo, (
	/** The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Leaderboards_QueryLeaderboardRanks. */
	void* ClientData;
));

/**
 * Function prototype definition for callbacks passed to EOS_Leaderboards_QueryLeaderboardRanks
 * @param Data A EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallback, const EOS_Leaderboards_OnQueryLeaderboardRanksCompleteCallbackInfo* Data);

#pragma pack(pop)

#include "eos_leaderboards_types_deprecated.inl"