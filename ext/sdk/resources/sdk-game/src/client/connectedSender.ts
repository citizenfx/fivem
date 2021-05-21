import { sendSdkBackendMessage, sendSdkMessage } from "./sendSdkMessage";

on('onClientResourceStart', (resourceName: string) => {
	if (resourceName === 'sdk-game') {
		sendSdkMessage('connected');
		sendSdkBackendMessage('connected');
	}
});

RegisterCommand('sdk:ackConnected', () => sendSdkMessage('connected'), false);
