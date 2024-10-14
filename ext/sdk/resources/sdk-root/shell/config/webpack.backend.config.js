const prerequisites = require('./webpack.prerequisites');

module.exports = (env, args) => {
  const isProd = prerequisites.isProd(env, args);

  return {
    target: 'node',
    node: {
      global: false,
      __dirname: false,
    },
    mode: prerequisites.getMode(env, args),
    devtool: false,
    entry: prerequisites.srcPath('fxdk/node/server.ts'),
    output: {
      path: prerequisites.paths.buildServer,
      filename: 'index.js',
    },
    module: {
      rules: [
        {
          test: /\.raw\.js$/,
          type: 'asset/source',
        },
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
    optimization: prerequisites.optimization(env, args),
    plugins: prerequisites.plugins(env, args, {
      sentryProject: process.env.CFX_SENTRY_PROJECT_NAME_FXDK_BACKEND,
      sentryDsn: process.env.CFX_SENTRY_DSN_FXDK_BACKEND,
      include: prerequisites.paths.buildServer,
    }),
    ...prerequisites.common,
  };
};
