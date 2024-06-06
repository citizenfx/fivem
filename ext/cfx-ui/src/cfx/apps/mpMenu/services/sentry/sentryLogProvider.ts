import * as Sentry from '@sentry/react';
import { injectable } from 'inversify';

import { LogProvider } from 'cfx/common/services/log/logService.extensions';

@injectable()
export class SentryLogProvider implements LogProvider {
  error<T extends Error>(error: T, extra?: Record<string, any> | undefined): void {
    Sentry.captureException(error, { extra });
  }
}
