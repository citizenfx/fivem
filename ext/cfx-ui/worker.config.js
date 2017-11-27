const config = {
  entry: './src/worker/worker.ts',
  output: {
    filename: './src/worker/index.js'
  },
  resolve: {
    extensions: ['.ts', '.tsx', '.js']
  },
  module: {
    loaders: [
      {test: /\.tsx?$/, loader: 'ts-loader'}
    ]
  }
};
  
module.exports = config;