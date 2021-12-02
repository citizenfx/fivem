import { SingleEventEmitter } from "utils/singleEventEmitter";

export namespace ProjectStateEvents {
  export const BeforeClose = new SingleEventEmitter<void>();
}
