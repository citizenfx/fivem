import * as Sentry from '@sentry/react';
import { BrowserTracing } from '@sentry/tracing';
import { formatCFXID } from 'cfx/base/identifiers';
import { ServicesContainer } from 'cfx/base/servicesContainer';
import { IAccountService } from 'cfx/common/services/account/account.service';
import { IAccount } from 'cfx/common/services/account/types';
import { AppContribution, registerAppContribution } from 'cfx/common/services/app/app.extensions';
import { inject, injectable } from 'inversify';
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
    await this.convarService.whenPopulated();

    Sentry.setContext('convars', this.convarService.getAll());
  }

  private readonly setSentryUser = (account: IAccount | null) => {
    if (account) {
      Sentry.setUser({
        id: formatCFXID(account.id),
        username: account.username,
      });
    } else {
      Sentry.setUser(null);
    }
  };
}
