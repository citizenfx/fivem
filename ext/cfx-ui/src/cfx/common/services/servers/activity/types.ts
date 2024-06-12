import { TCFXID } from 'cfx/base/identifiers';

export interface IServerActivityUserPlaytime {
  cfxId: TCFXID;
  seconds: number;
  formattedSeconds: string;
}
