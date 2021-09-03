import { runInAction } from "mobx";
import { onApiMessage } from "utils/api";

export function onApiMessageAction(type: string, cb: (data?: any) => void) {
  onApiMessage(type, (data) => runInAction(() => cb(data)));
}
