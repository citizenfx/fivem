// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "eos_common.h"
#include "eos_ui_types.h"

#pragma pack(push, 8)

EXTERN_C typedef struct EOS_PresenceHandle* EOS_HPresence;
EXTERN_C typedef struct EOS_PresenceModificationHandle* EOS_HPresenceModification;

/**
 * Presence Status states of a user
 *
 * @see EOS_Presence_CopyPresence
 * @see EOS_PresenceModification_SetStatus
 */
EOS_ENUM(EOS_Presence_EStatus,
	/** The status of the account is offline or not known */
	EOS_PS_Offline = 0,
	/** The status of the account is online */
	EOS_PS_Online = 1,
	/** The status of the account is away */
	EOS_PS_Away = 2,
	/** The status of the account is away, and has been away for a while */
	EOS_PS_ExtendedAway = 3,
	/** The status of the account is do-not-disturb */
	EOS_PS_DoNotDisturb = 4
);


#define EOS_PRESENCE_DATARECORD_API_LATEST 1

/**
 * An individual presence data record that belongs to a EOS_Presence_Info object. This object is released when its parent EOS_Presence_Info object is released.
 *
 * @see EOS_Presence_Info
 */
EOS_STRUCT(EOS_Presence_DataRecord, (
	/** API Version: Set this to EOS_PRESENCE_DATARECORD_API_LATEST. */
	int32_t ApiVersion;
	/** The name of this data */
	const char* Key;
	/** The value of this data */
	const char* Value;
));


#define EOS_PRESENCE_INFO_API_LATEST 2

/**
 * All the known presence information for a specific user. This object must be released by calling EOS_Presence_Info_Release.
 *
 * @see EOS_Presence_CopyPresence
 * @see EOS_Presence_Info_Release
 */
EOS_STRUCT(EOS_Presence_Info, (
	/** API Version: Set this to EOS_PRESENCE_INFO_API_LATEST. */
	int32_t ApiVersion;
	/** The status of the user */
	EOS_Presence_EStatus Status;
	/** The Epic Online Services Account ID of the user */
	EOS_EpicAccountId UserId;
	/** The product ID that the user is logged in from */
	const char* ProductId;
	/** The version of the product the user is logged in from */
	const char* ProductVersion;
	/** The platform of that the user is logged in from */
	const char* Platform;
	/** The rich-text of the user */
	const char* RichText;
	/** The count of records available */
	int32_t RecordsCount;
	/** The first data record, or NULL if RecordsCount is not at least 1 */
	const EOS_Presence_DataRecord* Records;
	/** The user-facing name for the product the user is logged in from */
	const char* ProductName;
));


#define EOS_PRESENCE_QUERYPRESENCE_API_LATEST 1

/**
 * Data for the EOS_Presence_QueryPresence function
 */
EOS_STRUCT(EOS_Presence_QueryPresenceOptions, (
	/** API Version: Set this to EOS_PRESENCE_QUERYPRESENCE_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local, logged-in user making the request */
	EOS_EpicAccountId LocalUserId;
	/** The Epic Online Services Account ID of the user whose presence data you want to retrieve; this value must be either the user making the request, or a friend of that user */
	EOS_EpicAccountId TargetUserId;
));

/**
 * The result meta-data for a presence query.
 */
EOS_STRUCT(EOS_Presence_QueryPresenceCallbackInfo, (
	/** Result code for the operation. EOS_Success is returned for a successful query, other codes indicate an error */
	EOS_EResult ResultCode;
	/** Client-specified data passed into EOS_Presence_QueryPresence */
	void* ClientData;
	/** The Epic Online Services Account ID of the local user who made this request */
	EOS_EpicAccountId LocalUserId;
	/** The Epic Online Services Account ID of the user whose presence was potentially queried */
	EOS_EpicAccountId TargetUserId;
));

/**
 * Callback for information related to EOS_Presence_QueryPresence finishing.
 */
EOS_DECLARE_CALLBACK(EOS_Presence_OnQueryPresenceCompleteCallback, const EOS_Presence_QueryPresenceCallbackInfo* Data);


#define EOS_PRESENCE_HASPRESENCE_API_LATEST 1

/**
 * Data for the EOS_Presence_HasPresence function.
 */
EOS_STRUCT(EOS_Presence_HasPresenceOptions, (
	/** API Version: Set this to EOS_PRESENCE_HASPRESENCE_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local, logged-in user making the request */
	EOS_EpicAccountId LocalUserId;
	/** The Epic Online Services Account ID of the user whose cached presence data you want to locate */
	EOS_EpicAccountId TargetUserId;
));


#define EOS_PRESENCE_COPYPRESENCE_API_LATEST 2
/**
 * Data for the EOS_Presence_CopyPresence function.
 */
EOS_STRUCT(EOS_Presence_CopyPresenceOptions, (
	/** API Version: Set this to EOS_PRESENCE_COPYPRESENCE_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local, logged-in user making the request */
	EOS_EpicAccountId LocalUserId;
	/** The Epic Online Services Account ID of the user whose cached presence data you want to copy from the cache */
	EOS_EpicAccountId TargetUserId;
));


/**
 * Release the memory associated with an EOS_Presence_Info structure and its sub-objects. This must be called on data retrieved from EOS_Presence_CopyPresence.
 * This can be safely called on a NULL presence info object.
 *
 * @param PresenceInfo The presence info structure to be release
 */
EOS_DECLARE_FUNC(void) EOS_Presence_Info_Release(EOS_Presence_Info* PresenceInfo);


#define EOS_PRESENCE_CREATEPRESENCEMODIFICATION_API_LATEST 1
/**
 * Data for the EOS_Presence_CreatePresenceModification function.
 */
EOS_STRUCT(EOS_Presence_CreatePresenceModificationOptions, (
	/** API Version: Set this to EOS_PRESENCE_CREATEPRESENCEMODIFICATION_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user's Epic Online Services Account ID */
	EOS_EpicAccountId LocalUserId;
));


/**
 * Release the memory associated with an EOS_HPresenceModification handle. This must be called on Handles retrieved from EOS_Presence_CreatePresenceModification.
 * This can be safely called on a NULL presence modification handle. This also may be safely called while a call to SetPresence is still pending.
 *
 * @param PresenceModificationHandle The presence modification handle to release
 *
 * @see EOS_Presence_CreatePresenceModification
 */
EOS_DECLARE_FUNC(void) EOS_PresenceModification_Release(EOS_HPresenceModification PresenceModificationHandle);


#define EOS_PRESENCE_SETPRESENCE_API_LATEST 1
/**
 * Data for the EOS_Presence_SetPresence function.
 */
EOS_STRUCT(EOS_Presence_SetPresenceOptions, (
	/** API Version: Set this to EOS_PRESENCE_SETPRESENCE_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user's Epic Online Services Account ID */
	EOS_EpicAccountId LocalUserId;
	/** The handle to the presence update */
	EOS_HPresenceModification PresenceModificationHandle;
));

/**
 * The result meta-data from setting a user's presence.
 */
EOS_STRUCT(EOS_Presence_SetPresenceCallbackInfo, (
	/** Result code for the operation. EOS_Success is returned if presence was successfully set, other codes indicate an error */
	EOS_EResult ResultCode;
	/** Client-specified data passed into EOS_Presence_SetPresence */
	void* ClientData;
	/** The Epic Online Services Account ID of the local user that had their presence set */
	EOS_EpicAccountId LocalUserId;
));

/**
 * Callback for information related to EOS_Presence_SetPresence finishing.
 */
EOS_DECLARE_CALLBACK(EOS_Presence_SetPresenceCompleteCallback, const EOS_Presence_SetPresenceCallbackInfo* Data);


#define EOS_PRESENCE_ADDNOTIFYONPRESENCECHANGED_API_LATEST 1
/**
 * Data for the EOS_Presence_AddNotifyOnPresenceChanged function.
 */
EOS_STRUCT(EOS_Presence_AddNotifyOnPresenceChangedOptions, (
	/** API Version: Set this to EOS_PRESENCE_ADDNOTIFYONPRESENCECHANGED_API_LATEST. */
	int32_t ApiVersion;
));

/**
 * Data containing which users presence has changed
 */
EOS_STRUCT(EOS_Presence_PresenceChangedCallbackInfo, (
	/** Client-specified data passed into EOS_Presence_AddNotifyOnPresenceChanged */
	void* ClientData;
	/** The Epic Online Services Account ID of the local user who is being informed for PresenceUserId's presence change */
	EOS_EpicAccountId LocalUserId;
	/** The Epic Online Services Account ID of the user who had their presence changed */
	EOS_EpicAccountId PresenceUserId;
));

/** The most recent version of the EOS_Presence_AddNotifyJoinGameAccepted API. */
#define EOS_PRESENCE_ADDNOTIFYJOINGAMEACCEPTED_API_LATEST 2

EOS_STRUCT(EOS_Presence_AddNotifyJoinGameAcceptedOptions, (
	/** API Version: Set this to EOS_PRESENCE_ADDNOTIFYJOINGAMEACCEPTED_API_LATEST. */
	int32_t ApiVersion;
));

/**
 * Output parameters for the EOS_Presence_OnJoinGameAcceptedCallback Function.
 */
EOS_STRUCT(EOS_Presence_JoinGameAcceptedCallbackInfo, (
	/** Context that was passed into EOS_Presence_AddNotifyJoinGameAccepted */
	void* ClientData;
	/** 
	 * The Join Info custom game-data string to use to join the target user.
	 * Set to a null pointer to delete the value.
	 */
	const char* JoinInfo;
	/** The Epic Online Services Account ID of the user who accepted the invitation */
	EOS_EpicAccountId LocalUserId;
	/** The Epic Online Services Account ID of the user who sent the invitation */
	EOS_EpicAccountId TargetUserId;
	/** 
	 * If the value is not EOS_UI_EVENTID_INVALID then it must be passed back to the SDK using EOS_UI_AcknowledgeEventId.
	 * This should be done after attempting to join the game and either succeeding or failing to connect.
	 * This is necessary to allow the Social Overlay UI to manage the `Join` button.
	 */
	EOS_UI_EventId UiEventId;
));

/**
 * Function prototype definition for notifications that come from EOS_Presence_AddNotifyJoinGameAccepted
 *
 * @param Data A EOS_Presence_JoinGameAcceptedCallbackInfo containing the output information and result
 * 
 * @note EOS_UI_AcknowledgeEventId must be called with any valid UiEventId passed via the data.
 */
EOS_DECLARE_CALLBACK(EOS_Presence_OnJoinGameAcceptedCallback, const EOS_Presence_JoinGameAcceptedCallbackInfo* Data);

/**
 * Callback for information related to notifications from EOS_Presence_AddNotifyOnPresenceChanged triggering.
 */
EOS_DECLARE_CALLBACK(EOS_Presence_OnPresenceChangedCallback, const EOS_Presence_PresenceChangedCallbackInfo* Data);

#define EOS_PRESENCE_GETJOININFO_API_LATEST 1
/**
 * Data for the EOS_Presence_GetJoinInfo function.
 */
EOS_STRUCT(EOS_Presence_GetJoinInfoOptions, (
	/** API Version: Set this to EOS_PRESENCE_GETJOININFO_API_LATEST. */
	int32_t ApiVersion;
	/** The local user's Epic Online Services Account ID */
	EOS_EpicAccountId LocalUserId;
	/** The Epic Online Services Account ID to query for join info; this value must either be a logged-in local user, or a friend of that user */
	EOS_EpicAccountId TargetUserId;
));

#define EOS_PRESENCEMODIFICATION_JOININFO_MAX_LENGTH EOS_PRESENCE_DATA_MAX_VALUE_LENGTH

#define EOS_PRESENCEMODIFICATION_SETJOININFO_API_LATEST 1
/**
 * Data for the EOS_PresenceModification_SetJoinInfo function.
 */
EOS_STRUCT(EOS_PresenceModification_SetJoinInfoOptions, (
	/** API Version: Set this to EOS_PRESENCEMODIFICATION_SETJOININFO_API_LATEST. */
	int32_t ApiVersion;
	/** 
	 * The string which will be advertised as this player's join info.
	 * An application is expected to freely define the meaning of this string to use for connecting to an active game session.
	 * The string should not exceed EOS_PRESENCEMODIFICATION_JOININFO_MAX_LENGTH in length.
	 * This affects the ability of the Social Overlay to show game related actions to take in the player's social graph.
	 *
	 * @note The Social Overlay can handle only one of the following three options at a time:
	 * * using the bPresenceEnabled flags within the Sessions interface
	 * * using the bPresenceEnabled flags within the Lobby interface
	 * * using EOS_PresenceModification_SetJoinInfo
	 *
	 * @see EOS_Lobby_CreateLobbyOptions
	 * @see EOS_Lobby_JoinLobbyOptions
	 * @see EOS_Sessions_CreateSessionModificationOptions
	 * @see EOS_Sessions_JoinSessionOptions
	 */
	const char* JoinInfo;
));


/**
 * The maximum of allowed individual pieces of data a user may have. This value is subject to change and data structures should be designed to allow for greater
 * numbers than this.
 */
#define EOS_PRESENCE_DATA_MAX_KEYS 32

/**
 * The maximum allowed length a data's key may be. This value is subject to change and data structures should be designed to allow for greater numbers than this.
 */
#define EOS_PRESENCE_DATA_MAX_KEY_LENGTH 64

/**
 * The maximum allowed length a data's value may be. This value is subject to change and data structures should be designed to allow for greater numbers than this.
 */
#define EOS_PRESENCE_DATA_MAX_VALUE_LENGTH 255

/**
 * The maximum allowed length a user's rich text string may be. This value is subject to change and data structures should be designed to allow for greater numbers
 * than this.
 */
#define EOS_PRESENCE_RICH_TEXT_MAX_VALUE_LENGTH 255

/** The most recent version of the EOS_PresenceModification_SetStatus API. */
#define EOS_PRESENCEMODIFICATION_SETSTATUS_API_LATEST 1

/** DEPRECATED! Use EOS_PRESENCEMODIFICATION_SETSTATUS_API_LATEST instead. */
#define EOS_PRESENCE_SETSTATUS_API_LATEST EOS_PRESENCEMODIFICATION_SETSTATUS_API_LATEST

/**
 * Data for the EOS_PresenceModification_SetStatus function.
 */
EOS_STRUCT(EOS_PresenceModification_SetStatusOptions, (
	/** API Version: Set this to EOS_PRESENCEMODIFICATION_SETSTATUS_API_LATEST. */
	int32_t ApiVersion;
	/** The status of the user */
	EOS_Presence_EStatus Status;
));

/** The most recent version of the EOS_PresenceModification_SetRawRichText function. */
#define EOS_PRESENCEMODIFICATION_SETRAWRICHTEXT_API_LATEST 1

/** DEPRECATED! Use EOS_PRESENCEMODIFICATION_SETRAWRICHTEXT_API_LATEST instead. */
#define EOS_PRESENCE_SETRAWRICHTEXT_API_LATEST EOS_PRESENCEMODIFICATION_SETRAWRICHTEXT_API_LATEST

/**
 * Data for the EOS_PresenceModification_SetRawRichText API.
 */
EOS_STRUCT(EOS_PresenceModification_SetRawRichTextOptions, (
	/** API Version: Set this to EOS_PRESENCEMODIFICATION_SETRAWRICHTEXT_API_LATEST. */
	int32_t ApiVersion;
	/** The status of the user */
	const char* RichText;
));

/** The most recent version of the EOS_PresenceModification_SetData API. */
#define EOS_PRESENCEMODIFICATION_SETDATA_API_LATEST 1

/** DEPRECATED! Use EOS_PRESENCEMODIFICATION_SETDATA_API_LATEST instead. */
#define EOS_PRESENCE_SETDATA_API_LATEST EOS_PRESENCEMODIFICATION_SETDATA_API_LATEST

/**
 * Data for the EOS_PresenceModification_SetData function.
 */
EOS_STRUCT(EOS_PresenceModification_SetDataOptions, (
	/** API Version: Set this to EOS_PRESENCEMODIFICATION_SETDATA_API_LATEST. */
	int32_t ApiVersion;
	/** The count of records to set */
	int32_t RecordsCount;
	/** The pointer to start of a sequential array of Presence DataRecords */
	const EOS_Presence_DataRecord* Records;
));

#define EOS_PRESENCEMODIFICATION_DATARECORDID_API_LATEST 1

/**
 * Data for identifying which data records should be deleted.
 */
EOS_STRUCT(EOS_PresenceModification_DataRecordId, (
	/** API Version: Set this to EOS_PRESENCEMODIFICATION_DATARECORDID_API_LATEST. */
	int32_t ApiVersion;
	/** The key to be deleted from the data record */
	const char* Key;
));

/** Most recent version of the EOS_PresenceModification_DeleteData API. */
#define EOS_PRESENCEMODIFICATION_DELETEDATA_API_LATEST 1

/** DEPRECATED! Use EOS_PRESENCEMODIFICATION_DELETEDATA_API_LATEST instead. */
#define EOS_PRESENCE_DELETEDATA_API_LATEST EOS_PRESENCEMODIFICATION_DELETEDATA_API_LATEST

/**
 * Data for the EOS_PresenceModification_DeleteData function.
 */
EOS_STRUCT(EOS_PresenceModification_DeleteDataOptions, (
	/** API Version: Set this to EOS_PRESENCEMODIFICATION_DELETEDATA_API_LATEST. */
	int32_t ApiVersion;
	/** The count of data keys to delete */
	int32_t RecordsCount;
	/** The pointer to start of a sequential array */
	const EOS_PresenceModification_DataRecordId* Records;
));

#pragma pack(pop)
