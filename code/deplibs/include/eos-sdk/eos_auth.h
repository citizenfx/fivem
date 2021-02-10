// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "eos_auth_types.h"

/**
 * The Auth Interface is used to manage local user permissions and access to backend services through the verification of various forms of credentials.
 * All Auth Interface calls take a handle of type EOS_HAuth as the first parameter.
 * This handle can be retrieved from an EOS_HPlatform handle by using the EOS_Platform_GetAuthInterface function.
 *
 * @see EOS_Platform_GetAuthInterface
 */

/**
 * Login/Authenticate with user credentials.
 *
 * @param Options structure containing the account credentials to use during the login operation
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the login operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Auth_Login(EOS_HAuth Handle, const EOS_Auth_LoginOptions* Options, void* ClientData, const EOS_Auth_OnLoginCallback CompletionDelegate);

/**
 * Signs the player out of the online service.
 *
 * @param Options structure containing information about which account to log out.
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the logout operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Auth_Logout(EOS_HAuth Handle, const EOS_Auth_LogoutOptions* Options, void* ClientData, const EOS_Auth_OnLogoutCallback CompletionDelegate);

/**
 * Link external account by continuing previous login attempt with a continuance token.
 *
 * On Desktop and Mobile platforms, the user will be presented the Epic Account Portal to resolve their identity.
 *
 * On Console, the user will login to their Epic Account using an external device, e.g. a mobile device or a desktop PC,
 * by browsing to the presented authentication URL and entering the device code presented by the game on the console.
 *
 * On success, the user will be logged in at the completion of this action.
 * This will commit this external account to the Epic Account and cannot be undone in the SDK.
 *
 * @param Options structure containing the account credentials to use during the link account operation
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the link account operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Auth_LinkAccount(EOS_HAuth Handle, const EOS_Auth_LinkAccountOptions* Options, void* ClientData, const EOS_Auth_OnLinkAccountCallback CompletionDelegate);

/**
 * Deletes a previously received and locally stored persistent auth access token for the currently logged in user of the local device.
 *
 * On Desktop and Mobile platforms, the access token is deleted from the keychain of the local user and a backend request is made to revoke the token on the authentication server.
 * On Console platforms, even though the caller is responsible for storing and deleting the access token on the local device,
 * this function should still be called with the access token before its deletion to make the best effort in attempting to also revoke it on the authentication server.
 * If the function would fail on Console, the caller should still proceed as normal to delete the access token locally as intended.
 *
 * @param Options structure containing operation input parameters
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the deletion operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Auth_DeletePersistentAuth(EOS_HAuth Handle, const EOS_Auth_DeletePersistentAuthOptions* Options, void* ClientData, const EOS_Auth_OnDeletePersistentAuthCallback CompletionDelegate);

/**
 * Contact the backend service to verify validity of an existing user auth token.
 * This function is intended for server-side use only.
 *
 * @param Options structure containing information about the auth token being verified
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the logout operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Auth_VerifyUserAuth(EOS_HAuth Handle, const EOS_Auth_VerifyUserAuthOptions* Options, void* ClientData, const EOS_Auth_OnVerifyUserAuthCallback CompletionDelegate);

/**
 * Fetch the number of accounts that are logged in.
 *
 * @return the number of accounts logged in.
 */
EOS_DECLARE_FUNC(int32_t) EOS_Auth_GetLoggedInAccountsCount(EOS_HAuth Handle);

/**
 * Fetch an Epic Online Services Account ID that is logged in.
 *
 * @param Index An index into the list of logged in accounts. If the index is out of bounds, the returned Epic Online Services Account ID will be invalid.
 *
 * @return The Epic Online Services Account ID associated with the index passed
 */
EOS_DECLARE_FUNC(EOS_EpicAccountId) EOS_Auth_GetLoggedInAccountByIndex(EOS_HAuth Handle, int32_t Index);

/**
 * Fetches the login status for an Epic Online Services Account ID.
 *
 * @param LocalUserId The Epic Online Services Account ID of the user being queried
 *
 * @return The enum value of a user's login status
 */
EOS_DECLARE_FUNC(EOS_ELoginStatus) EOS_Auth_GetLoginStatus(EOS_HAuth Handle, EOS_EpicAccountId LocalUserId);

/**
 * Fetches a user auth token for an Epic Online Services Account ID.
 *
 * @param Options Structure containing the api version of CopyUserAuthToken to use
 * @param LocalUserId The Epic Online Services Account ID of the user being queried
 * @param OutUserAuthToken The auth token for the given user, if it exists and is valid; use EOS_Auth_Token_Release when finished
 *
 * @see EOS_Auth_Token_Release
 *
 * @return EOS_Success if the information is available and passed out in OutUserAuthToken
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the auth token is not found or expired.
 *
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Auth_CopyUserAuthToken(EOS_HAuth Handle, const EOS_Auth_CopyUserAuthTokenOptions* Options, EOS_EpicAccountId LocalUserId, EOS_Auth_Token ** OutUserAuthToken);

/**
 * Register to receive login status updates.
 * @note must call RemoveNotifyLoginStatusChanged to remove the notification
 *
 * @param Options structure containing the api version of AddNotifyLoginStatusChanged to use
 * @param ClientData arbitrary data that is passed back to you in the callback
 * @param Notification a callback that is fired when the login status for a user changes
 *
 * @return handle representing the registered callback
 */
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_Auth_AddNotifyLoginStatusChanged(EOS_HAuth Handle, const EOS_Auth_AddNotifyLoginStatusChangedOptions* Options, void* ClientData, const EOS_Auth_OnLoginStatusChangedCallback Notification);

/**
 * Unregister from receiving login status updates.
 *
 * @param InId handle representing the registered callback
 */
EOS_DECLARE_FUNC(void) EOS_Auth_RemoveNotifyLoginStatusChanged(EOS_HAuth Handle, EOS_NotificationId InId);
