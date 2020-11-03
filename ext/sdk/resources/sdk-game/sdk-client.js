console.log('sdk-game:client started');

const sendSdkData = (data) => {
	Citizen.invokeNative("0x21550059", JSON.stringify(data));
};

// Letting shell know that we have connected
on('onClientResourceStart', (resourceName) => {
	if (resourceName === 'sdk-game') {
		sendSdkData({
			type: 'connected',
		});
	}
});

setTick(() => {
  const playerPedId = PlayerPedId();

	sendSdkData({
		type: 'fxdk:data',
		data: [
      {
        key: 'player_ped_pos',
        value: GetEntityCoords(playerPedId),
      },
      {
        key: 'player_ped_rot',
        value: GetEntityRotation(playerPedId, 2),
      },
    ],
	});
});

RegisterCommand('sdk:ackConnected', () => {
	sendSdkData({
		type: 'connected',
	});
});
