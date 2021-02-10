// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "eos_ui_types.h"

/**
 * The UI Interface is used to access the Social Overlay UI.  Each UI component will have a function for
 * opening it.  All UI Interface calls take a handle of type EOS_HUI as the first parameter.
 * This handle can be retrieved from an EOS_HPlatform handle by using the EOS_Platform_GetUIInterface function.
 *
 * @see EOS_Platform_GetUIInterface
 */

/**
 * Opens the Social Overlay with a request to show the friends list.
 *
 * @param Options Structure containing the Epic Online Services Account ID of the friends list to show.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param CompletionDelegate A callback that is fired when the request to show the friends list has been sent to the Social Overlay, or on an error.
 *
 * @return EOS_Success If the Social Overlay has been notified about the request.
 *         EOS_InvalidParameters If any of the options are incorrect.
 *         EOS_NotConfigured If the Social Overlay is not properly configured.
 *         EOS_NoChange If the Social Overlay is already visible.
 */
EOS_DECLARE_FUNC(void) EOS_UI_ShowFriends(EOS_HUI Handle, const EOS_UI_ShowFriendsOptions* Options, void* ClientData, const EOS_UI_OnShowFriendsCallback CompletionDelegate);

/**
 * Hides the active Social Overlay.
 *
 * @param Options Structure containing the Epic Online Services Account ID of the browser to close.
 * @param ClientData Arbitrary data that is passed back to you in the CompletionDelegate.
 * @param CompletionDelegate A callback that is fired when the request to hide the friends list has been processed, or on an error.
 *
 * @return EOS_Success If the Social Overlay has been notified about the request.
 *         EOS_InvalidParameters If any of the options are incorrect.
 *         EOS_NotConfigured If the Social Overlay is not properly configured.
 *         EOS_NoChange If the Social Overlay is already hidden.
 */
EOS_DECLARE_FUNC(void) EOS_UI_HideFriends(EOS_HUI Handle, const EOS_UI_HideFriendsOptions* Options, void* ClientData, const EOS_UI_OnHideFriendsCallback CompletionDelegate);

/**
 * Gets the friends overlay visibility.
 *
 * @param Options Structure containing the Epic Online Services Account ID of the friends Social Overlay owner.
 *
 * @return EOS_TRUE If the overlay is visible.
 */
EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_GetFriendsVisible(EOS_HUI Handle, const EOS_UI_GetFriendsVisibleOptions* Options);

/**
 * Register to receive notifications when the overlay display settings are updated.
 * Newly registered handlers will always be called the next tick with the current state.
 * @note must call RemoveNotifyDisplaySettingsUpdated to remove the notification.
 *
 * @param Options Structure containing information about the request.
 * @param ClientData Arbitrary data that is passed back to you in the NotificationFn.
 * @param NotificationFn A callback that is fired when the overlay display settings are updated.
 *
 * @return handle representing the registered callback
 */
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_UI_AddNotifyDisplaySettingsUpdated(EOS_HUI Handle, const EOS_UI_AddNotifyDisplaySettingsUpdatedOptions* Options, void* ClientData, const EOS_UI_OnDisplaySettingsUpdatedCallback NotificationFn);

/**
 * Unregister from receiving notifications when the overlay display settings are updated.
 *
 * @param Id Handle representing the registered callback
 */
EOS_DECLARE_FUNC(void) EOS_UI_RemoveNotifyDisplaySettingsUpdated(EOS_HUI Handle, EOS_NotificationId Id);

/**
 * Updates the current Toggle Friends Key.  This key can be used by the user to toggle the friends
 * overlay when available. The default value represents `Shift + F3` as `((int32_t)EOS_UIK_Shift | (int32_t)EOS_UIK_F3)`.
 * The provided key should satisfy EOS_UI_IsValidKeyCombination. The value EOS_UIK_None is specially handled
 * by resetting the key binding to the system default.
 *
 * @param Options Structure containing the key combination to use.
 *
 * @return EOS_Success If the overlay has been notified about the request.
 *         EOS_InvalidParameters If any of the options are incorrect.
 *         EOS_NotConfigured If the overlay is not properly configured.
 *         EOS_NoChange If the key combination did not change.
 *
 * @see EOS_UI_IsValidKeyCombination
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_SetToggleFriendsKey(EOS_HUI Handle, const EOS_UI_SetToggleFriendsKeyOptions* Options);

/**
 * Returns the current Toggle Friends Key.  This key can be used by the user to toggle the friends
 * overlay when available. The default value represents `Shift + F3` as `((int32_t)EOS_UIK_Shift | (int32_t)EOS_UIK_F3)`.
 *
 * @param Options Structure containing any options that are needed to retrieve the key.
 * @return A valid key combination which represent a single key with zero or more modifier keys.
 *         EOS_UIK_None will be returned if any error occurs.
 */
EOS_DECLARE_FUNC(EOS_UI_EKeyCombination) EOS_UI_GetToggleFriendsKey(EOS_HUI Handle, const EOS_UI_GetToggleFriendsKeyOptions* Options);

/**
 * Determine if a key combination is valid. A key combinations must have a single key and at least one modifier.
 * The single key must be one of the following: F1 through F12, Space, Backspace, Escape, or Tab.
 * The modifier key must be one or more of the following: Shift, Control, or Alt.
 *
 * @param KeyCombination The key to test.
 * @return  EOS_TRUE if the provided key combination is valid.
 */
EOS_DECLARE_FUNC(EOS_Bool) EOS_UI_IsValidKeyCombination(EOS_HUI Handle, EOS_UI_EKeyCombination KeyCombination);

/**
 * Define any preferences for any display settings.
 *
 * @param Options Structure containing any options that are needed to set
 * @return EOS_Success If the overlay has been notified about the request.
 *         EOS_InvalidParameters If any of the options are incorrect.
 *         EOS_NotConfigured If the overlay is not properly configured.
 *         EOS_NoChange If the preferences did not change.
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_SetDisplayPreference(EOS_HUI Handle, const EOS_UI_SetDisplayPreferenceOptions* Options);

/**
 * Returns the current notification location display preference.
 * @return The current notification location display preference.
 */
EOS_DECLARE_FUNC(EOS_UI_ENotificationLocation) EOS_UI_GetNotificationLocationPreference(EOS_HUI Handle);

/**
 * Lets the SDK know that the given UI event ID has been acknowledged and should be released.
 *
 * @return An EOS_EResult is returned to indicate success or an error.
 *
 * EOS_Success is returned if the UI event ID has been acknowledged.
 * EOS_NotFound is returned if the UI event ID does not exist.
 *
 * @see EOS_Presence_JoinGameAcceptedCallbackInfo
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_UI_AcknowledgeEventId(EOS_HUI Handle, const EOS_UI_AcknowledgeEventIdOptions* Options);
