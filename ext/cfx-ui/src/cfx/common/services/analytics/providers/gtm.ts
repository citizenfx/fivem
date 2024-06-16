import { inject, injectable, optional } from 'inversify';

import { GameName } from 'cfx/base/game';
import { CurrentGameBrand, CurrentGameName } from 'cfx/base/gameRuntime';
import { IAccountService } from 'cfx/common/services/account/account.service';
import { AnalyticsProvider } from 'cfx/common/services/analytics/analytics.extensions';
import { ILinkedIdentitiesService } from 'cfx/common/services/linkedIdentities/linkedIdentities.service';
import { LinkedIdentityProvider } from 'cfx/common/services/linkedIdentities/types';

import { EventActionNames, TrackEventParams } from '../types';

type GTMEvent = TrackEventParams['properties'] & {
  event: TrackEventParams['action'];
};

type GTMEnvironment =
  | 'FiveM - prod'
  | 'FiveM - dev'
  | 'RedM - prod'
  | 'RedM - dev';

// parameters that will be included to each event to push in to dataLayer
interface GTMEventDefaults {
  internal_referrer: string | undefined; // should be the previous page referrer if possible
  environment: GTMEnvironment;
  user_id: number | undefined;
  cfx_id: string | undefined;
  cfx_login_state: boolean;
  games_played: string;
  display_type: 'mobile' | 'desktop';
}

export type DataLayerType = Array<GTMEvent & GTMEventDefaults>;
// declared in index.html by default and processing by google tag manager script
declare let dataLayer: DataLayerType;

// cast params in to google tag manager event type
export const getGTMEvent = (params: TrackEventParams): GTMEvent | null => {
  if (!Object.values(EventActionNames).includes(params.action)) {
    return null;
  }

  const {
    action,
    properties,
  } = params;

  return {
    event: action,
    ...properties,
  };
};

// GTM - google tag manager
@injectable()
export class GTMAnalyticsProvider implements AnalyticsProvider {
  private _userId: number | undefined = undefined;

  private _cfxId: string | undefined = undefined;

  private _displayType: GTMEventDefaults['display_type'] = 'desktop';

  constructor(
    @inject(IAccountService)
    @optional()
    protected readonly accountService: IAccountService | undefined,
    @inject(ILinkedIdentitiesService)
    @optional()
    protected readonly linkedIdentitiesService: ILinkedIdentitiesService | undefined,
  ) {
    this.initializeUserID();
    this.initializeCFXID();
  }

  private async initializeUserID() {
    this.accountService?.accountChange.addListener((event) => {
      if (event.account) {
        this._userId = event.account.id;
      }
    });
  }

  private async initializeCFXID() {
    this.linkedIdentitiesService?.identitiesChange.addListener((event) => {
      if (event.linkedIdentities) {
        const cfxIdentity = event.linkedIdentities.find(({
          provider,
        }) => provider === LinkedIdentityProvider.Cfxre);
        this._cfxId = cfxIdentity
          ? cfxIdentity.id
          : undefined;
      }
    });
  }

  private getEventDefaults(): GTMEventDefaults {
    const baseEnvironment = CurrentGameName === GameName.FiveM
      ? 'FiveM'
      : 'RedM';
    const environment: GTMEnvironment = __CFXUI_DEV__
      ? `${baseEnvironment} - dev`
      : `${baseEnvironment} - prod`;

    return {
      environment,
      user_id: this._userId,
      cfx_id: this._cfxId,
      cfx_login_state: !!this._cfxId,
      display_type: this._displayType,
      games_played: `${CurrentGameBrand} PC`,
      internal_referrer: undefined,
    };
  }

  trackEvent(params: TrackEventParams): void {
    const event = getGTMEvent(params);

    if (event === null) {
      return;
    }

    try {
      dataLayer.push({
        ...event,
        ...this.getEventDefaults(),
      });
    } catch (e) {
      if (!(e instanceof ReferenceError)) {
        throw e;
      }
    }
  }
}
