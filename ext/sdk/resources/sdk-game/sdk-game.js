setTick(() => {
	//DrawRect(0, 0, .5, .5, 255, 255, 255, 255);
	Citizen.invokeNative("0x21550059", JSON.stringify({
		type: 'pos',
		payload: GetEntityCoords(GetPlayerPed()),
	}));
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
			return Citizen.invokeNative("0x21550059", JSON.stringify({
				type: 'pos',
				payload: GetEntityCoords(GetPlayerPed()),
			}));
		}
	}
});