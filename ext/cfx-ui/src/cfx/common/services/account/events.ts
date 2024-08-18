import { IAccount } from './types';

export class SSOAuthCompleteEvent {
  public static success(): SSOAuthCompleteEvent {
    return new SSOAuthCompleteEvent(true);
  }

  public static error(error: string): SSOAuthCompleteEvent {
    return new SSOAuthCompleteEvent(false, error);
  }

  private constructor(
    public readonly success: boolean,
    public readonly error?: string,
    // eslint-disable-next-line no-empty-function
  ) {}
}

export interface AccountChangeEvent {
  account: IAccount | null;
}
