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
	sendSdkData({
		type: 'pos',
		payload: GetEntityCoords(PlayerPedId()),
	});
});

RegisterCommand('sdk:dataRequest', (_, [cmd]) => {
	switch (cmd) {
		case 'pos': {
			return sendSdkData({
				type: 'pos',
				payload: GetEntityCoords(GetPlayerPed()),
			});
		}
	}
});

RegisterCommand('sdk:ackConnected', () => {
	sendSdkData({
		type: 'connected',
	});
});
