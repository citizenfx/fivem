// Copyright Epic Games, Inc. All Rights Reserved.

// This file is not intended to be included directly. Include eos_common.h instead.

/** Successful result. no further error processing needed */
EOS_RESULT_VALUE(EOS_Success, 0)

/** Failed due to no connection */
EOS_RESULT_VALUE(EOS_NoConnection, 1)
/** Failed login due to invalid credentials */
EOS_RESULT_VALUE(EOS_InvalidCredentials, 2)
/** Failed due to invalid or missing user */
EOS_RESULT_VALUE(EOS_InvalidUser, 3)
/** Failed due to invalid or missing authentication token for user (e.g. not logged in) */
EOS_RESULT_VALUE(EOS_InvalidAuth, 4)
/** Failed due to invalid access */
EOS_RESULT_VALUE(EOS_AccessDenied, 5)
/** If the client does not possess the permission required */
EOS_RESULT_VALUE(EOS_MissingPermissions, 6)
/** If the token provided does not represent an account */
EOS_RESULT_VALUE(EOS_Token_Not_Account, 7)
/** Throttled due to too many requests */
EOS_RESULT_VALUE(EOS_TooManyRequests, 8)
/** Async request was already pending */
EOS_RESULT_VALUE(EOS_AlreadyPending, 9)
/** Invalid parameters specified for request */
EOS_RESULT_VALUE(EOS_InvalidParameters, 10)
/** Invalid request */
EOS_RESULT_VALUE(EOS_InvalidRequest, 11)
/** Failed due to unable to parse or recognize a backend response */
EOS_RESULT_VALUE(EOS_UnrecognizedResponse, 12)
/** Incompatible client for backend version */
EOS_RESULT_VALUE(EOS_IncompatibleVersion, 13)
/** Not configured correctly for use */
EOS_RESULT_VALUE(EOS_NotConfigured, 14)
/** Already configured for use. */
EOS_RESULT_VALUE(EOS_AlreadyConfigured, 15)
/** Feature not available on this implementation */
EOS_RESULT_VALUE(EOS_NotImplemented, 16)
/** Operation was canceled (likely by user) */
EOS_RESULT_VALUE(EOS_Canceled, 17)
/** The requested information was not found */
EOS_RESULT_VALUE(EOS_NotFound, 18)
/** An error occurred during an asynchronous operation, and it will be retried. Callbacks receiving this result will be called again in the future. */
EOS_RESULT_VALUE(EOS_OperationWillRetry, 19)
/** The request had no effect */
EOS_RESULT_VALUE(EOS_NoChange, 20)
/** The request attempted to use multiple or inconsistent API versions */
EOS_RESULT_VALUE(EOS_VersionMismatch, 21)
/** A maximum limit was exceeded on the client, different from EOS_TooManyRequests */
EOS_RESULT_VALUE(EOS_LimitExceeded, 22)
/** Feature or client ID performing the operation has been disabled. */
EOS_RESULT_VALUE(EOS_Disabled, 23)
/** Duplicate entry not allowed */
EOS_RESULT_VALUE(EOS_DuplicateNotAllowed, 24)
/** Required parameters are missing. DEPRECATED: This error is no longer used. */
EOS_RESULT_VALUE(EOS_MissingParameters_DEPRECATED, 25)
/** Sandbox ID is invalid */
EOS_RESULT_VALUE(EOS_InvalidSandboxId, 26)
/** Request timed out */
EOS_RESULT_VALUE(EOS_TimedOut, 27)
/** A query returned some but not all of the requested results.  */
EOS_RESULT_VALUE(EOS_PartialResult, 28)
/** Client is missing the whitelisted role */
EOS_RESULT_VALUE(EOS_Missing_Role, 29)
/** Client is missing the whitelisted feature */
EOS_RESULT_VALUE(EOS_Missing_Feature, 30)
/** The sandbox given to the backend is invalid */
EOS_RESULT_VALUE(EOS_Invalid_Sandbox, 31)
/** The deployment given to the backend is invalid */
EOS_RESULT_VALUE(EOS_Invalid_Deployment, 32)
/** The product ID specified to the backend is invalid */
EOS_RESULT_VALUE(EOS_Invalid_Product, 33)
/** The product user ID specified to the backend is invalid */
EOS_RESULT_VALUE(EOS_Invalid_ProductUserID, 34)
/** There was a failure with the backend service */
EOS_RESULT_VALUE(EOS_ServiceFailure, 35)
/** Cache directory is not set in platform options. */
EOS_RESULT_VALUE(EOS_CacheDirectoryMissing, 36)
/** Cache directory is not accessible. */
EOS_RESULT_VALUE(EOS_CacheDirectoryInvalid, 37)
/** The request failed because resource was in an invalid state */
EOS_RESULT_VALUE(EOS_InvalidState, 38)
/** Request is in progress */
EOS_RESULT_VALUE(EOS_RequestInProgress, 39)

/** Account locked due to login failures */
EOS_RESULT_VALUE(EOS_Auth_AccountLocked, 1001)
/** Account locked by update operation. */
EOS_RESULT_VALUE(EOS_Auth_AccountLockedForUpdate, 1002)
/** Refresh token used was invalid */
EOS_RESULT_VALUE(EOS_Auth_InvalidRefreshToken, 1003)
/** Invalid access token, typically when switching between backend environments */
EOS_RESULT_VALUE(EOS_Auth_InvalidToken, 1004)
/** Invalid bearer token */
EOS_RESULT_VALUE(EOS_Auth_AuthenticationFailure, 1005)
/** Invalid platform token */
EOS_RESULT_VALUE(EOS_Auth_InvalidPlatformToken, 1006)
/** Auth parameters are not associated with this account */
EOS_RESULT_VALUE(EOS_Auth_WrongAccount, 1007)
/** Auth parameters are not associated with this client */
EOS_RESULT_VALUE(EOS_Auth_WrongClient, 1008)
/** Full account is required */
EOS_RESULT_VALUE(EOS_Auth_FullAccountRequired, 1009)
/** Headless account is required */
EOS_RESULT_VALUE(EOS_Auth_HeadlessAccountRequired, 1010)
/** Password reset is required */
EOS_RESULT_VALUE(EOS_Auth_PasswordResetRequired, 1011)
/** Password was previously used and cannot be reused */
EOS_RESULT_VALUE(EOS_Auth_PasswordCannotBeReused, 1012)
/** Authorization code/exchange code has expired */
EOS_RESULT_VALUE(EOS_Auth_Expired, 1013)
/** Consent has not been given by the user */
EOS_RESULT_VALUE(EOS_Auth_ScopeConsentRequired, 1014)
/** The application has no profile on the backend */
EOS_RESULT_VALUE(EOS_Auth_ApplicationNotFound, 1015)
/** The requested consent wasn't found on the backend */
EOS_RESULT_VALUE(EOS_Auth_ScopeNotFound, 1016)
/** This account has been denied access to login */
EOS_RESULT_VALUE(EOS_Auth_AccountFeatureRestricted, 1017)

/** Pin grant code initiated */
EOS_RESULT_VALUE(EOS_Auth_PinGrantCode, 1020)
/** Pin grant code attempt expired */
EOS_RESULT_VALUE(EOS_Auth_PinGrantExpired, 1021)
/** Pin grant code attempt pending */
EOS_RESULT_VALUE(EOS_Auth_PinGrantPending, 1022)

/** External auth source did not yield an account */
EOS_RESULT_VALUE(EOS_Auth_ExternalAuthNotLinked, 1030)
/** External auth access revoked */
EOS_RESULT_VALUE(EOS_Auth_ExternalAuthRevoked, 1032)
/** External auth token cannot be interpreted */
EOS_RESULT_VALUE(EOS_Auth_ExternalAuthInvalid, 1033)
/** External auth cannot be linked to his account due to restrictions */
EOS_RESULT_VALUE(EOS_Auth_ExternalAuthRestricted, 1034)
/** External auth cannot be used for login */
EOS_RESULT_VALUE(EOS_Auth_ExternalAuthCannotLogin, 1035)
/** External auth is expired */
EOS_RESULT_VALUE(EOS_Auth_ExternalAuthExpired, 1036)
/** External auth cannot be removed since it's the last possible way to login */
EOS_RESULT_VALUE(EOS_Auth_ExternalAuthIsLastLoginType, 1037)

/** Exchange code not found */
EOS_RESULT_VALUE(EOS_Auth_ExchangeCodeNotFound, 1040)
/** Originating exchange code session has expired */
EOS_RESULT_VALUE(EOS_Auth_OriginatingExchangeCodeSessionExpired, 1041)

/** The account has been disabled and cannot be used for authentication */
EOS_RESULT_VALUE(EOS_Auth_PersistentAuth_AccountNotActive, 1050)

/** MFA challenge required */
EOS_RESULT_VALUE(EOS_Auth_MFARequired, 1060)

/** Parental locks are in place */
EOS_RESULT_VALUE(EOS_Auth_ParentalControls, 1070)

/** Korea real ID association required but missing */
EOS_RESULT_VALUE(EOS_Auth_NoRealId, 1080)

/** An outgoing friend invitation is awaiting acceptance; sending another invite to the same user is erroneous */
EOS_RESULT_VALUE(EOS_Friends_InviteAwaitingAcceptance, 2000)
/** There is no friend invitation to accept/reject */
EOS_RESULT_VALUE(EOS_Friends_NoInvitation, 2001)
/** Users are already friends, so sending another invite is erroneous */
EOS_RESULT_VALUE(EOS_Friends_AlreadyFriends, 2003)
/** Users are not friends, so deleting the friend is erroneous */
EOS_RESULT_VALUE(EOS_Friends_NotFriends, 2004)
/** Remote user has too many invites to receive new invites */
EOS_RESULT_VALUE(EOS_Friends_TargetUserTooManyInvites, 2005)
/** Local user has too many invites to send new invites */
EOS_RESULT_VALUE(EOS_Friends_LocalUserTooManyInvites, 2006)
/** Remote user has too many friends to make a new friendship */
EOS_RESULT_VALUE(EOS_Friends_TargetUserFriendLimitExceeded, 2007)
/** Local user has too many friends to make a new friendship */
EOS_RESULT_VALUE(EOS_Friends_LocalUserFriendLimitExceeded, 2008)

/** Request data was null or invalid */
EOS_RESULT_VALUE(EOS_Presence_DataInvalid, 3000)
/** Request contained too many or too few unique data items, or the request would overflow the maximum amount of data allowed */
EOS_RESULT_VALUE(EOS_Presence_DataLengthInvalid, 3001)
/** Request contained data with an invalid key */
EOS_RESULT_VALUE(EOS_Presence_DataKeyInvalid, 3002)
/** Request contained data with a key too long or too short */
EOS_RESULT_VALUE(EOS_Presence_DataKeyLengthInvalid, 3003)
/** Request contained data with an invalid value */
EOS_RESULT_VALUE(EOS_Presence_DataValueInvalid, 3004)
/** Request contained data with a value too long */
EOS_RESULT_VALUE(EOS_Presence_DataValueLengthInvalid, 3005)
/** Request contained an invalid rich text string */
EOS_RESULT_VALUE(EOS_Presence_RichTextInvalid, 3006)
/** Request contained a rich text string that was too long */
EOS_RESULT_VALUE(EOS_Presence_RichTextLengthInvalid, 3007)
/** Request contained an invalid status state */
EOS_RESULT_VALUE(EOS_Presence_StatusInvalid, 3008)

/** The entitlement retrieved is stale, requery for updated information */
EOS_RESULT_VALUE(EOS_Ecom_EntitlementStale, 4000)
/** The offer retrieved is stale, requery for updated information */
EOS_RESULT_VALUE(EOS_Ecom_CatalogOfferStale, 4001)
/** The item or associated structure retrieved is stale, requery for updated information */
EOS_RESULT_VALUE(EOS_Ecom_CatalogItemStale, 4002)
/** The one or more offers has an invalid price. This may be caused by the price setup. */
EOS_RESULT_VALUE(EOS_Ecom_CatalogOfferPriceInvalid, 4003)
/** The checkout page failed to load */
EOS_RESULT_VALUE(EOS_Ecom_CheckoutLoadError, 4004)

/** Session is already in progress */
EOS_RESULT_VALUE(EOS_Sessions_SessionInProgress, 5000)
/** Too many players to register with this session */
EOS_RESULT_VALUE(EOS_Sessions_TooManyPlayers, 5001)
/** Client has no permissions to access this session */
EOS_RESULT_VALUE(EOS_Sessions_NoPermission, 5002)
/** Session already exists in the system */
EOS_RESULT_VALUE(EOS_Sessions_SessionAlreadyExists, 5003)
/** Session lock required for operation */
EOS_RESULT_VALUE(EOS_Sessions_InvalidLock, 5004)
/** Invalid session reference */
EOS_RESULT_VALUE(EOS_Sessions_InvalidSession, 5005)
/** Sandbox ID associated with auth didn't match */
EOS_RESULT_VALUE(EOS_Sessions_SandboxNotAllowed, 5006)
/** Invite failed to send */
EOS_RESULT_VALUE(EOS_Sessions_InviteFailed, 5007)
/** Invite was not found with the service */
EOS_RESULT_VALUE(EOS_Sessions_InviteNotFound, 5008)
/** This client may not modify the session */
EOS_RESULT_VALUE(EOS_Sessions_UpsertNotAllowed, 5009)
/** Backend nodes unavailable to process request */
EOS_RESULT_VALUE(EOS_Sessions_AggregationFailed, 5010)
/** Individual backend node is as capacity */
EOS_RESULT_VALUE(EOS_Sessions_HostAtCapacity, 5011)
/** Sandbox on node is at capacity */
EOS_RESULT_VALUE(EOS_Sessions_SandboxAtCapacity, 5012)
/** An anonymous operation was attempted on a non anonymous session */
EOS_RESULT_VALUE(EOS_Sessions_SessionNotAnonymous, 5013)
/** Session is currently out of sync with the backend, data is saved locally but needs to sync with backend */
EOS_RESULT_VALUE(EOS_Sessions_OutOfSync, 5014)
/** User has received too many invites */
EOS_RESULT_VALUE(EOS_Sessions_TooManyInvites, 5015)
/** Presence session already exists for the client */
EOS_RESULT_VALUE(EOS_Sessions_PresenceSessionExists, 5016)
/** Deployment on node is at capacity */
EOS_RESULT_VALUE(EOS_Sessions_DeploymentAtCapacity, 5017)
/** Session operation not allowed */
EOS_RESULT_VALUE(EOS_Sessions_NotAllowed, 5018)

/** Request filename was invalid */
EOS_RESULT_VALUE(EOS_PlayerDataStorage_FilenameInvalid, 6000)
/** Request filename was too long */
EOS_RESULT_VALUE(EOS_PlayerDataStorage_FilenameLengthInvalid, 6001)
/** Request filename contained invalid characters */
EOS_RESULT_VALUE(EOS_PlayerDataStorage_FilenameInvalidChars, 6002)
/** Request operation would grow file too large */
EOS_RESULT_VALUE(EOS_PlayerDataStorage_FileSizeTooLarge, 6003)
/** Request file length is not valid */
EOS_RESULT_VALUE(EOS_PlayerDataStorage_FileSizeInvalid, 6004)
/** Request file handle is not valid */
EOS_RESULT_VALUE(EOS_PlayerDataStorage_FileHandleInvalid, 6005)
/** Request data is invalid */
EOS_RESULT_VALUE(EOS_PlayerDataStorage_DataInvalid, 6006)
/** Request data length was invalid */
EOS_RESULT_VALUE(EOS_PlayerDataStorage_DataLengthInvalid, 6007)
/** Request start index was invalid */
EOS_RESULT_VALUE(EOS_PlayerDataStorage_StartIndexInvalid, 6008)
/** Request is in progress */
EOS_RESULT_VALUE(EOS_PlayerDataStorage_RequestInProgress, 6009)
/** User is marked as throttled which means he can't perform some operations because limits are exceeded.  */
EOS_RESULT_VALUE(EOS_PlayerDataStorage_UserThrottled, 6010)
/** Encryption key is not set during SDK init.  */
EOS_RESULT_VALUE(EOS_PlayerDataStorage_EncryptionKeyNotSet, 6011)
/** User data callback returned error (EOS_PlayerDataStorage_EWriteResult::EOS_WR_FailRequest or EOS_PlayerDataStorage_EReadResult::EOS_RR_FailRequest) */
EOS_RESULT_VALUE(EOS_PlayerDataStorage_UserErrorFromDataCallback, 6012)
/** User is trying to read file that has header from newer version of SDK. Game/SDK needs to be updated. */
EOS_RESULT_VALUE(EOS_PlayerDataStorage_FileHeaderHasNewerVersion, 6013)
/** The file is corrupted. In some cases retry can fix the issue. */
EOS_RESULT_VALUE(EOS_PlayerDataStorage_FileCorrupted, 6014)

/** EOS Auth service deemed the external token invalid */
EOS_RESULT_VALUE(EOS_Connect_ExternalTokenValidationFailed, 7000)
/** EOS Auth user already exists */
EOS_RESULT_VALUE(EOS_Connect_UserAlreadyExists, 7001)
/** EOS Auth expired */
EOS_RESULT_VALUE(EOS_Connect_AuthExpired, 7002)
/** EOS Auth invalid token */
EOS_RESULT_VALUE(EOS_Connect_InvalidToken, 7003)
/** EOS Auth doesn't support this token type */
EOS_RESULT_VALUE(EOS_Connect_UnsupportedTokenType, 7004)
/** EOS Auth Account link failure */
EOS_RESULT_VALUE(EOS_Connect_LinkAccountFailed, 7005)
/** EOS Auth External service for validation was unavailable */
EOS_RESULT_VALUE(EOS_Connect_ExternalServiceUnavailable, 7006)
/** EOS Auth External Service configuration failure with Dev Portal */
EOS_RESULT_VALUE(EOS_Connect_ExternalServiceConfigurationFailure, 7007)
/** EOS Auth Account link failure. Tried to link Nintendo Network Service Account without first linking Nintendo Account. DEPRECATED: The requirement has been removed and this error is no longer used. */
EOS_RESULT_VALUE(EOS_Connect_LinkAccountFailedMissingNintendoIdAccount_DEPRECATED, 7008)

/** The social overlay page failed to load */
EOS_RESULT_VALUE(EOS_UI_SocialOverlayLoadError, 8000)

/** Client has no permissions to modify this lobby */
EOS_RESULT_VALUE(EOS_Lobby_NotOwner, 9000)
/** Lobby lock required for operation */
EOS_RESULT_VALUE(EOS_Lobby_InvalidLock, 9001)
/** Lobby already exists in the system */
EOS_RESULT_VALUE(EOS_Lobby_LobbyAlreadyExists, 9002)
/** Lobby is already in progress */
EOS_RESULT_VALUE(EOS_Lobby_SessionInProgress, 9003)
/** Too many players to register with this lobby */
EOS_RESULT_VALUE(EOS_Lobby_TooManyPlayers, 9004)
/** Client has no permissions to access this lobby */
EOS_RESULT_VALUE(EOS_Lobby_NoPermission, 9005)
/** Invalid lobby session reference */
EOS_RESULT_VALUE(EOS_Lobby_InvalidSession, 9006)
/** Sandbox ID associated with auth didn't match */
EOS_RESULT_VALUE(EOS_Lobby_SandboxNotAllowed, 9007)
/** Invite failed to send */
EOS_RESULT_VALUE(EOS_Lobby_InviteFailed, 9008)
/** Invite was not found with the service */
EOS_RESULT_VALUE(EOS_Lobby_InviteNotFound, 9009)
/** This client may not modify the lobby */
EOS_RESULT_VALUE(EOS_Lobby_UpsertNotAllowed, 9010)
/** Backend nodes unavailable to process request */
EOS_RESULT_VALUE(EOS_Lobby_AggregationFailed, 9011)
/** Individual backend node is as capacity */
EOS_RESULT_VALUE(EOS_Lobby_HostAtCapacity, 9012)
/** Sandbox on node is at capacity */
EOS_RESULT_VALUE(EOS_Lobby_SandboxAtCapacity, 9013)
/** User has received too many invites */
EOS_RESULT_VALUE(EOS_Lobby_TooManyInvites, 9014)
/** Deployment on node is at capacity */
EOS_RESULT_VALUE(EOS_Lobby_DeploymentAtCapacity, 9015)
/** Lobby operation not allowed */
EOS_RESULT_VALUE(EOS_Lobby_NotAllowed, 9016)
/** While restoring a lost connection lobby ownership changed and only local member data was updated */
EOS_RESULT_VALUE(EOS_Lobby_MemberUpdateOnly, 9017)
/** Presence lobby already exists for the client */
EOS_RESULT_VALUE(EOS_Lobby_PresenceLobbyExists, 9018)

/** User callback that receives data from storage returned error. */
EOS_RESULT_VALUE(EOS_TitleStorage_UserErrorFromDataCallback, 10000)
/** User forgot to set Encryption key during platform init. Title Storage can't work without it. */
EOS_RESULT_VALUE(EOS_TitleStorage_EncryptionKeyNotSet, 10001)
/** Downloaded file is corrupted. */
EOS_RESULT_VALUE(EOS_TitleStorage_FileCorrupted, 10002)
/** Downloaded file's format is newer than client SDK version. */
EOS_RESULT_VALUE(EOS_TitleStorage_FileHeaderHasNewerVersion, 10003)

/** ModSdk process is already running. This error comes from the EOSSDK. */
EOS_RESULT_VALUE(EOS_Mods_ModSdkProcessIsAlreadyRunning, 11000)
/** ModSdk command is empty. Either the ModSdk configuration file is missing or the manifest location is empty. */
EOS_RESULT_VALUE(EOS_Mods_ModSdkCommandIsEmpty, 11001)
/** Creation of the ModSdk process failed. This error comes from the SDK. */
EOS_RESULT_VALUE(EOS_Mods_ModSdkProcessCreationFailed, 11002)
/** A critical error occurred in the external ModSdk process that we were unable to resolve. */
EOS_RESULT_VALUE(EOS_Mods_CriticalError, 11003)
/** A internal error occurred in the external ModSdk process that we were unable to resolve. */
EOS_RESULT_VALUE(EOS_Mods_ToolInternalError, 11004)
/** A IPC failure occurred in the external ModSdk process. */
EOS_RESULT_VALUE(EOS_Mods_IPCFailure, 11005)
/** A invalid IPC response received in the external ModSdk process. */
EOS_RESULT_VALUE(EOS_Mods_InvalidIPCResponse, 11006)
/** A URI Launch failure occurred in the external ModSdk process. */
EOS_RESULT_VALUE(EOS_Mods_URILaunchFailure, 11007)
/** Attempting to perform an action with a mod that is not installed. This error comes from the external ModSdk process. */
EOS_RESULT_VALUE(EOS_Mods_ModIsNotInstalled, 11008)
/** Attempting to perform an action on a game that the user doesn't own. This error comes from the external ModSdk process. */
EOS_RESULT_VALUE(EOS_Mods_UserDoesNotOwnTheGame, 11009)
/** Invalid result of the request to get the offer for the mod. This error comes from the external ModSdk process. */
EOS_RESULT_VALUE(EOS_Mods_OfferRequestByIdInvalidResult, 11010)
/** Could not find the offer for the mod. This error comes from the external ModSdk process. */
EOS_RESULT_VALUE(EOS_Mods_CouldNotFindOffer, 11011)
/** Request to get the offer for the mod failed. This error comes from the external ModSdk process. */
EOS_RESULT_VALUE(EOS_Mods_OfferRequestByIdFailure, 11012)
/** Request to purchase the mod failed. This error comes from the external ModSdk process. */
EOS_RESULT_VALUE(EOS_Mods_PurchaseFailure, 11013)
/** Attempting to perform an action on a game that is not installed or is partially installed. This error comes from the external ModSdk process. */
EOS_RESULT_VALUE(EOS_Mods_InvalidGameInstallInfo, 11014)
/** Failed to get manifest location. Either the ModSdk configuration file is missing or the manifest location is empty */
EOS_RESULT_VALUE(EOS_Mods_CannotGetManifestLocation, 11015)
/** Attempting to perform an action with a mod that does not support the current operating system. */
EOS_RESULT_VALUE(EOS_Mods_UnsupportedOS, 11016)

/** An unexpected error that we cannot identify has occurred. */
EOS_RESULT_VALUE_LAST(EOS_UnexpectedError, 0x7FFFFFFF)
