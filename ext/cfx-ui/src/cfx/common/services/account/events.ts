import { IAccount } from './types';

export class ExternalAuthCompleteEvent {
  public static success(): ExternalAuthCompleteEvent {
    return new ExternalAuthCompleteEvent(true);
  }

  public static error(error: string): ExternalAuthCompleteEvent {
    return new ExternalAuthCompleteEvent(false, error);
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
