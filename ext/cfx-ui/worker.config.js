const config = {
  entry: __dirname + '/src/worker/worker.ts',
  output: {
    path: __dirname + '/src/worker/',
    filename: 'index.js'
  },
  resolve: {
    extensions: ['.ts', '.tsx', '.js']
  },
  module: {
    rules: [
      {test: /\.tsx?$/, loader: 'ts-loader'}
    ]
  }
};
  
module.exports = config;