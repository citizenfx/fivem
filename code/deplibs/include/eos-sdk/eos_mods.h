// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "eos_mods_types.h"

/**
 * The Mods Interface is used to manage a user's mods. Allowing a user to install/uninstall/enumerate mods.
 * All Mods Interface calls take a handle of type EOS_HMods as the first parameter.
 * This handle can be retrieved from a EOS_HPlatform handle by using the EOS_Platform_GetModsInterface function.
 *
 * NOTE: At this time, this feature is only available for desktop platforms and for products that are part of the Epic Games store.
 *
 * @see EOS_Platform_GetModsInterface
 */

/**
 * Starts an asynchronous task that makes a request to install the specified mod.
 *
 * @param Options structure containing the game and mod identifiers
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 *
 * @see the section related to mods in eos_result.h for more details.
 */
EOS_DECLARE_FUNC(void) EOS_Mods_InstallMod(EOS_HMods Handle, const EOS_Mods_InstallModOptions* Options, void* ClientData, const EOS_Mods_OnInstallModCallback CompletionDelegate);

/**
 * Starts an asynchronous task that makes a request to uninstall the specified mod.
 *
 * @param Options structure containing the game and mod identifiers
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 *
 * @see the section related to mods in eos_result.h for more details.
 */
EOS_DECLARE_FUNC(void) EOS_Mods_UninstallMod(EOS_HMods Handle, const EOS_Mods_UninstallModOptions* Options, void* ClientData, const EOS_Mods_OnUninstallModCallback CompletionDelegate);

/**
 * Starts an asynchronous task that makes a request to enumerate mods for the specified game.
 * Types of the mods to enumerate can be specified through EOS_Mods_EnumerateModsOptions
 *
 * @param Options structure containing the game identifiers
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 *
 * @see the section related to mods in eos_result.h for more details.
 */
EOS_DECLARE_FUNC(void) EOS_Mods_EnumerateMods(EOS_HMods Handle, const EOS_Mods_EnumerateModsOptions* Options, void* ClientData, const EOS_Mods_OnEnumerateModsCallback CompletionDelegate);

/**
 * Get cached enumerated mods object. If successful, this data must be released by calling EOS_Mods_ModInfo_Release
 * Types of the cached enumerated mods can be specified through EOS_Mods_CopyModInfoOptions
 *
 * @param Options structure containing the game identifier for which requesting enumerated mods
 * @param OutEnumeratedMods Enumerated mods Info. If the returned result is success, this will be set to data that must be later released, otherwise this will be set to NULL
 * @return Success if we have cached data, or an error result if the request was invalid or we do not have cached data.
 *
 * @see EOS_Mods_ModInfo_Release
 *
 * This request may fail with an EOS_NotFound code if an enumeration of a certain type was not performed before this call.
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Mods_CopyModInfo(EOS_HMods Handle, const EOS_Mods_CopyModInfoOptions* Options, EOS_Mods_ModInfo ** OutEnumeratedMods);

/**
 * Starts an asynchronous task that makes a request to update the specified mod to the latest version.
 *
 * @param Options structure containing the game and mod identifiers
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error. If the mod is up to date then the operation will complete with success.
 *
 * @see the section related to mods in eos_result.h for more details.
 */
EOS_DECLARE_FUNC(void) EOS_Mods_UpdateMod(EOS_HMods Handle, const EOS_Mods_UpdateModOptions* Options, void* ClientData, const EOS_Mods_OnUpdateModCallback CompletionDelegate);
