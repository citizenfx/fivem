// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "eos_common.h"

#pragma pack(push, 8)

EXTERN_C typedef struct EOS_StatsHandle* EOS_HStats;

/** The most recent version of the EOS_Stats_IngestData struct. */
#define EOS_STATS_INGESTDATA_API_LATEST 1

/**
 * Contains information about a single stat to ingest.
 */
EOS_STRUCT(EOS_Stats_IngestData, (
	/** API Version: Set this to EOS_STATS_INGESTDATA_API_LATEST. */
	int32_t ApiVersion;
	/** The name of the stat to ingest. */
	const char* StatName;
	/** The amount to ingest the stat. */
	int32_t IngestAmount;
));

/** Maximum number of stats that can be ingested in a single EOS_Stats_IngestStat operation. */
#define EOS_STATS_MAX_INGEST_STATS 3000

/** The most recent version of the EOS_Stats_IngestStat struct. */
#define EOS_STATS_INGESTSTAT_API_LATEST 3

/**
 * Input parameters for the EOS_Stats_IngestStat function.
 */
EOS_STRUCT(EOS_Stats_IngestStatOptions, (
	/** API Version: Set this to EOS_STATS_INGESTSTAT_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user requesting the ingest.  Set to null for dedicated server. */
	EOS_ProductUserId LocalUserId;
	/** Stats to ingest. */
	const EOS_Stats_IngestData* Stats;
	/** The number of stats to ingest, may not exceed EOS_STATS_MAX_INGEST_STATS. */
	uint32_t StatsCount;
	/** The Product User ID for the user whose stat is being ingested. */
	EOS_ProductUserId TargetUserId;
));

/**
 * Data containing the result information for an ingest stat request.
 */
EOS_STRUCT(EOS_Stats_IngestStatCompleteCallbackInfo, (
	/** Result code for the operation. EOS_Success is returned for a successful request, other codes indicate an error. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Stats_IngestStat. */
	void* ClientData;
	/** The Product User ID for the user requesting the ingest */
	EOS_ProductUserId LocalUserId;
	/** The Product User ID for the user whose stat is being ingested */
	EOS_ProductUserId TargetUserId;
));

/**
 * Function prototype definition for callbacks passed to EOS_Stats_IngestStat
 * @param Data A EOS_Stats_IngestStatCompleteCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Stats_OnIngestStatCompleteCallback, const EOS_Stats_IngestStatCompleteCallbackInfo* Data);


/** Maximum number of stats that can be queried in a single EOS_Stats_QueryStats operation. */
#define EOS_STATS_MAX_QUERY_STATS 1000

/** The most recent version of the EOS_Stats_QueryStats struct. */
#define EOS_STATS_QUERYSTATS_API_LATEST 3

/**
 * Input parameters for the EOS_Stats_QueryStats function.
 */
EOS_STRUCT(EOS_Stats_QueryStatsOptions, (
	/** API Version: Set this to EOS_STATS_QUERYSTATS_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user requesting the stats. Set to null for dedicated server. */
	EOS_ProductUserId LocalUserId;
	/** If not EOS_STATS_TIME_UNDEFINED then this is the POSIX timestamp for start time (Optional). */
	int64_t StartTime;
	/** If not EOS_STATS_TIME_UNDEFINED then this is the POSIX timestamp for end time (Optional). */
	int64_t EndTime;
	/** An array of stat names to query for (Optional). */
	const char** StatNames;
	/** The number of stat names included in query (Optional), may not exceed EOS_STATS_MAX_QUERY_STATS. */
	uint32_t StatNamesCount;
	/** The Product User ID for the user whose stats are being retrieved */
	EOS_ProductUserId TargetUserId;
));

/** Timestamp value representing an undefined StartTime or EndTime for EOS_Stats_Stat */
#define EOS_STATS_TIME_UNDEFINED -1

/** The most recent version of the EOS_Stats_Stat struct. */
#define EOS_STATS_STAT_API_LATEST 1

/**
 * Contains information about a single player stat.
 */
EOS_STRUCT(EOS_Stats_Stat, (
	/** API Version: Set this to EOS_STATS_STAT_API_LATEST. */
	int32_t ApiVersion;
	/** Name of the stat. */
	const char* Name;
	/** If not EOS_STATS_TIME_UNDEFINED then this is the POSIX timestamp for start time. */
	int64_t StartTime;
	/** If not EOS_STATS_TIME_UNDEFINED then this is the POSIX timestamp for end time. */
	int64_t EndTime;
	/** Current value for the stat. */
	int32_t Value;
));

/** The most recent version of the EOS_Stats_GetStatsCount API. */
#define EOS_STATS_GETSTATSCOUNT_API_LATEST 1

/** DEPRECATED! Use EOS_STATS_GETSTATSCOUNT_API_LATEST instead. */
#define EOS_STATS_GETSTATCOUNT_API_LATEST EOS_STATS_GETSTATSCOUNT_API_LATEST

/**
 * Input parameters for the EOS_Stats_GetStatsCount function.
 */
EOS_STRUCT(EOS_Stats_GetStatCountOptions, (
	/** API Version: Set this to EOS_STATS_GETSTATSCOUNT_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID for the user whose stats are being counted */
	EOS_ProductUserId TargetUserId;
));

/** The most recent version of the EOS_Stats_CopyStatByIndexOptions struct. */
#define EOS_STATS_COPYSTATBYINDEX_API_LATEST 1

/**
 * Input parameters for the EOS_Stats_CopyStatByIndex function.
 */
EOS_STRUCT(EOS_Stats_CopyStatByIndexOptions, (
	/** API Version: Set this to EOS_STATS_COPYSTATBYINDEX_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the user who owns the stat */
	EOS_ProductUserId TargetUserId;
	/** Index of the stat to retrieve from the cache */
	uint32_t StatIndex;
));

/** The most recent version of the EOS_Stats_CopyStatByNameOptions struct. */
#define EOS_STATS_COPYSTATBYNAME_API_LATEST 1

/**
 * Input parameters for the EOS_Stats_CopyStatByName function.
 */
EOS_STRUCT(EOS_Stats_CopyStatByNameOptions, (
	/** API Version: Set this to EOS_STATS_COPYSTATBYNAME_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the user who owns the stat */
	EOS_ProductUserId TargetUserId;
	/** Name of the stat to retrieve from the cache */
	const char* Name;
));

/**
 * Release the memory associated with a stat. This must be called on data retrieved from EOS_Stats_CopyStatByIndex or EOS_Stats_CopyStatByName.
 *
 * @param Stat - The stat data to release.
 *
 * @see EOS_Stats_Stat
 * @see EOS_Stats_CopyStatByIndex
 * @see EOS_Stats_CopyStatByName
 */
EOS_DECLARE_FUNC(void) EOS_Stats_Stat_Release(EOS_Stats_Stat* Stat);

/**
 * Data containing the result information for querying a player's stats request.
 */
EOS_STRUCT(EOS_Stats_OnQueryStatsCompleteCallbackInfo, (
	/** The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Stats_QueryStats */
	void* ClientData;
	/** The Product User ID of the user who initiated this request */
	EOS_ProductUserId LocalUserId;
	/** The Product User ID whose stats which were retrieved */
	EOS_ProductUserId TargetUserId;
));

/**
 * Function prototype definition for callbacks passed to EOS_Stats_QueryStats
 *
 * @param Data A EOS_Stats_OnQueryStatsCompleteCallbackInfo containing the output information and result
 *
 * @see EOS_Stats_Stat_Release
 */
EOS_DECLARE_CALLBACK(EOS_Stats_OnQueryStatsCompleteCallback, const EOS_Stats_OnQueryStatsCompleteCallbackInfo* Data);

#pragma pack(pop)
