export function sendSdkMessage(type: string, data?: any) {
  Citizen.invokeNative("0x21550059", JSON.stringify({
    type,
    data,
  }));
}
