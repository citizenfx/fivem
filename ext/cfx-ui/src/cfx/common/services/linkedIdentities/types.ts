export enum LinkedIdentityProvider {
  Steam = 'steam',
  Discord = 'discord',
  ROS = 'ros',
  Cfxre = 'fivem',
}

export interface ILinkedIdentity {
  id: string;
  provider: LinkedIdentityProvider;
  username: string | null | undefined;
}
