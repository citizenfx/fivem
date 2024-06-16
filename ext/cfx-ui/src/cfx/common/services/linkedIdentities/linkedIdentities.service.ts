import { defineService } from 'cfx/base/servicesContainer';
import { AwaitableValue } from 'cfx/utils/observable';
import { SingleEventEmitter } from 'cfx/utils/singleEventEmitter';

import { IdentitiesChangeEvent } from './events';
import { ILinkedIdentity } from './types';

export const ILinkedIdentitiesService = defineService<ILinkedIdentitiesService>('LinkedIdentitiesService');
export interface ILinkedIdentitiesService {
  readonly identitiesChange: SingleEventEmitter<IdentitiesChangeEvent>;
  readonly linkedIdentities: AwaitableValue<ILinkedIdentity[]>;
  addRockstar(): void;
}
