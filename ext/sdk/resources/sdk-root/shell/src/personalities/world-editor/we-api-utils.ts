import { WEApiMethod, WEApiMethodRequest } from "backend/world-editor/world-editor-game-api";
import { ShellEvents } from "shell-api/events";

export function onWEApi<Method extends WEApiMethod>(method: Method, cb: (request: WEApiMethodRequest<Method>) => void) {
  ShellEvents.on(method, cb);
}

export function invokeWEApi<Method extends WEApiMethod>(method: Method, request: WEApiMethodRequest<Method>) {
  sendGameClientEvent(method, JSON.stringify(request) || '');
}
