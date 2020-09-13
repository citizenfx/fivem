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
	//DrawRect(0, 0, .5, .5, 255, 255, 255, 255);
	sendSdkData({
		type: 'pos',
		payload: GetEntityCoords(GetPlayerPed()),
	});
});

RegisterCommand('test', (...args) => {
	console.log('test', ...args);
});

RegisterCommand('weapon', (_, [weapon]) => {
	console.log('giving weapon', weapon);

	GiveWeaponToPed(PlayerPedId(), weapon, 1000, false, true);
});

RegisterCommand('sdk:data_request', (_, [cmd]) => {
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