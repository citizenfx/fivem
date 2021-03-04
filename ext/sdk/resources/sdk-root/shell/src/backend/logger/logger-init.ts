import * as Sentry from '@sentry/node';

Sentry.init({
  dsn: "https://e857fab197de465881a78d43cc151933@sentry.fivem.net/8",
  release: `cfx-${process.env.CI_PIPELINE_ID}`,
  tracesSampleRate: 1.0,
});
