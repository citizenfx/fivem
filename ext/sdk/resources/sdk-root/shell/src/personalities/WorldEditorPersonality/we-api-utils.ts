import { WEApiMethod, WEApiMethodRequest } from "backend/world-editor/world-editor-game-api";
import { onWindowEvent } from "utils/windowMessages";

export function onWEApi<Method extends WEApiMethod>(method: Method, cb: (request: WEApiMethodRequest<Method>) => void) {
  onWindowEvent(method, cb);
}

export function invokeWEApi<Method extends WEApiMethod>(method: Method, request: WEApiMethodRequest<Method>) {
  sendGameClientEvent(method, JSON.stringify(request));
}
