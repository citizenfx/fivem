module.exports = {
  getSentryAuthToken: (isProd) => isProd && process.env.SENTRY_AUTH_TOKEN,
};
