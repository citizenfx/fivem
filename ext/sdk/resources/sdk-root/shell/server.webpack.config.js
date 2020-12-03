const path = require('path');

module.exports = {
  target: 'node',
  node: {
    global: false,
    __dirname: false,
  },
  resolve: {
    alias: {
      shared: path.join(__dirname, 'src/shared/'),
      utils: path.join(__dirname, 'src/utils/'),
      server: path.join(__dirname, 'src/server/'),
    },
    extensions: ['.ts', '.tsx', '.js'],
  },
  entry: {
    filename: path.join(__dirname, 'src/server/index.ts'),
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
          configFile: 'server.tsconfig.json',
        },
        exclude: /node_modules/,
      },
    ],
  },
};
