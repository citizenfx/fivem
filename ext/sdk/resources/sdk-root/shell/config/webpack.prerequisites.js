const fs = require('fs');
const path = require('path');
const webpack = require('webpack');
const SentryWebpackPlugin = require('@sentry/webpack-plugin');
const TerserPlugin = require("terser-webpack-plugin");
const ForkTsCheckerWebpackPlugin = require('fork-ts-checker-webpack-plugin');

const rootPath = path.join(__dirname, '../');

const paths = {
  root: rootPath,
  src: path.join(rootPath, 'src'),
  public: path.join(rootPath, 'public'),
  buildClient: path.join(rootPath, 'build'),
  buildServer: path.join(rootPath, 'build_server'),
};

module.exports = {
  paths,

  isProd(env, args) {
    return args.mode === 'production' || env === 'production';
  },

  rootPath(subpath) {
    return path.join(paths.root, subpath);
  },

  srcPath(subpath) {
    return path.join(paths.src, subpath);
  },

  publicPath(subpath) {
    return path.join(paths.public, subpath);
  },

  buildClientPath(subpath) {
    return path.join(paths.buildClient, subpath);
  },

  getMode(env, args) {
    return this.isProd(env, args) ? 'production' : 'development';
  },

  plugins(env, args, { project, include }) {
    const isProd = this.isProd(env, args);

    const sentryAuthToken = isProd && process.env.SENTRY_AUTH_TOKEN;
    const sentryRelease = isProd && `cfx-${process.env.CI_PIPELINE_ID}`;

    return [
      new ForkTsCheckerWebpackPlugin(),
      new webpack.DefinePlugin({
        'process.env.CI_PIPELINE_ID': JSON.stringify(process.env.CI_PIPELINE_ID || 'dev'),
      }),
      sentryAuthToken && new SentryWebpackPlugin({
        url: 'https://sentry.fivem.net/',
        authToken: sentryAuthToken,
        release: sentryRelease,
        org: "citizenfx",
        project,
        include,
      }),
    ].filter(Boolean);
  },

  common: {
    ignoreWarnings: [
      /export .* was not found in/,
      /bufferutil/,
      /utf-8-validate/,
      /the request of a dependency is an expression/,
      /No instantiations of threads\.js workers found/, // temp hack, tho workers are working fine with hacked loader in ArchetypesState.ts
    ],
    performance: false,
    resolve: {
      alias: fs.readdirSync(paths.src).reduce((acc, file) => {
        acc[file] = path.join(paths.src, file);

        return acc;
      }, {}),
      extensions: ['.ts', '.tsx', '.js'],
    },
  },

  optimization(env, args) {
    const isProd = this.isProd(env, args);

    return {
      runtimeChunk: false,
      minimize: isProd,
      minimizer: [
        // This is only used in production mode
        new TerserPlugin(),
      ],
    };
  }
};
