// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "eos_p2p_types.h"

/**
 * P2P functions to help manage sending and receiving of messages to peers.
 *
 * These functions will attempt to punch through NATs, but will fallback to using Epic relay servers if a direct connection cannot be established.
 */

/**
 * Send a packet to a peer at the specified address. If there is already an open connection to this peer, it will be
 * sent immediately. If there is no open connection, an attempt to connect to the peer will be made. An EOS_Success
 * result only means the data was accepted to be sent, not that it has been successfully delivered to the peer.
 *
 * @param Options Information about the data being sent, by who, to who
 * @return EOS_EResult::EOS_Success           - If packet was queued to be sent successfully
 *         EOS_EResult::EOS_InvalidParameters - If input was invalid
 *         EOS_EResult::EOS_LimitExceeded     - If amount of data being sent is too large
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_SendPacket(EOS_HP2P Handle, const EOS_P2P_SendPacketOptions* Options);

/**
 * Gets the size of the packet that will be returned by ReceivePacket for a particular user, if there is any available
 * packets to be retrieved.
 *
 * @param Options Information about who is requesting the size of their next packet
 * @param OutPacketSizeBytes The amount of bytes required to store the data of the next packet for the requested user
 * @return EOS_EResult::EOS_Success - If OutPacketSizeBytes was successfully set and there is data to be received
 *         EOS_EResult::EOS_InvalidParameters - If input was invalid
 *         EOS_EResult::EOS_NotFound  - If there are no packets available for the requesting user
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_GetNextReceivedPacketSize(EOS_HP2P Handle, const EOS_P2P_GetNextReceivedPacketSizeOptions* Options, uint32_t* OutPacketSizeBytes);

/**
 * Receive the next packet for the local user, and information associated with this packet, if it exists.
 *
 * @param Options Information about who is requesting the size of their next packet, and how much data can be stored safely
 * @param OutPeerId The Remote User who sent data. Only set if there was a packet to receive.
 * @param OutSocketId The Socket ID of the data that was sent. Only set if there was a packet to receive.
 * @param OutChannel The channel the data was sent on. Only set if there was a packet to receive.
 * @param OutData Buffer to store the data being received. Must be at least EOS_P2P_GetNextReceivedPacketSize in length or data will be truncated
 * @param OutBytesWritten The amount of bytes written to OutData. Only set if there was a packet to receive.
 * @return EOS_EResult::EOS_Success - If the packet was received successfully
 *         EOS_EResult::EOS_InvalidParameters - If input was invalid
 *         EOS_EResult::EOS_NotFound - If there are no packets available for the requesting user
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_ReceivePacket(EOS_HP2P Handle, const EOS_P2P_ReceivePacketOptions* Options, EOS_ProductUserId* OutPeerId, EOS_P2P_SocketId* OutSocketId, uint8_t* OutChannel, void* OutData, uint32_t* OutBytesWritten);

/**
 * Listen for incoming connection requests on a particular Socket ID, or optionally all Socket IDs. The bound function
 * will only be called if the connection has not already been accepted.
 *
 * @param Options Information about who would like notifications, and (optionally) only for a specific socket
 * @param ClientData This value is returned to the caller when ConnectionRequestHandler is invoked
 * @param ConnectionRequestHandler The callback to be fired when we receive a connection request
 * @return A valid notification ID if successfully bound, or EOS_INVALID_NOTIFICATIONID otherwise
 */
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_P2P_AddNotifyPeerConnectionRequest(EOS_HP2P Handle, const EOS_P2P_AddNotifyPeerConnectionRequestOptions* Options, void* ClientData, EOS_P2P_OnIncomingConnectionRequestCallback ConnectionRequestHandler);

/**
 * Stop listening for connection requests on a previously bound handler
 *
 * @param NotificationId The previously bound notification ID
 */
EOS_DECLARE_FUNC(void) EOS_P2P_RemoveNotifyPeerConnectionRequest(EOS_HP2P Handle, EOS_NotificationId NotificationId);

/**
 * Listen for when a previously opened connection is closed
 *
 * @param Options Information about who would like notifications about closed connections, and for which socket
 * @param ClientData This value is returned to the caller when ConnectionClosedHandler is invoked
 * @param ConnectionClosedHandler The callback to be fired when we an open connection has been closed
 * @return A valid notification ID if successfully bound, or EOS_INVALID_NOTIFICATIONID otherwise
 */
EOS_DECLARE_FUNC(EOS_NotificationId) EOS_P2P_AddNotifyPeerConnectionClosed(EOS_HP2P Handle, const EOS_P2P_AddNotifyPeerConnectionClosedOptions* Options, void* ClientData, EOS_P2P_OnRemoteConnectionClosedCallback ConnectionClosedHandler);

/**
 * Stop notifications for connections being closed on a previously bound handler
 *
 * @param NotificationId The previously bound notification ID
 */
EOS_DECLARE_FUNC(void) EOS_P2P_RemoveNotifyPeerConnectionClosed(EOS_HP2P Handle, EOS_NotificationId NotificationId);

/**
 * Accept connections from a specific peer. If this peer has not attempted to connect yet, when they do, they will automatically be accepted.
 *
 * @param Options Information about who would like to accept a connection, and which connection
 * @return EOS_EResult::EOS_Success - if the provided data is valid
 *         EOS_EResult::EOS_InvalidParameters - if the provided data is invalid
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_AcceptConnection(EOS_HP2P Handle, const EOS_P2P_AcceptConnectionOptions* Options);

/**
 * Stop accepting new connections from a specific peer and close any open connections.
 *
 * @param Options Information about who would like to close a connection, and which connection.
 * @return EOS_EResult::EOS_Success - if the provided data is valid
 *         EOS_EResult::EOS_InvalidParameters - if the provided data is invalid
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_CloseConnection(EOS_HP2P Handle, const EOS_P2P_CloseConnectionOptions* Options);

/**
 * Close any open Connections for a specific Peer Connection ID.
 *
 * @param Options Information about who would like to close connections, and by what socket ID
 * @return EOS_EResult::EOS_Success - if the provided data is valid
 *         EOS_EResult::EOS_InvalidParameters - if the provided data is invalid
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_CloseConnections(EOS_HP2P Handle, const EOS_P2P_CloseConnectionsOptions* Options);

/**
 * Query the current NAT-type of our connection.
 *
 * @param Options Information about what version of the EOS_P2P_QueryNATType API is supported
 * @param NATTypeQueriedHandler The callback to be fired when we finish querying our NAT type
 */
EOS_DECLARE_FUNC(void) EOS_P2P_QueryNATType(EOS_HP2P Handle, const EOS_P2P_QueryNATTypeOptions* Options, void* ClientData, const EOS_P2P_OnQueryNATTypeCompleteCallback NATTypeQueriedHandler);

/**
 * Get our last-queried NAT-type, if it has been successfully queried.
 *
 * @param Options Information about what version of the EOS_P2P_GetNATType API is supported
 * @param OutNATType The queried NAT Type, or unknown if unknown
 * @return EOS_EResult::EOS_Success - if we have cached data
 *         EOS_EResult::EOS_NotFound - If we do not have queried data cached
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_GetNATType(EOS_HP2P Handle, const EOS_P2P_GetNATTypeOptions* Options, EOS_ENATType* OutNATType);

/**
 * Set how relay servers are to be used. This setting does not immediately apply to existing connections, but may apply to existing
 * connections if the connection requires renegotiation.
 *
 * @param Options Information about relay server config options
 * @return EOS_EResult::EOS_Success - if the options were set successfully
 *         EOS_EResult::EOS_InvalidParameters - if the options are invalid in some way
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_SetRelayControl(EOS_HP2P Handle, const EOS_P2P_SetRelayControlOptions* Options);

/**
 * Get the current relay control setting.
 *
 * @param Options Information about what version of the EOS_P2P_GetRelayControl API is supported
 * @param OutRelayControl The relay control setting currently configured
 * @return EOS_EResult::EOS_Success - if the input was valid
 *         EOS_EResult::EOS_InvalidParameters - if the input was invalid in some way
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_GetRelayControl(EOS_HP2P Handle, const EOS_P2P_GetRelayControlOptions* Options, EOS_ERelayControl* OutRelayControl);

/**
 * Set configuration options related to network ports.
 *
 * @param Options Information about network ports config options
 * @return EOS_EResult::EOS_Success - if the options were set successfully
 *         EOS_EResult::EOS_InvalidParameters - if the options are invalid in some way
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_SetPortRange(EOS_HP2P Handle, const EOS_P2P_SetPortRangeOptions* Options);

/**
 * Get the current chosen port and the amount of other ports to try above the chosen port if the chosen port is unavailable.
 *
 * @param Options Information about what version of the EOS_P2P_GetPortRange API is supported
 * @param OutPort The port that will be tried first
 * @param OutNumAdditionalPortsToTry The amount of ports to try above the value in OutPort, if OutPort is unavailable
 * @return EOS_EResult::EOS_Success - if the input options were valid
 *         EOS_EResult::EOS_InvalidParameters - if the input was invalid in some way
 */
EOS_DECLARE_FUNC(EOS_EResult) EOS_P2P_GetPortRange(EOS_HP2P Handle, const EOS_P2P_GetPortRangeOptions* Options, uint16_t* OutPort, uint16_t* OutNumAdditionalPortsToTry);
