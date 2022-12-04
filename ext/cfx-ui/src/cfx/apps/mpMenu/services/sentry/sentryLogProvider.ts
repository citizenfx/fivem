import { LogProvider } from "cfx/common/services/log/logService.extensions";
import { injectable } from "inversify";
import * as Sentry from '@sentry/react';

@injectable()
export class SentryLogProvider implements LogProvider {
  error<T extends Error>(error: T, extra?: Record<string, any> | undefined): void {
    Sentry.captureException(error, { extra });
  }
}
