const path = require('path');
const fs = require('fs');
const ForkTsCheckerWebpackPlugin = require('fork-ts-checker-webpack-plugin');

const srcPath = path.join(__dirname, '../src');

module.exports = {
  target: 'node',
  node: {
    global: false,
    __dirname: false,
  },
  mode: 'development',
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
          // configFile: 'backend.tsconfig.json',
        },
        exclude: /node_modules/,
      },
    ],
  },
  plugins: [
    new ForkTsCheckerWebpackPlugin(),
  ],
};
