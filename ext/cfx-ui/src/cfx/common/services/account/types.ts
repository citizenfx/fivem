export interface IAccount {
  readonly id: number;
  readonly username: string;
  readonly avatarTemplate: string;

  readonly isStaff: boolean;
  readonly isPremium: boolean;

  getAvatarUrl(size?: number): string;
  getAvatarUrlForCss(size?: number): string;
}
