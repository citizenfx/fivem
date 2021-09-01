import { makeAutoObservable } from "mobx";

export class OpenFlag {
  get isOpen(): boolean {
    return this.state;
  }

  constructor(private state = false) {
    makeAutoObservable(this);
  }

  readonly open = () => {
    this.state = true;
  };

  readonly close = () => {
    this.state = false;
  };

  readonly toggle = () => {
    this.state = !this.state;
  };
}
