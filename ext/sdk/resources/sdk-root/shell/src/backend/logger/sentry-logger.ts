import { injectable } from "inversify";
import * as Sentry from '@sentry/node';
import { LogProvider } from "./log-provider";

@injectable()
export class SentryLogger implements LogProvider {
  constructor() {
    console.log('Started SentryLogger');
  }

  error(error, extra) {
    Sentry.captureException(error, { extra });
  }
}
