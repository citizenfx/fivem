// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "eos_common.h"

#pragma pack(push, 8)

EXTERN_C typedef struct EOS_EcomHandle* EOS_HEcom;

/**
 * This handle is copied when EOS_Ecom_CopyTransactionById or EOS_Ecom_CopyTransactionByIndex is called.
 * A EOS_Ecom_CheckoutCallbackInfo provides the ID for the copy.
 * After being copied, EOS_Ecom_Transaction_Release must be called.
 *
 * @see EOS_Ecom_CheckoutCallbackInfo
 * @see EOS_Ecom_CopyTransactionById
 * @see EOS_Ecom_CopyTransactionByIndex
 * @see EOS_Ecom_Transaction_Release
 */
EXTERN_C typedef struct EOS_Ecom_TransactionHandle* EOS_Ecom_HTransaction;

/**
 * A unique identifier for a catalog item defined and stored with the backend catalog service.
 * A catalog item represents a distinct object within the catalog.  When acquired by an account, an
 * entitlement is granted that references a specific catalog item.
 */
EXTERN_C typedef const char* EOS_Ecom_CatalogItemId;

/**
 * A unique identifier for a catalog offer defined and stored with the backend catalog service.
 * A catalog offer is a purchasable collection of 1 or more items, associated with a price (which
 * could be 0).  When an offer is purchased an entitlement is granted for each of the items
 * referenced by the offer.
 */
EXTERN_C typedef const char* EOS_Ecom_CatalogOfferId;

/**
 * An identifier which is defined on a catalog item and stored with the backend catalog service.
 * The entitlement name may not be unique.  A catalog may be configured with multiple items with the
 * same entitlement name in order to define a logical grouping of entitlements.  This is used to
 * retrieve all entitlements granted to an account grouped in this way.
 *
 * @see EOS_Ecom_QueryEntitlements
 */
EXTERN_C typedef const char* EOS_Ecom_EntitlementName;

/**
 * A unique identifier for an entitlement owned by an account.  An entitlement is always associated
 * with a single account.  The entitlement ID is provided to allow redeeming the entitlement as
 * well as identify individual entitlement grants.
 *
 * @see EOS_Ecom_QueryEntitlements
 * @see EOS_Ecom_RedeemEntitlements
 */
EXTERN_C typedef const char* EOS_Ecom_EntitlementId;


/**
 * An enumeration of the different ownership statuses.
 */
EOS_ENUM(EOS_EOwnershipStatus,
	/** The catalog item is not owned by the local user */
	EOS_OS_NotOwned = 0,
	/** The catalog item is owned by the local user */
	EOS_OS_Owned = 1
);

/**
 * An enumeration defining the type of catalog item.  The primary use is to identify how the item is expended.
 */
EOS_ENUM(EOS_EEcomItemType,
	/** This entitlement is intended to persist. */
	EOS_EIT_Durable = 0,
	/**
	 * This entitlement is intended to be transient and redeemed.
	 *
	 * @see EOS_Ecom_RedeemEntitlements
	 */
	EOS_EIT_Consumable = 1,
	/** This entitlement has a type that is not currently intneded for an in-game store. */
	EOS_EIT_Other = 2
);

/** The most recent version of the EOS_Ecom_Entitlement struct. */
#define EOS_ECOM_ENTITLEMENT_API_LATEST 2

/** Timestamp value representing an undefined EndTimestamp for EOS_Ecom_Entitlement */
#define EOS_ECOM_ENTITLEMENT_ENDTIMESTAMP_UNDEFINED -1

/**
 * Contains information about a single entitlement associated with an account. Instances of this structure are
 * created by EOS_Ecom_CopyEntitlementByIndex, EOS_Ecom_CopyEntitlementByNameAndIndex, or EOS_Ecom_CopyEntitlementById.
 * They must be passed to EOS_Ecom_Entitlement_Release.
 */
EOS_STRUCT(EOS_Ecom_Entitlement, (
	/** API Version: Set this to EOS_ECOM_ENTITLEMENT_API_LATEST. */
	int32_t ApiVersion;
	/** Name of the entitlement */
	EOS_Ecom_EntitlementName EntitlementName;
	/** ID of the entitlement owned by an account */
	EOS_Ecom_EntitlementId EntitlementId;
	/** ID of the item associated with the offer which granted this entitlement */
	EOS_Ecom_CatalogItemId CatalogItemId;
	/**
	 * If queried using pagination then ServerIndex represents the index of the entitlement as it
	 * exists on the server.  If not queried using pagination then ServerIndex will be -1.
	 */
	int32_t ServerIndex;
	/** If true then the catalog has this entitlement marked as redeemed */
	EOS_Bool bRedeemed;
	/** If not -1 then this is a POSIX timestamp that this entitlement will end */
	int64_t EndTimestamp;
));

/**
 * Release the memory associated with an EOS_Ecom_Entitlement structure. This must be called on data
 * retrieved from EOS_Ecom_CopyEntitlementByIndex and EOS_Ecom_CopyEntitlementById.
 *
 * @param Entitlement - The entitlement structure to be released
 *
 * @see EOS_Ecom_Entitlement
 * @see EOS_Ecom_CopyEntitlementByIndex
 * @see EOS_Ecom_CopyEntitlementById
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_Entitlement_Release(EOS_Ecom_Entitlement* Entitlement);

/** The most recent version of the EOS_Ecom_ItemOwnership struct. */
#define EOS_ECOM_ITEMOWNERSHIP_API_LATEST 1

/**
 * Contains information about a single item ownership associated with an account. This structure is
 * returned as part of the EOS_Ecom_QueryOwnershipCallbackInfo structure.
 */
EOS_STRUCT(EOS_Ecom_ItemOwnership, (
	/** API Version: Set this to EOS_ECOM_ITEMOWNERSHIP_API_LATEST. */
	int32_t ApiVersion;
	/** ID of the catalog item */
	EOS_Ecom_CatalogItemId Id;
	/** Is this catalog item owned by the local user */
	EOS_EOwnershipStatus OwnershipStatus;
));

/** The most recent version of the EOS_Ecom_CatalogItem struct. */
#define EOS_ECOM_CATALOGITEM_API_LATEST 1

/** Timestamp value representing an undefined EntitlementEndTimestamp for EOS_Ecom_CatalogItem */
#define EOS_ECOM_CATALOGITEM_ENTITLEMENTENDTIMESTAMP_UNDEFINED -1

/**
 * Contains information about a single item within the catalog. Instances of this structure are created
 * by EOS_Ecom_CopyOfferItemByIndex. They must be passed to EOS_Ecom_CatalogItem_Release.
 */
EOS_STRUCT(EOS_Ecom_CatalogItem, (
	/** API Version: Set this to EOS_ECOM_CATALOGITEM_API_LATEST. */
	int32_t ApiVersion;
	/** Product namespace in which this item exists */
	const char* CatalogNamespace;
	/** The ID of this item */
	EOS_Ecom_CatalogItemId Id;
	/** The entitlement name associated with this item */
	EOS_Ecom_EntitlementName EntitlementName;
	/** Localized UTF-8 title of this item */
	const char* TitleText;
	/** Localized UTF-8 description of this item */
	const char* DescriptionText;
	/** Localized UTF-8 long description of this item */
	const char* LongDescriptionText;
	/** Localized UTF-8 technical details of this item */
	const char* TechnicalDetailsText;
	/** Localized UTF-8 developer of this item */
	const char* DeveloperText;
	/** The type of item as defined in the catalog */
	EOS_EEcomItemType ItemType;
	/** If not -1 then this is the POSIX timestamp that the entitlement will end */
	int64_t EntitlementEndTimestamp;
));

/**
 * Release the memory associated with an EOS_Ecom_CatalogItem structure. This must be called on data
 * retrieved from EOS_Ecom_CopyOfferItemByIndex.
 *
 * @param CatalogItem - The catalog item structure to be released
 *
 * @see EOS_Ecom_CatalogItem
 * @see EOS_Ecom_CopyOfferItemByIndex
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_CatalogItem_Release(EOS_Ecom_CatalogItem* CatalogItem);

/** The most recent version of the EOS_Ecom_CatalogOffer struct. */
#define EOS_ECOM_CATALOGOFFER_API_LATEST 2

/** Timestamp value representing an undefined ExpirationTimestamp for EOS_Ecom_CatalogOffer */
#define EOS_ECOM_CATALOGOFFER_EXPIRATIONTIMESTAMP_UNDEFINED -1

/**
 * Contains information about a single offer within the catalog. Instances of this structure are
 * created by EOS_Ecom_CopyOfferByIndex. They must be passed to EOS_Ecom_CatalogOffer_Release.
 * Prices are stored in the lowest denomination for the associated currency.  If CurrencyCode is
 * "USD" then a price of 299 represents "$2.99".
 */
EOS_STRUCT(EOS_Ecom_CatalogOffer, (
	/** API Version: Set this to EOS_ECOM_CATALOGOFFER_API_LATEST. */
	int32_t ApiVersion;
	/**
	 * The index of this offer as it exists on the server.
	 * This is useful for understanding pagination data.
	 */
	int32_t ServerIndex;
	/** Product namespace in which this offer exists */
	const char* CatalogNamespace;
	/** The ID of this offer */
	EOS_Ecom_CatalogOfferId Id;
	/** Localized UTF-8 title of this offer */
	const char* TitleText;
	/** Localized UTF-8 description of this offer */
	const char* DescriptionText;
	/** Localized UTF-8 long description of this offer */
	const char* LongDescriptionText;
	/**
	 * Deprecated.
	 * EOS_Ecom_CatalogOffer::TechnicalDetailsText has been deprecated.
	 * EOS_Ecom_CatalogItem::TechnicalDetailsText is still valid.
	 */
	const char* TechnicalDetailsText_DEPRECATED;
	/** The Currency Code for this offer */
	const char* CurrencyCode;
	/**
	 * If this value is EOS_Success then OriginalPrice, CurrentPrice, and DiscountPercentage contain valid data.
	 * Otherwise this value represents the error that occurred on the price query.
	 */
	EOS_EResult PriceResult;
	/** The original price of this offer. */
	uint32_t OriginalPrice;
	/** The current price including discounts of this offer. */
	uint32_t CurrentPrice;
	/** A value from 0 to 100 define the percentage of the OrignalPrice that the CurrentPrice represents */
	uint8_t DiscountPercentage;
	/** Contains the POSIX timestamp that the offer expires or -1 if it does not expire */
	int64_t ExpirationTimestamp;
	/** The number of times that the requesting account has purchased this offer. */
	uint32_t PurchasedCount;
	/**
	 * The maximum number of times that the offer can be purchased.
	 * A negative value implies there is no limit.
	 */
	int32_t PurchaseLimit;
	/** True if the user can purchase this offer. */
	EOS_Bool bAvailableForPurchase;
));

/**
 * Release the memory associated with an EOS_Ecom_CatalogOffer structure. This must be called on data
 * retrieved from EOS_Ecom_CopyOfferByIndex.
 *
 * @param CatalogOffer - The catalog offer structure to be released
 *
 * @see EOS_Ecom_CatalogOffer
 * @see EOS_Ecom_CopyOfferByIndex
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_CatalogOffer_Release(EOS_Ecom_CatalogOffer* CatalogOffer);

/** The most recent version of the EOS_Ecom_KeyImageInfo struct. */
#define EOS_ECOM_KEYIMAGEINFO_API_LATEST 1

/**
 * Contains information about a key image used by the catalog.  Instances of this structure are
 * created by EOS_Ecom_CopyItemImageInfoByIndex.  They must be passed to EOS_Ecom_KeyImageInfo_Release.
 * A Key Image is defined within Dev Portal and is associated with a Catalog Item.  A Key Image is
 * intended to be used to provide imagery for an in-game store.
 *
 * @see EOS_Ecom_CopyItemImageInfoByIndex
 * @see EOS_Ecom_KeyImageInfo_Release
 */
EOS_STRUCT(EOS_Ecom_KeyImageInfo, (
	/** API Version: Set this to EOS_ECOM_KEYIMAGEINFO_API_LATEST. */
	int32_t ApiVersion;
	/** Describes the usage of the image (ex: home_thumbnail) */
	const char* Type;
	/** The URL of the image */
	const char* Url;
	/** The expected width in pixels of the image */
	uint32_t Width;
	/** The expected height in pixels of the image */
	uint32_t Height;
));

/**
 * Release the memory associated with an EOS_Ecom_KeyImageInfo structure. This must be called on data
 * retrieved from EOS_Ecom_CopyItemImageInfoByIndex.
 *
 * @param KeyImageInfo - The key image info structure to be released
 *
 * @see EOS_Ecom_KeyImageInfo
 * @see EOS_Ecom_CopyItemImageInfoByIndex
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_KeyImageInfo_Release(EOS_Ecom_KeyImageInfo* KeyImageInfo);

/** The most recent version of the EOS_Ecom_CatalogRelease struct. */
#define EOS_ECOM_CATALOGRELEASE_API_LATEST 1

/**
 * Contains information about a single release within the catalog. Instances of this structure are
 * created by EOS_Ecom_CopyItemReleaseByIndex. They must be passed to EOS_Ecom_CatalogRelease_Release.
 */
EOS_STRUCT(EOS_Ecom_CatalogRelease, (
	/** API Version: Set this to EOS_ECOM_CATALOGRELEASE_API_LATEST. */
	int32_t ApiVersion;
	/** The number of APP IDs */
	uint32_t CompatibleAppIdCount;
	/** A list of compatible APP IDs */
	const char** CompatibleAppIds;
	/** The number of platforms */
	uint32_t CompatiblePlatformCount;
	/** A list of compatible Platforms */
	const char** CompatiblePlatforms;
	/** Release note for compatible versions */
	const char* ReleaseNote;
));

/**
 * Release the memory associated with an EOS_Ecom_CatalogRelease structure. This must be called on
 * data retrieved from EOS_Ecom_CopyItemReleaseByIndex.
 *
 * @param CatalogRelease - The catalog release structure to be released
 *
 * @see EOS_Ecom_CatalogRelease
 * @see EOS_Ecom_CopyItemReleaseByIndex
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_CatalogRelease_Release(EOS_Ecom_CatalogRelease* CatalogRelease);

/** The most recent version of the EOS_Ecom_CheckoutEntry struct. */
#define EOS_ECOM_CHECKOUTENTRY_API_LATEST 1

/**
 * Contains information about a request to purchase a single offer.  This structure is set as part
 * of the EOS_Ecom_CheckoutOptions structure.
 */
EOS_STRUCT(EOS_Ecom_CheckoutEntry, (
	/** API Version: Set this to EOS_ECOM_CHECKOUTENTRY_API_LATEST. */
	int32_t ApiVersion;
	/** The ID of the offer to purchase */
	EOS_Ecom_CatalogOfferId OfferId;
));

/** The most recent version of the EOS_Ecom_QueryOwnership API. */
#define EOS_ECOM_QUERYOWNERSHIP_API_LATEST 2

/**
 * The maximum number of catalog items that may be queried in a single pass
 */
#define EOS_ECOM_QUERYOWNERSHIP_MAX_CATALOG_IDS 32

/**
 * Input parameters for the EOS_Ecom_QueryOwnership function.
 */
EOS_STRUCT(EOS_Ecom_QueryOwnershipOptions, (
	/** API Version: Set this to EOS_ECOM_QUERYOWNERSHIP_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose ownership to query */
	EOS_EpicAccountId LocalUserId;
	/** The array of Catalog Item IDs to check for ownership */
	EOS_Ecom_CatalogItemId* CatalogItemIds;
	/** The number of Catalog Item IDs to in the array */
	uint32_t CatalogItemIdCount;
	/** Optional product namespace, if not the one specified during initialization */
	const char* CatalogNamespace;
));

/**
 * Output parameters for the EOS_Ecom_QueryOwnership Function.
 */
EOS_STRUCT(EOS_Ecom_QueryOwnershipCallbackInfo, (
	/** The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Ecom_QueryOwnership */
	void* ClientData;
	/** The Epic Online Services Account ID of the local user whose ownership was queried */
	EOS_EpicAccountId LocalUserId;
	/** List of catalog items and their ownership status */
	const EOS_Ecom_ItemOwnership* ItemOwnership;
	/** Number of ownership results are included in this callback */
	uint32_t ItemOwnershipCount;
));

/**
 * Function prototype definition for callbacks passed to EOS_Ecom_QueryOwnership
 * @param Data A EOS_Ecom_QueryOwnershipCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Ecom_OnQueryOwnershipCallback, const EOS_Ecom_QueryOwnershipCallbackInfo* Data);

/** The most recent version of the EOS_Ecom_QueryOwnershipToken API. */
#define EOS_ECOM_QUERYOWNERSHIPTOKEN_API_LATEST 2

/**
 * The maximum number of catalog items that may be queried in a single pass
 */
#define EOS_ECOM_QUERYOWNERSHIPTOKEN_MAX_CATALOGITEM_IDS 32

/**
 * Input parameters for the EOS_Ecom_QueryOwnershipToken function.
 */
EOS_STRUCT(EOS_Ecom_QueryOwnershipTokenOptions, (
	/** API Version: Set this to EOS_ECOM_QUERYOWNERSHIPTOKEN_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose ownership token you want to query */
	EOS_EpicAccountId LocalUserId;
	/** The array of Catalog Item IDs to check for ownership, matching in number to the CatalogItemIdCount */
	EOS_Ecom_CatalogItemId* CatalogItemIds;
	/** The number of catalog item IDs to query */
	uint32_t CatalogItemIdCount;
	/** Optional product namespace, if not the one specified during initialization */
	const char* CatalogNamespace;
));

/**
 * Output parameters for the EOS_Ecom_QueryOwnershipToken Function.
 */
EOS_STRUCT(EOS_Ecom_QueryOwnershipTokenCallbackInfo, (
	/** The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Ecom_QueryOwnershipToken */
	void* ClientData;
	/** The Epic Online Services Account ID of the local user whose ownership token was queried */
	EOS_EpicAccountId LocalUserId;
	/** Ownership token containing details about the catalog items queried */
	const char* OwnershipToken;
));

/**
 * Function prototype definition for callbacks passed to EOS_Ecom_QueryOwnershipToken
 * @param Data A EOS_Ecom_QueryOwnershipTokenCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Ecom_OnQueryOwnershipTokenCallback, const EOS_Ecom_QueryOwnershipTokenCallbackInfo* Data);

/** The most recent version of the EOS_Ecom_QueryEntitlements API. */
#define EOS_ECOM_QUERYENTITLEMENTS_API_LATEST 2

/**
 * The maximum number of entitlements that may be queried in a single pass
 */
#define EOS_ECOM_QUERYENTITLEMENTS_MAX_ENTITLEMENT_IDS 32

/**
 * Input parameters for the EOS_Ecom_QueryEntitlements function.
 */
EOS_STRUCT(EOS_Ecom_QueryEntitlementsOptions, (
	/** API Version: Set this to EOS_ECOM_QUERYENTITLEMENTS_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose Entitlements you want to retrieve */
	EOS_EpicAccountId LocalUserId;
	/** An array of Entitlement Names that you want to check */
	EOS_Ecom_EntitlementName* EntitlementNames;
	/** The number of Entitlement Names included in the array, up to EOS_ECOM_QUERYENTITLEMENTS_MAX_ENTITLEMENT_IDS; use zero to request all Entitlements associated with the user's Epic Online Services account. */
	uint32_t EntitlementNameCount;
	/** If true, Entitlements that have been redeemed will be included in the results. */
	EOS_Bool bIncludeRedeemed;
));

/**
 * Output parameters for the EOS_Ecom_QueryEntitlements Function.
 */
EOS_STRUCT(EOS_Ecom_QueryEntitlementsCallbackInfo, (
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Ecom_QueryEntitlements */
	void* ClientData;
	/** The Epic Online Services Account ID of the local user whose entitlement was queried */
	EOS_EpicAccountId LocalUserId;
));

/**
 * Function prototype definition for callbacks passed to EOS_Ecom_QueryOwnershipToken
 * @param Data A EOS_Ecom_QueryEntitlementsCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Ecom_OnQueryEntitlementsCallback, const EOS_Ecom_QueryEntitlementsCallbackInfo* Data);


/** The most recent version of the EOS_Ecom_QueryOffers API. */
#define EOS_ECOM_QUERYOFFERS_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_QueryOffers function.
 */
EOS_STRUCT(EOS_Ecom_QueryOffersOptions, (
	/** API Version: Set this to EOS_ECOM_QUERYOFFERS_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose offer to query */
	EOS_EpicAccountId LocalUserId;
	/** If not provided then the SandboxId is used as the catalog namespace */
	const char* OverrideCatalogNamespace;
));

/**
 * Output parameters for the EOS_Ecom_QueryOffers Function.
 */
EOS_STRUCT(EOS_Ecom_QueryOffersCallbackInfo, (
	/** The EOS_EResult code for the operation. EOS_Success indicates that the operation succeeded; other codes indicate errors. */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Ecom_QueryOffers */
	void* ClientData;
	/** The Epic Online Services Account ID of the local user whose offer was queried; needed for localization of Catalog Item (Item) description text and pricing information */
	EOS_EpicAccountId LocalUserId;
));

/**
 * Function prototype definition for callbacks passed to EOS_Ecom_QueryOffers
 * @param Data A EOS_Ecom_QueryOffersCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Ecom_OnQueryOffersCallback, const EOS_Ecom_QueryOffersCallbackInfo* Data);


/** The most recent version of the EOS_Ecom_Checkout API. */
#define EOS_ECOM_CHECKOUT_API_LATEST 1

/** The maximum number of entries in a single checkout. */
#define EOS_ECOM_CHECKOUT_MAX_ENTRIES 10

/** The maximum length of a transaction ID. */
#define EOS_ECOM_TRANSACTIONID_MAXIMUM_LENGTH 64
/**
 * Input parameters for the EOS_Ecom_Checkout function.
 */
EOS_STRUCT(EOS_Ecom_CheckoutOptions, (
	/** API Version: Set this to EOS_ECOM_CHECKOUT_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user who is making the purchase */
	EOS_EpicAccountId LocalUserId;
	/** The catalog namespace will be the current Sandbox ID (in EOS_Platform_Options) unless overridden by this field */
	const char* OverrideCatalogNamespace;
	/** The number of EOS_Ecom_CheckoutEntry elements contained in Entries */
	uint32_t EntryCount;
	/** An array of EOS_Ecom_CheckoutEntry elements, each containing the details of a single offer */
	const EOS_Ecom_CheckoutEntry* Entries;
));

/**
 * Output parameters for the EOS_Ecom_Checkout Function.
 */
EOS_STRUCT(EOS_Ecom_CheckoutCallbackInfo, (
	/** Result code for the operation. EOS_Success is returned for a successful request, otherwise one of the error codes is returned. See eos_common.h */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Ecom_Checkout */
	void* ClientData;
	/** The Epic Online Services Account ID of the user who initiated the purchase */
	EOS_EpicAccountId LocalUserId;
	/** The transaction ID which can be used to obtain an EOS_Ecom_HTransaction using EOS_Ecom_CopyTransactionById. */
	const char* TransactionId;
));

/**
 * Function prototype definition for callbacks passed to EOS_Ecom_Checkout
 * @param Data A EOS_Ecom_CheckoutCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Ecom_OnCheckoutCallback, const EOS_Ecom_CheckoutCallbackInfo* Data);


/** The most recent version of the EOS_Ecom_RedeemEntitlements API. */
#define EOS_ECOM_REDEEMENTITLEMENTS_API_LATEST 1

/**
 * The maximum number of entitlement IDs that may be redeemed in a single pass
 */
#define EOS_ECOM_REDEEMENTITLEMENTS_MAX_IDS 32

/**
 * Input parameters for the EOS_Ecom_RedeemEntitlements function.
 */
EOS_STRUCT(EOS_Ecom_RedeemEntitlementsOptions, (
	/** API Version: Set this to EOS_ECOM_REDEEMENTITLEMENTS_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the user who is redeeming Entitlements */
	EOS_EpicAccountId LocalUserId;
	/** The number of Entitlements to redeem */
	uint32_t EntitlementIdCount;
	/** The array of Entitlements to redeem */
	EOS_Ecom_EntitlementId* EntitlementIds;
));

/**
 * Output parameters for the EOS_Ecom_RedeemEntitlements Function.
 */
EOS_STRUCT(EOS_Ecom_RedeemEntitlementsCallbackInfo, (
	/** Result code for the operation. EOS_Success is returned for a successful request, otherwise one of the error codes is returned. See eos_common.h */
	EOS_EResult ResultCode;
	/** Context that was passed into EOS_Ecom_RedeemEntitlements */
	void* ClientData;
	/** The Epic Online Services Account ID of the user who has redeemed entitlements */
	EOS_EpicAccountId LocalUserId;
));

/**
 * Function prototype definition for callbacks passed to EOS_Ecom_RedeemEntitlements
 * @param Data A EOS_Ecom_RedeemEntitlementsCallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Ecom_OnRedeemEntitlementsCallback, const EOS_Ecom_RedeemEntitlementsCallbackInfo* Data);


/** The most recent version of the EOS_Ecom_GetEntitlementsCount API. */
#define EOS_ECOM_GETENTITLEMENTSCOUNT_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_GetEntitlementsCount function.
 */
EOS_STRUCT(EOS_Ecom_GetEntitlementsCountOptions, (
	/** API Version: Set this to EOS_ECOM_GETENTITLEMENTSCOUNT_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user for which to retrieve the entitlement count */
	EOS_EpicAccountId LocalUserId;
));

/** The most recent version of the EOS_Ecom_GetEntitlementsByNameCount API. */
#define EOS_ECOM_GETENTITLEMENTSBYNAMECOUNT_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_GetEntitlementsByNameCount function.
 */
EOS_STRUCT(EOS_Ecom_GetEntitlementsByNameCountOptions, (
	/** API Version: Set this to EOS_ECOM_GETENTITLEMENTSBYNAMECOUNT_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user for which to retrieve the entitlement count */
	EOS_EpicAccountId LocalUserId;
	/** Name of the entitlement to count in the cache */
	EOS_Ecom_EntitlementName EntitlementName;
));

/** The most recent version of the EOS_Ecom_CopyEntitlementByIndex API. */
#define EOS_ECOM_COPYENTITLEMENTBYINDEX_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_CopyEntitlementByIndex function.
 */
EOS_STRUCT(EOS_Ecom_CopyEntitlementByIndexOptions, (
	/** API Version: Set this to EOS_ECOM_COPYENTITLEMENTBYINDEX_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose entitlement is being copied */
	EOS_EpicAccountId LocalUserId;
	/** Index of the entitlement to retrieve from the cache */
	uint32_t EntitlementIndex;
));

/** The most recent version of the EOS_Ecom_CopyEntitlementByNameAndIndex API. */
#define EOS_ECOM_COPYENTITLEMENTBYNAMEANDINDEX_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_CopyEntitlementByNameAndIndex function.
 */
EOS_STRUCT(EOS_Ecom_CopyEntitlementByNameAndIndexOptions, (
	/** API Version: Set this to EOS_ECOM_COPYENTITLEMENTBYNAMEANDINDEX_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose entitlement is being copied */
	EOS_EpicAccountId LocalUserId;
	/** Name of the entitlement to retrieve from the cache */
	EOS_Ecom_EntitlementName EntitlementName;
	/** Index of the entitlement within the named set to retrieve from the cache. */
	uint32_t Index;
));

/** The most recent version of the EOS_Ecom_CopyEntitlementById API. */
#define EOS_ECOM_COPYENTITLEMENTBYID_API_LATEST 2

/**
 * Input parameters for the EOS_Ecom_CopyEntitlementById function.
 */
EOS_STRUCT(EOS_Ecom_CopyEntitlementByIdOptions, (
	/** API Version: Set this to EOS_ECOM_COPYENTITLEMENTBYID_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose entitlement is being copied */
	EOS_EpicAccountId LocalUserId;
	/** ID of the entitlement to retrieve from the cache */
	EOS_Ecom_EntitlementId EntitlementId;
));

/** The most recent version of the EOS_Ecom_GetOfferCount API. */
#define EOS_ECOM_GETOFFERCOUNT_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_GetOfferCount function.
 */
EOS_STRUCT(EOS_Ecom_GetOfferCountOptions, (
	/** API Version: Set this to EOS_ECOM_GETOFFERCOUNT_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose offers are being accessed */
	EOS_EpicAccountId LocalUserId;
));

/** The most recent version of the EOS_Ecom_CopyOfferByIndex API. */
#define EOS_ECOM_COPYOFFERBYINDEX_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_CopyOfferByIndex function.
 */
EOS_STRUCT(EOS_Ecom_CopyOfferByIndexOptions, (
	/** API Version: Set this to EOS_ECOM_COPYOFFERBYINDEX_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose offer is being copied */
	EOS_EpicAccountId LocalUserId;
	/** The index of the offer to get. */
	uint32_t OfferIndex;
));

/** The most recent version of the EOS_Ecom_CopyOfferById API. */
#define EOS_ECOM_COPYOFFERBYID_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_CopyOfferById function.
 */
EOS_STRUCT(EOS_Ecom_CopyOfferByIdOptions, (
	/** API Version: Set this to EOS_ECOM_COPYOFFERBYID_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose offer is being copied */
	EOS_EpicAccountId LocalUserId;
	/** The ID of the offer to get. */
	EOS_Ecom_CatalogOfferId OfferId;
));

/** The most recent version of the EOS_Ecom_GetOfferItemCount API. */
#define EOS_ECOM_GETOFFERITEMCOUNT_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_GetOfferItemCount function.
 */
EOS_STRUCT(EOS_Ecom_GetOfferItemCountOptions, (
	/** API Version: Set this to EOS_ECOM_GETOFFERITEMCOUNT_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user who made the initial request for the Catalog Offer through EOS_Ecom_QueryOffers */
	EOS_EpicAccountId LocalUserId;
	/** An ID that corresponds to a cached Catalog Offer (retrieved by EOS_Ecom_CopyOfferByIndex) */
	EOS_Ecom_CatalogOfferId OfferId;
));

/** The most recent version of the EOS_Ecom_CopyOfferItemByIndex API. */
#define EOS_ECOM_COPYOFFERITEMBYINDEX_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_CopyOfferItemByIndex function.
 */
EOS_STRUCT(EOS_Ecom_CopyOfferItemByIndexOptions, (
	/** API Version: Set this to EOS_ECOM_COPYOFFERITEMBYINDEX_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose item is being copied */
	EOS_EpicAccountId LocalUserId;
	/** The ID of the offer to get the items for. */
	EOS_Ecom_CatalogOfferId OfferId;
	/** The index of the item to get. */
	uint32_t ItemIndex;
));

/** The most recent version of the EOS_Ecom_CopyItemById API. */
#define EOS_ECOM_COPYITEMBYID_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_CopyItemById function.
 */
EOS_STRUCT(EOS_Ecom_CopyItemByIdOptions, (
	/** API Version: Set this to EOS_ECOM_COPYITEMBYID_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose item is being copied */
	EOS_EpicAccountId LocalUserId;
	/** The ID of the item to get. */
	EOS_Ecom_CatalogItemId ItemId;
));

/** The most recent version of the EOS_Ecom_GetOfferImageInfoCount API. */
#define EOS_ECOM_GETOFFERIMAGEINFOCOUNT_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_GetOfferImageInfoCount function.
 */
EOS_STRUCT(EOS_Ecom_GetOfferImageInfoCountOptions, (
	/** API Version: Set this to EOS_ECOM_GETOFFERIMAGEINFOCOUNT_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose offer image is being accessed. */
	EOS_EpicAccountId LocalUserId;
	/** The ID of the offer to get the images for. */
	EOS_Ecom_CatalogOfferId OfferId;
));

/** The most recent version of the EOS_Ecom_CopyOfferImageInfoByIndex API. */
#define EOS_ECOM_COPYOFFERIMAGEINFOBYINDEX_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_CopyOfferImageInfoByIndex function.
 */
EOS_STRUCT(EOS_Ecom_CopyOfferImageInfoByIndexOptions, (
	/** API Version: Set this to EOS_ECOM_COPYOFFERIMAGEINFOBYINDEX_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose offer image is being copied. */
	EOS_EpicAccountId LocalUserId;
	/** The ID of the offer to get the images for. */
	EOS_Ecom_CatalogOfferId OfferId;
	/** The index of the image to get. */
	uint32_t ImageInfoIndex;
));

/** The most recent version of the EOS_Ecom_GetItemImageInfoCount API. */
#define EOS_ECOM_GETITEMIMAGEINFOCOUNT_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_GetItemImageInfoCount function.
 */
EOS_STRUCT(EOS_Ecom_GetItemImageInfoCountOptions, (
	/** API Version: Set this to EOS_ECOM_GETITEMIMAGEINFOCOUNT_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose item image is being accessed */
	EOS_EpicAccountId LocalUserId;
	/** The ID of the item to get the images for. */
	EOS_Ecom_CatalogItemId ItemId;
));

/** The most recent version of the EOS_Ecom_CopyItemImageInfoByIndex API. */
#define EOS_ECOM_COPYITEMIMAGEINFOBYINDEX_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_CopyItemImageInfoByIndex function.
 */
EOS_STRUCT(EOS_Ecom_CopyItemImageInfoByIndexOptions, (
	/** API Version: Set this to EOS_ECOM_COPYITEMIMAGEINFOBYINDEX_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose item image is being copied */
	EOS_EpicAccountId LocalUserId;
	/** The ID of the item to get the images for. */
	EOS_Ecom_CatalogItemId ItemId;
	/** The index of the image to get. */
	uint32_t ImageInfoIndex;
));

/** The most recent version of the EOS_Ecom_GetItemReleaseCount API. */
#define EOS_ECOM_GETITEMRELEASECOUNT_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_GetItemReleaseCount function.
 */
EOS_STRUCT(EOS_Ecom_GetItemReleaseCountOptions, (
	/** API Version: Set this to EOS_ECOM_GETITEMRELEASECOUNT_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose item release is being accessed */
	EOS_EpicAccountId LocalUserId;
	/** The ID of the item to get the releases for. */
	EOS_Ecom_CatalogItemId ItemId;
));

/** The most recent version of the EOS_Ecom_CopyItemReleaseByIndex API. */
#define EOS_ECOM_COPYITEMRELEASEBYINDEX_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_CopyItemReleaseByIndex function.
 */
EOS_STRUCT(EOS_Ecom_CopyItemReleaseByIndexOptions, (
	/** API Version: Set this to EOS_ECOM_COPYITEMRELEASEBYINDEX_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose item release is being copied */
	EOS_EpicAccountId LocalUserId;
	/** The ID of the item to get the releases for. */
	EOS_Ecom_CatalogItemId ItemId;
	/** The index of the release to get. */
	uint32_t ReleaseIndex;
));

/** The most recent version of the EOS_Ecom_GetTransactionCount Function. */
#define EOS_ECOM_GETTRANSACTIONCOUNT_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_GetTransactionCount function.
 */
EOS_STRUCT(EOS_Ecom_GetTransactionCountOptions, (
	/** API Version: Set this to EOS_ECOM_GETTRANSACTIONCOUNT_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user whose transaction count to get */
	EOS_EpicAccountId LocalUserId;
));

/** The most recent version of the EOS_Ecom_CopyTransactionByIndex Function. */
#define EOS_ECOM_COPYTRANSACTIONBYINDEX_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_CopyTransactionByIndex function.
 */
EOS_STRUCT(EOS_Ecom_CopyTransactionByIndexOptions, (
	/** API Version: Set this to EOS_ECOM_COPYTRANSACTIONBYINDEX_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user who is associated with the transaction */
	EOS_EpicAccountId LocalUserId;
	/** The index of the transaction to get */
	uint32_t TransactionIndex;
));

/** The most recent version of the EOS_Ecom_CopyTransactionById Function. */
#define EOS_ECOM_COPYTRANSACTIONBYID_API_LATEST 1
/**
 * Input parameters for the EOS_Ecom_CopyTransactionById function.
 */
EOS_STRUCT(EOS_Ecom_CopyTransactionByIdOptions, (
	/** API Version: Set this to EOS_ECOM_COPYTRANSACTIONBYID_API_LATEST. */
	int32_t ApiVersion;
	/** The Epic Online Services Account ID of the local user who is associated with the transaction */
	EOS_EpicAccountId LocalUserId;
	/** The ID of the transaction to get */
	const char* TransactionId;
));

/** The most recent version of the EOS_Ecom_Transaction_GetEntitlementsCount Function. */
#define EOS_ECOM_TRANSACTION_GETENTITLEMENTSCOUNT_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_Transaction_GetEntitlementsCount function.
 */
EOS_STRUCT(EOS_Ecom_Transaction_GetEntitlementsCountOptions, (
	/** API Version: Set this to EOS_ECOM_TRANSACTION_GETENTITLEMENTSCOUNT_API_LATEST. */
	int32_t ApiVersion;
));

/** The most recent version of the EOS_Ecom_Transaction_CopyEntitlementByIndex Function. */
#define EOS_ECOM_TRANSACTION_COPYENTITLEMENTBYINDEX_API_LATEST 1

/**
 * Input parameters for the EOS_Ecom_Transaction_CopyEntitlementByIndex function.
 */
EOS_STRUCT(EOS_Ecom_Transaction_CopyEntitlementByIndexOptions, (
	/** API Version: Set this to EOS_ECOM_TRANSACTION_COPYENTITLEMENTBYINDEX_API_LATEST. */
	int32_t ApiVersion;
	/** The index of the entitlement to get */
	uint32_t EntitlementIndex;
));

/**
 * Release the memory associated with an EOS_Ecom_HTransaction.  Is is expected to be called after
 * being received from a EOS_Ecom_CheckoutCallbackInfo.
 *
 * @param Transaction A handle to a transaction.
 *
 * @see EOS_Ecom_CheckoutCallbackInfo
 * @see EOS_Ecom_GetTransactionCount
 * @see EOS_Ecom_CopyTransactionByIndex
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_Transaction_Release(EOS_Ecom_HTransaction Transaction);

#pragma pack(pop)
