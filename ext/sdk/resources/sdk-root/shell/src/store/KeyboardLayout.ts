import { makeAutoObservable } from "mobx";


export const KeyboardLayout = new class KeyboardLayout {
  private map: KeyboardLayoutMap | null = null;

  constructor() {
    makeAutoObservable(this);

    this.load();
  }

  get(code: string): string {
    return this.map?.get(code) || code;
  }

  private async load() {
    this.map = await navigator.keyboard.getLayoutMap();
  }
}();
