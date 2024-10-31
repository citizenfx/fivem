import { inject, injectable } from 'inversify';

import { ServicesContainer } from 'cfx/base/servicesContainer';
import { AppContribution, registerAppContribution } from 'cfx/common/services/app/app.extensions';
import { IdentitiesChangeEvent } from 'cfx/common/services/linkedIdentities/events';
import { ILinkedIdentitiesService } from 'cfx/common/services/linkedIdentities/linkedIdentities.service';
import { ILinkedIdentity } from 'cfx/common/services/linkedIdentities/types';
import { fetcher } from 'cfx/utils/fetcher';
import { AwaitableValue } from 'cfx/utils/observable';
import { SingleEventEmitter } from 'cfx/utils/singleEventEmitter';
import { serializeQueryString } from 'cfx/utils/url';

import { IConvarService } from '../convars/convars.service';

export function registerLinkedIdentitiesService(container: ServicesContainer) {
  container.registerImpl(ILinkedIdentitiesService, LinkedIdentitiesService);

  registerAppContribution(container, LinkedIdentitiesService);
}

@injectable()
export class LinkedIdentitiesService implements AppContribution, ILinkedIdentitiesService {
  @inject(IConvarService)
  protected readonly convarService: IConvarService;

  private hadRockstar = false;

  readonly identitiesChange = new SingleEventEmitter<IdentitiesChangeEvent>();

  readonly linkedIdentities = new AwaitableValue<ILinkedIdentity[]>([]);

  afterRender() {
    this.updateLinkedIdentities();
  }

  addRockstar() {
    if (!this.hadRockstar) {
      this.updateLinkedIdentities(true);
      this.hadRockstar = true;
    }
  }

  private async updateLinkedIdentities(withRockstar = false) {
    await this.convarService.whenPopulated();

    const ownershipTicket = this.convarService.get('cl_ownershipTicket');

    if (!ownershipTicket) {
      console.warn('No ownership ticket during profiles update');

      return;
    }

    try {
      const json = await fetcher.json(`${__CFXUI_CNL_ENDPOINT__}api/ticket/identities`, {
        method: 'POST',
        body: serializeQueryString({
          token: ownershipTicket,
          withRockstar,
        }),
        headers: {
          'content-type': 'application/x-www-form-urlencoded',
        },
      });

      if (typeof json !== 'object' || json === null) {
        console.warn('Unexpected response for linked identities', json);

        return;
      }

      if (!Array.isArray(json.identities)) {
        console.warn('Unexpected response for linked identities, indetities is not an array', json);

        return;
      }

      const linkedIdentities: ILinkedIdentity[] = [];

      for (const identity of json.identities) {
        const {
          username,
        } = identity;
        const [provider, id] = identity.id.split(':');

        linkedIdentities.push({
          id,
          provider,
          username,
        });
      }

      this.linkedIdentities.value = linkedIdentities;
      this.identitiesChange.emit({ linkedIdentities });
    } catch (e) {
      console.warn('Failed to fetch linked identities', e);
    }
  }
}
