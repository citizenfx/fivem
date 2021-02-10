// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "eos_common.h"

#pragma pack(push, 8)

EXTERN_C typedef struct EOS_MetricsHandle* EOS_HMetrics;

/** User game controller types. */
EOS_ENUM(EOS_EUserControllerType,
	/** The game controller type is unknown. */
	EOS_UCT_Unknown = 0,
	/** Mouse and keyboard controller. */
	EOS_UCT_MouseKeyboard = 1,
	/** Gamepad controller. */
	EOS_UCT_GamepadControl = 2,
	/** Touch controller. */
	EOS_UCT_TouchControl = 3
);

/** Account ID type for EOS_Metrics_BeginPlayerSession and EOS_Metrics_EndPlayerSession. */
EOS_ENUM(EOS_EMetricsAccountIdType,
	/** An Epic Online Services Account ID. */
	EOS_MAIT_Epic = 0,
	/** An external service Account ID. */
	EOS_MAIT_External = 1
);

/** The most recent version of the EOS_Metrics_BeginPlayerSessionOptions struct. */
#define EOS_METRICS_BEGINPLAYERSESSION_API_LATEST 1

/** BeginPlayerSession. */
EOS_STRUCT(EOS_Metrics_BeginPlayerSessionOptions, (
	/** API Version: Set this to EOS_METRICS_BEGINPLAYERSESSION_API_LATEST. */
	int32_t ApiVersion;
	/** Account ID type that is set in the union. */
	EOS_EMetricsAccountIdType AccountIdType;
	/** The Account ID for the player whose session is beginning. */
	union
	{
		/** An Epic Online Services Account ID. Set this field when AccountIdType is set to EOS_MAIT_Epic. */
		EOS_EpicAccountId Epic;
		/** An Account ID for another service. Set this field when AccountIdType is set to EOS_MAIT_External. */
		const char* External;
	} AccountId;
	/** The in-game display name for the user as UTF-8 string. */
	const char* DisplayName;
	/** The user's game controller type. */
	EOS_EUserControllerType ControllerType;
	/**
	 * IP address of the game server hosting the game session. For a localhost session, set to NULL.
	 *
	 * @details Must be in either one of the following IPv4 or IPv6 string formats:
	 * * "127.0.0.1".
	 * * "1200:0000:AB00:1234:0000:2552:7777:1313".
	 * If both IPv4 and IPv6 addresses are available, use the IPv6 address.
	 */
	const char* ServerIp;
	/**
	 * Optional, application-defined custom match session identifier. If the identifier is not used, set to NULL.
	 *
	 * @details The game can tag each game session with a custom session match identifier,
	 * which will be shown in the Played Sessions listing at the user profile dashboard.
	 */
	const char* GameSessionId;
));

/** The most recent version of the EOS_Metrics_EndPlayerSessionOptions struct. */
#define EOS_METRICS_ENDPLAYERSESSION_API_LATEST 1

/** EndPlayerSession. */
EOS_STRUCT(EOS_Metrics_EndPlayerSessionOptions, (
	/** API Version: Set this to EOS_METRICS_ENDPLAYERSESSION_API_LATEST. */
	int32_t ApiVersion;
	/** The Account ID type that is set in the union. */
	EOS_EMetricsAccountIdType AccountIdType;
	/** The Account ID for the player whose session is ending. */
	union
	{
		/** An Epic Online Services Account ID. Set this field when AccountIdType is set to EOS_MAIT_Epic. */
		EOS_EpicAccountId Epic;
		/** An Account ID for another service. Set this field when AccountIdType is set to EOS_MAIT_External. */
		const char* External;
	} AccountId;
));

#pragma pack(pop)
