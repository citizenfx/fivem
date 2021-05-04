export const sendCommandToGameClient = (cmd: string) => {
  window.invokeNative('sendCommand', cmd);
};
