// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "eos_sessions_types.h"

/**
 * The Session Interface is used to manage sessions that can be advertised with the backend service
 * All Session Interface calls take a handle of type EOS_HSessions as the first parameter.
 * This handle can be retrieved from an EOS_HPlatform handle by using the EOS_Platform_GetSessionsInterface function.
 *
 * @see EOS_Platform_GetSessionsInterface
 */

/**
 * Creates a session modification handle (EOS_HSessionModification).  The session modification handle is used to build a new session and can be applied with EOS_Sessions_UpdateSession
 * The EOS_HSessionModification must be released by calling EOS_SessionModification_Release once it no longer needed.
 *
 * @param Options Required fields for the creation of a session such as a name, bucketid, and max players
 * @param OutSessionModificationHandle Pointer to a Session Modification Handle only set if successful
 * @return EOS_Success if we successfully created the Session Modification Handle pointed at in OutSessionModificationHandle, or an error result if the input data was invalid
 *
 * @see EOS_SessionModification_Release
 * @see EOS_Sessions_UpdateSession
 * @see EOS_HSessionModification
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CreateSessionModification(EOS_HSessions Handle, const EOS_Sessions_CreateSessionModificationOptions* Options, EOS_HSessionModification* OutSessionModificationHandle);

/**
 * Creates a session modification handle (EOS_HSessionModification). The session modification handle is used to modify an existing session and can be applied with EOS_Sessions_UpdateSession.
 * The EOS_HSessionModification must be released by calling EOS_SessionModification_Release once it is no longer needed.
 *
 * @param Options Required fields such as session name
 * @param OutSessionModificationHandle Pointer to a Session Modification Handle only set if successful
 * @return EOS_Success if we successfully created the Session Modification Handle pointed at in OutSessionModificationHandle, or an error result if the input data was invalid
 *
 * @see EOS_SessionModification_Release
 * @see EOS_Sessions_UpdateSession
 * @see EOS_HSessionModification
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_UpdateSessionModification(EOS_HSessions Handle, const EOS_Sessions_UpdateSessionModificationOptions* Options, EOS_HSessionModification* OutSessionModificationHandle);

/**
 * Update a session given a session modification handle created by EOS_Sessions_CreateSessionModification or EOS_Sessions_UpdateSessionModification
 *
 * @param Options Structure containing information about the session to be updated
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the update operation completes, either successfully or in error
 *
 * @return EOS_Success if the update completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_Sessions_OutOfSync if the session is out of sync and will be updated on the next connection with the backend
 *         EOS_NotFound if a session to be updated does not exist
 */
EOS_DECLARE_FUNC(void) EOS_Sessions_UpdateSession(EOS_HSessions Handle, const EOS_Sessions_UpdateSessionOptions* Options, void* ClientData, const EOS_Sessions_OnUpdateSessionCallback CompletionDelegate);

/**
 * Destroy a session given a session name
 *
 * @param Options Structure containing information about the session to be destroyed
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the destroy operation completes, either successfully or in error
 *
 * @return EOS_Success if the destroy completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_AlreadyPending if the session is already marked for destroy
 *         EOS_NotFound if a session to be destroyed does not exist
 */
EOS_DECLARE_FUNC(void) EOS_Sessions_DestroySession(EOS_HSessions Handle, const EOS_Sessions_DestroySessionOptions* Options, void* ClientData, const EOS_Sessions_OnDestroySessionCallback CompletionDelegate);

/**
 * Join a session, creating a local session under a given session name.  Backend will validate various conditions to make sure it is possible to join the session.
 *
 * @param Options Structure containing information about the session to be joined
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the join operation completes, either successfully or in error
 *
 * @return EOS_Success if the join completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_Sessions_SessionAlreadyExists if the session is already exists or is in the process of being joined
 */
EOS_DECLARE_FUNC(void) EOS_Sessions_JoinSession(EOS_HSessions Handle, const EOS_Sessions_JoinSessionOptions* Options, void* ClientData, const EOS_Sessions_OnJoinSessionCallback CompletionDelegate);

/**
 * Mark a session as started, making it unable to find if session properties indicate "join in progress" is not available
 *
 * @param Options Structure containing information about the session to be started
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the start operation completes, either successfully or in error
 *
 * @return EOS_Success if the start completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_Sessions_OutOfSync if the session is out of sync and will be updated on the next connection with the backend
 *         EOS_NotFound if a session to be started does not exist
 */
EOS_DECLARE_FUNC(void) EOS_Sessions_StartSession(EOS_HSessions Handle, const EOS_Sessions_StartSessionOptions* Options, void* ClientData, const EOS_Sessions_OnStartSessionCallback CompletionDelegate);

/**
 * Mark a session as ended, making it available to find if "join in progress" was disabled.  The session may be started again if desired
 *
 * @param Options Structure containing information about the session to be ended
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the end operation completes, either successfully or in error
 *
 * @return EOS_Success if the end completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_Sessions_OutOfSync if the session is out of sync and will be updated on the next connection with the backend
 *         EOS_NotFound if a session to be ended does not exist
 */
EOS_DECLARE_FUNC(void) EOS_Sessions_EndSession(EOS_HSessions Handle, const EOS_Sessions_EndSessionOptions* Options, void* ClientData, const EOS_Sessions_OnEndSessionCallback CompletionDelegate);

/**
 * Register a group of players with the session, allowing them to invite others or otherwise indicate they are part of the session for determining a full session
 *
 * @param Options Structure containing information about the session and players to be registered
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the registration operation completes, either successfully or in error
 *
 * @return EOS_Success if the register completes successfully
 *         EOS_NoChange if the players to register registered previously
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_Sessions_OutOfSync if the session is out of sync and will be updated on the next connection with the backend
 *         EOS_NotFound if a session to register players does not exist
 */
EOS_DECLARE_FUNC(void) EOS_Sessions_RegisterPlayers(EOS_HSessions Handle, const EOS_Sessions_RegisterPlayersOptions* Options, void* ClientData, const EOS_Sessions_OnRegisterPlayersCallback CompletionDelegate);

/**
 * Unregister a group of players with the session, freeing up space for others to join
 *
 * @param Options Structure containing information about the session and players to be unregistered
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the unregistration operation completes, either successfully or in error
 *
 * @return EOS_Success if the unregister completes successfully
 *         EOS_NoChange if the players to unregister were not found
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_Sessions_OutOfSync if the session is out of sync and will be updated on the next connection with the backend
 *         EOS_NotFound if a session to be unregister players does not exist
 */
EOS_DECLARE_FUNC(void) EOS_Sessions_UnregisterPlayers(EOS_HSessions Handle, const EOS_Sessions_UnregisterPlayersOptions* Options, void* ClientData, const EOS_Sessions_OnUnregisterPlayersCallback CompletionDelegate);

/**
 * Send an invite to another player.  User must have created the session or be registered in the session or else the call will fail
 *
 * @param Options Structure containing information about the session and player to invite
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the send invite operation completes, either successfully or in error
 *
 * @return EOS_Success if the send invite completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_NotFound if the session to send the invite from does not exist
 */
EOS_DECLARE_FUNC(void) EOS_Sessions_SendInvite(EOS_HSessions Handle, const EOS_Sessions_SendInviteOptions* Options, void* ClientData, const EOS_Sessions_OnSendInviteCallback CompletionDelegate);

/**
 * Reject an invite from another player.
 *
 * @param Options Structure containing information about the invite to reject
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the reject invite operation completes, either successfully or in error
 *
 * @return EOS_Success if the invite rejection completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_NotFound if the invite does not exist
 */
EOS_DECLARE_FUNC(void) EOS_Sessions_RejectInvite(EOS_HSessions Handle, const EOS_Sessions_RejectInviteOptions* Options, void* ClientData, const EOS_Sessions_OnRejectInviteCallback CompletionDelegate);

/**
 * Retrieve all existing invites for a single user
 *
 * @param Options Structure containing information about the invites to query
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the query invites operation completes, either successfully or in error
 *
 */
EOS_DECLARE_FUNC(void) EOS_Sessions_QueryInvites(EOS_HSessions Handle, const EOS_Sessions_QueryInvitesOptions* Options, void* ClientData, const EOS_Sessions_OnQueryInvitesCallback CompletionDelegate);

/**
 * Get the number of known invites for a given user
 *
 * @param Options the Options associated with retrieving the current invite count
 *
 * @return number of known invites for a given user or 0 if there is an error
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Sessions_GetInviteCount(EOS_HSessions Handle, const EOS_Sessions_GetInviteCountOptions* Options);

/**
 * Retrieve an invite ID from a list of active invites for a given user
 *
 * @param Options Structure containing the input parameters
 *
 * @return EOS_Success if the input is valid and an invite ID was returned
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_NotFound if the invite doesn't exist
 *
 * @see EOS_Sessions_GetInviteCount
 * @see EOS_Sessions_CopySessionHandleByInviteId
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_GetInviteIdByIndex(EOS_HSessions Handle, const EOS_Sessions_GetInviteIdByIndexOptions* Options, char* OutBuffer, int32_t* InOutBufferLength);

/**
 * Create a session search handle.  This handle may be modified to include various search parameters.
 * Searching is possible in three methods, all mutually exclusive
 * - set the session ID to find a specific session
 * - set the target user ID to find a specific user
 * - set session parameters to find an array of sessions that match the search criteria
 *
 * @param Options Structure containing required parameters such as the maximum number of search results
 * @param OutSessionSearchHandle The new search handle or null if there was an error creating the search handle
 *
 * @return EOS_Success if the search creation completes successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CreateSessionSearch(EOS_HSessions Handle, const EOS_Sessions_CreateSessionSearchOptions* Options, EOS_HSessionSearch* OutSessionSearchHandle);

/**
 * Create a handle to an existing active session.
 *
 * @param Options Structure containing information about the active session to retrieve
 * @param OutSessionHandle The new active session handle or null if there was an error
 *
 * @return EOS_Success if the session handle was created successfully
 *         EOS_InvalidParameters if any of the options are incorrect
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *         EOS_NotFound if the active session doesn't exist
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CopyActiveSessionHandle(EOS_HSessions Handle, const EOS_Sessions_CopyActiveSessionHandleOptions* Options, EOS_HActiveSession* OutSessionHandle);

/**
 * Register to receive session invites.
 * @note must call RemoveNotifySessionInviteReceived to remove the notification
 *
 * @param Options Structure containing information about the session invite notification
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param NotificationFn A callback that is fired when a session invite for a user has been received
 *
 * @return handle representing the registered callback
 */
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifySessionInviteReceived(EOS_HSessions Handle, const EOS_Sessions_AddNotifySessionInviteReceivedOptions* Options, void* ClientData, const EOS_Sessions_OnSessionInviteReceivedCallback NotificationFn);

/**
 * Unregister from receiving session invites.
 *
 * @param InId Handle representing the registered callback
 */
EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifySessionInviteReceived(EOS_HSessions Handle, EOS_NotificationId InId);

/**
 * Register to receive notifications when a user accepts a session invite via the social overlay.
 * @note must call RemoveNotifySessionInviteAccepted to remove the notification
 *
 * @param Options Structure containing information about the request.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param NotificationFn A callback that is fired when a a notification is received.
 *
 * @return handle representing the registered callback
 */
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifySessionInviteAccepted(EOS_HSessions Handle, const EOS_Sessions_AddNotifySessionInviteAcceptedOptions* Options, void* ClientData, const EOS_Sessions_OnSessionInviteAcceptedCallback NotificationFn);

/**
 * Unregister from receiving notifications when a user accepts a session invite via the social overlay.
 *
 * @param InId Handle representing the registered callback
 */
EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifySessionInviteAccepted(EOS_HSessions Handle, EOS_NotificationId InId);

/**
 * Register to receive notifications when a user accepts a session join game via the social overlay.
 * @note must call RemoveNotifyJoinSessionAccepted to remove the notification
 *
 * @param Options Structure containing information about the request.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param NotificationFn A callback that is fired when a a notification is received.
 *
 * @return handle representing the registered callback
 */
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Sessions_AddNotifyJoinSessionAccepted(EOS_HSessions Handle, const EOS_Sessions_AddNotifyJoinSessionAcceptedOptions* Options, void* ClientData, const EOS_Sessions_OnJoinSessionAcceptedCallback NotificationFn);

/**
 * Unregister from receiving notifications when a user accepts a session join game via the social overlay.
 *
 * @param InId Handle representing the registered callback
 */
EOS_DECLARE_FUNC(void) EOS_Sessions_RemoveNotifyJoinSessionAccepted(EOS_HSessions Handle, EOS_NotificationId InId);

/**
 * EOS_Sessions_CopySessionHandleByInviteId is used to immediately retrieve a handle to the session information from after notification of an invite
 * If the call returns an EOS_Success result, the out parameter, OutSessionHandle, must be passed to EOS_SessionDetails_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutSessionHandle out parameter used to receive the session handle
 *
 * @return EOS_Success if the information is available and passed out in OutSessionHandle
 *         EOS_InvalidParameters if you pass an invalid invite ID or a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *         EOS_NotFound if the invite ID cannot be found
 *
 * @see EOS_Sessions_CopySessionHandleByInviteIdOptions
 * @see EOS_SessionDetails_Release
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CopySessionHandleByInviteId(EOS_HSessions Handle, const EOS_Sessions_CopySessionHandleByInviteIdOptions* Options, EOS_HSessionDetails* OutSessionHandle);

/**
 * EOS_Sessions_CopySessionHandleByUiEventId is used to immediately retrieve a handle to the session information from after notification of a join game event.
 * If the call returns an EOS_Success result, the out parameter, OutSessionHandle, must be passed to EOS_SessionDetails_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutSessionHandle out parameter used to receive the session handle
 *
 * @return EOS_Success if the information is available and passed out in OutSessionHandle
 *         EOS_InvalidParameters if you pass an invalid invite ID or a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *         EOS_NotFound if the invite ID cannot be found
 *
 * @see EOS_Sessions_CopySessionHandleByUiEventIdOptions
 * @see EOS_SessionDetails_Release
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CopySessionHandleByUiEventId(EOS_HSessions Handle, const EOS_Sessions_CopySessionHandleByUiEventIdOptions* Options, EOS_HSessionDetails* OutSessionHandle);

/**
 * EOS_Sessions_CopySessionHandleForPresence is used to immediately retrieve a handle to the session information which was marked with bPresenceEnabled on create or join.
 * If the call returns an EOS_Success result, the out parameter, OutSessionHandle, must be passed to EOS_SessionDetails_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutSessionHandle out parameter used to receive the session handle
 *
 * @return EOS_Success if the information is available and passed out in OutSessionHandle
 *         EOS_InvalidParameters if you pass an invalid invite ID or a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *         EOS_NotFound if there is no session with bPresenceEnabled
 *
 * @see EOS_Sessions_CopySessionHandleForPresenceOptions
 * @see EOS_SessionDetails_Release
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_CopySessionHandleForPresence(EOS_HSessions Handle, const EOS_Sessions_CopySessionHandleForPresenceOptions* Options, EOS_HSessionDetails* OutSessionHandle);

/**
 * EOS_Sessions_IsUserInSession returns whether or not a given user can be found in a specified session
 *
 * @param Options Structure containing the input parameters
 *
 * @return EOS_Success if the user is found in the specified session
 *		   EOS_NotFound if the user is not found in the specified session
 *		   EOS_InvalidParameters if you pass an invalid invite ID or a null pointer for the out parameter
 *		   EOS_IncompatibleVersion if the API version passed in is incorrect
 *		   EOS_Invalid_ProductUserID if an invalid target user is specified
 *		   EOS_Sessions_InvalidSession if the session specified is invalid
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_IsUserInSession(EOS_HSessions Handle, const EOS_Sessions_IsUserInSessionOptions* Options);

/**
 * Dump the contents of active sessions that exist locally to the log output, purely for debug purposes
 *
 * @param Options Options related to dumping session state such as the session name
 *
 * @return EOS_Success if the output operation completes successfully
 *         EOS_NotFound if the session specified does not exist
 *         EOS_InvalidParameters if any of the options are incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Sessions_DumpSessionState(EOS_HSessions Handle, const EOS_Sessions_DumpSessionStateOptions* Options);

/**
 * To modify sessions, you must call EOS_Sessions_CreateSessionModification to create a Session Modification handle. To modify that handle, call
 * EOS_HSessionModification methods. Once you are finished, call EOS_Sessions_UpdateSession with your handle. You must then release your Session Modification
 * handle by calling EOS_SessionModification_Release.
 */

/**
 * Set the bucket ID associated with this session.
 * Values such as region, game mode, etc can be combined here depending on game need.
 * Setting this is strongly recommended to improve search performance.
 *
 * @param Options Options associated with the bucket ID of the session
 *
 * @return EOS_Success if setting this parameter was successful
 *         EOS_InvalidParameters if the bucket ID is invalid or null
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetBucketId(EOS_HSessionModification Handle, const EOS_SessionModification_SetBucketIdOptions* Options);

/**
 * Set the host address associated with this session
 * Setting this is optional, if the value is not set the SDK will fill the value in from the service.
 * It is useful to set if other addressing mechanisms are desired or if LAN addresses are preferred during development
 *
 * @note No validation of this value occurs to allow for flexibility in addressing methods
 *
 * @param Options Options associated with the host address of the session
 *
 * @return EOS_Success if setting this parameter was successful
 *         EOS_InvalidParameters if the host ID is an empty string
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetHostAddress(EOS_HSessionModification Handle, const EOS_SessionModification_SetHostAddressOptions* Options);

/**
 * Set the session permissions associated with this session.
 * The permissions range from "public" to "invite only" and are described by EOS_EOnlineSessionPermissionLevel
 *
 * @param Options Options associated with the permission level of the session
 *
 * @return EOS_Success if setting this parameter was successful
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetPermissionLevel(EOS_HSessionModification Handle, const EOS_SessionModification_SetPermissionLevelOptions* Options);

/**
 * Set whether or not join in progress is allowed
 * Once a session is started, it will no longer be visible to search queries unless this flag is set or the session returns to the pending or ended state
 *
 * @param Options Options associated with setting the join in progress state the session
 *
 * @return EOS_Success if setting this parameter was successful
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetJoinInProgressAllowed(EOS_HSessionModification Handle, const EOS_SessionModification_SetJoinInProgressAllowedOptions* Options);

/**
 * Set the maximum number of players allowed in this session.
 * When updating the session, it is not possible to reduce this number below the current number of existing players
 *
 * @param Options Options associated with max number of players in this session
 *
 * @return EOS_Success if setting this parameter was successful
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetMaxPlayers(EOS_HSessionModification Handle, const EOS_SessionModification_SetMaxPlayersOptions* Options);

/**
 * Allows enabling or disabling invites for this session.
 * The session will also need to have `bPresenceEnabled` true.
 *
 * @param Options Options associated with invites allowed flag for this session.
 *
 * @return EOS_Success if setting this parameter was successful
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_SetInvitesAllowed(EOS_HSessionModification Handle, const EOS_SessionModification_SetInvitesAllowedOptions* Options);

/**
 * Associate an attribute with this session
 * An attribute is something that may or may not be advertised with the session.
 * If advertised, it can be queried for in a search, otherwise the data remains local to the client
 *
 * @param Options Options to set the attribute and its advertised state
 *
 * @return EOS_Success if setting this parameter was successful
 *		   EOS_InvalidParameters if the attribution is missing information or otherwise invalid
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_AddAttribute(EOS_HSessionModification Handle, const EOS_SessionModification_AddAttributeOptions* Options);

/**
 * Remove an attribute from this session
 *
 * @param Options Specify the key of the attribute to remove
 *
 * @return EOS_Success if removing this parameter was successful
 *		   EOS_InvalidParameters if the key is null or empty
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionModification_RemoveAttribute(EOS_HSessionModification Handle, const EOS_SessionModification_RemoveAttributeOptions* Options);

/**
 * Representation of an existing session some local players are actively involved in (via Create/Join)
 */

/**
 * EOS_ActiveSession_CopyInfo is used to immediately retrieve a copy of active session information
 * If the call returns an EOS_Success result, the out parameter, OutActiveSessionInfo, must be passed to EOS_ActiveSession_Info_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutActiveSessionInfo Out parameter used to receive the EOS_ActiveSession_Info structure.
 *
 * @return EOS_Success if the information is available and passed out in OutActiveSessionInfo
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_ActiveSession_Info
 * @see EOS_ActiveSession_CopyInfoOptions
 * @see EOS_ActiveSession_Info_Release
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_ActiveSession_CopyInfo(EOS_HActiveSession Handle, const EOS_ActiveSession_CopyInfoOptions* Options, EOS_ActiveSession_Info ** OutActiveSessionInfo);

/**
 * Get the number of registered players associated with this active session
 *
 * @param Options the Options associated with retrieving the registered player count
 *
 * @return number of registered players in the active session or 0 if there is an error
 */
EOS_DECLARE_FUNC(uint32_t) EOS_ActiveSession_GetRegisteredPlayerCount(EOS_HActiveSession Handle, const EOS_ActiveSession_GetRegisteredPlayerCountOptions* Options);

/**
 * EOS_ActiveSession_GetRegisteredPlayerByIndex is used to immediately retrieve individual players registered with the active session.
 *
 * @param Options Structure containing the input parameters
 *
 * @return the product user ID for the registered player at a given index or null if that index is invalid
 *
 * @see EOS_ActiveSession_GetRegisteredPlayerCount
 * @see EOS_ActiveSession_GetRegisteredPlayerByIndexOptions
 */
EOS_DECLARE_FUNC(EOS_ProductUserId) EOS_ActiveSession_GetRegisteredPlayerByIndex(EOS_HActiveSession Handle, const EOS_ActiveSession_GetRegisteredPlayerByIndexOptions* Options);

/**
 * This class represents the details of a session, including its session properties and the attribution associated with it
 * Locally created or joined active sessions will contain this information as will search results.   
 * A handle to a session is required to join a session via search or invite
 */

/**
 * EOS_SessionDetails_CopyInfo is used to immediately retrieve a copy of session information from a given source such as a active session or a search result.
 * If the call returns an EOS_Success result, the out parameter, OutSessionInfo, must be passed to EOS_SessionDetails_Info_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutSessionInfo Out parameter used to receive the EOS_SessionDetails_Info structure.
 *
 * @return EOS_Success if the information is available and passed out in OutSessionInfo
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_SessionDetails_Info
 * @see EOS_SessionDetails_CopyInfoOptions
 * @see EOS_SessionDetails_Info_Release
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionDetails_CopyInfo(EOS_HSessionDetails Handle, const EOS_SessionDetails_CopyInfoOptions* Options, EOS_SessionDetails_Info ** OutSessionInfo);

/**
 * Get the number of attributes associated with this session
 *
 * @param Options the Options associated with retrieving the attribute count
 *
 * @return number of attributes on the session or 0 if there is an error
 */
EOS_DECLARE_FUNC(uint32_t) EOS_SessionDetails_GetSessionAttributeCount(EOS_HSessionDetails Handle, const EOS_SessionDetails_GetSessionAttributeCountOptions* Options);

/**
 * EOS_SessionDetails_CopySessionAttributeByIndex is used to immediately retrieve a copy of session attribution from a given source such as a active session or a search result.
 * If the call returns an EOS_Success result, the out parameter, OutSessionAttribute, must be passed to EOS_SessionDetails_Attribute_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutSessionAttribute Out parameter used to receive the EOS_SessionDetails_Attribute structure.
 *
 * @return EOS_Success if the information is available and passed out in OutSessionAttribute
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_SessionDetails_Attribute
 * @see EOS_SessionDetails_CopySessionAttributeByIndexOptions
 * @see EOS_SessionDetails_Attribute_Release
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionDetails_CopySessionAttributeByIndex(EOS_HSessionDetails Handle, const EOS_SessionDetails_CopySessionAttributeByIndexOptions* Options, EOS_SessionDetails_Attribute ** OutSessionAttribute);

/**
 * EOS_SessionDetails_CopySessionAttributeByKey is used to immediately retrieve a copy of session attribution from a given source such as a active session or a search result.
 * If the call returns an EOS_Success result, the out parameter, OutSessionAttribute, must be passed to EOS_SessionDetails_Attribute_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutSessionAttribute Out parameter used to receive the EOS_SessionDetails_Attribute structure.
 *
 * @return EOS_Success if the information is available and passed out in OutSessionAttribute
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_SessionDetails_Attribute
 * @see EOS_SessionDetails_CopySessionAttributeByKeyOptions
 * @see EOS_SessionDetails_Attribute_Release
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionDetails_CopySessionAttributeByKey(EOS_HSessionDetails Handle, const EOS_SessionDetails_CopySessionAttributeByKeyOptions* Options, EOS_SessionDetails_Attribute ** OutSessionAttribute);

/**
 * Class responsible for the creation, setup, and execution of a search query.
 * Search parameters are defined, the query is executed and the search results are returned within this object
 */

/**
 * Set a session ID to find and will return at most one search result.  Setting TargetUserId or SearchParameters will result in EOS_SessionSearch_Find failing
 *
 * @param Options A specific session ID for which to search
 *
 * @return EOS_Success if setting this session ID was successful
 *         EOS_InvalidParameters if the session ID is invalid or null
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_SetSessionId(EOS_HSessionSearch Handle, const EOS_SessionSearch_SetSessionIdOptions* Options);

/**
 * Set a target user ID to find and will return at most one search result.  Setting SessionId or SearchParameters will result in EOS_SessionSearch_Find failing
 * @note a search result will only be found if this user is in a public session
 *
 * @param Options a specific target user ID to find
 *
 * @return EOS_Success if setting this target user ID was successful
 *         EOS_InvalidParameters if the target user ID is invalid or null
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_SetTargetUserId(EOS_HSessionSearch Handle, const EOS_SessionSearch_SetTargetUserIdOptions* Options);

/**
 * Add a parameter to an array of search criteria combined via an implicit AND operator.  Setting SessionId or TargetUserId will result in EOS_SessionSearch_Find failing
 *
 * @param Options a search parameter and its comparison op
 *
 * @return EOS_Success if setting this search parameter was successful
 *         EOS_InvalidParameters if the search criteria is invalid or null
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_Sessions_AttributeData
 * @see EOS_EComparisonOp
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_SetParameter(EOS_HSessionSearch Handle, const EOS_SessionSearch_SetParameterOptions* Options);

/**
 * Remove a parameter from the array of search criteria.
 *
 * @params Options a search parameter key name to remove
 *
 * @return EOS_Success if removing this search parameter was successful
 *         EOS_InvalidParameters if the search key is invalid or null
 *		   EOS_NotFound if the parameter was not a part of the search criteria
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_RemoveParameter(EOS_HSessionSearch Handle, const EOS_SessionSearch_RemoveParameterOptions* Options);

/**
 * Set the maximum number of search results to return in the query, can't be more than EOS_SESSIONS_MAX_SEARCH_RESULTS
 *
 * @param Options maximum number of search results to return in the query
 *
 * @return EOS_Success if setting the max results was successful
 *         EOS_InvalidParameters if the number of results requested is invalid
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_SetMaxResults(EOS_HSessionSearch Handle, const EOS_SessionSearch_SetMaxResultsOptions* Options);

/**
 * Find sessions matching the search criteria setup via this session search handle.
 * When the operation completes, this handle will have the search results that can be parsed
 *
 * @param Options Structure containing information about the search criteria to use
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate A callback that is fired when the search operation completes, either successfully or in error
 *
 * @return EOS_Success if the find operation completes successfully
 *         EOS_NotFound if searching for an individual session by sessionid or targetuserid returns no results
 *         EOS_InvalidParameters if any of the options are incorrect
 */
EOS_DECLARE_FUNC(void) EOS_SessionSearch_Find(EOS_HSessionSearch Handle, const EOS_SessionSearch_FindOptions* Options, void* ClientData, const EOS_SessionSearch_OnFindCallback CompletionDelegate);

/**
 * Get the number of search results found by the search parameters in this search
 *
 * @param Options Options associated with the search count
 *
 * @return return the number of search results found by the query or 0 if search is not complete
 */
EOS_DECLARE_FUNC(uint32_t) EOS_SessionSearch_GetSearchResultCount(EOS_HSessionSearch Handle, const EOS_SessionSearch_GetSearchResultCountOptions* Options);

/**
 * EOS_SessionSearch_CopySearchResultByIndex is used to immediately retrieve a handle to the session information from a given search result.
 * If the call returns an EOS_Success result, the out parameter, OutSessionHandle, must be passed to EOS_SessionDetails_Release to release the memory associated with it.
 *
 * @param Options Structure containing the input parameters
 * @param OutSessionHandle out parameter used to receive the session handle
 *
 * @return EOS_Success if the information is available and passed out in OutSessionHandle
 *         EOS_InvalidParameters if you pass an invalid index or a null pointer for the out parameter
 *         EOS_IncompatibleVersion if the API version passed in is incorrect
 *
 * @see EOS_SessionSearch_CopySearchResultByIndexOptions
 * @see EOS_SessionDetails_Release
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_SessionSearch_CopySearchResultByIndex(EOS_HSessionSearch Handle, const EOS_SessionSearch_CopySearchResultByIndexOptions* Options, EOS_HSessionDetails* OutSessionHandle);
