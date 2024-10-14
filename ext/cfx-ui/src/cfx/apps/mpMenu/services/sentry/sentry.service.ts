import * as Sentry from '@sentry/react';
import { BrowserTracing } from '@sentry/tracing';
import { inject, injectable } from 'inversify';

import { CurrentGameBuild, CurrentGameName, CurrentGamePureLevel } from 'cfx/base/gameRuntime';
import { ServicesContainer } from 'cfx/base/servicesContainer';
import { IAccountService } from 'cfx/common/services/account/account.service';
import { IAccount } from 'cfx/common/services/account/types';
import { AppContribution, registerAppContribution } from 'cfx/common/services/app/app.extensions';
import { ASID } from 'cfx/utils/asid';
import { fetcher } from 'cfx/utils/fetcher';

import { mpMenu } from '../../mpMenu';
import { IConvarService } from '../convars/convars.service';

const ENABLE_SENTRY = !__CFXUI_DEV__ && __CFXUI_SENTRY_RELEASE__ && __CFXUI_SENTRY_DSN__;

if (ENABLE_SENTRY) {
  Sentry.init({
    dsn: __CFXUI_SENTRY_DSN__,

    integrations: [
      new BrowserTracing({
        tracingOrigins: ['localhost', 'nui-game-internal', /\.fivem.net/, /\.cfx.re/],
      }),
    ],

    tracesSampleRate: 0.05,

    release: __CFXUI_SENTRY_RELEASE__,

    beforeSend(event, hint) {
      // Ignore errors from console
      if (event.exception?.values?.[0].stacktrace?.frames?.[0].filename === '<anonymous>') {
        return null;
      }

      // Ignore handled exceptions
      if (event.exception?.values?.[0]?.mechanism?.handled) {
        return null;
      }

      const og = hint.originalException;

      if (typeof og === 'object' && og instanceof Error) {
        event.tags ??= {};
        event.extra ??= {};

        if (fetcher.HttpError.is(og)) {
          event.tags.http_status_code = og.response.status;
          event.tags.http_request_url = og.response.url;

          event.extra.http_response_headers = og.response.headers;
        }

        if (fetcher.JsonParseError.is(og)) {
          event.extra.json_original_string = og.originalString;
        }
      }

      return event;
    },
  });

  const id = ASID;

  Sentry.setUser({
    id,
    username: id,
  });
}

export function registerSentryService(container: ServicesContainer) {
  if (!ENABLE_SENTRY) {
    return;
  }

  registerAppContribution(container, SentryService);
}

@injectable()
class SentryService implements AppContribution {
  @inject(IAccountService)
  protected readonly accountService: IAccountService;

  @inject(IConvarService)
  protected readonly convarService: IConvarService;

  init() {
    this.accountService.accountChange.addListener(({
      account,
    }) => this.setSentryUser(account));

    this.setSentryUser(this.accountService.account);

    this.setSentryContext();
  }

  private async setSentryContext() {
    try {
      const storageEstimate = await navigator.storage.estimate();

      Sentry.setContext('System Limits', {
        hasAtLeastThisAmountOfRam: (navigator as any).deviceMemory,
        storageEstimateQuota: storageEstimate.quota || 'unknown',
        storageEstimateUsage: storageEstimate.usage || 'unknown',
      });
    } catch {
      // Do nothing
    }

    Sentry.setContext('NUI', {
      systemLanguages: mpMenu.systemLanguages,
      gameName: CurrentGameName,
      gameBuild: CurrentGameBuild,
      gamePureLevel: CurrentGamePureLevel,
    });

    await this.convarService.whenPopulated();

    Sentry.setContext(
      'convars',
      Object.fromEntries(
        Object.entries(this.convarService.getAll()).filter(([key]) => {
          if (key.startsWith('cl_')) {
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
      ),
    );
  }

  private readonly setSentryUser = (account: IAccount | null) => {
    if (account) {
      Sentry.setContext('cfxUser', {
        id: account.id,
        link: `https://forum.cfx.re/u/${account.id}`,
        username: account.username,
      });
    } else {
      Sentry.setContext('cfxUser', null);
    }
  };
}
