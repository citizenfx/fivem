import { joaat } from "../shared";

const SEND_SDK_MESSAGE = joaat('SEND_SDK_MESSAGE');
const SEND_SDK_MESSAGE_TO_BACKEND = joaat('SEND_SDK_MESSAGE_TO_BACKEND');

export function sendSdkMessage(type: string, data?: any) {
  Citizen.invokeNative(SEND_SDK_MESSAGE, JSON.stringify({
    type,
    data,
  }));
}

export function sendSdkBackendMessage(type: string, data?: any) {
  Citizen.invokeNative(SEND_SDK_MESSAGE_TO_BACKEND, JSON.stringify({
    type,
    data,
  }));
}
