export enum LoginStatus {
  Error,
  Success,
  TOTPRequest,
}
export interface ILoginCredentials {
  email: string;
  password: string;
  totp?: string;
}
export type ILoginResponse =
  | { status: LoginStatus.Success }
  | { status: LoginStatus.TOTPRequest }
  | { status: LoginStatus.Error; error: string };

export enum RegisterStatus {
  Error,
  Success,
}
export interface IRegisterCredentials {
  email: string;
  password: string;
  username: string;
}
export type IRegisterResponse = { status: RegisterStatus.Error; error: string } | { status: RegisterStatus.Success };

export interface IAccount {
  readonly id: number;
  readonly username: string;
  readonly avatarTemplate: string;

  readonly isStaff: boolean;
  readonly isPremium: boolean;

  getAvatarUrl(size?: number): string;
  getAvatarUrlForCss(size?: number): string;
}
