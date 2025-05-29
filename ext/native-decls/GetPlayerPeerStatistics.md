---
ns: CFX
apiset: server
---
## GET_PLAYER_PEER_STATISTICS

```c
int GET_PLAYER_PEER_STATISTICS(char* playerSrc, int peerStatistic);
```

```cpp
const int ENET_PACKET_LOSS_SCALE = 65536;

enum PeerStatistics
{
	// PacketLoss will only update once every 10 seconds, use PacketLossEpoch if you want the time
	// since the last time the packet loss was updated.

	// the amount of packet loss the player has, needs to be scaled with PACKET_LOSS_SCALE
	PacketLoss = 0,
	// The variance in the packet loss
	PacketLossVariance = 1,
	// The time since the last packet update in ms, relative to the peers connection time
	PacketLossEpoch = 2,
	// The mean amount of time it takes for a packet to get to the client (ping)
	RoundTripTime = 3,
	// The variance in the round trip time
	RoundTripTimeVariance = 4,
	// Despite their name, these are only updated once every 5 seconds, you can get the last time this was updated with PacketThrottleEpoch
	// The last recorded round trip time of a packet
	LastRoundTripTime = 5,
	// The last round trip time variance
	LastRoundTripTimeVariance = 6,
	// The time since the last packet throttle update, relative to the peers connection time
	PacketThrottleEpoch = 7,
};
```

These statistics only update once every 10 seconds.

## Parameters
* **playerSrc**: The player to get the stats of
* **peerStatistic**: The statistic to get, this will error if its out of range

## Return value
See `ENetStatisticType` for what this will return.

## Examples
```js

setInterval(() => {
	const ENET_PACKET_LOSS_SCALE = 65536;

	const PLAYER_SERVER_ID = 1;

	const packetLoss = GetPlayerPeerStatistics(PLAYER_SERVER_ID, 0 /* PacketLoss */);
	const packetLossVariance = GetPlayerPeerStatistics(PLAYER_SERVER_ID, 1 /* PacketLossVariance */);
	const packetLossEpoch = GetPlayerPeerStatistics(PLAYER_SERVER_ID, 2 /* PacketLossEpoch */)
	const rtt = GetPlayerPeerStatistics(PLAYER_SERVER_ID, 3 /* RoundTripTime */);
	const rttVariance = GetPlayerPeerStatistics(PLAYER_SERVER_ID, 4 /* RoundTripTimeVariance */);
	const lastRtt = GetPlayerPeerStatistics(PLAYER_SERVER_ID, 5 /* LastRoundTripTime */);
	const lastRttVariance = GetPlayerPeerStatistics(PLAYER_SERVER_ID, 6 /* LastRoundTripTimeVariance */);
	const packetThrottleEpoch = GetPlayerPeerStatistics(PLAYER_SERVER_ID, 7 /* PacketThrottleEpoch */);

	console.log(`packetLoss: ${packetLoss}`);
	console.log(`packetLossVariance: ${packetLossVariance}`);
	console.log(`packetLossEpch: ${packetLossEpoch}`);

	console.log(`packetLossScaled: ${packetLoss / ENET_PACKET_LOSS_SCALE}`);
	console.log(`packetLossVarianceScaled: ${packetLossVariance / ENET_PACKET_LOSS_SCALE}`);

	console.log(`rtt: ${rtt}`);
	console.log(`rttVariance: ${rttVariance}`);

	console.log(`lastRtt: ${lastRtt}`);
	console.log(`lastRttVariance: ${lastRttVariance}`);
	console.log(`packetThrottleEpoch: ${packetThrottleEpoch}`);
}, 10000)
```
