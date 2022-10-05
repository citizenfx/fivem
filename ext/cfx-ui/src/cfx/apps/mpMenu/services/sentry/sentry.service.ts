import * as Sentry from '@sentry/react';
import { BrowserTracing } from '@sentry/tracing';
import { CurrentGameBuild, CurrentGameName, CurrentGamePureLevel } from 'cfx/base/gameRuntime';
import { formatCFXID } from 'cfx/base/identifiers';
import { ServicesContainer } from 'cfx/base/servicesContainer';
import { IAccountService } from 'cfx/common/services/account/account.service';
import { IAccount } from 'cfx/common/services/account/types';
import { AppContribution, registerAppContribution } from 'cfx/common/services/app/app.extensions';
import { fastRandomId } from 'cfx/utils/random';
import { inject, injectable } from 'inversify';
import { mpMenu } from '../../mpMenu';
import { IConvarService } from '../convars/convars.service';

const ENABLE_SENTRY = !__CFXUI_DEV__ && process.env.CI_PIPELINE_ID;

if (ENABLE_SENTRY) {
  Sentry.init({
    dsn: "https://cb2ef6d1855b4c5f83c5530b72b8aacb@sentry.fivem.net/12",

    integrations: [new BrowserTracing({
      tracingOrigins: ['localhost', /\.fivem.net/, /\.cfx.re/],
    })],

    tracesSampleRate: 0.05,

    release: `cfx-${process.env.CI_PIPELINE_ID || 'dev'}`,
  });

  try {
    if (!window.localStorage['sentryUserId']) {
      window.localStorage['sentryUserId'] = `${fastRandomId()}-${fastRandomId()}`;
    }

    Sentry.setUser({
      id: window.localStorage['sentryUserId'],
    });
  } catch (e) {
    // no-op
  }
}

export function registerSentryService(container: ServicesContainer) {
  if (!ENABLE_SENTRY) {
    return;
  }

  registerAppContribution(container, SentryService);
}

@injectable()
class SentryService implements AppContribution {
  constructor(
    @inject(IAccountService)
    protected readonly accountService: IAccountService,
    @inject(IConvarService)
    protected readonly convarService: IConvarService,
  ) { }

  init() {
    this.accountService.accountChange.addListener(({ account }) => this.setSentryUser(account));

    this.setSentryUser(this.accountService.account);

    this.setSentryContext();
  }

  private async setSentryContext() {
    try {
      const storageEstimate = await navigator.storage.estimate();

      Sentry.setContext('systemLimits', {
        hasAtLeastThisAmountOfRam: (navigator as any).deviceMemory,
        storageEstimateQuota: storageEstimate.quota || 'unknown',
        storageEstimateUsage: storageEstimate.usage || 'unknown',
      });
    } catch (e) { }

    Sentry.setContext('NUI', {
      systemLanguages: mpMenu.systemLanguages,
      gameName: CurrentGameName,
      gameBuild: CurrentGameBuild,
      gamePureLevel: CurrentGamePureLevel,
    });

    await this.convarService.whenPopulated();

    Sentry.setContext('convars', Object.fromEntries(
      Object.entries(this.convarService.getAll()).filter(([key, value]) => {
        if (key.startsWith('cl_'))  {
          return false;
        }
        if (key.startsWith('cam_')) {
          return false;
        }
        if (key.startsWith('net_')) {
          return false;
        }
        if (key.startsWith('game_')) {
          return false;
        }
        if (key.startsWith('voice_')) {
          return false;
        }
        if (key.startsWith('profile_')) {
          return false;
        }

        return true;
      }),
    ));
  }

  private readonly setSentryUser = (account: IAccount | null) => {
    if (account) {
      Sentry.setContext('cfxUser', {
        id: account.id,
        link: `https://forum.cfx.re/u/${account.id}`,
        username: account.username,
      })
    } else {
      Sentry.setContext('cfxUser', null);
    }
  };
}
