export enum LinkedIdentityProvider {
  Steam = 'steam',
  Discord = 'discord',
  XboxLive = 'xbl',
  ROS = 'ros',
  Cfxre = 'fivem',
}

export interface ILinkedIdentity {
  id: string;
  provider: LinkedIdentityProvider;
  username: string | null | undefined;
}
