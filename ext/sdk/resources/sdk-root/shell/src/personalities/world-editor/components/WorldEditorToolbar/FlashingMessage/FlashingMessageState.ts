import { makeAutoObservable } from "mobx";

let activityTimer: NullableTimer = null;
let animationTimer: NullableTimer = null;

export const FlashingMessageState = new class FlashingMessageState {
  public message: string = '';
  public active = false;

  constructor() {
    makeAutoObservable(this);
  }

  setMessage(message: string) {
    if (this.message !== message) {
      this.active = true;

      if (activityTimer !== null) {
        clearTimeout(activityTimer);
      }
      if (animationTimer !== null) {
        clearTimeout(animationTimer);
      }

      activityTimer = setTimeout(this.setInactive, 2000);
    }

    this.message = message;
  }

  private readonly setInactive = () => {
    activityTimer = null;
    this.active = false;

    animationTimer = setTimeout(() => {
      animationTimer = null;
      this.message = '';
    }, 200);
  }
}
