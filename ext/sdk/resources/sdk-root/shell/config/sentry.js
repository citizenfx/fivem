module.exports = {
  getSentryAuthToken: (isProd) => isProd && process.env.SENTRY_AUTH_TOKEN,
  getSentryRelease: (isProd) => isProd && `cfx-${process.env.CI_PIPELINE_ID}`,
};
