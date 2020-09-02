export const sendCommand = (cmd: string) => {
  window.invokeNative('sendCommand', cmd);
};
