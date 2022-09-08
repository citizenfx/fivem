import { IAccount } from "./types";

export class SSOAuthCompleteEvent {
  public static success(): SSOAuthCompleteEvent {
    return new SSOAuthCompleteEvent(true);
  }

  public static error(error: string): SSOAuthCompleteEvent {
    return new SSOAuthCompleteEvent(false, error);
  }

  private _preventDefault = false;
  public get defaultPrevented(): boolean {
    return this._preventDefault;
  }

  private constructor(
    public readonly success: boolean,
    public readonly error?: string,
  ) {}

  preventDefault() {
    this._preventDefault = true;
  }
}

export interface AccountChangeEvent {
  account: IAccount | null,
}
