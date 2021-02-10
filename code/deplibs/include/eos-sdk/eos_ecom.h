// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "eos_ecom_types.h"

/**
 * The Ecom Interface exposes all catalog, purchasing, and ownership entitlement features available with the Epic Games store
 * All Ecom Interface calls take a handle of type EOS_HEcom as the first parameter.
 * This handle can be retrieved from an EOS_HPlatform handle by using the EOS_Platform_GetEcomInterface function.
 *
 * @note At this time, this feature is only available for products that are part of the Epic Games store.
 *
 * @see EOS_Platform_GetEcomInterface
 */

/**
 * Query the ownership status for a given list of catalog item IDs defined with Epic Online Services.
 * This data will be cached for a limited time and retrieved again from the backend when necessary
 *
 * @param Options structure containing the account and catalog item IDs to retrieve
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOwnership(EOS_HEcom Handle, const EOS_Ecom_QueryOwnershipOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipCallback CompletionDelegate);

/**
 * Query the ownership status for a given list of catalog item IDs defined with Epic Online Services.
 * The data is return via the callback in the form of a signed JWT that should be verified by an external backend server using a public key for authenticity.
 *
 * @param Options structure containing the account and catalog item IDs to retrieve in token form
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOwnershipToken(EOS_HEcom Handle, const EOS_Ecom_QueryOwnershipTokenOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOwnershipTokenCallback CompletionDelegate);

/**
 * Query the entitlement information defined with Epic Online Services.
 * A set of entitlement names can be provided to filter the set of entitlements associated with the account.
 * This data will be cached for a limited time and retrieved again from the backend when necessary.
 * Use EOS_Ecom_CopyEntitlementByIndex, EOS_Ecom_CopyEntitlementByNameAndIndex, and EOS_Ecom_CopyEntitlementById to get the entitlement details.
 * Use EOS_Ecom_GetEntitlementsByNameCount to retrieve the number of entitlements with a specific entitlement name.
 *
 * @param Options structure containing the account and entitlement names to retrieve
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_QueryEntitlements(EOS_HEcom Handle, const EOS_Ecom_QueryEntitlementsOptions* Options, void* ClientData, const EOS_Ecom_OnQueryEntitlementsCallback CompletionDelegate);

/**
 * Query for a list of catalog offers defined with Epic Online Services.
 * This data will be cached for a limited time and retrieved again from the backend when necessary.
 *
 * @param Options structure containing filter criteria
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_QueryOffers(EOS_HEcom Handle, const EOS_Ecom_QueryOffersOptions* Options, void* ClientData, const EOS_Ecom_OnQueryOffersCallback CompletionDelegate);

/**
 * Initiates the purchase flow for a set of offers.  The callback is triggered after the purchase flow.
 * On success, the set of entitlements that were unlocked will be cached.
 * On success, a Transaction ID will be returned. The Transaction ID can be used to obtain an
 * EOS_Ecom_HTransaction handle. The handle can then be used to retrieve the entitlements rewarded by the purchase.
 *
 * @see EOS_Ecom_Transaction_Release
 *
 * @param Options structure containing filter criteria
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_Checkout(EOS_HEcom Handle, const EOS_Ecom_CheckoutOptions* Options, void* ClientData, const EOS_Ecom_OnCheckoutCallback CompletionDelegate);

/**
 * Requests that the provided entitlement be marked redeemed.  This will cause that entitlement
 * to no longer be returned from QueryEntitlements unless the include redeemed request flag is set true.
 *
 * @param Options structure containing entitlement to redeem
 * @param ClientData arbitrary data that is passed back to you in the CompletionDelegate
 * @param CompletionDelegate a callback that is fired when the async operation completes, either successfully or in error
 */
EOS_DECLARE_FUNC(void) EOS_Ecom_RedeemEntitlements(EOS_HEcom Handle, const EOS_Ecom_RedeemEntitlementsOptions* Options, void* ClientData, const EOS_Ecom_OnRedeemEntitlementsCallback CompletionDelegate);

/**
 * Fetch the number of entitlements that are cached for a given local user.
 *
 * @param Options structure containing the Epic Online Services Account ID being accessed
 *
 * @see EOS_Ecom_CopyEntitlementByIndex
 *
 * @return the number of entitlements found.
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetEntitlementsCount(EOS_HEcom Handle, const EOS_Ecom_GetEntitlementsCountOptions* Options);

/**
 * Fetch the number of entitlements with the given Entitlement Name that are cached for a given local user.
 *
 * @param Options structure containing the Epic Online Services Account ID and name being accessed
 *
 * @see EOS_Ecom_CopyEntitlementByNameAndIndex
 *
 * @return the number of entitlements found.
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetEntitlementsByNameCount(EOS_HEcom Handle, const EOS_Ecom_GetEntitlementsByNameCountOptions* Options);

/**
 * Fetches an entitlement from a given index.
 *
 * @param Options structure containing the Epic Online Services Account ID and index being accessed
 * @param OutEntitlement the entitlement for the given index, if it exists and is valid, use EOS_Ecom_Entitlement_Release when finished
 *
 * @see EOS_Ecom_Entitlement_Release
 *
 * @return EOS_Success if the information is available and passed out in OutEntitlement
 *         EOS_Ecom_EntitlementStale if the entitlement information is stale and passed out in OutEntitlement
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the entitlement is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyEntitlementByIndexOptions* Options, EOS_Ecom_Entitlement ** OutEntitlement);

/**
 * Fetches a single entitlement with a given Entitlement Name.  The Index is used to access individual
 * entitlements among those with the same Entitlement Name.  The Index can be a value from 0 to
 * one less than the result from EOS_Ecom_GetEntitlementsByNameCount.
 *
 * @param Options structure containing the Epic Online Services Account ID, entitlement name, and index being accessed
 * @param OutEntitlement the entitlement for the given name index pair, if it exists and is valid, use EOS_Ecom_Entitlement_Release when finished
 *
 * @see EOS_Ecom_Entitlement_Release
 *
 * @return EOS_Success if the information is available and passed out in OutEntitlement
 *         EOS_Ecom_EntitlementStale if the entitlement information is stale and passed out in OutEntitlement
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the entitlement is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementByNameAndIndex(EOS_HEcom Handle, const EOS_Ecom_CopyEntitlementByNameAndIndexOptions* Options, EOS_Ecom_Entitlement ** OutEntitlement);

/**
 * Fetches the entitlement with the given ID.
 *
 * @param Options structure containing the Epic Online Services Account ID and entitlement ID being accessed
 * @param OutEntitlement the entitlement for the given ID, if it exists and is valid, use EOS_Ecom_Entitlement_Release when finished
 *
 * @see EOS_Ecom_CopyEntitlementByNameAndIndex
 * @see EOS_Ecom_Entitlement_Release
 *
 * @return EOS_Success if the information is available and passed out in OutEntitlement
 *         EOS_Ecom_EntitlementStale if the entitlement information is stale and passed out in OutEntitlement
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the entitlement is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyEntitlementById(EOS_HEcom Handle, const EOS_Ecom_CopyEntitlementByIdOptions* Options, EOS_Ecom_Entitlement ** OutEntitlement);

/**
 * Fetch the number of offers that are cached for a given local user.
 *
 * @param Options structure containing the Epic Online Services Account ID being accessed
 *
 * @see EOS_Ecom_CopyOfferByIndex
 *
 * @return the number of offers found.
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferCount(EOS_HEcom Handle, const EOS_Ecom_GetOfferCountOptions* Options);

/**
 * Fetches an offer from a given index.  The pricing and text are localized to the provided account.
 *
 * @param Options structure containing the Epic Online Services Account ID and index being accessed
 * @param OutOffer the offer for the given index, if it exists and is valid, use EOS_Ecom_CatalogOffer_Release when finished
 *
 * @see EOS_Ecom_CatalogOffer_Release
 * @see EOS_Ecom_GetOfferItemCount
 *
 * @return EOS_Success if the information is available and passed out in OutOffer
 *         EOS_Ecom_CatalogOfferStale if the offer information is stale and passed out in OutOffer
 *         EOS_Ecom_CatalogOfferPriceInvalid if the offer information has an invalid price and passed out in OutOffer
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the offer is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyOfferByIndexOptions* Options, EOS_Ecom_CatalogOffer ** OutOffer);

/**
 * Fetches an offer with a given ID.  The pricing and text are localized to the provided account.
 *
 * @param Options structure containing the Epic Online Services Account ID and offer ID being accessed
 * @param OutOffer the offer for the given index, if it exists and is valid, use EOS_Ecom_CatalogOffer_Release when finished
 *
 * @see EOS_Ecom_CatalogOffer_Release
 * @see EOS_Ecom_GetOfferItemCount
 *
 * @return EOS_Success if the information is available and passed out in OutOffer
 *         EOS_Ecom_CatalogOfferStale if the offer information is stale and passed out in OutOffer
 *         EOS_Ecom_CatalogOfferPriceInvalid if the offer information has an invalid price and passed out in OutOffer
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the offer is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferById(EOS_HEcom Handle, const EOS_Ecom_CopyOfferByIdOptions* Options, EOS_Ecom_CatalogOffer ** OutOffer);

/**
 * Fetch the number of items that are associated with a given cached offer for a local user.
 *
 * @return the number of items found.
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferItemCount(EOS_HEcom Handle, const EOS_Ecom_GetOfferItemCountOptions* Options);

/**
 * Fetches an item from a given index.
 *
 * @param Options structure containing the Epic Online Services Account ID and index being accessed
 * @param OutItem the item for the given index, if it exists and is valid, use EOS_Ecom_CatalogItem_Release when finished
 *
 * @see EOS_Ecom_CatalogItem_Release
 * @see EOS_Ecom_GetItemImageInfoCount
 * @see EOS_Ecom_GetItemReleaseCount
 *
 * @return EOS_Success if the information is available and passed out in OutItem
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_Ecom_CatalogItemStale if the item information is stale
 *         EOS_NotFound if the item is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferItemByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyOfferItemByIndexOptions* Options, EOS_Ecom_CatalogItem ** OutItem);

/**
 * Fetches an item with a given ID.
 *
 * @param Options structure containing the item ID being accessed
 * @param OutItem the item for the given index, if it exists and is valid, use EOS_Ecom_CatalogItem_Release when finished
 *
 * @see EOS_Ecom_CatalogItem_Release
 * @see EOS_Ecom_GetItemImageInfoCount
 * @see EOS_Ecom_GetItemReleaseCount
 *
 * @return EOS_Success if the information is available and passed out in OutItem
 *         EOS_Ecom_CatalogItemStale if the item information is stale and passed out in OutItem
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the offer is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemById(EOS_HEcom Handle, const EOS_Ecom_CopyItemByIdOptions* Options, EOS_Ecom_CatalogItem ** OutItem);

/**
 * Fetch the number of images that are associated with a given cached offer for a local user.
 *
 * @return the number of images found.
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetOfferImageInfoCount(EOS_HEcom Handle, const EOS_Ecom_GetOfferImageInfoCountOptions* Options);

/**
 * Fetches an image from a given index.
 *
 * @param Options structure containing the offer ID and index being accessed
 * @param OutImageInfo the image for the given index, if it exists and is valid, use EOS_Ecom_KeyImageInfo_Release when finished
 *
 * @see EOS_Ecom_KeyImageInfo_Release
 *
 * @return EOS_Success if the information is available and passed out in OutImageInfo
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_Ecom_CatalogOfferStale if the associated offer information is stale
 *         EOS_NotFound if the image is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyOfferImageInfoByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyOfferImageInfoByIndexOptions* Options, EOS_Ecom_KeyImageInfo ** OutImageInfo);

/**
 * Fetch the number of images that are associated with a given cached item for a local user.
 *
 * @return the number of images found.
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetItemImageInfoCount(EOS_HEcom Handle, const EOS_Ecom_GetItemImageInfoCountOptions* Options);

/**
 * Fetches an image from a given index.
 *
 * @param Options structure containing the item ID and index being accessed
 * @param OutImageInfo the image for the given index, if it exists and is valid, use EOS_Ecom_KeyImageInfo_Release when finished
 *
 * @see EOS_Ecom_KeyImageInfo_Release
 *
 * @return EOS_Success if the information is available and passed out in OutImageInfo
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_Ecom_CatalogItemStale if the associated item information is stale
 *         EOS_NotFound if the image is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemImageInfoByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyItemImageInfoByIndexOptions* Options, EOS_Ecom_KeyImageInfo ** OutImageInfo);

/**
 * Fetch the number of releases that are associated with a given cached item for a local user.
 *
 * @return the number of releases found.
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetItemReleaseCount(EOS_HEcom Handle, const EOS_Ecom_GetItemReleaseCountOptions* Options);

/**
 * Fetches a release from a given index.
 *
 * @param Options structure containing the item ID and index being accessed
 * @param OutRelease the release for the given index, if it exists and is valid, use EOS_Ecom_CatalogRelease_Release when finished
 *
 * @see EOS_Ecom_CatalogRelease_Release
 *
 * @return EOS_Success if the information is available and passed out in OutRelease
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_Ecom_CatalogItemStale if the associated item information is stale
 *         EOS_NotFound if the release is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyItemReleaseByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyItemReleaseByIndexOptions* Options, EOS_Ecom_CatalogRelease ** OutRelease);

/**
 * Fetch the number of transactions that are cached for a given local user.
 *
 * @see EOS_Ecom_CheckoutCallbackInfo
 * @see EOS_Ecom_CopyTransactionByIndex
 *
 * @return the number of transactions found.
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_GetTransactionCount(EOS_HEcom Handle, const EOS_Ecom_GetTransactionCountOptions* Options);

/**
 * Fetches the transaction handle at the given index.
 *
 * @param Options structure containing the Epic Online Services Account ID and index being accessed
 *
 * @see EOS_Ecom_CheckoutCallbackInfo
 * @see EOS_Ecom_Transaction_Release
 *
 * @return EOS_Success if the information is available and passed out in OutTransaction
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the transaction is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyTransactionByIndex(EOS_HEcom Handle, const EOS_Ecom_CopyTransactionByIndexOptions* Options, EOS_Ecom_HTransaction* OutTransaction);

/**
 * Fetches the transaction handle at the given index.
 *
 * @param Options structure containing the Epic Online Services Account ID and transaction ID being accessed
 *
 * @see EOS_Ecom_CheckoutCallbackInfo
 * @see EOS_Ecom_Transaction_Release
 *
 * @return EOS_Success if the information is available and passed out in OutTransaction
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the transaction is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_CopyTransactionById(EOS_HEcom Handle, const EOS_Ecom_CopyTransactionByIdOptions* Options, EOS_Ecom_HTransaction* OutTransaction);

/**
 * The Ecom Transaction Interface exposes getters for accessing information about a completed transaction.
 * All Ecom Transaction Interface calls take a handle of type EOS_Ecom_HTransaction as the first parameter.
 * An EOS_Ecom_HTransaction handle is originally returned as part of the EOS_Ecom_CheckoutCallbackInfo struct.
 * An EOS_Ecom_HTransaction handle can also be retrieved from an EOS_HEcom handle using EOS_Ecom_CopyTransactionByIndex.
 * It is expected that after a transaction that EOS_Ecom_Transaction_Release is called.
 * When EOS_Platform_Release is called any remaining transactions will also be released.
 *
 * @see EOS_Ecom_CheckoutCallbackInfo
 * @see EOS_Ecom_GetTransactionCount
 * @see EOS_Ecom_CopyTransactionByIndex
 */

EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_Transaction_GetTransactionId(EOS_Ecom_HTransaction Handle, char* OutBuffer, int32_t* InOutBufferLength);

/**
 * Fetch the number of entitlements that are part of this transaction.
 *
 * @param Options structure containing the Epic Online Services Account ID being accessed
 *
 * @see EOS_Ecom_Transaction_CopyEntitlementByIndex
 *
 * @return the number of entitlements found.
 */
EOS_DECLARE_FUNC(uint32_t) EOS_Ecom_Transaction_GetEntitlementsCount(EOS_Ecom_HTransaction Handle, const EOS_Ecom_Transaction_GetEntitlementsCountOptions* Options);

/**
 * Fetches an entitlement from a given index.
 *
 * @param Options structure containing the index being accessed
 * @param OutEntitlement the entitlement for the given index, if it exists and is valid, use EOS_Ecom_Entitlement_Release when finished
 *
 * @see EOS_Ecom_Entitlement_Release
 *
 * @return EOS_Success if the information is available and passed out in OutEntitlement
 *         EOS_Ecom_EntitlementStale if the entitlement information is stale and passed out in OutEntitlement
 *         EOS_InvalidParameters if you pass a null pointer for the out parameter
 *         EOS_NotFound if the entitlement is not found
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_Ecom_Transaction_CopyEntitlementByIndex(EOS_Ecom_HTransaction Handle, const EOS_Ecom_Transaction_CopyEntitlementByIndexOptions* Options, EOS_Ecom_Entitlement ** OutEntitlement);
