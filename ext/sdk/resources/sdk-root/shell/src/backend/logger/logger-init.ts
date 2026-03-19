import * as Sentry from '@sentry/node';

if (process.env.__CFX_SENTRY_DSN__ && process.env.__CFX_SENTRY_RELEASE__) {
  Sentry.init({
    dsn: process.env.__CFX_SENTRY_DSN__,
    release: process.env.__CFX_SENTRY_RELEASE__,
    tracesSampleRate: 0.01,
    sampleRate: 0.01,
  });
}
