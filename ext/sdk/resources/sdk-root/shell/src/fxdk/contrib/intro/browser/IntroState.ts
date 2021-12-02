import { makeAutoObservable } from "mobx";
import { LocalStorageValue } from "store/generic/LocalStorageValue";
import { OpenFlag } from "store/generic/OpenFlag";

export const IntroState = new class IntroState {
  private openFlag = new OpenFlag(false);
  private firstLaunch = new LocalStorageValue({
    key: 'firstLaunch',
    defaultValue: true,
  });

  get isOpen(): boolean {
    return this.openFlag.isOpen;
  }

  get isFirstLaunch(): boolean {
    return this.firstLaunch.get();
  }

  constructor() {
    makeAutoObservable(this);
  }

  readonly open = () => this.openFlag.open();

  readonly close = () => {
    this.firstLaunch.set(false);
    this.openFlag.close();
  };
}();
