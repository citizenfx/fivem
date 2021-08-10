import { SingleEventEmitter } from "utils/singleEventEmitter";

export const Events = new class Events {
  public readonly additionDeleted = new SingleEventEmitter<string>();
};
