const fs = require('fs');
const path = require('path');
const webpack = require('webpack');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const MiniCssExtractPlugin = require('mini-css-extract-plugin');
const ForkTsCheckerWebpackPlugin = require('fork-ts-checker-webpack-plugin');
const ReactRefreshWebpackPlugin = require('@pmmmwh/react-refresh-webpack-plugin');
const ImageMinimizerPlugin = require("image-minimizer-webpack-plugin");
const SentryWebpackPlugin = require('@sentry/webpack-plugin');

const srcPath = path.join(__dirname, '../src');
const buildPath = path.join(__dirname, '../build');

module.exports = (env, argv) => {
  const isProd = argv.mode === 'production';
  const isDev = !isProd;

  const sentryAuthToken = process.env.SENTRY_AUTH_TOKEN;
  const sentryRelease = `cfx-${process.env.CI_PIPELINE_ID || 'dev'}`;
  const sentryProjectName = process.env.CFX_SENTRY_PROJECT_NAME_CFXUI || 'mpmenu';

  const app = env.app;

  const isWithGtm = env.withGtm == true || env.withGtm == "true";

  verifyApp(app);
  const appPath = path.join(srcPath, 'cfx/apps', app);
  const appBuildPath = path.join(buildPath, app);

  const defines = {
    '__CFXUI_DEV__': JSON.stringify(isDev),
    '__CFXUI_USE_SOUNDS__': app === 'mpMenu',
    '__CFXUI_CNL_ENDPOINT__': JSON.stringify(process.env.CFX_CNL_ENDPOINT || 'https://lambda.fivem.net/'),
    '__CFXUI_SENTRY_DSN__': JSON.stringify(process.env.CFX_SENTRY_DSN_CFXUI || ''),
    '__CFXUI_SENTRY_RELEASE__': JSON.stringify(sentryRelease),
  };

  return {
    devtool: isProd ? 'source-map' : 'eval',

    entry: path.join(appPath, 'index.tsx'),

    resolveLoader: {
      modules: [
        'node_modules',
        path.resolve(__dirname, 'loaders'),
      ],
    },

    devServer: {
      hot: true,
      allowedHosts: 'all',
      port: 4200,
      historyApiFallback: {
        index: '/',
      },
      headers: {
        'Service-Worker-Allowed': '/',
      },
    },

    output: {
      clean: isProd,
      path: appBuildPath,
      filename: 'static/js/[name].js',
      chunkFilename: 'static/js/[name].chunk.js',
    },

    resolve: {
      alias: fs.readdirSync(srcPath).reduce((acc, file) => {
        acc[file] = path.join(srcPath, file);

        return acc;
      }, {}),

      extensions: ['.ts', '.tsx', '.js', '.json'],
    },

    module: {
      rules: [
        {
          test: /\.(bmp|gif|jpg|jpeg|png|woff|woff2|mp3|ogg|wav|svg|webp)$/,
          type: 'asset/resource',
          generator: {
            filename: 'static/media/[hash][name][ext]',
          },
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
                    ? '[hash:base64:8]'
                    : '[name]_[local]_[hash:base64:6]',
                },
              },
            },
            {
              loader: 'sass-loader',
              options: {
                sassOptions: {
                  includePaths: [
                    path.join(srcPath, 'cfx/styles'),
                  ],
                },
                additionalData: '@use "~@cfx-dev/ui-components/dist/styles-scss/ui" as ui;\n',
              },
            },
          ],
        },

        {
          test: /\.raw\.js$/,
          type: 'asset/source',
        },

        {
          test: /\.tsx?$/,
          loader: 'ts-loader',
          options: isDev
            ? {
              transpileOnly: true,
              getCustomTransformers: () => ({
                before: [require('react-refresh-typescript')({})],
              }),
            }
            : undefined,
          exclude: /node_modules/,
        },

        {
          test: /\.(mjs|js)$/,
          enforce: 'pre',
          use: [
            {
              loader: 'source-map-loader',
              options: {
                filterSourceMappingUrl: (url, resourcePath) => {
                  if (resourcePath.includes('inversify')) {
                    return false;
                  }

                  if (/prime\.worker\.min\.js$/i.test(resourcePath)) {
                    return false;
                  }

                  return true;
                },
              },
            },
          ],
        },
      ]
    },

    ignoreWarnings: [/Failed to parse source map/],

    plugins: [
      new webpack.DefinePlugin(defines),
      new HtmlWebpackPlugin({
        template: path.join(appPath, 'index.html'),
        templateParameters: {
          isProd,
          isWithGtm,
        },
      }),

      isDev && new ForkTsCheckerWebpackPlugin(),
      isDev && new ReactRefreshWebpackPlugin(),

      isProd && new MiniCssExtractPlugin(),

      !!sentryAuthToken && new SentryWebpackPlugin({
        url: 'https://sentry.fivem.net/',
        authToken: sentryAuthToken,

        release: sentryRelease,

        org: 'citizenfx',
        project: sentryProjectName,

        include: appBuildPath,
        urlPrefix: 'https://nui-game-internal/ui/app/',

        errorHandler: (err, invokeErr, compilation) => {
          compilation.warnings.push('Sentry CLI Plugin: ' + err.message)
        },
      }),
    ].filter(Boolean),

    performance: {
      maxAssetSize: Infinity,
      maxEntrypointSize: Infinity,
    },

    optimization: {
      usedExports: isProd,

      ...(
        isProd
          ? { chunkIds: 'named', splitChunks: false }
          : {}
      ),

      minimizer: [
        isProd && new ImageMinimizerPlugin({
          minimizer: {
            implementation: ImageMinimizerPlugin.sharpMinify,
            options: {
              encodeOptions: {
                jpeg: {
                  quality: 75,
                },
                png: {
                  quality: 90,
                },
              },
            },
          },
        }),

        '...',
      ].filter(Boolean),
    },
  };
};

function verifyApp(app) {
  try {
    const stats = fs.statSync(path.join(srcPath, 'cfx/apps', app, 'index.tsx'));
    if (stats.isDirectory()) {
      console.error(new Error(`Entry point 'index.tsx' of ${app} app can not be a directory`));
      process.exit(1);
    }
  } catch (e) {
    console.error(new Error(`Entry point 'index.tsx' of ${app} app not found`));
    process.exit(1);
  }
}
