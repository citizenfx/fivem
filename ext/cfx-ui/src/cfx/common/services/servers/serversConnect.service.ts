import { SingleEventEmitter } from 'cfx/utils/singleEventEmitter';

import { IServerView } from './types';
import { defineService } from '../../../base/servicesContainer';

export const IServersConnectService = defineService<IServersConnectService>('ServersConnectService');
export interface IServersConnectService {
  readonly canConnect: boolean;
  readonly connectFailed: SingleEventEmitter<string>;

  connectTo(server: string | IServerView): void;
}
