const prerequisites = require('./webpack.prerequisites');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const CopyWebpackPlugin = require('copy-webpack-plugin');
const MiniCssExtractPlugin = require('mini-css-extract-plugin');
const webpack = require('webpack');
const NotifierPlugin = require('./notifier-plugin');

module.exports = (env, args) => {
  const isProd = prerequisites.isProd(env, args);

  const notifierPlugin = !isProd && (NotifierPlugin.init(), new NotifierPlugin());

  return {
    mode: prerequisites.getMode(env, args),
    devtool: isProd
      ? 'source-map'
      : 'eval',

    entry: {
      main: prerequisites.srcPath('fxdk/browser/Shell.tsx'),
      'archetypes-worker': prerequisites.srcPath('personalities/world-editor/archetypes-worker.ts'),
    },
    output: {
      path: prerequisites.paths.buildClient,
      filename: 'static/js/[name].js',
      chunkFilename: 'static/js/[name].chunk.js',
    },

    module: {
      rules: [
        {
          test: /\.raw\.js$/,
          type: 'asset/source',
        },
        {
          test: /\.tsx?$/,
          loader: 'ts-loader',
          options: {
            transpileOnly: true,
          },
          exclude: /node_modules/,
        },

        {
          test: /\.css$/,
          use: [
            isProd
              ? MiniCssExtractPlugin.loader
              : 'style-loader',
            'css-loader',
          ],
        },

        {
          test: /\.scss$/,
          use: [
            isProd
              ? MiniCssExtractPlugin.loader
              : 'style-loader',
            {
              loader: 'css-loader',
              options: {
                modules: {
                  mode: 'local',
                  auto: (resourcePath) => resourcePath.endsWith('.module.scss'),
                  localIdentName: isProd
                    ? '[hash:base64:6]'
                    : '[local]_[hash:base64:6]',
                },
              },
            },
            {
              loader: 'sass-loader',
              options: {
                sassOptions: {
                  includePaths: [
                    prerequisites.srcPath('styles'),
                  ],
                },
              },
            },
          ],
        },

        {
          test: /\.(bmp|gif|jpg|jpeg|png|woff|woff2|svg)$/,
          type: 'asset/resource',
          generator: {
            filename: 'static/media/[name][ext]',
          },
        },
      ],
    },

    plugins: [
      ...prerequisites.plugins(env, args, {
        project: 'fxdk',
        include: prerequisites.paths.buildClient,
      }),
      new CopyWebpackPlugin({
        patterns: [
          {
            from: prerequisites.publicPath('paintlet.js'),
            to: prerequisites.buildClientPath('paintlet.js'),
          },
        ],
      }),
      new HtmlWebpackPlugin({
        template: prerequisites.publicPath('index.html'),
        templateParameters: {
          notifierPlugin: notifierPlugin?.client || '',
        },
      }),
      isProd && new MiniCssExtractPlugin({
        filename: 'static/css/[name].css',
        chunkFilename: 'static/css/[name].chunk.css',
      }),
      new webpack.ProvidePlugin({
        Buffer: ['buffer', 'Buffer'],
      }),
      !isProd && notifierPlugin,
    ].filter(Boolean),

    ...prerequisites.common,
  };
};
