const path = require('path');

module.exports = {
  target: 'node',
  node: {
    global: false,
    __dirname: false,
  },
  mode: 'development',
  resolve: {
    alias: {
      shared: path.join(__dirname, 'src/shared/'),
      utils: path.join(__dirname, 'src/utils/'),
      backend: path.join(__dirname, 'src/backend/'),
    },
    extensions: ['.ts', '.tsx', '.js'],
  },
  entry: {
    filename: path.join(__dirname, 'src/backend/index.ts'),
  },
  output: {
    path: path.join(__dirname, 'build_server'),
    filename: 'index.js',
  },
  module: {
    rules: [
      {
        test: /\.ts/,
        loader: 'ts-loader',
        options: {
          transpileOnly: true,
          configFile: 'backend.tsconfig.json',
        },
        exclude: /node_modules/,
      },
    ],
  },
};
