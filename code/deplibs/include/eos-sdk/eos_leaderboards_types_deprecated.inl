// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "eos_common.h"

/*
 * This file contains the deprecated types for EOS Leaderboards. In a future version, these types will be removed.
 */

#pragma pack(push, 8)

/* Do not use. Alias to the previous API name for backwards compatibility in compilation. */
EOS_DECLARE_FUNC(void) EOS_Leaderboards_LeaderboardDefinition_Release(EOS_Leaderboards_Definition* LeaderboardDefinition);

#pragma pack(pop)
