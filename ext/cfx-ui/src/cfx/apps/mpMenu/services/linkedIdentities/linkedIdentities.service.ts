import { AppContribution, registerAppContribution } from "cfx/common/services/app/app.extensions";
import { defineService, ServicesContainer } from "cfx/base/servicesContainer";
import { fetcher } from "cfx/utils/fetcher";
import { serializeQueryString } from "cfx/utils/url";
import { inject, injectable } from "inversify";
import { IConvarService } from "../convars/convars.service";
import { ILinkedIdentity } from "./types";
import { AwaitableValue } from "cfx/utils/observable";

export const ILinkedIdentitiesService = defineService<ILinkedIdentitiesService>('LinkedIdentitiesService');
export type ILinkedIdentitiesService = LinkedIdentitiesService;

export function registerLinkedIdentitiesService(container: ServicesContainer) {
  container.registerImpl(ILinkedIdentitiesService, LinkedIdentitiesService);

  registerAppContribution(container, LinkedIdentitiesService);
}

@injectable()
export class LinkedIdentitiesService implements AppContribution {
  @inject(IConvarService)
  protected readonly convarService: IConvarService;

  private hadRockstar = false;

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
      const json = await fetcher.json('https://lambda.fivem.net/api/ticket/identities', {
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
        const { username } = identity;
        const [provider, id] = identity.id.split(':');

        linkedIdentities.push({
          id,
          provider,
          username,
        });
      }

      this.linkedIdentities.value = linkedIdentities;
    } catch (e) {
      console.warn('Failed to fetch linked identities', e);
    }
  }
}
