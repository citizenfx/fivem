import { sendSdkMessage } from "../sendSdkMessage";

let loadingScreenWasShutDown = false;
let screenFadedOut = false;

const tick = setTick(() => {
  if (!screenFadedOut && IsScreenFadedOut()) {
    screenFadedOut = true;
  }

  if (!loadingScreenWasShutDown && !GetIsLoadingScreenActive()){
    loadingScreenWasShutDown = true;
  }

  if (screenFadedOut && loadingScreenWasShutDown) {
    clearTick(tick);
  }
});

setTimeout(() => {
  if (!loadingScreenWasShutDown || !screenFadedOut) {
    sendSdkMessage('fxdk:loadingScreenWarning');
  }
}, 10000);


