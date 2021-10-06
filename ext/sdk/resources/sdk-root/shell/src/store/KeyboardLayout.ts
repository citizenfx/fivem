import { Deferred } from "backend/deferred";
import { makeAutoObservable } from "mobx";


export const KeyboardLayout = new class KeyboardLayout {
  private map: KeyboardLayoutMap | null = null;

  private loadDeferred = new Deferred<void>();

  constructor() {
    makeAutoObservable(this);

    this.load();
  }

  get(code: string): string {
    return this.map?.get(code) || code;
  }

  async getLayout(): Promise<KeyboardLayoutMap> {
    await this.loadDeferred.promise;

    return this.map!;
  }

  private async load() {
    this.map = await navigator.keyboard.getLayoutMap();

    this.loadDeferred.resolve();
  }
}();
