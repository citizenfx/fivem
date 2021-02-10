// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "eos_common.h"

#pragma pack(push, 8)

EXTERN_C typedef struct EOS_ModsHandle* EOS_HMods;

/** The most recent version of the EOS_Mod_Identifier struct. */
#define EOS_MOD_IDENTIFIER_API_LATEST 1

/**
 * EOS_Mod_Identifier is used to identify a mod.
 */
EOS_STRUCT(EOS_Mod_Identifier, (
	/** API Version: Set this to EOS_MOD_IDENTIFIER_API_LATEST. */
	int32_t ApiVersion;
	/** Product namespace id in which this mod item exists */
	const char* NamespaceId;
	/* Item id of the Mod */
	const char* ItemId;
	/* Artifact id of the Mod */
	const char* ArtifactId;
	/** Represent mod item title. */
	const char* Title;
	/** Represent mod item version. */
	const char* Version;
));


/**
 * EOS_Mods_InstallMod is used to start an asynchronous request to make a mod install request for a game.
 * The following types are used to work with the API.
 */


/** The most recent version of the EOS_Mods_InstallMod API. */
#define EOS_MODS_INSTALLMOD_API_LATEST 1

/**
 * Input parameters for the EOS_Mods_InstallMod Function.
 */
EOS_STRUCT(EOS_Mods_InstallModOptions, (
	/** API Version: Set this to EOS_MODS_INSTALLMOD_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the user for which the mod should be installed */
	EOS_EpicAccountId LocalUserId;
	/** The mod to install */
	const EOS_Mod_Identifier* Mod;
	/** Indicates whether the mod should be uninstalled after exiting the game or not. */
	EOS_Bool bRemoveAfterExit;
));

/**
 * Output parameters for the EOS_Mods_InstallMod Function. These parameters are received through the callback provided to EOS_Mods_InstallMod
 */
EOS_STRUCT(EOS_Mods_InstallModCallbackInfo, (
	/** Result code for the operation. EOS_Success is returned if the installation was successfull, otherwise one of the error codes is returned. */
	EOS_EResult ResultCode;
	/** The Epic Online Services Account ID of the user for which mod installation was requested */
	EOS_EpicAccountId LocalUserId;
	/** Context that was passed into to EOS_Mods_InstallMod */
	void* ClientData;
	/** Mod for which installation was requested */
	const EOS_Mod_Identifier* Mod;
));

/**
 * Function prototype definition for callbacks passed to EOS_Mods_InstallMod
 * @param Data A EOS_Mods_InstallModCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Mods_OnInstallModCallback, const EOS_Mods_InstallModCallbackInfo* Data);


/**
 * EOS_Mods_UninstallMod is used to start an asynchronous request to make a mod uninstall request for a game.
 * The following types are used to work with the API.
 */


/** The most recent version of the EOS_Mods_UninstallModOptions API. */
#define EOS_MODS_UNINSTALLMOD_API_LATEST 1

/**
 * Input parameters for the EOS_Mods_UninstallMod Function.
 */
EOS_STRUCT(EOS_Mods_UninstallModOptions, (
	/** API Version: Set this to EOS_MODS_UNINSTALLMOD_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the user for which the mod should be uninstalled */
	EOS_EpicAccountId LocalUserId;
	/** The mod to uninstall */
	const EOS_Mod_Identifier* Mod;
));

/**
 * Output parameters for the EOS_Mods_UninstallMod Function. These parameters are received through the callback provided to EOS_Mods_UninstallMod
 */
EOS_STRUCT(EOS_Mods_UninstallModCallbackInfo, (
	/** Result code for the operation. EOS_Success is returned if the uninstallation was successfull, otherwise one of the error codes is returned. */
	EOS_EResult ResultCode;
	/** The Epic Online Services Account ID of the user for which mod uninstallation was requested */
	EOS_EpicAccountId LocalUserId;
	/** Context that was passed into to EOS_Mods_UninstallMod */
	void* ClientData;
	/** Mod for which uninstallation was requested */
	const EOS_Mod_Identifier* Mod;
));

/**
 * Function prototype definition for callbacks passed to EOS_Mods_UninstallMod
 * @param Data A EOS_Mods_UninstallModCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Mods_OnUninstallModCallback, const EOS_Mods_UninstallModCallbackInfo* Data);


/**
 * EOS_Mods_EnumerateMods is used to start an asynchronous request to enumerate mods for a game.
 * The following types are used to work with the API.
 */


/** The type of mod enumeration. */
EOS_ENUM(EOS_EModEnumerationType,
	/** Installed mods */
	EOS_MET_INSTALLED = 0,
	/** All available mods*/
	EOS_MET_ALL_AVAILABLE
);

 /** The most recent version of the EOS_Mods_EnumerateModsOptions API. */
#define EOS_MODS_ENUMERATEMODS_API_LATEST 1

/**
 * Input parameters for the EOS_Mods_EnumerateMods Function.
 */
EOS_STRUCT(EOS_Mods_EnumerateModsOptions, (
	/** API Version: Set this to EOS_MODS_ENUMERATEMODS_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the user for which the mod should be enumerated */
	EOS_EpicAccountId LocalUserId;
	/** Type of the mods to enumerate */
	EOS_EModEnumerationType Type;
));

/**
 * Output parameters for the EOS_Mods_EnumerateMods Function. These parameters are received through the callback provided to EOS_Mods_EnumerateMods
 */
EOS_STRUCT(EOS_Mods_EnumerateModsCallbackInfo, (
	/** Result code for the operation. EOS_Success is returned if the enumeration was successfull, otherwise one of the error codes is returned. */
	EOS_EResult ResultCode;
	/** The Epic Online Services Account ID of the user for which mod enumeration was requested */
	EOS_EpicAccountId LocalUserId;
	/** Context that was passed into to EOS_Mods_EnumerateMods */
	void* ClientData;
	/** Type of the enumerated mods */
	EOS_EModEnumerationType Type;
));

/**
 * Function prototype definition for callbacks passed to EOS_Mods_EnumerateMods
 * @param Data A EOS_Mods_EnumerateModsCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Mods_OnEnumerateModsCallback, const EOS_Mods_EnumerateModsCallbackInfo* Data);

/** The most recent version of the EOS_Mods_CopyModInfoOptions API. */
#define EOS_MODS_COPYMODINFO_API_LATEST 1

/**
 * Data for the EOS_Mods_CopyModInfo function.
 */
EOS_STRUCT(EOS_Mods_CopyModInfoOptions, (
	/** API Version: Set this to EOS_MODS_COPYMODINFO_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the user for which mods should be copied */
	EOS_EpicAccountId LocalUserId;
	/** Type of the enumerated mod to copy */
	EOS_EModEnumerationType Type;
));

/** The most recent version of the EOS_Mods_ModInfo struct. */
#define EOS_MODS_MODINFO_API_LATEST 1

/**
 * Data for the EOS_Mods_CopyModInfo function.
 *
 * @see EOS_Mods_CopyModInfo
 * @see EOS_Mods_ModInfo_Release
 */
EOS_STRUCT(EOS_Mods_ModInfo, (
	/** API Version: Set this to EOS_MODS_MODINFO_API_LATEST. */
	int32_t ApiVersion;
	/** The count of enumerated mods */
	int32_t ModsCount;
	/** The array of enumerated mods or NULL if no such type of mods were enumerated */
	EOS_Mod_Identifier* Mods;
	/** Type of the mods */
	EOS_EModEnumerationType Type;
));

/**
 * Release the memory associated with an EOS_Mods_ModInfo structure and its sub-objects. This must be called on data retrieved from EOS_Mods_CopyModInfo.
 *
 * @param ModInfo the info structure to be release
 *
 * @see EOS_Mods_ModInfo
 * @see EOS_Mods_CopyModInfo
 */
EOS_DECLARE_FUNC(void) EOS_Mods_ModInfo_Release(EOS_Mods_ModInfo* ModInfo);

/** The most recent version of the EOS_Mods_UpdateModOptions API. */
#define EOS_MODS_UPDATEMOD_API_LATEST 1

/**
 * Input parameters for the EOS_Mods_UpdateMod Function.
 */
EOS_STRUCT(EOS_Mods_UpdateModOptions, (
	/** API Version: Set this to EOS_MODS_UPDATEMOD_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the user for which the mod should be updated */
	EOS_EpicAccountId LocalUserId;
	/** The mod to update */
	const EOS_Mod_Identifier* Mod;
));

/**
 * Output parameters for the EOS_Mods_UpdateMod Function. These parameters are received through the callback provided to EOS_Mods_UpdateMod
 */
EOS_STRUCT(EOS_Mods_UpdateModCallbackInfo, (
	/** Result code for the operation. EOS_Success is returned if the request to update was successfull, otherwise one of the error codes is returned. */
	EOS_EResult ResultCode;
	/** The Epic Online Services Account ID of the user for which mod update was requested */
	EOS_EpicAccountId LocalUserId;
	/** Context that was passed into to EOS_Mods_UpdateMod */
	void* ClientData;
	/** Mod for which update was requested */
	const EOS_Mod_Identifier* Mod;
));

/**
 * Function prototype definition for callbacks passed to EOS_Mods_UpdateMod
 * @param Data A EOS_Mods_UpdateModCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Mods_OnUpdateModCallback, const EOS_Mods_UpdateModCallbackInfo* Data);

#pragma pack(pop)