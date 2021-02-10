// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "eos_common.h"
#include "eos_ui_types.h"

#pragma pack(push, 8)

/** Handle to the sessions interface */
EXTERN_C typedef struct EOS_SessionsHandle* EOS_HSessions;
/** Handle to the calls responsible for creating/updating sessions */
EXTERN_C typedef struct EOS_SessionModificationHandle* EOS_HSessionModification;
/** Handle to a single named session that exists locally */
EXTERN_C typedef struct EOS_ActiveSessionHandle* EOS_HActiveSession;
/** Handle to a single session that may be part of a named session, search result, or invite */
EXTERN_C typedef struct EOS_SessionDetailsHandle* EOS_HSessionDetails;
/** Handle to the calls responsible for creating a search object */
EXTERN_C typedef struct EOS_SessionSearchHandle* EOS_HSessionSearch;

/**
 * Release the memory associated with session modification. 
 * This must be called on data retrieved from EOS_Sessions_CreateSessionModification or EOS_Sessions_UpdateSessionModification
 *
 * @param SessionModificationHandle - The session modification handle to release
 *
 * @see EOS_Sessions_CreateSessionModification
 * @see EOS_Sessions_UpdateSessionModification
 */
EOS_DECLARE_FUNC(void) EOS_SessionModification_Release(EOS_HSessionModification SessionModificationHandle);

/**
 * Release the memory associated with an active session.
 * This must be called on data retrieved from EOS_Sessions_CopyActiveSessionHandle
 *
 * @param ActiveSessionHandle - The active session handle to release
 *
 * @see EOS_Sessions_CopyActiveSessionHandle
 */
EOS_DECLARE_FUNC(void) EOS_ActiveSession_Release(EOS_HActiveSession ActiveSessionHandle);

/**
 * Release the memory associated with a single session. This must be called on data retrieved from EOS_SessionSearch_CopySearchResultByIndex.
 *
 * @param SessionHandle - The session handle to release
 *
 * @see EOS_SessionSearch_CopySearchResultByIndex
 */
EOS_DECLARE_FUNC(void) EOS_SessionDetails_Release(EOS_HSessionDetails SessionHandle);

/**
 * Release the memory associated with a session search. This must be called on data retrieved from EOS_Sessions_CreateSessionSearch.
 *
 * @param SessionSearchHandle - The session search handle to release
 *
 * @see EOS_Sessions_CreateSessionSearch
 */
EOS_DECLARE_FUNC(void) EOS_SessionSearch_Release(EOS_HSessionSearch SessionSearchHandle);


/** All possible states of an existing named session */
EOS_ENUM(EOS_EOnlineSessionState,
	/** An online session has not been created yet */
	EOS_OSS_NoSession = 0,
	/** An online session is in the process of being created */
	EOS_OSS_Creating = 1,
	/** Session has been created but the session hasn't started (pre match lobby) */
	EOS_OSS_Pending = 2,
	/** Session has been asked to start (may take time due to communication with backend) */
	EOS_OSS_Starting = 3,
	/** The current session has started. Sessions with join in progress disabled are no longer joinable */
	EOS_OSS_InProgress = 4,
	/** The session is still valid, but the session is no longer being played (post match lobby) */
	EOS_OSS_Ending = 5,
	/** The session is closed and any stats committed */
	EOS_OSS_Ended = 6,
	/** The session is being destroyed */
	EOS_OSS_Destroying = 7
);

/** Advertisement properties for a single attribute associated with a session */
EOS_ENUM(EOS_ESessionAttributeAdvertisementType,
	/** Don't advertise via the online service */
	EOS_SAAT_DontAdvertise = 0,
	/** Advertise via the online service only */
	EOS_SAAT_Advertise = 1
);

/** Deprecated 1.4 types, do not use */
#define EOS_SAT_Boolean EOS_AT_BOOLEAN
#define EOS_SAT_Int64	EOS_AT_INT64
#define EOS_SAT_Double	EOS_AT_DOUBLE
#define EOS_SAT_String	EOS_AT_STRING

/** Deprecated 1.4 types, do not use */
#define EOS_OCO_EQUAL				EOS_CO_EQUAL
#define EOS_OCO_NOTEQUAL			EOS_CO_NOTEQUAL
#define EOS_OCO_GREATERTHAN			EOS_CO_GREATERTHAN
#define EOS_OCO_GREATERTHANOREQUAL	EOS_CO_GREATERTHANOREQUAL
#define EOS_OCO_LESSTHAN			EOS_CO_LESSTHAN
#define EOS_OCO_LESSTHANOREQUAL		EOS_CO_LESSTHANOREQUAL
#define EOS_OCO_DISTANCE			EOS_CO_DISTANCE
#define EOS_OCO_ANYOF				EOS_CO_ANYOF
#define EOS_OCO_NOTANYOF			EOS_CO_NOTANYOF


/** Maximum number of attributes allowed on the session */
#define EOS_SESSIONMODIFICATION_MAX_SESSION_ATTRIBUTES 64
/** Maximum length of the name of the attribute associated with the session */
#define EOS_SESSIONMODIFICATION_MAX_SESSION_ATTRIBUTE_LENGTH 64

/** Minimum number of characters the session id override */
#define EOS_SESSIONMODIFICATION_MIN_SESSIONIDOVERRIDE_LENGTH 16
/** Maximum number of characters a session id override */
#define EOS_SESSIONMODIFICATION_MAX_SESSIONIDOVERRIDE_LENGTH 64

/** The most recent version of the EOS_Sessions_CreateSessionModification API. */
#define EOS_SESSIONS_CREATESESSIONMODIFICATION_API_LATEST 3

/**
 * Input parameters for the EOS_Sessions_CreateSessionModification function.
 */
EOS_STRUCT(EOS_Sessions_CreateSessionModificationOptions, (
	/** API Version: Set this to EOS_SESSIONS_CREATESESSIONMODIFICATION_API_LATEST. */
	int32_t ApiVersion;
	/** Name of the session to create */
	const char* SessionName;
	/** Bucket ID associated with the session */
	const char* BucketId;
	/** Maximum number of players allowed in the session */
	uint32_t MaxPlayers;
	/** The Product User ID of the local user associated with the session */
	EOS_ProductUserId LocalUserId;
	/** 
	 * If true, this session will be associated with presence. Only one session at a time can have this flag true.
	 * This affects the ability of the Social Overlay to show game related actions to take in the user's social graph.
	 * 
	 * @note The Social Overlay can handle only one of the following three options at a time:
	 * * using the bPresenceEnabled flags within the Sessions interface
	 * * using the bPresenceEnabled flags within the Lobby interface
	 * * using EOS_PresenceModification_SetJoinInfo
	 *
	 * @see EOS_PresenceModification_SetJoinInfoOptions
	 * @see EOS_Lobby_CreateLobbyOptions
	 * @see EOS_Lobby_JoinLobbyOptions
	 * @see EOS_Sessions_JoinSessionOptions
	 */
	EOS_Bool bPresenceEnabled;
	/**
	 * Optional session id - set to a globally unique value to override the backend assignment
	 * If not specified the backend service will assign one to the session.  Do not mix and match.
	 * This value can be of size [EOS_SESSIONMODIFICATION_MIN_SESSIONIDOVERRIDE_LENGTH, EOS_SESSIONMODIFICATION_MAX_SESSIONIDOVERRIDE_LENGTH]
	 */
	const char* SessionId;
));

/** The most recent version of the EOS_Sessions_UpdateSessionModification API. */
#define EOS_SESSIONS_UPDATESESSIONMODIFICATION_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_UpdateSessionModification function.
 */
EOS_STRUCT(EOS_Sessions_UpdateSessionModificationOptions, (
	/** API Version: Set this to EOS_SESSIONS_UPDATESESSIONMODIFICATION_API_LATEST. */
	int32_t ApiVersion;
	/** Name of the session to update */
	const char* SessionName;
));

/** Max length of an invite ID */
#define EOS_SESSIONS_INVITEID_MAX_LENGTH 64

/** The most recent version of the EOS_Sessions_SendInvite API. */
#define EOS_SESSIONS_SENDINVITE_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_SendInvite function.
 */
EOS_STRUCT(EOS_Sessions_SendInviteOptions, (
	/** API Version: Set this to EOS_SESSIONS_SENDINVITE_API_LATEST. */
	int32_t ApiVersion;
	/** Name of the session associated with the invite */
	const char* SessionName;
	/** The Product User ID of the local user sending the invitation */
	EOS_ProductUserId LocalUserId;
	/** The Product User of the remote user receiving the invitation */
	EOS_ProductUserId TargetUserId;
));

/**
 * Output parameters for the EOS_Sessions_SendInvite function.
 */
EOS_STRUCT(EOS_Sessions_SendInviteCallbackInfo, (
	/** The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Sessions_SendInvite */
	void* ClientData;
));

/**
 * Function prototype definition for callbacks passed to EOS_Sessions_SendInvite
 * @param Data A EOS_Sessions_SendInviteCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Sessions_OnSendInviteCallback, const EOS_Sessions_SendInviteCallbackInfo* Data);

/** The most recent version of the EOS_Sessions_RejectInvite API. */
#define EOS_SESSIONS_REJECTINVITE_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_RejectInvite function.
 */
EOS_STRUCT(EOS_Sessions_RejectInviteOptions, (
	/** API Version: Set this to EOS_SESSIONS_REJECTINVITE_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user rejecting the invitation */
	EOS_ProductUserId LocalUserId;
	/** The invite ID to reject */
	const char* InviteId;
));

/**
 * Output parameters for the EOS_Sessions_RejectInvite function.
 */
EOS_STRUCT(EOS_Sessions_RejectInviteCallbackInfo, (
	/** The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Sessions_RejectInvite */
	void* ClientData;
));

/**
 * Function prototype definition for callbacks passed to EOS_Sessions_RejectInvite
 * @param Data A EOS_Sessions_RejectInviteCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Sessions_OnRejectInviteCallback, const EOS_Sessions_RejectInviteCallbackInfo* Data);

/** The most recent version of the EOS_Sessions_QueryInvites API. */
#define EOS_SESSIONS_QUERYINVITES_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_QueryInvites function.
 */
EOS_STRUCT(EOS_Sessions_QueryInvitesOptions, (
	/** API Version: Set this to EOS_SESSIONS_QUERYINVITES_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID to query for invitations */
	EOS_ProductUserId LocalUserId;
));

/**
 * Output parameters for the EOS_Sessions_QueryInvites function.
 */
EOS_STRUCT(EOS_Sessions_QueryInvitesCallbackInfo, (
	/** The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Sessions_QueryInvites */
	void* ClientData;
	/** The Product User of the local user who made the request */
	EOS_ProductUserId LocalUserId;
));

/**
 * Function prototype definition for callbacks passed to EOS_Sessions_QueryInvites
 * @param Data A EOS_Sessions_QueryInvites CallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Sessions_OnQueryInvitesCallback, const EOS_Sessions_QueryInvitesCallbackInfo* Data);

/** The most recent version of the EOS_Sessions_GetInviteCount API. */
#define EOS_SESSIONS_GETINVITECOUNT_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_GetInviteCount function.
 */
EOS_STRUCT(EOS_Sessions_GetInviteCountOptions, (
	/** API Version: Set this to EOS_SESSIONS_GETINVITECOUNT_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user who has one or more invitations in the cache */
	EOS_ProductUserId LocalUserId;
));

/** The most recent version of the EOS_Sessions_GetInviteIdByIndex API. */
#define EOS_SESSIONS_GETINVITEIDBYINDEX_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_GetInviteIdByIndex function.
 */
EOS_STRUCT(EOS_Sessions_GetInviteIdByIndexOptions, (
	/** API Version: Set this to EOS_SESSIONS_GETINVITEIDBYINDEX_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user who has an invitation in the cache */
	EOS_ProductUserId LocalUserId;
	/** Index of the invite ID to retrieve */
	uint32_t Index;
));

/** The most recent version of the EOS_Sessions_CreateSessionSearch API. */
#define EOS_SESSIONS_CREATESESSIONSEARCH_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_CreateSessionSearch function.
 */
EOS_STRUCT(EOS_Sessions_CreateSessionSearchOptions, (
	/** API Version: Set this to EOS_SESSIONS_CREATESESSIONSEARCH_API_LATEST. */
	int32_t ApiVersion;
	/** Max number of results to return */
	uint32_t MaxSearchResults;
));

/** The most recent version of the EOS_Sessions_UpdateSession API. */
#define EOS_SESSIONS_UPDATESESSION_API_LATEST 1
EOS_STRUCT(EOS_Sessions_UpdateSessionOptions, (
	/** API Version: Set this to EOS_SESSIONS_UPDATESESSION_API_LATEST. */
	int32_t ApiVersion;
	/** Builder handle */
	EOS_HSessionModification SessionModificationHandle;
));

/**
 * Output parameters for the EOS_Sessions_UpdateSession function.
 */
EOS_STRUCT(EOS_Sessions_UpdateSessionCallbackInfo, (
	/** The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Sessions_UpdateSession */
	void* ClientData;
	/** Name of the session that was created/modified */
	const char* SessionName;
	/** ID of the session that was created/modified */
	const char* SessionId;
));

/**
 * Function prototype definition for callbacks passed to EOS_Sessions_UpdateSession
 * @param Data A EOS_Sessions_UpdateSessionCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Sessions_OnUpdateSessionCallback, const EOS_Sessions_UpdateSessionCallbackInfo* Data);

/** The most recent version of the EOS_Sessions_DestroySession API. */
#define EOS_SESSIONS_DESTROYSESSION_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_DestroySession function.
 */
EOS_STRUCT(EOS_Sessions_DestroySessionOptions, (
	/** API Version: Set this to EOS_SESSIONS_DESTROYSESSION_API_LATEST. */
	int32_t ApiVersion;
	/** Name of the session to destroy */
	const char* SessionName;
));

/**
 * Output parameters for the EOS_Sessions_DestroySession function.
 */
EOS_STRUCT(EOS_Sessions_DestroySessionCallbackInfo, (
	/** The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Sessions_DestroySession */
	void* ClientData;
));

/**
 * Function prototype definition for callbacks passed to EOS_Sessions_DestroySession
 * @param Data A EOS_Sessions_DestroySessionCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Sessions_OnDestroySessionCallback, const EOS_Sessions_DestroySessionCallbackInfo* Data);

/** The most recent version of the EOS_Sessions_JoinSession API. */
#define EOS_SESSIONS_JOINSESSION_API_LATEST 2

/**
 * Input parameters for the EOS_Sessions_JoinSession function.
 */
EOS_STRUCT(EOS_Sessions_JoinSessionOptions, (
	/** API Version: Set this to EOS_SESSIONS_JOINSESSION_API_LATEST. */
	int32_t ApiVersion;
	/** Name of the session to create after joining session */
	const char* SessionName;
	/** Session handle to join */
	EOS_HSessionDetails SessionHandle;
	/** The Product User ID of the local user who is joining the session */
	EOS_ProductUserId LocalUserId;
	/** 
	 * If true, this session will be associated with presence. Only one session at a time can have this flag true.
	 * This affects the ability of the Social Overlay to show game related actions to take in the user's social graph.
	 *
	 * @note The Social Overlay can handle only one of the following three options at a time:
	 * * using the bPresenceEnabled flags within the Sessions interface
	 * * using the bPresenceEnabled flags within the Lobby interface
	 * * using EOS_PresenceModification_SetJoinInfo
	 *
	 * @see EOS_PresenceModification_SetJoinInfoOptions
	 * @see EOS_Lobby_CreateLobbyOptions
	 * @see EOS_Lobby_JoinLobbyOptions
	 * @see EOS_Sessions_CreateSessionModificationOptions
	 */
	EOS_Bool bPresenceEnabled;
));

/**
 * Output parameters for the EOS_Sessions_JoinSession function.
 */
EOS_STRUCT(EOS_Sessions_JoinSessionCallbackInfo, (
	/** The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Sessions_JoinSession */
	void* ClientData;
));

/**
 * Function prototype definition for callbacks passed to EOS_Sessions_JoinSession
 * @param Data A EOS_Sessions_JoinSessionCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Sessions_OnJoinSessionCallback, const EOS_Sessions_JoinSessionCallbackInfo* Data);

/** The most recent version of the EOS_Sessions_StartSession API. */
#define EOS_SESSIONS_STARTSESSION_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_StartSession function.
 */
EOS_STRUCT(EOS_Sessions_StartSessionOptions, (
	/** API Version: Set this to EOS_SESSIONS_STARTSESSION_API_LATEST. */
	int32_t ApiVersion;
	/** Name of the session to set in progress */
	const char* SessionName;
));

EOS_STRUCT(EOS_Sessions_StartSessionCallbackInfo, (
	/** The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Sessions_StartSession */
	void* ClientData;
));

/**
 * Function prototype definition for callbacks passed to EOS_Sessions_StartSession
 * @param Data A EOS_Sessions_StartSessionCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Sessions_OnStartSessionCallback, const EOS_Sessions_StartSessionCallbackInfo* Data);

/** The most recent version of the EOS_Sessions_EndSession API. */
#define EOS_SESSIONS_ENDSESSION_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_EndSession function.
 */
EOS_STRUCT(EOS_Sessions_EndSessionOptions, (
	/** API Version: Set this to EOS_SESSIONS_ENDSESSION_API_LATEST. */
	int32_t ApiVersion;
	/** Name of the session to set as no long in progress */
	const char* SessionName;
));

EOS_STRUCT(EOS_Sessions_EndSessionCallbackInfo, (
	/** The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Sessions_EndSession */
	void* ClientData;
));

/**
 * Function prototype definition for callbacks passed to EOS_Sessions_EndSession
 * @param Data A EOS_Sessions_EndSessionCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Sessions_OnEndSessionCallback, const EOS_Sessions_EndSessionCallbackInfo* Data);

/** The most recent version of the EOS_Sessions_RegisterPlayers API. */
#define EOS_SESSIONS_REGISTERPLAYERS_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_RegisterPlayers function.
 */
EOS_STRUCT(EOS_Sessions_RegisterPlayersOptions, (
	/** API Version: Set this to EOS_SESSIONS_REGISTERPLAYERS_API_LATEST. */
	int32_t ApiVersion;
	/** Name of the session for which to register players */
	const char* SessionName;
	/** Array of players to register with the session */
	EOS_ProductUserId* PlayersToRegister;
	/** Number of players in the array */
	uint32_t PlayersToRegisterCount;
));

EOS_STRUCT(EOS_Sessions_RegisterPlayersCallbackInfo, (
	/** The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Sessions_RegisterPlayers */
	void* ClientData;
));

/**
 * Function prototype definition for callbacks passed to EOS_Sessions_RegisterPlayers
 * @param Data A EOS_Sessions_RegisterPlayersCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Sessions_OnRegisterPlayersCallback, const EOS_Sessions_RegisterPlayersCallbackInfo* Data);

/** The most recent version of the EOS_Sessions_UnregisterPlayers API. */
#define EOS_SESSIONS_UNREGISTERPLAYERS_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_UnregisterPlayers function.
 */
EOS_STRUCT(EOS_Sessions_UnregisterPlayersOptions, (
	/** API Version: Set this to EOS_SESSIONS_UNREGISTERPLAYERS_API_LATEST. */
	int32_t ApiVersion;
	/** Name of the session for which to unregister players */
	const char* SessionName;
	/** Array of players to unregister from the session */
	EOS_ProductUserId* PlayersToUnregister;
	/** Number of players in the array */
	uint32_t PlayersToUnregisterCount;
));

EOS_STRUCT(EOS_Sessions_UnregisterPlayersCallbackInfo, (
	/** The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Sessions_UnregisterPlayers */
	void* ClientData;
));

/**
 * Function prototype definition for callbacks passed to EOS_Sessions_UnregisterPlayers
 * @param Data A EOS_Sessions_UnregisterPlayersCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Sessions_OnUnregisterPlayersCallback, const EOS_Sessions_UnregisterPlayersCallbackInfo* Data);

/** The most recent version of the EOS_SessionModification_SetBucketId API. */
#define EOS_SESSIONMODIFICATION_SETBUCKETID_API_LATEST 1

/**
 * Input parameters for the EOS_SessionModification_SetBucketId function.
 */
EOS_STRUCT(EOS_SessionModification_SetBucketIdOptions, (
	/** API Version: Set this to EOS_SESSIONMODIFICATION_SETBUCKETID_API_LATEST. */
	int32_t ApiVersion;
	/** The new bucket id associated with the session */
	const char* BucketId;
));

/** The most recent version of the EOS_SessionModification_SetHostAddress API. */
#define EOS_SESSIONMODIFICATION_SETHOSTADDRESS_API_LATEST 1

/**
 * Input parameters for the EOS_SessionModification_SetHostAddress function.
 */
EOS_STRUCT(EOS_SessionModification_SetHostAddressOptions, (
	/** API Version: Set this to EOS_SESSIONMODIFICATION_SETHOSTADDRESS_API_LATEST. */
	int32_t ApiVersion;
	/** A string representing the host address for the session, its meaning is up to the application */
	const char* HostAddress;
));

/** Permission level gets more restrictive further down */
EOS_ENUM(EOS_EOnlineSessionPermissionLevel,
	/** Anyone can find this session as long as it isn't full */
	EOS_OSPF_PublicAdvertised = 0,
	/** Players who have access to presence can see this session */
	EOS_OSPF_JoinViaPresence = 1,
	/** Only players with invites registered can see this session */
	EOS_OSPF_InviteOnly = 2
);

/** The most recent version of the EOS_SessionModification_SetPermissionLevel API. */
#define EOS_SESSIONMODIFICATION_SETPERMISSIONLEVEL_API_LATEST 1

/**
 * Input parameters for the EOS_SessionModification_SetPermissionLevel function.
 */
EOS_STRUCT(EOS_SessionModification_SetPermissionLevelOptions, (
	/** API Version: Set this to EOS_SESSIONMODIFICATION_SETPERMISSIONLEVEL_API_LATEST. */
	int32_t ApiVersion;
	/** Permission level to set on the sesion */
	EOS_EOnlineSessionPermissionLevel PermissionLevel;
));

/** The most recent version of the EOS_SessionModification_SetJoinInProgressAllowed API. */
#define EOS_SESSIONMODIFICATION_SETJOININPROGRESSALLOWED_API_LATEST 1

/**
 * Input parameters for the EOS_SessionModification_SetJoinInProgressAllowed function.
 */
EOS_STRUCT(EOS_SessionModification_SetJoinInProgressAllowedOptions, (
	/** API Version: Set this to EOS_SESSIONMODIFICATION_SETJOININPROGRESSALLOWED_API_LATEST. */
	int32_t ApiVersion;
	/** Does the session allow join in progress */
	EOS_Bool bAllowJoinInProgress;
));

/** Maximum number of players allowed in a single session */
#define EOS_SESSIONS_MAXREGISTEREDPLAYERS 1000

/** The most recent version of the EOS_SessionModification_SetMaxPlayers API. */
#define EOS_SESSIONMODIFICATION_SETMAXPLAYERS_API_LATEST 1

/**
 * Input parameters for the EOS_SessionModification_SetMaxPlayers function.
 */
EOS_STRUCT(EOS_SessionModification_SetMaxPlayersOptions, (
	/** API Version: Set this to EOS_SESSIONMODIFICATION_SETMAXPLAYERS_API_LATEST. */
	int32_t ApiVersion;
	/** Max number of players to allow in the session */
	uint32_t MaxPlayers;
));

/** The most recent version of the EOS_SessionModification_SetInvitesAllowed API. */
#define EOS_SESSIONMODIFICATION_SETINVITESALLOWED_API_LATEST 1

/**
 * Input parameters for the EOS_SessionModification_SetInvitesAllowed function.
 */
EOS_STRUCT(EOS_SessionModification_SetInvitesAllowedOptions, (
	/** API Version: Set this to EOS_SESSIONMODIFICATION_SETINVITESALLOWED_API_LATEST. */
	int32_t ApiVersion;
	/** If true then invites can currently be sent for the associated session */
	EOS_Bool bInvitesAllowed;
));


/** Search for a matching bucket ID (value is string) */
#define EOS_SESSIONS_SEARCH_BUCKET_ID "bucket"
/** Search for empty servers only (value is true/false) */
#define EOS_SESSIONS_SEARCH_EMPTY_SERVERS_ONLY "emptyonly"
/** Search for non empty servers only (value is true/false) */
#define EOS_SESSIONS_SEARCH_NONEMPTY_SERVERS_ONLY "nonemptyonly"
/** Search for a match with min free space (value is int) */
#define EOS_SESSIONS_SEARCH_MINSLOTSAVAILABLE "minslotsavailable"

/** The most recent version of the EOS_Sessions_AttributeData struct. */
#define EOS_SESSIONS_ATTRIBUTEDATA_API_LATEST 1

/** DEPRECATED! Use EOS_SESSIONS_ATTRIBUTEDATA_API_LATEST instead. */
#define EOS_SESSIONS_SESSIONATTRIBUTEDATA_API_LATEST EOS_SESSIONS_ATTRIBUTEDATA_API_LATEST

/**
 * Contains information about both session and search parameter attribution
 */
EOS_STRUCT(EOS_Sessions_AttributeData, (
	/** API Version: Set this to EOS_SESSIONS_ATTRIBUTEDATA_API_LATEST. */
	int32_t ApiVersion;
	/** Name of the session attribute */
	const char* Key;
	union
	{
		/** Stored as an 8 byte integer */
		int64_t AsInt64;
		/** Stored as a double precision floating point */
		double AsDouble;
		/** Stored as a boolean */
		EOS_Bool AsBool;
		/** Stored as a null terminated UTF8 string */
		const char* AsUtf8;
	} Value;

	/** Type of value stored in the union */
	EOS_ESessionAttributeType ValueType;
));

/** The most recent version of the EOS_ActiveSession_CopyInfo function. */
#define EOS_ACTIVESESSION_COPYINFO_API_LATEST 1

/**
 * Input parameters for the EOS_ActiveSession_CopyInfo function.
 */
EOS_STRUCT(EOS_ActiveSession_CopyInfoOptions, (
	/** API Version: Set this to EOS_ACTIVESESSION_COPYINFO_API_LATEST. */
	int32_t ApiVersion;
));

/** The most recent version of the EOS_ActiveSession_GetRegisteredPlayerCount function. */
#define EOS_ACTIVESESSION_GETREGISTEREDPLAYERCOUNT_API_LATEST 1

/**
 * Input parameters for the EOS_ActiveSession_GetRegisteredPlayerCount function.
 */
EOS_STRUCT(EOS_ActiveSession_GetRegisteredPlayerCountOptions, (
	/** API Version: Set this to EOS_ACTIVESESSION_GETREGISTEREDPLAYERCOUNT_API_LATEST. */
	int32_t ApiVersion;
));

/** The most recent version of the EOS_ActiveSession_GetRegisteredPlayerByIndex function. */
#define EOS_ACTIVESESSION_GETREGISTEREDPLAYERBYINDEX_API_LATEST 1

/**
 * Input parameters for the EOS_ActiveSession_GetRegisteredPlayerByIndex function.
 */
EOS_STRUCT(EOS_ActiveSession_GetRegisteredPlayerByIndexOptions, (
	/** API Version: Set this to EOS_ACTIVESESSION_GETREGISTEREDPLAYERBYINDEX_API_LATEST. */
	int32_t ApiVersion;
	/** Index of the registered player to retrieve */
	uint32_t PlayerIndex;
));

/** The most recent version of the EOS_SessionDetails_Attribute struct. */
#define EOS_SESSIONDETAILS_ATTRIBUTE_API_LATEST 1

/** DEPRECATED! Use EOS_SESSIONDETAILS_ATTRIBUTE_API_LATEST instead. */
#define EOS_SESSIONS_SESSIONATTRIBUTE_API_LATEST EOS_SESSIONDETAILS_ATTRIBUTE_API_LATEST

/**
 *  An attribution value and its advertisement setting stored with a session.  
 */
EOS_STRUCT(EOS_SessionDetails_Attribute, (
	/** API Version: Set this to EOS_SESSIONDETAILS_ATTRIBUTE_API_LATEST. */
	int32_t ApiVersion;
	/** Key/Value pair describing the attribute */
	EOS_Sessions_AttributeData* Data;
	/** Is this attribution advertised with the backend or simply stored locally */
	EOS_ESessionAttributeAdvertisementType AdvertisementType;
));

/**
 * Release the memory associated with a session attribute. This must be called on data retrieved from EOS_SessionDetails_CopySessionAttributeByIndex.
 *
 * @param SessionAttribute - The session attribute to release
 *
 * @see EOS_SessionDetails_CopySessionAttributeByIndex
 */
EOS_DECLARE_FUNC(void) EOS_SessionDetails_Attribute_Release(EOS_SessionDetails_Attribute* SessionAttribute);


/** The most recent version of the EOS_SessionModification_AddAttribute API. */
#define EOS_SESSIONMODIFICATION_ADDATTRIBUTE_API_LATEST 1

/**
 * Input parameters for the EOS_SessionModification_AddAttribute function.
 */
EOS_STRUCT(EOS_SessionModification_AddAttributeOptions, (
	/** API Version: Set this to EOS_SESSIONMODIFICATION_ADDATTRIBUTE_API_LATEST. */
	int32_t ApiVersion;
	/** Key/Value pair describing the attribute to add to the session */
	const EOS_Sessions_AttributeData* SessionAttribute;
	/** Is this attribution advertised with the backend or simply stored locally */
	EOS_ESessionAttributeAdvertisementType AdvertisementType;
));

/** The most recent version of the EOS_SessionModification_RemoveAttribute API. */
#define EOS_SESSIONMODIFICATION_REMOVEATTRIBUTE_API_LATEST 1

/**
 * Input parameters for the EOS_SessionModification_RemoveAttribute function.
 */
EOS_STRUCT(EOS_SessionModification_RemoveAttributeOptions, (
	/** API Version: Set this to EOS_SESSIONDETAILS_SETTINGS_API_LATEST. */
	int32_t ApiVersion;
	/** Session attribute to remove from the session */
	const char* Key;
));

/** Maximum number of search results allowed with a given query */
#define EOS_SESSIONS_MAX_SEARCH_RESULTS 200

/** The most recent version of the EOS_SessionSearch_SetMaxResults API. */
#define EOS_SESSIONSEARCH_SETMAXSEARCHRESULTS_API_LATEST 1

/**
 * Input parameters for the EOS_SessionSearch_SetMaxResults function.
 */
EOS_STRUCT(EOS_SessionSearch_SetMaxResultsOptions, (
	/** API Version: Set this to EOS_SESSIONSEARCH_SETMAXSEARCHRESULTS_API_LATEST. */
	int32_t ApiVersion;
	/** Maximum number of search results returned with this query, may not exceed EOS_SESSIONS_MAX_SEARCH_RESULTS */
	uint32_t MaxSearchResults;
));

/** The most recent version of the EOS_SessionSearch_Find API. */
#define EOS_SESSIONSEARCH_FIND_API_LATEST 2

/**
 * Input parameters for the EOS_SessionSearch_Find function.
 */
EOS_STRUCT(EOS_SessionSearch_FindOptions, (
	/** API Version: Set this to EOS_SESSIONDETAILS_SETTINGS_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user who is searching */
	EOS_ProductUserId LocalUserId;
));

EOS_STRUCT(EOS_SessionSearch_FindCallbackInfo, (
	/** The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_SessionSearch_Find */
	void* ClientData;
));

/**
 * Function prototype definition for callbacks passed to EOS_SessionSearch_Find
 * @param Data A EOS_SessionSearch_FindCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_SessionSearch_OnFindCallback, const EOS_SessionSearch_FindCallbackInfo* Data);

/** The most recent version of the EOS_SessionSearch_GetSearchResultCount API. */
#define EOS_SESSIONSEARCH_GETSEARCHRESULTCOUNT_API_LATEST 1

/**
 * Input parameters for the EOS_SessionSearch_GetSearchResultCount function.
 */
EOS_STRUCT(EOS_SessionSearch_GetSearchResultCountOptions, (
	/** API Version: Set this to EOS_SESSIONDETAILS_SETTINGS_API_LATEST. */
	int32_t ApiVersion;
));
		
/** The most recent version of the EOS_SessionSearch_CopySearchResultByIndex API. */
#define EOS_SESSIONSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST 1

/**
 * Input parameters for the EOS_SessionSearch_CopySearchResultByIndex function.
 */
EOS_STRUCT(EOS_SessionSearch_CopySearchResultByIndexOptions, (
	/** API Version: Set this to EOS_SESSIONDETAILS_SETTINGS_API_LATEST. */
	int32_t ApiVersion;
	/** 
	 * The index of the session to retrieve within the completed search query
	 * @see EOS_SessionSearch_GetSearchResultCount
	 */
	uint32_t SessionIndex;
));

/** The most recent version of the EOS_SessionSearch_SetSessionId API. */
#define EOS_SESSIONSEARCH_SETSESSIONID_API_LATEST 1

/**
 * Input parameters for the EOS_SessionSearch_SetSessionId function.
 */
EOS_STRUCT(EOS_SessionSearch_SetSessionIdOptions, (
	/** API Version: Set this to EOS_SESSIONDETAILS_SETTINGS_API_LATEST. */
	int32_t ApiVersion;
	/** Search sessions for a specific session ID, returning at most one session */
	const char* SessionId;
));

/** The most recent version of the EOS_SessionSearch_SetTargetUserId API. */
#define EOS_SESSIONSEARCH_SETTARGETUSERID_API_LATEST 1

/**
 * Input parameters for the EOS_SessionSearch_SetTargetUserId function.
 */
EOS_STRUCT(EOS_SessionSearch_SetTargetUserIdOptions, (
	/** API Version: Set this to EOS_SESSIONDETAILS_SETTINGS_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID to find; return any sessions where the user matching this ID is currently registered */
	EOS_ProductUserId TargetUserId;
));

/** The most recent version of the EOS_SessionSearch_SetParameter API. */
#define EOS_SESSIONSEARCH_SETPARAMETER_API_LATEST 1

/**
 * Input parameters for the EOS_SessionSearch_SetParameter function.
 *
 * A search key may be set more than once to make multiple comparisons
 * The two comparisons are AND'd together
 * (ie, Key GREATER_THAN 5, Key NOT_EQUALS 10)
 */
EOS_STRUCT(EOS_SessionSearch_SetParameterOptions, (
	/** API Version: Set this to EOS_SESSIONDETAILS_SETTINGS_API_LATEST. */
	int32_t ApiVersion;
	/** Search parameter describing a key and a value to compare */
	const EOS_Sessions_AttributeData* Parameter;
	/** The type of comparison to make against the search parameter */
	EOS_EOnlineComparisonOp ComparisonOp;
));

/** The most recent version of the EOS_SessionSearch_RemoveParameter API. */
#define EOS_SESSIONSEARCH_REMOVEPARAMETER_API_LATEST 1

/**
 * Input parameters for the EOS_SessionSearch_RemoveParameter function.
 *
 * Removal requires both the key and its comparator in order to remove as the same key can be used in more than one operation
 */
EOS_STRUCT(EOS_SessionSearch_RemoveParameterOptions, (
	/** API Version: Set this to EOS_SESSIONDETAILS_SETTINGS_API_LATEST. */
	int32_t ApiVersion;
	/** Search parameter key to remove from the search */
	const char* Key;
	/** Search comparison operation associated with the key to remove */
	EOS_EOnlineComparisonOp ComparisonOp;
));

/** The most recent version of the EOS_SessionDetails_Settings struct. */
#define EOS_SESSIONDETAILS_SETTINGS_API_LATEST 2

/** Common settings associated with a single session */
EOS_STRUCT(EOS_SessionDetails_Settings, (
	/** API Version: Set this to EOS_SESSIONDETAILS_SETTINGS_API_LATEST. */
	int32_t ApiVersion;
	/** The main indexed parameter for this session, can be any string (ie "Region:GameMode") */
	const char* BucketId;
	/** Number of total players allowed in the session */
	uint32_t NumPublicConnections;
	/** Are players allowed to join the session while it is in the "in progress" state */
	EOS_Bool bAllowJoinInProgress;
	/** Permission level describing allowed access to the session when joining or searching for the session */
	EOS_EOnlineSessionPermissionLevel PermissionLevel;
	/** Are players allowed to send invites for the session */
	EOS_Bool bInvitesAllowed;
));

/** The most recent version of the EOS_SessionDetails_Info struct. */
#define EOS_SESSIONDETAILS_INFO_API_LATEST 1

/** Internal details about a session, found on both active sessions and within search results */
EOS_STRUCT(EOS_SessionDetails_Info, (
	/** API Version: Set this to EOS_SESSIONDETAILS_INFO_API_LATEST. */
	int32_t ApiVersion;
	/** Session ID assigned by the backend service */
	const char* SessionId;
	/** IP address of this session as visible by the backend service */
	const char* HostAddress;
	/** Number of remaining open spaces on the session (NumPublicConnections - RegisteredPlayers */
	uint32_t NumOpenPublicConnections;
	/** Reference to the additional settings associated with this session */
	const EOS_SessionDetails_Settings* Settings;
));

EOS_DECLARE_FUNC(void) EOS_SessionDetails_Info_Release(EOS_SessionDetails_Info* SessionInfo);

/** The most recent version of the EOS_SessionDetails_CopyInfo API. */
#define EOS_SESSIONDETAILS_COPYINFO_API_LATEST 1

/**
 * Input parameters for the EOS_SessionDetails_CopyInfo function.
 */
EOS_STRUCT(EOS_SessionDetails_CopyInfoOptions, (
	/** API Version: Set this to EOS_SESSIONDETAILS_COPYINFO_API_LATEST. */
	int32_t ApiVersion;
));

/** The most recent version of the EOS_SessionDetails_GetSessionAttributeCount API. */
#define EOS_SESSIONDETAILS_GETSESSIONATTRIBUTECOUNT_API_LATEST 1

/**
 * Input parameters for the EOS_SessionDetails_GetSessionAttributeCount function.
 */
EOS_STRUCT(EOS_SessionDetails_GetSessionAttributeCountOptions, (
	/** API Version: Set this to EOS_SESSIONDETAILS_GETSESSIONATTRIBUTECOUNT_API_LATEST. */
	int32_t ApiVersion;
));

/** The most recent version of the EOS_SessionDetails_CopySessionAttributeByIndex API. */
#define EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYINDEX_API_LATEST 1

/**
 * Input parameters for the EOS_SessionDetails_CopySessionAttributeByIndex function.
 */
EOS_STRUCT(EOS_SessionDetails_CopySessionAttributeByIndexOptions, (
	/** API Version: Set this to EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYINDEX_API_LATEST. */
	int32_t ApiVersion;
	/** 
	 * The index of the attribute to retrieve
	 * @see EOS_SessionDetails_GetSessionAttributeCount
	 */
	uint32_t AttrIndex;
));

/** The most recent version of the EOS_SessionDetails_CopySessionAttributeByKey API. */
#define EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYKEY_API_LATEST 1

/**
 * Input parameters for the EOS_SessionDetails_CopySessionAttributeByKey function.
 */
EOS_STRUCT(EOS_SessionDetails_CopySessionAttributeByKeyOptions, (
	/** API Version: Set this to EOS_SESSIONDETAILS_COPYSESSIONATTRIBUTEBYKEY_API_LATEST. */
	int32_t ApiVersion;
	/** The name of the key to get the session attribution for 
	 * @see EOS_SessionModification_AddAttribute
	 */
	const char* AttrKey;
));

/** The most recent version of the EOS_ActiveSession_Info struct. */
#define EOS_ACTIVESESSION_INFO_API_LATEST 1

/**
 * Top level details about an active session
 */
EOS_STRUCT(EOS_ActiveSession_Info, (
	/** API Version: Set this to EOS_ACTIVESESSION_INFO_API_LATEST. */
	int32_t ApiVersion;
	/** Name of the session */
	const char* SessionName;
	/** The Product User ID of the local user who created or joined the session */
	EOS_ProductUserId LocalUserId;
	/** Current state of the session */
	EOS_EOnlineSessionState State;
	/** Session details */
	const EOS_SessionDetails_Info* SessionDetails;
));

/**
 * Release the memory associated with an EOS_ActiveSession_Info structure. This must be called on data retrieved from EOS_ActiveSession_CopyInfo.
 *
 * @param ActiveSessionInfo - The active session structure to be released
 *
 * @see EOS_ActiveSession_Info
 * @see EOS_ActiveSession_CopyInfo
 */
EOS_DECLARE_FUNC(void) EOS_ActiveSession_Info_Release(EOS_ActiveSession_Info* ActiveSessionInfo);

/** The most recent version of the EOS_Sessions_CopyActiveSessionHandle API. */
#define EOS_SESSIONS_COPYACTIVESESSIONHANDLE_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_CopyActiveSessionHandle function.
 */
EOS_STRUCT(EOS_Sessions_CopyActiveSessionHandleOptions, (
	/** API Version: Set this to EOS_SESSIONS_COPYACTIVESESSIONHANDLE_API_LATEST. */
	int32_t ApiVersion;
	/** Name of the session for which to retrieve a session handle */
	const char* SessionName;
));

/** The most recent version of the EOS_Sessions_AddNotifySessionInviteReceived API. */
#define EOS_SESSIONS_ADDNOTIFYSESSIONINVITERECEIVED_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_AddNotifySessionInviteReceived function.
 */
EOS_STRUCT(EOS_Sessions_AddNotifySessionInviteReceivedOptions, (
	/** API Version: Set this to EOS_SESSIONS_ADDNOTIFYSESSIONINVITERECEIVED_API_LATEST. */
	int32_t ApiVersion;
));

/**
 * Output parameters for the EOS_Sessions_OnSessionInviteReceivedCallback function.
 */
EOS_STRUCT(EOS_Sessions_SessionInviteReceivedCallbackInfo, (
	/** Context that was passed into EOS_Sessions_AddNotifySessionInviteReceived */
	void* ClientData;
	/** The Product User ID of the user who received the invite */
	EOS_ProductUserId LocalUserId;
	/** The Product User ID of the user who sent the invitation */
	EOS_ProductUserId TargetUserId;
	/** Invite ID used to retrieve the actual session details */
	const char* InviteId;
));

/**
 * Function prototype definition for notifications that come from EOS_Sessions_AddNotifySessionInviteReceived
 *
 * @param Data A EOS_Sessions_SessionInviteReceivedCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Sessions_OnSessionInviteReceivedCallback, const EOS_Sessions_SessionInviteReceivedCallbackInfo* Data);

/** The most recent version of the EOS_Sessions_AddNotifySessionInviteAccepted API. */
#define EOS_SESSIONS_ADDNOTIFYSESSIONINVITEACCEPTED_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_AddNotifySessionInviteAccepted function.
 */
EOS_STRUCT(EOS_Sessions_AddNotifySessionInviteAcceptedOptions, (
	/** API Version: Set this to EOS_SESSIONS_ADDNOTIFYSESSIONINVITEACCEPTED_API_LATEST. */
	int32_t ApiVersion;
));

/**
 * Output parameters for the EOS_Sessions_OnSessionInviteAcceptedCallback function.
 */
EOS_STRUCT(EOS_Sessions_SessionInviteAcceptedCallbackInfo, (
	/** Context that was passed into EOS_Sessions_AddNotifySessionInviteAccepted */
	void* ClientData;
	/** Session ID that should be used for joining */
	const char* SessionId;
	/** The Product User ID of the user who accepted the invitation */
	EOS_ProductUserId LocalUserId;
	/** The Product User ID of the user who sent the invitation */
	EOS_ProductUserId TargetUserId;
	/** Invite ID that was accepted */
	const char* InviteId;
));

/**
 * Function prototype definition for notifications that come from EOS_Sessions_AddNotifySessionInviteAccepted
 *
 * @param Data A EOS_Sessions_SessionInviteAcceptedCallbackInfo containing the output information and result
 *
 * @note The session for the invite must be joined.
 *
 * @see EOS_Sessions_CopySessionHandleByInviteId
 * @see EOS_Sessions_JoinSession
 */
EOS_DECLARE_CALLBACK(EOS_Sessions_OnSessionInviteAcceptedCallback, const EOS_Sessions_SessionInviteAcceptedCallbackInfo* Data);

/** The most recent version of the EOS_Sessions_AddNotifyJoinSessionAccepted API. */
#define EOS_SESSIONS_ADDNOTIFYJOINSESSIONACCEPTED_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_AddNotifyJoinSessionAccepted function.
 */
EOS_STRUCT(EOS_Sessions_AddNotifyJoinSessionAcceptedOptions, (
	/** API Version: Set this to EOS_SESSIONS_ADDNOTIFYJOINSESSIONACCEPTED_API_LATEST. */
	int32_t ApiVersion;
));

/**
 * Output parameters for the EOS_Sessions_OnJoinSessionAcceptedCallback function.
 */
EOS_STRUCT(EOS_Sessions_JoinSessionAcceptedCallbackInfo, (
	/** Context that was passed into EOS_Sessions_AddNotifyJoinSessionAccepted */
	void* ClientData;
	/** The Product User ID for the user who initialized the game */
	EOS_ProductUserId LocalUserId;
	/** 
	 * The UI Event associated with this Join Game event.
	 * This should be used with EOS_Sessions_CopySessionHandleByUiEventId to get a handle to be used
	 * when calling EOS_Sessions_JoinSession.
	 */
	EOS_UI_EventId UiEventId;
));

/**
 * Function prototype definition for notifications that come from EOS_Sessions_AddNotifyJoinSessionAccepted
 *
 * @param Data A EOS_Sessions_JoinSessionAcceptedCallbackInfo containing the output information and result
 *
 * @note The session for the join game must be joined.
 *
 * @see EOS_Sessions_CopySessionHandleByUiEventId
 * @see EOS_Sessions_JoinSession
 */
EOS_DECLARE_CALLBACK(EOS_Sessions_OnJoinSessionAcceptedCallback, const EOS_Sessions_JoinSessionAcceptedCallbackInfo* Data);


/** The most recent version of the EOS_Sessions_CopySessionHandleByInviteId API. */
#define EOS_SESSIONS_COPYSESSIONHANDLEBYINVITEID_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_CopySessionHandleByInviteId function.
 */
EOS_STRUCT(EOS_Sessions_CopySessionHandleByInviteIdOptions, (
	/** API Version: Set this to EOS_SESSIONS_COPYSESSIONHANDLEBYINVITEID_API_LATEST. */
	int32_t ApiVersion;
	/** Invite ID for which to retrieve a session handle */
	const char* InviteId;
));

/** The most recent version of the EOS_Sessions_CopySessionHandleByUiEventId API. */
#define EOS_SESSIONS_COPYSESSIONHANDLEBYUIEVENTID_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_CopySessionHandleByUiEventId function.
 */
EOS_STRUCT(EOS_Sessions_CopySessionHandleByUiEventIdOptions, (
	/** API Version: Set this to EOS_SESSIONS_COPYSESSIONHANDLEBYUIEVENTID_API_LATEST. */
	int32_t ApiVersion;
	/** UI Event associated with the session */
	EOS_UI_EventId UiEventId;
));

/** The most recent version of the EOS_Sessions_CopySessionHandleForPresence API. */
#define EOS_SESSIONS_COPYSESSIONHANDLEFORPRESENCE_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_CopySessionHandleForPresence function.
 */
EOS_STRUCT(EOS_Sessions_CopySessionHandleForPresenceOptions, (
	/** API Version: Set this to EOS_SESSIONS_COPYSESSIONHANDLEFORPRESENCE_API_LATEST. */
	int32_t ApiVersion;
	/** The Product User ID of the local user associated with the session */
	EOS_ProductUserId LocalUserId;
));

/** The most recent version of the EOS_Sessions_IsUserInSession API. */
#define EOS_SESSIONS_ISUSERINSESSION_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_IsUserInSession function.
 */
EOS_STRUCT(EOS_Sessions_IsUserInSessionOptions, (
	/** API Version: Set this to EOS_SESSIONS_ISUSERINSESSION_API_LATEST. */
	int32_t ApiVersion;
	/** Active session name to search within */
	const char* SessionName;
	/** Product User ID to search for in the session */
	EOS_ProductUserId TargetUserId;
));

/** The most recent version of the EOS_Sessions_DumpSessionState API. */
#define EOS_SESSIONS_DUMPSESSIONSTATE_API_LATEST 1

/**
 * Input parameters for the EOS_Sessions_DumpSessionState function.
 */
EOS_STRUCT(EOS_Sessions_DumpSessionStateOptions, (
	/** API Version: Set this to EOS_SESSIONS_DUMPSESSIONSTATE_API_LATEST. */
	int32_t ApiVersion;
	/** Name of the session */
	const char* SessionName;
));

#pragma pack(pop)
