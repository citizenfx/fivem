const path = require('path');
const fs = require('fs');
const sentry = require('./sentry');
const SentryWebpackPlugin = require("@sentry/webpack-plugin");
const ReplacePlugin = require('webpack-plugin-replace');
const ForkTsCheckerWebpackPlugin = require('fork-ts-checker-webpack-plugin');

const srcPath = path.join(__dirname, '../src');
const buildServerPath = path.join(__dirname, '../build_server');

module.exports = (env, args) => {
  const isProd = args.mode === 'production' || env === 'production';
  const sentryAuthToken = sentry.getSentryAuthToken(isProd);

  return {
    target: 'node',
    node: {
      global: false,
      __dirname: false,
    },
    mode: 'development',
    devtool: isProd
      ? 'source-map'
      : 'eval',
    resolve: {
      alias: fs.readdirSync(srcPath).reduce((acc, file) => {
        acc[file] = path.join(srcPath, file);

        return acc;
      }, {}),
      extensions: ['.ts', '.tsx', '.js'],
    },
    entry: {
      filename: path.join(srcPath, 'backend/index.ts'),
    },
    output: {
      path: path.join(__dirname, '../build_server'),
      filename: 'index.js',
    },
    module: {
      rules: [
        {
          test: /\.ts/,
          loader: 'ts-loader',
          options: {
            transpileOnly: true,
          },
          exclude: /node_modules/,
        },
      ],
    },
    plugins: [
      new ForkTsCheckerWebpackPlugin(),
      new ReplacePlugin({
        values: {
          'process.env.CI_PIPELINE_ID': JSON.stringify(process.env.CI_PIPELINE_ID || 'dev'),
        },
      }),
      sentryAuthToken && new SentryWebpackPlugin({
        url: 'https://sentry.fivem.net/',
        authToken: sentryAuthToken,
        org: "citizenfx",
        project: "fxdk-backend",

        include: buildServerPath,
      }),
    ].filter(Boolean),
  };
};
