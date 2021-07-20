import { makeAutoObservable } from "mobx";

let animationTimer = null;

export const FlashingMessageState = new class FlashingMessageState {
  public message: string = '';
  public active = false;

  constructor() {
    makeAutoObservable(this);
  }

  setMessage(message: string) {
    if (this.message !== message) {
      this.active = true;

      if (animationTimer !== null) {
        clearTimeout(animationTimer);
      }

      animationTimer = setTimeout(this.setInactive, 2000);
    }

    this.message = message;
  }

  private readonly setInactive = () => {
    animationTimer = null;
    this.active = false;
  }
}
